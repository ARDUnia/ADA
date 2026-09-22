#ifndef MOCK_SERVO_H
#define MOCK_SERVO_H
#include "Arduino.h"
class Servo {
public:
    static int minWritten;
    static int maxWritten;
    void attach(int, int, int) {}
    void write(int angle) { minWritten = std::min(minWritten, angle); maxWritten = std::max(maxWritten, angle); }
};
#endif
