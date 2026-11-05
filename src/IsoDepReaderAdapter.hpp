#pragma once
#include "NfcReader.hpp"
#include <utility>

// Common facade for standalone ISO-DEP/ECP drivers. The service owns the frame;
// each driver copies it, and only NCI readers require a hardware apply command.
template<class Device, bool ApplyEcpToHardware = false>
class IsoDepReaderAdapter : public INfcReader {
public:
    NfcReaderCapabilities capabilities() const override { return {true,true,true}; }
    bool init() override { return updateECP() && device.init(); }
    void stop() override { device.stop(); }
    bool isConnected() const override { return device.isConnected(); }
    uint8_t getFwMajor() const override { return device.getFwMajor(); }
    uint8_t getFwMinor() const override { return device.getFwMinor(); }
    bool beginDiscovery() override { return device.beginDiscovery(); }
    bool pollForTag(std::vector<uint8_t>& uid, std::array<uint8_t,2>& atqa,
                    uint8_t& sak, uint32_t timeoutMs) override {
        return device.pollForTag(uid,atqa,sak,timeoutMs);
    }
    bool isTagStillPresent() override { return device.isTagStillPresent(); }
    void releaseTag() override { device.releaseTag(); }
    void endDiscovery() override { device.endDiscovery(); }
    bool exchangeApdu(const std::vector<uint8_t>& send, std::vector<uint8_t>& recv,
                      uint32_t timeoutMs) override { return device.exchangeApdu(send,recv,timeoutMs); }
    bool healthCheck() override { return device.healthCheck(); }
    bool updateECP() override {
        if (!device.setEcpFrame(ecp.data(),ecp[0]==0x6a ? ecp.size() : 0)) return false;
        if constexpr (ApplyEcpToHardware) return !device.isConnected() || device.updateECP();
        return true;
    }
protected:
    template<class... Args>
    explicit IsoDepReaderAdapter(const std::array<uint8_t,18>& frame, Args&&... args)
        : device(std::forward<Args>(args)...),ecp(frame) {}
private:
    Device device;
    const std::array<uint8_t,18>& ecp;
};
