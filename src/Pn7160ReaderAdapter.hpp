#pragma once
#include "NfcReader.hpp"
#include <Wire.h>
#include <reader/pn7160reader.hpp>
// Standard UID-only PN7160 adapter. The legacy engine has no APDU/ECP API.
class Pn7160ReaderAdapter final:public INfcReader {
public:
    Pn7160ReaderAdapter(const PN7160Reader::Config& config,const std::array<uint8_t,18>& frame):device(config),ecp(frame){}
    bool init()override{return updateECP()&&device.init();}
    void stop()override{device.stop();}
    bool isConnected()const override{return device.isConnected();}
    bool discoveryReady()const override{return device.discoveryReady();}
    uint8_t getFwMajor()const override{return 0;} // Unknown: not exposed by this NCI API.
    uint8_t getFwMinor()const override{return 0;}
    bool beginDiscovery()override{return device.beginDiscovery();}
    bool pollForTag(std::vector<uint8_t>& uid,std::array<uint8_t,2>& atqa,uint8_t& sak,uint32_t)override{
        atqa.fill(0);sak=0;return device.pollForTag(uid);
    }
    bool isTagStillPresent()override{return device.isTagStillPresent();}
    void releaseTag()override{device.releaseTag();}
    void endDiscovery()override{device.stop();}
    bool healthCheck()override{return device.healthCheck();}
    bool updateECP()override{return ecp[0]!=0x6a;}
    bool exchangeApdu(const std::vector<uint8_t>&,std::vector<uint8_t>& response,uint32_t)override{
        response.clear();return false;
    }
private:
    PN7160Reader device;
    const std::array<uint8_t,18>& ecp;
};
