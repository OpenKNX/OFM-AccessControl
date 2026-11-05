#pragma once
#include "NfcReaderConfig.h"
#if defined(OPENKNX_ACC_NFC_READER)
#include "NfcReader.hpp"
#include "NfcPresentation.h"
#include <array>
#include <memory>
#ifdef ARDUINO_ARCH_ESP32
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#endif
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
    void suspend() {
        running=false;++epoch;
#ifndef ARDUINO_ARCH_ESP32
        if(reader)reader->stop();
        cooperativeConnected=false;cooperativeRetry=false;presentation.reset();
        queueCount=queueHead=0;
#endif
    }
    void beginTest() { suspend();testing=true;testSeen=false; }
    void endTest() { testing=false;suspend(); }
    bool testTagFound() const { return testSeen; }
    void attach(NfcReaderExtension* value) { extension = value; } // Setup only, before worker starts.
    bool active() const { return running && !failed; }
    uint32_t stackHeadroom() const {
#ifdef ARDUINO_ARCH_ESP32
        return task ? uxTaskGetStackHighWaterMark(task) : 0;
#else
        return 0; // Cooperative execution has no separate task stack.
#endif
    }
private:
    enum class Kind : uint8_t { Ready, Failed, Detected, Uid };
    struct Event { Kind kind; uint32_t epoch, token; uint8_t uid[10] = {}; uint8_t length = 0; };
    void createReader();
    void routeTag(uint32_t epoch,uint32_t token,const std::vector<uint8_t>& uid,uint8_t sak);
    bool receive(Event&);
#ifdef ARDUINO_ARCH_ESP32
    void run();
#else
    void pollCooperative();
#endif
    void send(Kind, uint32_t epoch, uint32_t token = 0,
              const std::vector<uint8_t>* uid = nullptr);
#ifdef ARDUINO_ARCH_ESP32
    std::atomic<bool> running{false},testing{false},testSeen{false};
    std::atomic<uint32_t> epoch{0};
#else
    bool running=false,testing=false,testSeen=false;
    uint32_t epoch=0;
#endif
    bool failed = false;
#ifdef ARDUINO_ARCH_ESP32
    TaskHandle_t task = nullptr;
    QueueHandle_t events = nullptr;
#else
    std::array<Event,4> events{};
    size_t queueHead=0,queueCount=0;
    bool cooperativeConnected=false,cooperativeRetry=false,cooperativeAnnounced=false;
    uint32_t cooperativeRetryAt=0;
#endif
    NfcReaderExtension* extension = nullptr;
    std::array<uint8_t,18> ecp{};
    std::unique_ptr<INfcReader> reader;
    NfcPresentation presentation;
};
extern NfcReaderService openknxNfc;
#endif
