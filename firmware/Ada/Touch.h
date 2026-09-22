/*
 * Touch.h - هدر ماژول مدیریت سنسورهای لمسی آدا
 * نویسنده: حمیدرضا میلانی نیا (ARDUnia)
 * نسخه: ۳.۰
 */

#ifndef TOUCH_H
#define TOUCH_H

#include "Config.h"

void initTouch();
void updateTouch();

// توابع مدیریت حالت‌های خاص
void setShowBattery(bool state);
bool getShowBattery();
void setConfigMode(bool state);
bool getConfigMode();

// ============================================================
//  تابع جدید برای تشخیص فعالیت لمسی
// ============================================================
bool isTouchActivityDetected();
void clearTouchActivityFlag();

// Ignore the touch used to leave another UI until all sensors are released.
void suspendTouchUntilRelease();

// Monotonic event counter; lets reactions retrigger for the same gesture/face.
uint32_t getGestureEventCounter();

#endif // TOUCH_H

