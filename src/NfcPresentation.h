#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

// One action per presentation. Readers that repeatedly activate a held UID
// must observe a full second without that UID before it can trigger again.
class NfcPresentation {
public:
    void reset() { length = 0; }
    bool accept(const std::vector<uint8_t>& uid, uint32_t now) {
        if (uid.size()!=4 && uid.size()!=7 && uid.size()!=10) return false;
        const bool repeated = length==uid.size() && std::equal(uid.begin(),uid.end(),last.begin());
        std::copy(uid.begin(),uid.end(),last.begin());length=uid.size();seen(now);
        return !repeated;
    }
    void seen(uint32_t now) { lastSeen = now; }
    void absent(uint32_t now) { if (uint32_t(now-lastSeen)>=1000) reset(); }
private:
    std::array<uint8_t,10> last{};
    uint8_t length = 0;
    uint32_t lastSeen = 0;
};
