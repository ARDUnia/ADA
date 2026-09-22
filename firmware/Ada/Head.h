/*
 * Head.h - هدر ماژول مدیریت حرکت سر آدا
 * نویسنده: حمیدرضا میلانی نیا (ARDUnia)
 * نسخه: ۲.۰
 */

#ifndef HEAD_H
#define HEAD_H

#include "Config.h"

// تعریف حرکات ویژه
#define HEAD_MOTION_SLOW_RIGHT 1
#define HEAD_MOTION_SLOW_LEFT  2

void initHead();
void updateHead();
void setHeadSpecialMotion(int motionType);

#endif // HEAD_H
