#pragma once
#include "IsoDepReaderAdapter.hpp"
#include <ST25R3916.hpp>

class St25r3916Reader final : public IsoDepReaderAdapter<ST25R3916> {
public:
    St25r3916Reader(i2c_master_bus_handle_t bus,
                   const std::array<uint8_t,18>& frame,
                   uint8_t address=0x50, uint32_t hz=400000)
        : IsoDepReaderAdapter(frame,bus,address,hz) {}
};
