/*
 * Display.h - هدر ماژول مدیریت نمایشگر و چهره گرافیکی آدا
 * نویسنده: حمیدرضا میلانی نیا (ARDUnia)
 * نسخه: ۲.۴
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "Config.h"

extern FaceState currentFace;

// توابع اصلی نمایشگر
void initDisplay();
void updateDisplay();
void playStartupAnimation();

// توابع باتری
float readBatteryVoltage();
float getBatteryPercentage();
void drawBatteryStatus();

// توابع نمایش زمان اینترنتی
void showInternetTime();
void drawTimeDisplay();
bool isTimeDisplayActive();
void deactivateTimeDisplay();

// ============================================================
//  توابع مدیریت وضعیت سیستم و روشنایی (برای Ada.ino)
// ============================================================
void setSystemState(SystemState state);
SystemState getSystemState();
void setDisplayBrightness(uint8_t level);
void resetInactivityTimer();

#endif // DISPLAY_H

