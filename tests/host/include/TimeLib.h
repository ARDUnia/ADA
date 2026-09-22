#ifndef MOCK_TIME_LIB_H
#define MOCK_TIME_LIB_H
#include <ctime>
inline void setTime(std::time_t) {}
inline int year() { return 2026; }
inline int month() { return 9; }
inline int day() { return 22; }
inline int hour() { return 12; }
inline int minute() { return 34; }
inline int second() { return 56; }
#endif
