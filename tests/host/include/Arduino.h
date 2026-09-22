#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

using byte = uint8_t;
using std::size_t;

class String {
public:
    String() = default;
    String(const char* value) : data_(value ? value : "") {}
    String(const std::string& value) : data_(value) {}
    String(char value) : data_(1, value) {}
    String(int value) : data_(std::to_string(value)) {}
    String(unsigned int value) : data_(std::to_string(value)) {}
    String(long value) : data_(std::to_string(value)) {}
    String(unsigned long value) : data_(std::to_string(value)) {}
    String(float value) : data_(std::to_string(value)) {}
    unsigned int length() const { return static_cast<unsigned int>(data_.size()); }
    char charAt(unsigned int index) const { return index < data_.size() ? data_[index] : '\0'; }
    const char* c_str() const { return data_.c_str(); }
    char operator[](unsigned int index) const { return charAt(index); }
    String& operator+=(const String& rhs) { data_ += rhs.data_; return *this; }
    String& operator+=(const char* rhs) { data_ += rhs ? rhs : ""; return *this; }
    String& operator+=(char rhs) { data_ += rhs; return *this; }
    bool operator==(const String& rhs) const { return data_ == rhs.data_; }
    bool operator==(const char* rhs) const { return data_ == (rhs ? rhs : ""); }
    bool operator!=(const String& rhs) const { return !(*this == rhs); }
    bool operator!=(const char* rhs) const { return !(*this == rhs); }
    const std::string& stdstr() const { return data_; }
private:
    std::string data_;
};

inline String operator+(String lhs, const String& rhs) { lhs += rhs; return lhs; }
inline String operator+(String lhs, const char* rhs) { lhs += rhs; return lhs; }
inline String operator+(const char* lhs, const String& rhs) { String out(lhs); out += rhs; return out; }
inline std::ostream& operator<<(std::ostream& os, const String& value) { return os << value.c_str(); }

template <typename T> inline T constrain(T value, T low, T high) {
    return value < low ? low : (value > high ? high : value);
}
template <typename T> inline T min(T a, T b) { return a < b ? a : b; }
template <typename T> inline T max(T a, T b) { return a > b ? a : b; }

extern unsigned long mockMillis;
extern int mockAnalogValue;
unsigned long millis();
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
long random(long maxValue);
long random(long minValue, long maxValue);
int analogRead(int pin);

class SerialMock {
public:
    std::string log;
    void begin(unsigned long) {}
    template <typename T> void print(const T& value) { std::ostringstream os; os << value; log += os.str(); }
    template <typename T> void println(const T& value) { print(value); log += '\n'; }
    void println() { log += '\n'; }
    void clear() { log.clear(); }
};
extern SerialMock Serial;

constexpr int A0 = 0;

#endif
