/*
 * Audio.h - هدر ماژول مدیریت افکت های صوتی آدا از طریق DFPlayer Mini
 * نویسنده: حمیدرضا میلانی نیا (ARDUnia)
 */

#ifndef AUDIO_H
#define AUDIO_H

#include "Config.h"

// تابع راه اندازی اولیه سخت افزار و نرم افزار ماژول صدا
void initAudio();

// تابع بروز رسانی مستمر و پخش هوشمند صدا بر اساس تغییر وضعیت چهره
void updateAudio();

// تابع اختصاصی پخش صدای خوش آمدگویی در زمان روشن شدن
void playStartupSound();

void setAudioVolume(uint8_t volume);
uint8_t getAudioVolume();

#endif // AUDIO_H
