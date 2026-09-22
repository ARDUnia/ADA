#ifndef MOCK_ESP_WIFI_H
#define MOCK_ESP_WIFI_H
#include "Arduino.h"

constexpr int WIFI_AP_STA = 3;
constexpr int WIFI_AP = 2;
constexpr int WL_DISCONNECTED = 6;
constexpr int WL_CONNECTED = 3;

class IPAddress {
public:
    String toString() const { return "192.168.1.50"; }
};
inline std::ostream& operator<<(std::ostream& os, const IPAddress& ip) { return os << ip.toString(); }

class ESP8266WiFiClass {
public:
    int statusValue = WL_DISCONNECTED;
    int beginCalls = 0;
    String currentSsid;
    std::vector<String> scanSsids;
    void mode(int) {}
    void persistent(bool) {}
    void begin(const char* ssid, const char*) { currentSsid = ssid; beginCalls++; }
    int status() const { return statusValue; }
    String SSID() const { return currentSsid; }
    String SSID(int i) const { return scanSsids.at(i); }
    int RSSI(int) const { return -50; }
    int encryptionType(int) const { return 4; }
    IPAddress localIP() const { return IPAddress(); }
    IPAddress softAPIP() const { return IPAddress(); }
    bool softAP(const char*, const char*) { return true; }
    int scanComplete() const { return -2; }
    int scanNetworks(bool) { return 0; }
    void scanDelete() {}
};
extern ESP8266WiFiClass WiFi;

class ESPClass {
public:
    bool restarted = false;
    void restart() { restarted = true; }
};
extern ESPClass ESP;
#endif
