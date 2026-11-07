#pragma once
#if defined(ARDUINO_ARCH_ESP32) && !defined(OPENKNX_NFC_PN7160) && \
    !defined(OPENKNX_NFC_ST25R3916_I2C) && \
    (defined(OPENKNX_NFC_READER_I2C) || defined(HOMEKEY_READER_I2C))
#if !defined(ARDUINO_ARCH_ESP32)
#error Native PN7161 I2C requires ESP32
#endif
#include <esp_idf_version.h>
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)
#error Native PN7161 I2C requires the Arduino-ESP32 new I2C HAL and ESP-IDF 5.4+
#endif
#include <Wire.h>
#include <esp32-hal-i2c.h>
#include <i2c.hpp>
#include <memory>

// Application-only bridge. Call during setup, before starting any NFC workers.
// The bus owner must retain Wire, its pins and controller until reboot. Other
// users may transfer on different addresses; no Wire scans/access at NFC's
// address, Wire.end(), pin/clock reconfiguration, or asynchronous callbacks.
// Retaining the device here avoids add/remove races during reader recovery.
inline std::shared_ptr<IPN7160Transport> preparePn7161NativeI2c(
    TwoWire& wire, int sda, int scl, PN7160_I2C_Config config,
    bool ownerAlreadyStarted) {
    static std::shared_ptr<PN7160_I2C> transport;
    if (transport) return transport;
    const auto port = wire.getBusNum();
    // With no GPIO expanders, Common deliberately does not start Wire. In
    // that case this setup bridge is its explicit lifetime owner/bootstrap.
    // Never call begin again for lookup or from the NFC recovery task.
    if (!i2cIsInit(port)) {
        if (ownerAlreadyStarted || !wire.begin(sda, scl, config.clock_hz))
            return nullptr;
    }
    if (i2c_master_get_bus_handle(static_cast<i2c_port_num_t>(port), &config.bus) != ESP_OK ||
        !config.bus || config.bus != static_cast<i2c_master_bus_handle_t>(i2cBusHandle(port)))
        return nullptr;
    auto candidate = std::make_shared<PN7160_I2C>(config);
    const esp_err_t error = candidate->init();
    if (error != ESP_OK) {
        logError("NFC", "Native I2C device registration failed: 0x%X", error);
        return nullptr;
    }
    transport = candidate;
    logInfo("NFC", "Native I2C ready: IDF %s, port %u, SDA %d, SCL %d, address 0x%02X",
        esp_get_idf_version(), port, sda, scl, config.address);
    return transport;
}
#endif
