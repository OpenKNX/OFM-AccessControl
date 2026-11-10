#include "St25r3916NativeI2cBridge.hpp"
#include <cassert>
#include <cstdio>
int main() {
    TwoWire wire;
    // Common claimed ownership, but its bus failed: do not restart it.
    assert(!prepareSt25r3916NativeI2c(wire,21,22,400000,true));
    assert(wire.begins == 0);
    wire.beginOk = false;
    assert(!prepareSt25r3916NativeI2c(wire,21,22,400000,false));
    assert(wire.begins == 1);
    wire.beginOk = true;
    assert(prepareSt25r3916NativeI2c(wire,21,22,400000,false) == &busInstance);
    assert(wire.begins == 2);
    // Borrow an already started bus without begin or pin/clock changes.
    assert(prepareSt25r3916NativeI2c(wire,21,22,400000,true) == &busInstance);
    assert(wire.begins == 2);
    lookupFails = true;
    assert(!prepareSt25r3916NativeI2c(wire,21,22,400000,true));
    lookupFails = false; nullHandle = true;
    assert(!prepareSt25r3916NativeI2c(wire,21,22,400000,true));
    nullHandle = false; mismatch = true;
    assert(!prepareSt25r3916NativeI2c(wire,21,22,400000,true));
    assert(wire.begins == 2);
    std::puts("ST25 Arduino setup bridge: bootstrap, borrowing and lookup failures passed");
}
