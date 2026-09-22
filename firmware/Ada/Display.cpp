/*
 * Display.cpp - پیاده‌سازی کامل نمایشگر با پشتیبانی از چهره، باتری، زمان و خواب
 * نسخه: ۳.۶ (نمایش صحیح ساعت و نمونه‌برداری یکتای باتری)
 */

#include "Display.h"
#include "Touch.h"   // برای getShowBattery()
#include "internet.h"   // برای getFormattedTime()
#include <Wire.h>
#include <U8g2lib.h>

// ============================================================
//  راه‌اندازی OLED
// ============================================================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ============================================================
//  متغیرهای نمایش زمان اینترنتی
// ============================================================
static bool timeDisplayActive = false;
static unsigned long timeDisplayStartTime = 0;
const unsigned long TIME_DISPLAY_DURATION = 5000; // ۵ ثانیه نمایش
static String currentTimeStr = "--:--:--";

// ============================================================
//  متغیرهای انیمیشن چهره
// ============================================================
static int currentPupilX = 0;
static int currentPupilY = 0;
static int targetPupilX = 0;
static int targetPupilY = 0;
static unsigned long lastPupilMoveTime = 0;
static unsigned long pupilMoveInterval = 2500;
static unsigned long lastPupilStepTime = 0;
static int breathingOffset = 0;
static bool breathingDirection = true;
static unsigned long lastBreathingTime = 0;
static bool eatingToggle = false;
static unsigned long lastEatingTime = 0;
static bool rapidBlinkToggle = false;
static unsigned long lastRapidBlinkTime = 0;
static unsigned long lastBlinkTime = 0;
static unsigned long blinkInterval = 4000;
static bool isBlinking = false;
const unsigned long blinkDuration = 120;

// مختصات چشم‌ها
const int leftEyeX = 12;
const int rightEyeX = 74;
const int eyeY = 8;
const int eyeWidth = 42;
const int eyeHeight = 48;
const int eyeRadius = 10;

// ============================================================
//  متغیرهای باتری
// ============================================================
static float batteryVoltage = 0.0;
static float batteryPercent = 0.0;
static unsigned long lastBatteryRead = 0;
const unsigned long BATTERY_READ_INTERVAL = 500;

// ============================================================
//  متغیرهای وضعیت سیستم و روشنایی
// ============================================================
static uint8_t currentBrightness = BRIGHTNESS_NORMAL;
static unsigned long lastBrightnessChange = 0;

// ============================================================
//  متغیرهای حالت نیمه‌بسته (برای خواب)
// ============================================================
static bool halfClosedMode = false;

// ============================================================
//  پرتوتایپ توابع کمکی
// ============================================================
void updateAnimationTimers();
void drawNormalEyes();
void drawHappyEyes();
void drawAngryEyes();
void drawThinkingEyes();
void drawSleepingEyes();
void drawSurprisedEyes();
void drawSadEyes();
void drawWinkEyes();
void drawFearEyes();
void drawEcstasyEyes();
void drawEatingEyes();
void drawRapidBlinkEyes();
void drawSickEyes();
void drawHungryEyes();
void drawXOEyes();

// ============================================================
//  توابع عمومی (اعلان شده در هدر)
// ============================================================

void initDisplay() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(32, 36, "ADA SYSTEM O.S.");
    u8g2.sendBuffer();
    delay(800);

    unsigned long now = millis();
    lastPupilMoveTime = now;
    lastPupilStepTime = now;
    lastBreathingTime = now;
    lastEatingTime = now;
    lastRapidBlinkTime = now;
    lastBlinkTime = now;
    lastBatteryRead = now;

    setDisplayBrightness(BRIGHTNESS_NORMAL);
}

// ============================================================
//  توابع نمایش زمان اینترنتی
// ============================================================

void showInternetTime() {
    currentTimeStr = getFormattedTime();
    timeDisplayActive = true;
    timeDisplayStartTime = millis();
    Serial.println("Time display activated: " + currentTimeStr);
}

bool isTimeDisplayActive() {
    return timeDisplayActive;
}

void deactivateTimeDisplay() {
    timeDisplayActive = false;
    Serial.println("Time display deactivated by function call");
}

// ============================================================
//  تابع اصلی به‌روزرسانی نمایشگر (بهینه‌سازی شده)
// ============================================================
void updateDisplay() {
    // اولویت ۱: نمایش باتری
    if (getShowBattery()) {
        unsigned long now = millis();
        if (now - lastBatteryRead >= BATTERY_READ_INTERVAL) {
            batteryVoltage = readBatteryVoltage();
            float clamped = batteryVoltage;
            if (clamped < BATTERY_MIN_VOLTAGE) clamped = BATTERY_MIN_VOLTAGE;
            if (clamped > BATTERY_MAX_VOLTAGE) clamped = BATTERY_MAX_VOLTAGE;
            batteryPercent = (clamped - BATTERY_MIN_VOLTAGE) /
                             (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100.0f;
            lastBatteryRead = now;
        }
        drawBatteryStatus();
        return;
    }

    // ============================================================
    //  اولویت ۲: نمایش زمان اینترنتی (با سرعت بالا)
    // ============================================================
    if (timeDisplayActive) {
        if (millis() - timeDisplayStartTime >= TIME_DISPLAY_DURATION) {
            timeDisplayActive = false;
            Serial.println("Time display deactivated (timeout)");
        } else {
            // رسم مستقیم زمان (بدون فراخوانی تابع جداگانه برای سرعت)
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_6x10_tf);
            const char* title = "INTERNET TIME";
            u8g2.drawStr((128 - u8g2.getStrWidth(title)) / 2, 12, title);
            u8g2.setFont(u8g2_font_fub20_tf);
            int strWidth = u8g2.getStrWidth(currentTimeStr.c_str());
            int strX = (128 - strWidth) / 2;
            int strY = 43;
            u8g2.drawStr(strX, strY, currentTimeStr.c_str());
            u8g2.setFont(u8g2_font_6x10_tf);
            const char* hint = "Touch to exit";
            u8g2.drawStr((128 - u8g2.getStrWidth(hint)) / 2, 62, hint);
            u8g2.sendBuffer();
            return;
        }
    }

    // ============================================================
    //  رسم چهره (با فریم‌ریت بالاتر)
    // ============================================================
    unsigned long now = millis();
    static unsigned long lastRenderTime = 0;
    // کاهش فریم‌ریت از ۳۰ به ۱۵ میلی‌ثانیه برای پاسخ‌دهی بهتر
    if (now - lastRenderTime < 15) return;
    lastRenderTime = now;

    FaceState displayFace = currentFace;

    if (currentSystemState == STATE_HALF_CLOSED) {
        displayFace = FACE_SLEEPY;
        halfClosedMode = true;
    } else if (currentSystemState == STATE_SLEEPING) {
        displayFace = FACE_SLEEPY;
        halfClosedMode = false;
    } else {
        halfClosedMode = false;
    }

    updateAnimationTimers();
    u8g2.clearBuffer();

    switch (displayFace) {
        case FACE_NORMAL:      drawNormalEyes();      break;
        case FACE_HAPPY:       drawHappyEyes();       break;
        case FACE_ANGRY:       drawAngryEyes();       break;
        case FACE_THINKING:    drawThinkingEyes();    break;
        case FACE_SLEEPY:      drawSleepingEyes();    break;
        case FACE_SURPRISED:   drawSurprisedEyes();   break;
        case FACE_SAD:         drawSadEyes();         break;
        case FACE_WINK:        drawWinkEyes();        break;
        case FACE_FEAR:        drawFearEyes();        break;
        case FACE_ECSTASY:     drawEcstasyEyes();     break;
        case FACE_EATING:      drawEatingEyes();      break;
        case FACE_RAPID_BLINK: drawRapidBlinkEyes();  break;
        case FACE_SICK:        drawSickEyes();        break;
        case FACE_HUNGRY:      drawHungryEyes();      break;
        case FACE_XO:          drawXOEyes();          break;
        default:               drawNormalEyes();      break;
    }

    u8g2.sendBuffer();
}

// ============================================================
//  توابع مدیریت سیستم و روشنایی (بدون تغییر)
// ============================================================

void setSystemState(SystemState state) {
    currentSystemState = state;
    switch (state) {
        case STATE_ACTIVE:
            setDisplayBrightness(BRIGHTNESS_NORMAL);
            break;
        case STATE_DIMMED:
        case STATE_HALF_CLOSED:
            setDisplayBrightness(BRIGHTNESS_DIMMED);
            break;
        case STATE_SLEEPING:
            setDisplayBrightness(BRIGHTNESS_SLEEP);
            break;
        default:
            setDisplayBrightness(BRIGHTNESS_NORMAL);
            break;
    }
}

SystemState getSystemState() {
    return currentSystemState;
}

void setDisplayBrightness(uint8_t level) {
    currentBrightness = level;
    u8g2.setContrast(level);
    lastBrightnessChange = millis();
}

void resetInactivityTimer() {
    // فقط برای سازگاری
}

// ============================================================
//  توابع مربوط به حالت نیمه‌بسته
// ============================================================

void setHalfClosedMode(bool enabled) {
    halfClosedMode = enabled;
}

bool getHalfClosedMode() {
    return halfClosedMode;
}

// ============================================================
//  انیمیشن استارت‌آپ (با حروف ضخیم)
// ============================================================

void playStartupAnimation() {
    unsigned long duration = 5000;
    unsigned long startTime = millis();
    const float START_CHANGE = 0.33;

    float lA[4][2] = {{25,18}, {35,18}, {35,46}, {15,46}};
    float rA[4][2] = {{93,18}, {103,18}, {113,46}, {93,46}};
    float d[4][2] = {{54,18}, {68,24}, {68,40}, {54,46}};
    float targetEyeL[4][2] = {{12,8}, {54,8}, {54,56}, {12,56}};
    float targetEyeR[4][2] = {{74,8}, {116,8}, {116,56}, {74,56}};

    while (millis() - startTime < duration) {
        unsigned long elapsed = millis() - startTime;
        float t = (float)elapsed / duration;
        u8g2.clearBuffer();

        float transformT = 0;
        if (t >= START_CHANGE) {
            transformT = (t - START_CHANGE) / (1.0 - START_CHANGE);
            if (transformT > 1.0) transformT = 1.0;
        }

        float curL[4][2], curR[4][2];
        for (int i = 0; i < 4; i++) {
            curL[i][0] = lA[i][0] + transformT * (targetEyeL[i][0] - lA[i][0]);
            curL[i][1] = lA[i][1] + transformT * (targetEyeL[i][1] - lA[i][1]);
            curR[i][0] = rA[i][0] + transformT * (targetEyeR[i][0] - rA[i][0]);
            curR[i][1] = rA[i][1] + transformT * (targetEyeR[i][1] - rA[i][1]);
        }

        float dOffsetY = transformT * 60;
        float dScale = 1.0 - transformT * 0.8;
        if (dScale < 0.05) dScale = 0.05;
        float dCenterX = (d[0][0] + d[2][0]) / 2.0;
        float dCenterY = (d[0][1] + d[2][1]) / 2.0;
        float curD[4][2];
        for (int i = 0; i < 4; i++) {
            float dx = (d[i][0] - dCenterX) * dScale;
            float dy = (d[i][1] - dCenterY) * dScale;
            curD[i][0] = dCenterX + dx;
            curD[i][1] = dCenterY + dy + dOffsetY;
        }

        u8g2.drawTriangle(curL[0][0], curL[0][1],
                          curL[1][0], curL[1][1],
                          curL[2][0], curL[2][1]);
        u8g2.drawTriangle(curL[0][0], curL[0][1],
                          curL[2][0], curL[2][1],
                          curL[3][0], curL[3][1]);

        u8g2.drawTriangle(curR[0][0], curR[0][1],
                          curR[1][0], curR[1][1],
                          curR[2][0], curR[2][1]);
        u8g2.drawTriangle(curR[0][0], curR[0][1],
                          curR[2][0], curR[2][1],
                          curR[3][0], curR[3][1]);

        u8g2.drawTriangle(curD[0][0], curD[0][1],
                          curD[1][0], curD[1][1],
                          curD[2][0], curD[2][1]);
        u8g2.drawTriangle(curD[0][0], curD[0][1],
                          curD[2][0], curD[2][1],
                          curD[3][0], curD[3][1]);

        if (transformT > 0.85) {
            float p_t = (transformT - 0.85) / 0.15;
            int pupilRadius = (int)(p_t * 2);
            if (pupilRadius > 0) {
                u8g2.drawDisc(33, 32, pupilRadius);
                u8g2.drawDisc(95, 32, pupilRadius);
            }
        }

        u8g2.sendBuffer();
        delay(20);
    }

    currentFace = FACE_NORMAL;
}

// ============================================================
//  توابع خواندن باتری
// ============================================================

float readBatteryVoltage() {
    long sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += analogRead(BATTERY_PIN);
        delayMicroseconds(100);
    }
    int adcValue = sum / 10;
    return adcValue * BATTERY_FACTOR;
}

float getBatteryPercentage() {
    float voltage = readBatteryVoltage();
    float clamped = voltage;
    if (clamped < BATTERY_MIN_VOLTAGE) clamped = BATTERY_MIN_VOLTAGE;
    if (clamped > BATTERY_MAX_VOLTAGE) clamped = BATTERY_MAX_VOLTAGE;
    float percent = (clamped - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100.0;
    return constrain(percent, 0.0f, 100.0f);
}

void drawBatteryStatus() {
    u8g2.clearBuffer();

    int x = 24, y = 2, width = 80, height = 28, radius = 3;
    u8g2.setDrawColor(1);
    u8g2.drawRBox(x, y, width, height, radius);
    u8g2.setDrawColor(0);
    u8g2.drawRBox(x + 2, y + 2, width - 4, height - 4, radius - 1);
    u8g2.setDrawColor(1);
    int chargeWidth = (width - 6) * (batteryPercent / 100.0);
    if (chargeWidth > 0) {
        u8g2.drawBox(x + 3, y + 3, chargeWidth, height - 6);
    }

    int headX = x + width, headY = y + 5, headW = 6, headH = height - 10;
    u8g2.setDrawColor(1);
    u8g2.drawRBox(headX, headY, headW, headH, 2);

    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_helvB14_tf);
    char percentBuf[10], voltBuf[10];
    sprintf(percentBuf, "%d%%", (int)batteryPercent);
    sprintf(voltBuf, "%.2fV", batteryVoltage);
    int percentWidth = u8g2.getStrWidth(percentBuf);
    int voltWidth = u8g2.getStrWidth(voltBuf);
    int totalWidth = percentWidth + 6 + voltWidth;
    int startX = (128 - totalWidth) / 2;
    int textY = y + height + 18;
    u8g2.drawStr(startX, textY, percentBuf);
    u8g2.drawStr(startX + percentWidth + 6, textY, voltBuf);

    if (batteryVoltage < 0.5) {
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(10, 62, "NO BATTERY!");
    }

    u8g2.sendBuffer();
}

// ============================================================
//  توابع انیمیشن و کشیدن چشم‌ها
// ============================================================

void updateAnimationTimers() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastPupilStepTime >= 25) {
        lastPupilStepTime = currentTime;
        if (currentPupilX < targetPupilX) currentPupilX++;
        else if (currentPupilX > targetPupilX) currentPupilX--;
        if (currentPupilY < targetPupilY) currentPupilY++;
        else if (currentPupilY > targetPupilY) currentPupilY--;
    }

    if (currentTime - lastEatingTime >= 450) {
        lastEatingTime = currentTime;
        eatingToggle = !eatingToggle;
    }

    if (currentTime - lastRapidBlinkTime >= 120) {
        lastRapidBlinkTime = currentTime;
        rapidBlinkToggle = !rapidBlinkToggle;
    }

    if (currentTime - lastPupilMoveTime >= pupilMoveInterval) {
        lastPupilMoveTime = currentTime;
        pupilMoveInterval = random(1500, 4000);
        int lookDirection = random(0, 5);
        switch (lookDirection) {
            case 0: targetPupilX = 0;  targetPupilY = 0;  break;
            case 1: targetPupilX = -6; targetPupilY = 0;  break;
            case 2: targetPupilX = 6;  targetPupilY = 0;  break;
            case 3: targetPupilX = 0;  targetPupilY = -4; break;
            case 4: targetPupilX = 0;  targetPupilY = 4;  break;
        }
    }

    if (currentFace == FACE_NORMAL) {
        if (!isBlinking && (currentTime - lastBlinkTime >= blinkInterval)) {
            isBlinking = true;
            lastBlinkTime = currentTime;
        }
        if (isBlinking && (currentTime - lastBlinkTime >= blinkDuration)) {
            isBlinking = false;
            lastBlinkTime = currentTime;
            blinkInterval = random(2500, 6000);
        }
    }

    if (currentFace == FACE_SLEEPY) {
        if (currentTime - lastBreathingTime >= 350) {
            lastBreathingTime = currentTime;
            if (breathingDirection) {
                breathingOffset++;
                if (breathingOffset >= 3) breathingDirection = false;
            } else {
                breathingOffset--;
                if (breathingOffset <= 0) breathingDirection = true;
            }
        }
    }
}

void drawNormalEyes() {
    if (isBlinking) {
        u8g2.drawRBox(leftEyeX, eyeY + 20, eyeWidth, 8, 3);
        u8g2.drawRBox(rightEyeX, eyeY + 20, eyeWidth, 8, 3);
    } else {
        u8g2.drawRBox(leftEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
        u8g2.drawRBox(rightEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
        u8g2.setDrawColor(0);
        u8g2.drawDisc(33 + currentPupilX, 32 + currentPupilY, 8);
        u8g2.drawDisc(95 + currentPupilX, 32 + currentPupilY, 8);
        u8g2.setDrawColor(1);
        u8g2.drawDisc(30 + currentPupilX, 29 + currentPupilY, 2);
        u8g2.drawDisc(92 + currentPupilX, 29 + currentPupilY, 2);
    }
}

void drawHappyEyes() {
    u8g2.drawDisc(leftEyeX + 21, eyeY + 36, 22);
    u8g2.drawDisc(rightEyeX + 21, eyeY + 36, 22);
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, eyeY + 28, 128, 36);
    u8g2.drawDisc(leftEyeX + 21, eyeY + 41, 18);
    u8g2.drawDisc(rightEyeX + 21, eyeY + 41, 18);
    u8g2.setDrawColor(1);
}

void drawAngryEyes() {
    u8g2.drawRBox(leftEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
    u8g2.drawRBox(rightEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(36 + currentPupilX, 36 + currentPupilY, 8);
    u8g2.drawDisc(92 + currentPupilX, 36 + currentPupilY, 8);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(33 + currentPupilX, 33 + currentPupilY, 2);
    u8g2.drawDisc(89 + currentPupilX, 33 + currentPupilY, 2);
    u8g2.setDrawColor(0);
    u8g2.drawTriangle(leftEyeX - 2, eyeY - 2, leftEyeX + eyeWidth + 2, eyeY - 2, leftEyeX + eyeWidth + 2, eyeY + 18);
    u8g2.drawTriangle(rightEyeX + eyeWidth + 2, eyeY - 2, rightEyeX - 2, eyeY - 2, rightEyeX - 2, eyeY + 18);
    u8g2.setDrawColor(1);
}

void drawThinkingEyes() {
    u8g2.drawRBox(leftEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(41 + currentPupilX, 22 + currentPupilY, 7);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(39 + currentPupilX, 20 + currentPupilY, 2);
    u8g2.drawRBox(rightEyeX, eyeY + 12, eyeWidth, eyeHeight - 16, 6);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(85 + currentPupilX, 34 + currentPupilY, 6);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(83 + currentPupilX, 32 + currentPupilY, 1);
    u8g2.drawLine(rightEyeX - 4, eyeY + 2, rightEyeX + eyeWidth, eyeY + 6);
}

void drawSleepingEyes() {
    if (halfClosedMode) {
        // حالت نیمه‌بسته: چشم‌ها به شکل مستطیل افقی باریک با مردمک کوچک
        int halfHeight = 10;
        int yOffset = eyeY + (eyeHeight - halfHeight) / 2;
        
        u8g2.drawRBox(leftEyeX, yOffset, eyeWidth, halfHeight, 4);
        u8g2.drawRBox(rightEyeX, yOffset, eyeWidth, halfHeight, 4);
        
        u8g2.setDrawColor(0);
        u8g2.drawDisc(33 + currentPupilX, 32 + currentPupilY, 4);
        u8g2.drawDisc(95 + currentPupilX, 32 + currentPupilY, 4);
        u8g2.setDrawColor(1);
        u8g2.drawDisc(31 + currentPupilX, 30 + currentPupilY, 1);
        u8g2.drawDisc(93 + currentPupilX, 30 + currentPupilY, 1);
    } else {
        // خواب کامل
        u8g2.drawRBox(leftEyeX, eyeY + 24 + breathingOffset, eyeWidth, 6, 2);
        u8g2.drawRBox(rightEyeX, eyeY + 24 + breathingOffset, eyeWidth, 6, 2);
        u8g2.setFont(u8g2_font_6x12_tf);
        if (breathingOffset == 0)      u8g2.drawStr(108, 16, "z");
        else if (breathingOffset == 1) u8g2.drawStr(108, 16, "zZ");
        else                           u8g2.drawStr(108, 16, "zZz");
    }
}

void drawSurprisedEyes() {
    u8g2.drawRBox(leftEyeX, eyeY - 2, eyeWidth, eyeHeight + 4, 20);
    u8g2.drawRBox(rightEyeX, eyeY - 2, eyeWidth, eyeHeight + 4, 20);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(33 + currentPupilX, 32 + currentPupilY, 14);
    u8g2.drawDisc(95 + currentPupilX, 32 + currentPupilY, 14);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(30 + currentPupilX, 29 + currentPupilY, 3);
    u8g2.drawDisc(92 + currentPupilX, 29 + currentPupilY, 3);
}

void drawSadEyes() {
    u8g2.drawRBox(leftEyeX, eyeY + 4, eyeWidth, eyeHeight - 4, eyeRadius);
    u8g2.drawRBox(rightEyeX, eyeY + 4, eyeWidth, eyeHeight - 4, eyeRadius);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(30 + currentPupilX, 38 + currentPupilY, 7);
    u8g2.drawDisc(98 + currentPupilX, 38 + currentPupilY, 7);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(28 + currentPupilX, 36 + currentPupilY, 2);
    u8g2.drawDisc(96 + currentPupilX, 36 + currentPupilY, 2);
    u8g2.setDrawColor(0);
    u8g2.drawTriangle(leftEyeX - 2, eyeY - 2, leftEyeX + eyeWidth + 2, eyeY - 2, leftEyeX - 2, eyeY + 20);
    u8g2.drawTriangle(rightEyeX + eyeWidth + 2, eyeY - 2, rightEyeX - 2, eyeY - 2, rightEyeX + eyeWidth + 2, eyeY + 20);
    u8g2.setDrawColor(1);
}

void drawWinkEyes() {
    u8g2.drawRBox(leftEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(33 + currentPupilX, 32 + currentPupilY, 8);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(30 + currentPupilX, 29 + currentPupilY, 2);
    u8g2.drawDisc(rightEyeX + 21, eyeY + 32, 20);
    u8g2.setDrawColor(0);
    u8g2.drawBox(rightEyeX - 2, eyeY + 24, eyeWidth + 4, 30);
    u8g2.drawDisc(rightEyeX + 21, eyeY + 36, 16);
    u8g2.setDrawColor(1);
}

void drawFearEyes() {
    int shakeX = random(-1, 2);
    int shakeY = random(-1, 2);
    u8g2.drawRBox(leftEyeX + shakeX, eyeY + shakeY, eyeWidth, eyeHeight, 16);
    u8g2.drawRBox(rightEyeX + shakeX, eyeY + shakeY, eyeWidth, eyeHeight, 16);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(33 + shakeX + currentPupilX, 32 + shakeY + currentPupilY, 4);
    u8g2.drawDisc(95 + shakeX + currentPupilX, 32 + shakeY + currentPupilY, 4);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(31 + shakeX + currentPupilX, 30 + shakeY + currentPupilY, 1);
    u8g2.drawDisc(93 + shakeX + currentPupilX, 30 + shakeY + currentPupilY, 1);
}

void drawEcstasyEyes() {
    for(int i = -2; i <= 2; i++) {
        u8g2.drawLine(16, 16 + i, 46, 32 + i);
        u8g2.drawLine(16, 48 + i, 46, 32 + i);
        u8g2.drawLine(112, 16 + i, 82, 32 + i);
        u8g2.drawLine(112, 48 + i, 82, 32 + i);
    }
}

void drawEatingEyes() {
    if (eatingToggle) {
        u8g2.drawRBox(leftEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
        u8g2.drawRBox(rightEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
        u8g2.setDrawColor(0);
        u8g2.drawDisc(33 + currentPupilX, 42 + currentPupilY, 7);
        u8g2.drawDisc(95 + currentPupilX, 42 + currentPupilY, 7);
        u8g2.setDrawColor(1);
        u8g2.drawDisc(31 + currentPupilX, 40 + currentPupilY, 2);
        u8g2.drawDisc(93 + currentPupilX, 40 + currentPupilY, 2);
    } else {
        u8g2.drawRBox(leftEyeX, eyeY + 8, eyeWidth, eyeHeight - 12, 8);
        u8g2.drawRBox(rightEyeX, eyeY + 8, eyeWidth, eyeHeight - 12, 8);
        u8g2.setDrawColor(0);
        u8g2.drawDisc(33 + currentPupilX, 40 + currentPupilY, 6);
        u8g2.drawDisc(95 + currentPupilX, 40 + currentPupilY, 6);
        u8g2.setDrawColor(1);
        u8g2.drawDisc(31 + currentPupilX, 38 + currentPupilY, 1);
        u8g2.drawDisc(93 + currentPupilX, 38 + currentPupilY, 1);
    }
}

void drawRapidBlinkEyes() {
    if (rapidBlinkToggle) {
        u8g2.drawRBox(leftEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
        u8g2.drawRBox(rightEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
        u8g2.setDrawColor(0);
        u8g2.drawDisc(33 + currentPupilX, 32 + currentPupilY, 8);
        u8g2.drawDisc(95 + currentPupilX, 32 + currentPupilY, 8);
        u8g2.setDrawColor(1);
        u8g2.drawDisc(30 + currentPupilX, 29 + currentPupilY, 2);
        u8g2.drawDisc(92 + currentPupilX, 29 + currentPupilY, 2);
    } else {
        u8g2.drawRBox(leftEyeX, eyeY + 22, eyeWidth, 6, 2);
        u8g2.drawRBox(rightEyeX, eyeY + 22, eyeWidth, 6, 2);
    }
}

void drawSickEyes() {
    for(int i = -2; i <= 2; i++) {
        u8g2.drawLine(18 + i, 18, 44 + i, 46);
        u8g2.drawLine(18 + i, 46, 44 + i, 18);
        u8g2.drawLine(84 + i, 18, 110 + i, 46);
        u8g2.drawLine(84 + i, 46, 110 + i, 18);
    }
}

void drawHungryEyes() {
    u8g2.drawRBox(leftEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
    u8g2.drawRBox(rightEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, 0, 128, 20);
    u8g2.setDrawColor(1);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(36 + currentPupilX, 42 + currentPupilY, 7);
    u8g2.drawDisc(92 + currentPupilX, 42 + currentPupilY, 7);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(34 + currentPupilX, 40 + currentPupilY, 2);
    u8g2.drawDisc(90 + currentPupilX, 40 + currentPupilY, 2);
}

void drawXOEyes() {
    for(int i = -2; i <= 2; i++) {
        u8g2.drawLine(18 + i, 20, 44 + i, 44);
        u8g2.drawLine(18 + i, 44, 44 + i, 20);
    }
    u8g2.drawRBox(rightEyeX, eyeY, eyeWidth, eyeHeight, eyeRadius);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(95 + currentPupilX, 32 + currentPupilY, 8);
    u8g2.setDrawColor(1);
    u8g2.drawDisc(92 + currentPupilX, 29 + currentPupilY, 2);
}

