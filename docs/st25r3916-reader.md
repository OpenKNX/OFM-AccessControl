# ST25R3916 application integration

The standalone ST25R3916 library uses native ESP-IDF 5.4+ I2C, GPIO-independent
interrupt polling and FreeRTOS APIs. Its constructor borrows an initialized
`i2c_master_bus_handle_t`; it never starts, remaps or deletes the bus.

`St25r3916NativeI2cBridge.hpp` contains the Arduino-specific setup. It borrows
Common's existing Wire bus, or starts it once if no GPIO expander owns it, then
checks that the Arduino and ESP-IDF handles agree. Missing handles disable NFC.
Reader recovery only reinitializes the native device, without `Wire.begin()`.

`St25r3916Reader.hpp` adapts the native library to `INfcReader`. ECP is copied
through the existing adapter; OFM-HomeKey optionally generates the frame and
handles authentication. UID-only operation does not require OFM-HomeKey.

The bus owner must retain Wire, pins and clock until all reader calls have
stopped and the reader is destroyed. Serialize other bus/device lifecycle
changes. Do not scan/access the NFC address through Wire or change the clock
during NFC traffic. The Gira keypad's runtime clock switching is unsupported,
as with the native PN bridge.

Run `python3 test/native/st25_bridge/run.py` for bridge bootstrap/lookup checks.
The library's `test/run.py` tests protocol and device lifecycle without Arduino.
Physical UID/HomeKey checks are still required after changing the integration.
