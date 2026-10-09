#include <utility>
#include "KeypadForGira.h"

KeypadForGira::KeypadForGira() {};

void KeypadForGira::init(bool testMode)
{
#ifdef OPENKNX_GPIO_WIRE
    // if (!testMode && ParamACC_NfcScanner == 0)
    //     return;

#ifdef KEYPAD_PCA9633_ADDR
    _ledController.begin(KEYPAD_PCA9633_ADDR, &OPENKNX_GPIO_WIRE);
    _ledController.setLdrStateAll(LDR_STATE_IND);
    _ledController.setRGBW(0, 0, 0, 255);
    _ledInitialized = true;
    logInfoP("Initialized PCA9633.");
#endif

    // Bound every BS811X / bus I2C operation. While the chip is in its idle low-power state it
    // clock-stretches and ignores I2C; without this the loop blocks ~2 s per poll (one timeout
    // for the register-pointer write, one for the read). 20 ms is ample for a real transaction
    // and keeps us well under OPENKNX_LOOPTIME_WARNING. reset_with_timeout=true recovers the bus.
#ifdef ARDUINO_ARCH_ESP32
    OPENKNX_GPIO_WIRE.setTimeOut(20);
#else
    OPENKNX_GPIO_WIRE.setTimeout(20, true);
#endif

    // The BS8116A is limited to 100 kHz; talk to it at that speed, then restore the global speed.
    OPENKNX_GPIO_WIRE.setClock(BS8116_I2C_CLOCK);
    _configVerified = _keypad.begin("8116");
    OPENKNX_GPIO_WIRE.setClock(OPENKNX_GPIO_CLOCK);
    if (_configVerified)
        logInfoP("Initialized BS8116 (settings applied).");
    else
        logInfoP("BS8116 not yet configured (chip idle); will retry once it becomes responsive.");
#endif
    KeypadBase::init(testMode);
    _lastKeymap = 0;
    _initialized = true;
}

bool KeypadForGira::tryApplyConfig()
{
    // Re-send the option registers and read them back. With the short bus timeout this returns
    // quickly: success if the chip is awake, fast timeout (-> retry later) if it is still idle.
    if (_keypad.setSetting() != 0)
        return false;
    uint8_t readback[21] = {0};
    _keypad.readSetting(readback);
    return readback[4] == 0x98; // register 0xB4 (Option2): bit6 = LSC, expected cleared
}

void KeypadForGira::loop(bool testMode)
{
    if (!_initialized)
        return;

    // first call base (necessary for correct effect evaluation)
    KeypadBase::loop(testMode);

    if (!delayCheck(_readKeysTimer, _readKeysInterval))
        return;
    _readKeysTimer = delayTimerInit();

#ifdef OPENKNX_GPIO_WIRE
    // The BS8116A is limited to 100 kHz, so drop the shared bus to that speed only while we talk
    // to it (config retry + key readout), then restore the global speed for the other devices.
    OPENKNX_GPIO_WIRE.setClock(BS8116_I2C_CLOCK);

    // The option write only succeeds while the chip is awake (e.g. after the first key touch).
    // Keep retrying cheaply until LSC=0 is confirmed; afterwards the chip stays responsive.
    if (!_configVerified && delayCheck(_configRetryTimer, 1000))
    {
        _configRetryTimer = delayTimerInit();
        if (tryApplyConfig())
        {
            _configVerified = true;
            logInfoP("BS8116 settings applied and verified (LSC off).");
        }
    }

    // Read the keys, watching whether the bus timed out. A timeout means the chip is in standby
    // and clock-stretching, so polling fast is pointless: back off to the standby interval. As
    // soon as a read succeeds (chip awake) we return to the fast interval. The chip's own
    // wake-from-standby latency is 0.5-1 s, so the slower idle poll costs no real responsiveness.
#ifndef ARDUINO_ARCH_ESP32
    OPENKNX_GPIO_WIRE.clearTimeoutFlag();
#endif
    uint16_t keymap = _keypad.readKeys();
#ifdef ARDUINO_ARCH_ESP32
    // ESP32 Wire has no sticky timeout flag. Probe ACK after the read to back off
    // while the BS8116 is asleep or disconnected, keeping every transaction bounded.
    OPENKNX_GPIO_WIRE.beginTransmission(BS811X_LSF);
    _readKeysInterval = OPENKNX_GPIO_WIRE.endTransmission() != 0 ? READ_KEYS_INTERVAL_STANDBY : READ_KEYS_INTERVAL;
#else
    _readKeysInterval = OPENKNX_GPIO_WIRE.getTimeoutFlag() ? READ_KEYS_INTERVAL_STANDBY : READ_KEYS_INTERVAL;
#endif

    OPENKNX_GPIO_WIRE.setClock(OPENKNX_GPIO_CLOCK); // restore global bus speed

    if (testMode)
    {
        char map[16];
        for (uint8_t i = 0; i < 16; i++)
            map[i] = (keymap & (1 << i)) ? '1' : '0';

        Serial.print("Key status:");
        Serial.print(map);

        uint8_t key = _keypad.getKey_active();
        Serial.print("  Pressed key: ");
        Serial.println(key);

        delay(1000);
    }

    uint16_t newlyPressed = keymap & ~_lastKeymap;

    // send also a key up event
    if (_callback && keymap == 0 && _lastKeymap != 0 && !(_lastKeymap & (_lastKeymap - 1)))
        _callback('\0');

    _lastKeymap = keymap;

    if (!_callback || newlyPressed == 0)
        return;
    // skip, if more than 1 bit is set (more than one key is pressed)
    if (newlyPressed & (newlyPressed-1))
        return;
        
    for (uint8_t index = 0; index < 16; ++index)
    {
        if (newlyPressed & (1 << index))
        {
            char resolved = mapKey(index);
            if (resolved != '\0')
                _callback(resolved);
        }
    }
#endif
}

void KeypadForGira::setInfoLed(uint32_t ledColor)
{
#ifdef KEYPAD_PCA9633_ADDR
    _ledRed = (ledColor & 0xFF0000) >> 16;
    _ledGreen = (ledColor & 0xFF00) >> 8;
    _ledBlue = ledColor & 0xFF;
    
    updateLeds();
#endif
}

void KeypadForGira::setBackgroundLed(uint8_t brightness)
{
#ifdef KEYPAD_PCA9633_ADDR
    _ledBackground = brightness;

    updateLeds();
#endif
}

void KeypadForGira::updateLeds()
{
#ifdef KEYPAD_PCA9633_ADDR
    if (!_ledInitialized)
        return;
    
    uint8_t ledBackground = 255 - _ledBackground; // background is inverted (0 = max brightness, 255 = off)
    _ledController.setRGBW(_ledRed, _ledGreen, _ledBlue, ledBackground);
    logDebugP("LEDs now R:%d G:%d B:%d BG:%d", _ledRed, _ledGreen, _ledBlue, _ledBackground);
#endif
}

#ifdef KEYPAD_PCA9633_ADDR
void KeypadForGira::runTestMode()
{
    if (!_ledInitialized)
        return;
    
    logDebugP("RGB with background, all max");
    _ledController.setRGBW(255, 255, 255, 0);
    delay(1000);

    logDebugP("Red only");
    _ledController.setRGBW(255, 0, 0, 255);
    delay(1000);

    logDebugP("Green only");
    _ledController.setRGBW(0, 255, 0, 255);
    delay(1000);

    logDebugP("Blue only");
    _ledController.setRGBW(0, 0, 255, 255);
    delay(1000);

    logDebugP("Background only max");
    _ledController.setRGBW(0, 0, 0, 0);
    delay(1000);

    logDebugP("Background only 50%");
    _ledController.setRGBW(0, 0, 0, 128);
    delay(1000);

    logDebugP("Background only 25%");
    _ledController.setRGBW(0, 0, 0, 196);
    delay(1000);
}
#endif

char KeypadForGira::mapKey(uint8_t index) const
{
    static constexpr char lookup[] = {'F', '3', 'B', '6', '5', '4', '7', '8', '9', 'K', '*', '0', '#', 'C', '2', '1'};
    if (index < sizeof(lookup))
        return lookup[index];
    return '\0';
}
