#pragma once
#include <driver/i2c_master.h>
struct TwoWire {
    unsigned begins = 0;
    bool beginOk = true;
    uint8_t getBusNum() const { return 1; }
    bool begin(int, int, uint32_t) { ++begins; initialized = beginOk; return beginOk; }
};
