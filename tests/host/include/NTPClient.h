#ifndef MOCK_NTP_CLIENT_H
#define MOCK_NTP_CLIENT_H
#include "Arduino.h"
#include "WiFiUdp.h"
class NTPClient {
public:
    unsigned long interval;
    long offset;
    NTPClient(WiFiUDP&, const char*, long valueOffset, unsigned long valueInterval)
        : interval(valueInterval), offset(valueOffset) {}
    void begin() {}
    bool update() { return true; }
    bool forceUpdate() { return true; }
    void setUpdateInterval(unsigned long value) { interval = value; }
    unsigned long getEpochTime() const { return 0; }
    String getFormattedTime() const { return "12:34:56"; }
};
#endif
