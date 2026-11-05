#pragma once

#include "NfcReader.hpp"
#include <Pn7161Reader.hpp>

// Access-control reader adapter. Chip and ISO-DEP implementation lives in PN7161.
// The NFC service owns the referenced optional ECP data.
class Pn7161ReaderAdapter final : public INfcReader {
public:
    Pn7161ReaderAdapter(const Pn7161Reader::Config& config,
                        const std::array<uint8_t,18>& ecpData)
        : device(config), ecp(ecpData) {}
    bool init() override { return updateECP() && device.init(); }
    void stop() override { device.stop(); }
    bool isConnected() const override { return device.isConnected(); }
    uint8_t getFwMajor() const override { return device.getFwMajor(); }
    uint8_t getFwMinor() const override { return device.getFwMinor(); }
    bool beginDiscovery() override { return device.beginDiscovery(); }
    bool pollForTag(std::vector<uint8_t>& uid, std::array<uint8_t,2>& atqa,
                    uint8_t& sak, uint32_t timeoutMs) override {
        return device.pollForTag(uid, atqa, sak, timeoutMs);
    }
    bool isTagStillPresent() override { return device.isTagStillPresent(); }
    void releaseTag() override { device.releaseTag(); }
    void endDiscovery() override { device.endDiscovery(); }
    bool exchangeApdu(const std::vector<uint8_t>& send, std::vector<uint8_t>& recv,
                      uint32_t timeoutMs) override {
        return device.exchangeApdu(send, recv, timeoutMs);
    }
    bool healthCheck() override { return device.healthCheck(); }
    bool updateECP() override {
        // Before init, stage the frame. A live reader also needs the NCI
        // configuration command and discovery restart.
        return device.setEcpFrame(ecp.data(), ecp[0] == 0x6A ? ecp.size() : 0)
            && (!device.isConnected() || device.updateECP());
    }
private:
    Pn7161Reader device;
    const std::array<uint8_t,18>& ecp;
};
