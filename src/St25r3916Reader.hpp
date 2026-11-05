#pragma once
#include "IsoDepReaderAdapter.hpp"
#include <ST25R3916.hpp>

class St25r3916Reader final : public IsoDepReaderAdapter<ST25R3916> {
public:
    St25r3916Reader(TwoWire& wire, int sda, int scl,
                   const std::array<uint8_t,18>& frame,
                   uint8_t address=0x50, uint32_t hz=400000)
        : IsoDepReaderAdapter(frame,wire,sda,scl,address,hz) {}
};
