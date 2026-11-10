#pragma once
#if defined(ARDUINO_ARCH_ESP32) && defined(OPENKNX_NFC_ST25R3916_I2C)
#include <esp_idf_version.h>
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)
#error Native ST25R3916 I2C requires Arduino-ESP32 with ESP-IDF 5.4+
#endif
#include <Wire.h>
#include <esp32-hal-i2c.h>
#include <driver/i2c_master.h>

// Setup-only application bridge, before the NFC worker starts. The owner must
// retain the bus, pins and clock; no Wire.end(), remapping, scans/access at the
// NFC address, or concurrent device/bus lifecycle changes are supported.
inline i2c_master_bus_handle_t prepareSt25r3916NativeI2c(
    TwoWire& wire, int sda, int scl, uint32_t clockHz, bool ownerAlreadyStarted) {
    const auto port = wire.getBusNum();
    // Common starts Wire when GPIO expanders use it. Otherwise bootstrap it
    // here, once during setup, never from reader init/recovery.
    if (!i2cIsInit(port)) {
        if (ownerAlreadyStarted || !wire.begin(sda, scl, clockHz)) return nullptr;
    }
    i2c_master_bus_handle_t bus = nullptr;
    if (i2c_master_get_bus_handle(static_cast<i2c_port_num_t>(port), &bus) != ESP_OK ||
        !bus || bus != static_cast<i2c_master_bus_handle_t>(i2cBusHandle(port)))
        return nullptr;
    return bus;
}
#endif
