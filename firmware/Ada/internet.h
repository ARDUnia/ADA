/*
 * internet.h - هدر ماژول مدیریت اتصال به اینترنت
 * نویسنده: حمیدرضا میلانی نیا (ARDUnia)
 * نسخه: ۱.۰
 */

#ifndef INTERNET_H
#define INTERNET_H

#include "Config.h"

// توابع اصلی
void initInternet();
void handleInternet();

// وضعیت اتصال
bool isWiFiConnected();
String getWiFiStatus();

// دریافت زمان از NTP
String getFormattedTime();

// Force the local configuration access point from the Setup menu.
void startWiFiConfigPortal();

#endif // INTERNET_H
