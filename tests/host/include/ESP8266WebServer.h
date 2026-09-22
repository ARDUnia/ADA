#ifndef MOCK_WEB_SERVER_H
#define MOCK_WEB_SERVER_H
#include "Arduino.h"
class ESP8266WebServer {
public:
    explicit ESP8266WebServer(int) {}
    template <typename F> void on(const char*, F) {}
    template <typename F> void onNotFound(F) {}
    void begin() {}
    void stop() {}
    void handleClient() {}
    String arg(const char*) const { return ""; }
    void send(int, const char*, const String&) {}
    void send(int, const char*, const char*) {}
};
#endif
