#pragma once
#include <cstdint>
using esp_err_t = int;
constexpr esp_err_t ESP_OK = 0, ESP_FAIL = -1;
using i2c_port_num_t = int;
struct Bus {};
using i2c_master_bus_handle_t = Bus*;
inline Bus busInstance, otherBus;
inline bool initialized = false, lookupFails = false, nullHandle = false;
inline esp_err_t i2c_master_get_bus_handle(i2c_port_num_t port, Bus** bus) {
    if (port != 1 || lookupFails) return ESP_FAIL;
    *bus = nullHandle ? nullptr : &busInstance;
    return ESP_OK;
}
