#pragma once
#if defined(OPENKNX_ACC_NFC_READER) && defined(ARDUINO_ARCH_ESP32)
#include "NfcReader.hpp"
#include <array>
#include <atomic>
#include <memory>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#ifndef OPENKNX_NFC_STACK_SIZE
#define OPENKNX_NFC_STACK_SIZE 8192
#endif

// Optional application extension. No HomeKey or credential types in this API.
// prepare/tag run only on the NFC task; authorizeUid runs on the KNX task.
class NfcReaderExtension {
public:
    virtual ~NfcReaderExtension() = default;
    virtual bool nfcReady() const = 0;
    virtual bool prepareNfc(std::array<uint8_t,18>& ecp, uint32_t& token) = 0;
    // True consumes the tag; false requests ordinary UID processing.
    virtual bool processNfcTag(INfcReader&, const std::vector<uint8_t>&, uint8_t sak) = 0;
    virtual bool authorizeNfcUid(const uint8_t*, uint8_t length, uint32_t token) = 0;
};

class NfcReaderService {
public:
    void setup();
    void loop();
    void suspend() { running = false; ++epoch; }
    void attach(NfcReaderExtension* value) { extension = value; } // Setup only, before worker starts.
    bool active() const { return running && !failed; }
    uint32_t stackHeadroom() const { return task ? uxTaskGetStackHighWaterMark(task) : 0; }
private:
    enum class Kind : uint8_t { Ready, Failed, Detected, Uid };
    struct Event { Kind kind; uint32_t epoch, token; uint8_t uid[10] = {}; uint8_t length = 0; };
    void run();
    void send(Kind, uint32_t epoch, uint32_t token = 0,
              const std::vector<uint8_t>* uid = nullptr);
    std::atomic<bool> running{false};
    std::atomic<uint32_t> epoch{0};
    bool failed = false;
    TaskHandle_t task = nullptr;
    QueueHandle_t events = nullptr;
    NfcReaderExtension* extension = nullptr;
    std::array<uint8_t,18> ecp{};
    std::unique_ptr<INfcReader> reader;
};
extern NfcReaderService openknxNfc;
#endif
