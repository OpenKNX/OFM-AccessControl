OpenKNX - AccessControl module
===

Implementation of a knx access control module with up to 1500 action channels based on the [knx stack](https://github.com/OpenKNX/knx), a fork from [thelsing](https://github.com/thelsing/knx).

It supports the RP2040 and ESP32 version of the stack.

It is a PlatformIO project and needs a working ETS >=5.7 installed on the same PC.

## ESP32 platform configuration

ESP32 uses named `fingerprint`, `nfc`, and `keypad` data partitions through
`OpenKNX::Flash::Driver`. Reserve at least 61,097 bytes for NFC/keypad and 47,597
bytes for fingerprint records, with sector-aligned sizes and 64 KB-aligned
partition offsets required by the shared flash driver. RP2040 continues using
its existing `*_FLASH_OFFSET` and `*_FLASH_SIZE` definitions. Initialization
retains the existing zero-filled credential record format on both platforms.

Define `OPENKNX_ACC_DISABLE_LEGACY_NFC` and configure PlatformIO
`lib_ignore = PN7160` when another module will own NFC or no reader is present.
This excludes the legacy PN7160 headers, initialization, polling and diagnostic
wait; reader enrollment requests fail without changing stored records. Existing
NFC IDs, ETS actions and synchronization remain available. This flag does not
implement a replacement NFC reader or Apple Home Key authentication.

Omit `SCANNER_SERIAL` for builds without a fingerprint scanner. The module keeps
an inactive scanner object: startup returns false without opening a UART, and
scanner operations return failure/empty results without sending commands. Define
`SCANNER_SERIAL`, `SCANNER_SERIAL_RX_PIN` and `SCANNER_SERIAL_TX_PIN` together to
use a scanner (ESP32 supports HardwareSerial). Touch and LED pins omitted from
the board configuration default to -1; GPIO and interrupt operations skip those
pins. Existing real pins, including GPIO 0 and expander IDs, remain usable.

The XIAO ESP32-S3 target in OAM-AccessControl now enables PN7160 I2C on ESP32,
using the separately versioned local PN7160 transport port. Its NCI polling, UID
enrollment/lookup, ETS actions and KNX synchronization use the same module code
as RP2040. Do not define `OPENKNX_ACC_DISABLE_LEGACY_NFC` for this target.

On ESP32, configure `OPENKNX_GPIO_WIRE`, `OPENKNX_GPIO_SDA`, `OPENKNX_GPIO_SCL`,
`OPENKNX_GPIO_CLOCK`, `NFC_IRQ_PIN`, `NFC_VEN_PIN` and `NFC_PN7160_ADDR`.
The reader checks bus initialization and resets NCI before discovery. The
RP2040 reader initialization and polling path retain their existing behavior.

The Gira keypad uses ESP32 Wire's `setTimeOut` API and backs off on failed ACKs;
RP2040 retains its sticky-timeout recovery. Direct touch inputs on I2C expanders
are polled on ESP32 instead of registering expander IDs as MCU interrupts.
Fingerprint UART, matrix keypad, LED controllers, action channels and credential
storage are available on both architectures when the board maps their hardware.
The application includes a separate peripheral compile profile to check these
paths together. Hardware boot, KNX traffic and reader operation still require
device validation.

Native port regression checks (requires a C++17 compiler):

```sh
c++ -std=c++17 -Itest/native/stubs -Isrc \
  test/native/port_test.cpp src/Fingerprint.cpp -o /tmp/accesscontrol-port-test
/tmp/accesscontrol-port-test
```

The checks run the actual Fingerprint and GPIO adapter code against instrumented
hardware dependencies. They verify that missing devices perform no I/O, scanner
operations cannot succeed without hardware, GPIO 0 remains valid, and expander
IDs are preserved.
