#ifndef MOCK_WIRE_H
#define MOCK_WIRE_H
#include "Arduino.h"

class TwoWire {
public:
    uint8_t input = 0;
    bool readable = true;
    void begin(int, int) {}
    void beginTransmission(uint8_t) {}
    void write(uint8_t) {}
    int endTransmission() { return 0; }
    int requestFrom(uint8_t, int) { return readable ? 1 : 0; }
    int available() { return readable ? 1 : 0; }
    uint8_t read() { return input; }
};
extern TwoWire Wire;
#endif
