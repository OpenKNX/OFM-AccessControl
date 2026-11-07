# PN7161 application reader integration

OFM-AccessControl owns `src/Pn7161Reader.hpp` / `.cpp` and
`src/Pn7161NativeI2cBridge.hpp`. The separate PN7161 package contains the native
ESP-IDF NCI engine and SPI/I2C transports, retaining the upstream PN7160 names.

The facade manages initialization, discovery, UID reads, ISO-DEP/APDU exchange,
presence monitoring, health checks and connection state. The common NFC worker
owns recovery and presentation handling. OFM-HomeKey optionally supplies ECP data
and authentication; ordinary UID operation does not require it.

## Transport and ownership

PN7161 I2C profiles always use the native `PN7160_I2C` transport (IDF 5.4+).
There is no Wire transaction fallback or backend-selection flag. All configured
PN7161 ESP32 environments pin a compatible SDK. Other NFC chips keep their own
drivers; SPI profiles continue to use `PN7160_SPI`.

During setup the bridge obtains the existing Wire controller's native bus
handle, starting the bus owner once if no GPIO expander owner initialized it.
It registers the native device before NFC workers start and passes it through
`Config::transport`. This optional shared transport takes precedence over SPI
selection and remains owned by the bridge across reader restarts. The facade
stops NCI and lowers VEN without destroying that shared transport. Final
transport teardown requires stopping all users first. A missing native bus
disables NFC; it never silently switches to SPI or another I2C implementation.

Wire is still used by the application to own/share the bus, not to transfer NCI
frames. Other device registrations, controller/pins and clocks must remain
stable during NFC traffic. In particular, the Gira keypad's runtime clock
switching is not supported without shared lifecycle synchronization.

`Config::spiPins` is CS, SCLK, MISO, MOSI, with separate IRQ/VEN pins.
`vantMv` accepts 3300/5000 for explicit PMU configuration or zero for automatic
configuration. Match this to the board's actual supply. C++23 is required by the
NCI engine. Firmware validation is restricted to `debug_REG1_ESP_PN7161_IP`.

## ECP and validation

ECP is disabled by default. `setEcpFrame(frame, 18)` copies a caller-supplied
frame including CRC-A; only its length is checked. NCI uses the first 16 bytes
because the controller appends CRC-A. `setEcpFrame(nullptr, 0)` disables ECP.
Set it before init or call `updateECP()` to apply it to an initialized reader.
The reader does not create identities or authorize credentials.

Native transport tests live in `PN7161/test/native_i2c`; service, adapter and
presence regression tests live in the application. Hardware checks remain
necessary for reset/init, shared-bus stress, UID scans and HomeKey scans.

The module's original GPL license in `../LICENSE` is unchanged. Reader code
adapted from rednblkx/HomeKey-ESP32 retains its original MIT attribution and
notice in `../LICENSE.HomeKey-ESP32`; that notice does not replace the module's
license.
