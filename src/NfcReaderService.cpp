#include "NfcReaderConfig.h"
#if defined(OPENKNX_ACC_NFC_READER)
#include "NfcReaderService.h"
#include "AccessControl.h"
#ifdef OPENKNX_NFC_PN7160
#include "Pn7160ReaderAdapter.hpp"
#elif defined(OPENKNX_NFC_ST25R3916_I2C)
#include "St25r3916Reader.hpp"
#else
#include "Pn7161ReaderAdapter.hpp"
#endif
#include <algorithm>
#include "Pn7161NativeI2cBridge.hpp"

NfcReaderService openknxNfc;

void NfcReaderService::setup() {
    if (!reader) createReader();
    uid.reserve(10);
#ifdef ARDUINO_ARCH_ESP32
    if (!events) events = xQueueCreate(4, sizeof(Event));
    if (!events) { failed = true; logError("NFC", "Reader event queue allocation failed"); }
#endif
}
void NfcReaderService::loop() {
    bool requested = (testing || (knx.configured() && ParamACC_NfcScanner != 0)) && !failed;
#if defined(HK_Enabled)
    // A UID-only image cannot honor an enabled HomeKey policy. Require the
    // matching ETS mode rather than silently weakening that configuration.
    const bool unsupported = knx.configured() && ParamHK_Enabled && (!extension || !reader->capabilities().homeKey());
    if (requested && !testing && unsupported && !policyBlocked)
        logWarning("NFC", "HomeKey requested but reader or firmware lacks ISO-DEP/ECP support; scanning disabled");
    policyBlocked = requested && !testing && unsupported;
    requested = requested && (testing || !unsupported);
#endif
    if (!requested) { if (running) suspend(); return; }
    running = true;
#ifdef ARDUINO_ARCH_ESP32
    if (!task && (!extension || extension->nfcReady())) {
        if (xTaskCreate([](void* self) { static_cast<NfcReaderService*>(self)->run(); },
            "AccessNfc", OPENKNX_NFC_STACK_SIZE, this, 1, &task) != pdPASS) {
            failed = true; suspend(); logError("NFC", "Reader task allocation failed; reboot required"); return;
        }
    }
#else
    if(!extension||extension->nfcReady())pollCooperative();
    else if(cooperativeConnected){reader->stop();cooperativeConnected=false;cooperativeRetry=false;}
#endif
    Event event;
    for (unsigned i=0; i<4 && receive(event); ++i) {
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
    if (!running || scanEpoch != epoch) return;
#ifdef ARDUINO_ARCH_ESP32
    if (!events) return;
#endif
    Event event{}; event.kind=kind;event.epoch=scanEpoch;event.token=token;
    if (uid) {
        if (uid->size()!=4 && uid->size()!=7 && uid->size()!=10) return;
        event.length=uid->size();std::copy(uid->begin(),uid->end(),event.uid);
    }
#ifdef ARDUINO_ARCH_ESP32
    xQueueSend(events,&event,0); // Overflow drops access, never grants it.
#else
    if(queueCount<events.size()) { events[(queueHead+queueCount)%events.size()]=event;++queueCount; }
#endif
}
void NfcReaderService::createReader() {
#ifdef OPENKNX_NFC_PN7160
    PN7160Reader::Config config;
    config.irq=NFC_IRQ_PIN;config.ven=NFC_VEN_PIN;config.address=NFC_PN7160_ADDR;
#ifdef NFC_DWL_REQ_PIN
    config.dwl=NFC_DWL_REQ_PIN;
#endif
#ifdef ARDUINO_ARCH_ESP32
    config.wire=&OPENKNX_GPIO_WIRE;config.sda=OPENKNX_GPIO_SDA;config.scl=OPENKNX_GPIO_SCL;
    config.clock=OPENKNX_GPIO_CLOCK;
#endif
    reader=std::make_unique<Pn7160ReaderAdapter>(config,ecp);
#elif defined(OPENKNX_NFC_ST25R3916_I2C)
    reader=std::make_unique<St25r3916Reader>(OPENKNX_GPIO_WIRE,OPENKNX_GPIO_SDA,OPENKNX_GPIO_SCL,
        ecp,OPENKNX_ST25R3916_I2C_ADDRESS,OPENKNX_GPIO_CLOCK);
#else
    Pn7161Reader::Config config;
#if defined(OPENKNX_NFC_READER_I2C) || defined(HOMEKEY_READER_I2C)
#if defined(OPENKNX_GPIO_NUM) && OPENKNX_GPIO_NUM > 0
    constexpr bool gpioOwnsBus=true;
#else
    constexpr bool gpioOwnsBus=false;
#endif
    PN7160_I2C_Config native;
    native.address=NFC_PN7160_ADDR;native.clock_hz=OPENKNX_GPIO_CLOCK;
    native.irq=static_cast<gpio_num_t>(NFC_IRQ_PIN);
    native.ven=static_cast<gpio_num_t>(NFC_VEN_PIN);
#ifdef NFC_DWL_REQ_PIN
    native.dwl_req=static_cast<gpio_num_t>(NFC_DWL_REQ_PIN);
#endif
    // Runtime setClock() removes/re-adds Arduino device handles. It must not
    // overlap native NFC traffic without shared lifecycle synchronization.
    if (knx.configured() && ParamACC_Keypad == 1) {
        failed=true;
        logError("NFC", "Native I2C does not support the Gira keypad clock switching");
    } else config.transport=preparePn7161NativeI2c(OPENKNX_GPIO_WIRE,
        OPENKNX_GPIO_SDA,OPENKNX_GPIO_SCL,native,gpioOwnsBus);
    if(!config.transport) {
        failed=true;
        logError("NFC", "Native I2C unavailable; NFC disabled");
    }
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
}
bool NfcReaderService::readerHealthy() {
    // Transport errors remain immediately visible. Active register/NCI probes
    // are periodic, including while a phone is held against the antenna.
    if (healthFailed || !reader->isConnected()) return false;
    const uint32_t now=millis();
    if (uint32_t(now-lastHealthCheck)<10000) return true;
    lastHealthCheck=now;
    healthFailed=!reader->healthCheck();
    return !healthFailed;
}
bool NfcReaderService::receive(Event& event) {
#ifdef ARDUINO_ARCH_ESP32
    return events && xQueueReceive(events,&event,0)==pdTRUE;
#else
    if(!queueCount)return false;
    event=events[queueHead];queueHead=(queueHead+1)%events.size();--queueCount;return true;
#endif
}
void NfcReaderService::routeTag(uint32_t scanEpoch,uint32_t token,const std::vector<uint8_t>& uid,uint8_t sak) {
    if(uid.size()!=4&&uid.size()!=7&&uid.size()!=10)return;
    if(testing){testSeen=true;return;} // Hardware test must never enroll or grant access.
    const bool handled=extension && extension->processNfcTag(*reader,uid,sak);
    if(!handled)send(Kind::Uid,scanEpoch,token,&uid);
}
#ifdef ARDUINO_ARCH_ESP32
void NfcReaderService::run() {
    bool connected=false,announced=false;
    std::array<uint8_t,18> appliedEcp{};
    uint32_t appliedEpoch=UINT32_MAX;
    for (;;) {
        if (!running || (extension && !extension->nfcReady())) {
            if (connected) reader->stop();
            connected=false;presentation.reset();vTaskDelay(pdMS_TO_TICKS(100));continue;
        }
        const uint32_t scanEpoch=epoch;
        uint32_t token=0;
        std::array<uint8_t,18> desired{};
        if (extension && !extension->prepareNfc(desired,token)) { vTaskDelay(pdMS_TO_TICKS(20));continue; }
        ecp=desired;
        if (connected && appliedEpoch!=scanEpoch) { reader->stop();connected=false; }
        if (!connected) {
            presentation.reset();
            connected=reader->init() && reader->beginDiscovery() && reader->healthCheck();
            lastHealthCheck=millis();healthFailed=false;
            announced=connected&&reader->discoveryReady();
            if(announced)send(Kind::Ready,scanEpoch);
            if(!connected)send(Kind::Failed,scanEpoch);
            if (!connected) { reader->stop();vTaskDelay(pdMS_TO_TICKS(5000));continue; }
            appliedEcp=ecp;appliedEpoch=scanEpoch;
        } else if (ecp!=appliedEcp) {
            if (!reader->updateECP()) {
                reader->stop();connected=false;send(Kind::Failed,scanEpoch);
                vTaskDelay(pdMS_TO_TICKS(5000));continue;
            }
            appliedEcp=ecp;
        }
        std::array<uint8_t,2> atqa{};uint8_t sak=0;
        const auto result=reader->pollTag(uid,atqa,sak,250);
        if (result==NfcPollResult::ReaderError) {
            reader->stop();connected=false;send(Kind::Failed,scanEpoch);
            vTaskDelay(pdMS_TO_TICKS(5000));continue;
        }
        const bool found=result==NfcPollResult::TagFound;
        if(!announced&&reader->discoveryReady()){send(Kind::Ready,scanEpoch);announced=true;}
        if (found && running && scanEpoch==epoch) {
            if (presentation.accept(uid,millis())) {
                send(Kind::Detected,scanEpoch);
                routeTag(scanEpoch,token,uid,sak);
            }
            // A held ISO-DEP session must not be forcibly released after ten
            // seconds: that would re-trigger access while the phone stays put.
            while (running && scanEpoch==epoch && reader->isTagStillPresent()) {
                presentation.seen(millis());
                if (!readerHealthy()) break;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            presentation.seen(millis());
            reader->releaseTag();
        }
        if (!found) presentation.absent(millis());
        if (!readerHealthy()) {
            reader->stop();connected=false;send(Kind::Failed,scanEpoch);
            vTaskDelay(pdMS_TO_TICKS(5000));continue;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
#else
void NfcReaderService::pollCooperative() {
    // One NCI step per main-loop iteration; never sleep or wait for RF here.
    if(!reader)createReader();
    if(cooperativeRetry && uint32_t(millis()-cooperativeRetryAt)<5000)return;
    cooperativeRetry=false;
    if(!cooperativeConnected){
        ecp.fill(0);presentation.reset();
        cooperativeConnected=reader->init()&&reader->beginDiscovery()&&reader->healthCheck();
        lastHealthCheck=millis();healthFailed=false;
        cooperativeAnnounced=false;
        if(!cooperativeConnected){reader->stop();cooperativeRetry=true;cooperativeRetryAt=millis();send(Kind::Failed,epoch);return;}
    }
    uint32_t token=0;
    std::array<uint8_t,18> desired{};
    if(extension && !extension->prepareNfc(desired,token))return;
    if(desired!=ecp){ecp=desired;if(!reader->updateECP()){reader->stop();cooperativeConnected=false;cooperativeRetry=true;cooperativeRetryAt=millis();send(Kind::Failed,epoch);return;}}
    const uint32_t scanEpoch=epoch;
    std::array<uint8_t,2> atqa{};uint8_t sak=0;
    const auto result=reader->pollTag(uid,atqa,sak,0);
    if(result==NfcPollResult::ReaderError){
        reader->stop();cooperativeConnected=false;cooperativeRetry=true;cooperativeRetryAt=millis();send(Kind::Failed,scanEpoch);return;
    }
    const bool found=result==NfcPollResult::TagFound;
    if(!cooperativeAnnounced&&reader->discoveryReady()){send(Kind::Ready,scanEpoch);cooperativeAnnounced=true;}
    if(found && running && scanEpoch==epoch){
        if(presentation.accept(uid,millis())){send(Kind::Detected,scanEpoch);routeTag(scanEpoch,token,uid,sak);}
        reader->releaseTag();
    } else if(!found) presentation.absent(millis());
    if(!readerHealthy()){
        reader->stop();cooperativeConnected=false;cooperativeRetry=true;cooperativeRetryAt=millis();send(Kind::Failed,scanEpoch);
    }
}
#endif
#endif
