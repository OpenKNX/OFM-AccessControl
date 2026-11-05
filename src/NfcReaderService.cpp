#if defined(OPENKNX_ACC_NFC_READER) && defined(ARDUINO_ARCH_ESP32)
#include "NfcReaderService.h"
#include "AccessControl.h"
#ifdef OPENKNX_NFC_ST25R3916_I2C
#include "St25r3916Reader.hpp"
#else
#include "Pn7161ReaderAdapter.hpp"
#endif
#include <algorithm>

NfcReaderService openknxNfc;

void NfcReaderService::setup() {
    if (!events) events = xQueueCreate(4, sizeof(Event));
    if (!events) { failed = true; logError("NFC", "Reader event queue allocation failed"); }
}
void NfcReaderService::loop() {
    bool requested = knx.configured() && ParamACC_NfcScanner != 0 && !failed;
#if defined(OPENKNX_NFC_UID_ONLY) && defined(HK_Enabled)
    // A UID-only image cannot honor an enabled HomeKey policy. Require the
    // matching ETS mode rather than silently weakening that configuration.
    requested = requested && !ParamHK_Enabled;
#endif
    if (!requested) { if (running) suspend(); return; }
    running = true;
    if (!task && (!extension || extension->nfcReady())) {
        if (xTaskCreate([](void* self) { static_cast<NfcReaderService*>(self)->run(); },
            "AccessNfc", OPENKNX_NFC_STACK_SIZE, this, 1, &task) != pdPASS) {
            failed = true; suspend(); logError("NFC", "Reader task allocation failed; reboot required"); return;
        }
    }
    Event event;
    for (unsigned i=0; i<4 && events && xQueueReceive(events,&event,0)==pdTRUE; ++i) {
        if (!running || event.epoch != epoch) continue;
        switch (event.kind) {
        case Kind::Ready: logInfo("NFC", "Reader initialized; RF discovery active"); break;
        case Kind::Failed: logWarning("NFC", "Reader initialization/discovery failed; retry in 5 seconds"); break;
        case Kind::Detected: logInfo("NFC", "Tag detected; checking credential"); break;
        case Kind::Uid: {
            const bool granted = extension ? extension->authorizeNfcUid(event.uid,event.length,event.token)
                : openknxAccessControl.processExternalNfcUid(event.uid,event.length,nullptr,0);
            if (!granted) openknxAccessControl.rejectExternalCredential();
            logInfo("NFC", "UID access %s", granted ? "granted" : "denied");
            break;
        }
        }
    }
}
void NfcReaderService::send(Kind kind, uint32_t scanEpoch, uint32_t token, const std::vector<uint8_t>* uid) {
    if (!running || scanEpoch != epoch || !events) return;
    Event event{}; event.kind=kind;event.epoch=scanEpoch;event.token=token;
    if (uid) {
        if (uid->size()!=4 && uid->size()!=7 && uid->size()!=10) return;
        event.length=uid->size();std::copy(uid->begin(),uid->end(),event.uid);
    }
    xQueueSend(events,&event,0); // Overflow drops access, never grants it.
}
void NfcReaderService::run() {
#ifdef OPENKNX_NFC_ST25R3916_I2C
    reader=std::make_unique<St25r3916Reader>(OPENKNX_GPIO_WIRE,OPENKNX_GPIO_SDA,OPENKNX_GPIO_SCL,
        ecp,OPENKNX_ST25R3916_I2C_ADDRESS,OPENKNX_GPIO_CLOCK);
#else
    Pn7161Reader::Config config;
#if defined(OPENKNX_NFC_READER_I2C) || defined(HOMEKEY_READER_I2C)
    config.wire=&OPENKNX_GPIO_WIRE;config.sda=OPENKNX_GPIO_SDA;config.scl=OPENKNX_GPIO_SCL;
    config.irq=NFC_IRQ_PIN;config.ven=NFC_VEN_PIN;config.address=NFC_PN7160_ADDR;
    config.clock=OPENKNX_GPIO_CLOCK;
#ifdef NFC_DWL_REQ_PIN
    config.dwl=NFC_DWL_REQ_PIN;
#endif
#else
    config.spiPins={HOMEKEY_SPI_CS,HOMEKEY_SPI_SCLK,HOMEKEY_SPI_MISO,HOMEKEY_SPI_MOSI};
    config.irq=HOMEKEY_IRQ_PIN;config.ven=HOMEKEY_VEN_PIN;
#endif
#ifdef OPENKNX_NFC_PN7161_VANT_MV
    config.vantMv=OPENKNX_NFC_PN7161_VANT_MV;
#elif defined(HOMEKEY_PN7161_VANT_MV)
    config.vantMv=HOMEKEY_PN7161_VANT_MV;
#endif
    reader=std::make_unique<Pn7161ReaderAdapter>(config,ecp);
#endif
    bool connected=false;
    std::array<uint8_t,18> appliedEcp{};
    uint32_t appliedEpoch=UINT32_MAX;
    for (;;) {
        if (!running || (extension && !extension->nfcReady())) {
            if (connected) reader->stop();
            connected=false;vTaskDelay(pdMS_TO_TICKS(100));continue;
        }
        const uint32_t scanEpoch=epoch;
        uint32_t token=0;
        std::array<uint8_t,18> desired{};
        if (extension && !extension->prepareNfc(desired,token)) { vTaskDelay(pdMS_TO_TICKS(20));continue; }
        ecp=desired;
        if (connected && appliedEpoch!=scanEpoch) { reader->stop();connected=false; }
        if (!connected) {
            connected=reader->init() && reader->beginDiscovery();
            send(connected ? Kind::Ready : Kind::Failed,scanEpoch);
            if (!connected) { reader->stop();vTaskDelay(pdMS_TO_TICKS(5000));continue; }
            appliedEcp=ecp;appliedEpoch=scanEpoch;
        } else if (ecp!=appliedEcp) {
            if (!reader->updateECP()) { reader->stop();connected=false;continue; }
            appliedEcp=ecp;
        }
        std::vector<uint8_t> uid;std::array<uint8_t,2> atqa{};uint8_t sak=0;
        if (reader->pollForTag(uid,atqa,sak,250) && running && scanEpoch==epoch) {
            send(Kind::Detected,scanEpoch);
            const bool handled=extension && extension->processNfcTag(*reader,uid,sak);
            if (!handled) send(Kind::Uid,scanEpoch,token,&uid);
            const TickType_t detected=xTaskGetTickCount();
            while (running && scanEpoch==epoch && reader->isTagStillPresent() &&
                   xTaskGetTickCount()-detected<pdMS_TO_TICKS(10000)) vTaskDelay(pdMS_TO_TICKS(100));
            reader->releaseTag();
        }
        if (!reader->healthCheck()) { reader->stop();connected=false; }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
#endif
