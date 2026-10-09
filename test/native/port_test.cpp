// Exercise the actual port code with an instrumented scanner/GPIO dependency.
#include "Fingerprint.h"
#include "AccessControlHardware.h"
#include <cassert>
#include <cstdio>

static int scannerIoCalls = 0;
Adafruit_Fingerprint::Adafruit_Fingerprint(Stream *, uint32_t) {}
void Adafruit_Fingerprint::close() { ++scannerIoCalls; }
#define SCANNER_COMMAND(signature) uint8_t Adafruit_Fingerprint::signature { ++scannerIoCalls; return FINGERPRINT_OK; }
SCANNER_COMMAND(LEDcontrol(uint8_t, uint8_t, uint8_t, uint8_t))
SCANNER_COMMAND(getImage())
SCANNER_COMMAND(getTemplateCount())
SCANNER_COMMAND(getTemplateIndices())
SCANNER_COMMAND(image2Tz(uint8_t))
SCANNER_COMMAND(fingerSearch(uint8_t))
SCANNER_COMMAND(createModel())
SCANNER_COMMAND(get_template_buffer(int, uint8_t[]))
SCANNER_COMMAND(write_template_to_sensor(int, const uint8_t[]))
SCANNER_COMMAND(loadModel(uint16_t))
SCANNER_COMMAND(storeModel(uint16_t))
SCANNER_COMMAND(deleteModel(uint16_t))
SCANNER_COMMAND(setPassword(uint32_t))
SCANNER_COMMAND(emptyDatabase())
SCANNER_COMMAND(checkSensor())
#undef SCANNER_COMMAND

int main()
{
    Fingerprint scanner;
    uint8_t data[TEMPLATE_SIZE] = {};
    assert(!scanner.start());
    assert(!scanner.scannerReady);
    scanner.logSystemParameters();
    assert(!scanner.setLed(Fingerprint::Success));
    assert(!scanner.setLed(0, 0, 0, 0));
    assert(!scanner.hasFinger());
    assert(scanner.getTemplateCount() == 0);
    assert(scanner.getLocations() == nullptr);
    assert(scanner.getNextFreeLocation() == UINT16_MAX);
    assert(!scanner.findFingerprint().found);
    assert(!scanner.createTemplate());
    assert(!scanner.retrieveTemplate(data));
    assert(!scanner.sendTemplate(data));
    assert(!scanner.loadTemplate(0));
    assert(!scanner.storeTemplate(0));
    assert(!scanner.deleteTemplate(0));
    assert(!scanner.setPassword(0));
    assert(!scanner.emptyDatabase());
    assert(!scanner.checkSensor());
    scanner.close();
    assert(scannerIoCalls == 0);

    auto callback = +[]() {};
    AccessControlHardware::pinMode(-1, 1);
    AccessControlHardware::digitalWrite(-1, HIGH);
    assert(AccessControlHardware::digitalRead(-1) == LOW);
    AccessControlHardware::attachInterrupt(-1, callback, CHANGE);
    assert(openknx.gpio.calls == 0 && interruptCalls == 0);

    // GPIO zero is real, and expander pin IDs must not be truncated.
    AccessControlHardware::pinMode(0, 1);
    assert(openknx.gpio.calls == 1 && openknx.gpio.lastPin == 0);
    AccessControlHardware::digitalWrite(0x0205, HIGH);
    assert(openknx.gpio.lastPin == 0x0205);
    assert(AccessControlHardware::digitalRead(0x0204) == HIGH);
    assert(openknx.gpio.lastPin == 0x0204);
    AccessControlHardware::attachInterrupt(0, callback, CHANGE);
    assert(interruptCalls == 1 && lastInterruptPin == 0);
    std::puts("PASS: absent scanner performs no I/O; absent GPIO is skipped; real pin IDs survive.");
}
