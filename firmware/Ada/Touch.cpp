/*
 * Touch.cpp - مدیریت سنسورهای لمسی با تشخیص الگوهای جدید
 * نویسنده: حمیدرضا میلانی نیا (ARDUnia)
 * نسخه: ۳.۵ (قفل لمس طولانی و همگام‌سازی خروج از منو)
 */

#include "Touch.h"
#include "Display.h"   // برای showInternetTime()
#include "Head.h"      // برای setHeadSpecialMotion و ثابت‌های حرکتی
#include <Wire.h>

// ============================================================
//  اعلان متغیرهای خارجی (تعریف شده در Ada.ino)
// ============================================================
extern FaceState previousFace;

// ============================================================
//  پرتو تایپ توابع کمکی
// ============================================================
void resetStates();
void processGesture(String gesture);

// ============================================================
//  متغیرهای وضعیت سنسورها
// ============================================================
static unsigned long touchStartTime[4] = {0, 0, 0, 0};
static int tapCount[4] = {0, 0, 0, 0};
static bool isTouched[4] = {false, false, false, false};
static bool wasTouched[4] = {false, false, false, false};
static String onSequence = "";
static unsigned long globalGestureTimer = 0;
static unsigned long longTouchStart[4] = {0, 0, 0, 0};
static bool longTouchTriggered[4] = {false, false, false, false};

// ============================================================
//  پرچم فعالیت (برای تشخیص لمس)
// ============================================================
static bool touchActivityFlag = false;
static bool suppressUntilRelease = false;
static uint32_t gestureEventCounter = 0;

// ============================================================
//  ثابت‌های زمانی (افزایش یافته)
// ============================================================
const unsigned long SHORT_TOUCH_MAX = 700;    // افزایش از ۵۰۰ به ۷۰۰
const unsigned long LONG_TOUCH_MIN = 1000;
const unsigned long MULTI_TAP_GAP = 600;      // افزایش از ۴۰۰ به ۶۰۰

// ============================================================
//  متغیرهای حالت‌های خاص (باتری و کانفیگ)
// ============================================================
static bool showBattery = false;
static bool configMode = false;

// ============================================================
//  توابع عمومی (تعریف‌شده در هدر)
// ============================================================

void initTouch() {
    Wire.begin(4, 5); // SDA=GPIO4, SCL=GPIO5
    Wire.beginTransmission(PCF8574_ADDRESS);
    Wire.write(0xFF);
    Wire.endTransmission();
    delay(50);
    
    Serial.println("Touch Module Initialized (PCF8574 at 0x20)");
}

void updateTouch() {
    unsigned long currentMillis = millis();
    int activeTouches = 0;
    String result = "";

    Wire.requestFrom(PCF8574_ADDRESS, 1);
    if (!Wire.available()) return;
    uint8_t data = Wire.read();

    bool currentState[4];
    currentState[0] = (data & 0x01) ? true : false;
    currentState[1] = (data & 0x02) ? true : false;
    currentState[2] = (data & 0x04) ? true : false;
    currentState[3] = (data & 0x08) ? true : false;

    for (int i = 0; i < 4; i++) {
        isTouched[i] = currentState[i];
        if (isTouched[i]) activeTouches++;
    }

    // Prevent the key used to exit Setup from becoming a normal gesture.
    if (suppressUntilRelease) {
        for (int i = 0; i < 4; i++) wasTouched[i] = isTouched[i];
        if (activeTouches == 0) suppressUntilRelease = false;
        return;
    }

    for (int i = 0; i < 4; i++) {
        if (isTouched[i] && !wasTouched[i]) {
            // Register activity immediately; gesture recognition may finish later.
            touchActivityFlag = true;
            touchStartTime[i] = currentMillis;
            longTouchStart[i] = currentMillis;
            longTouchTriggered[i] = false;
            globalGestureTimer = currentMillis;
            
            char pinChar = '1' + i;
            if (onSequence.length() == 0 || onSequence.charAt(onSequence.length() - 1) != pinChar) {
                onSequence += pinChar;
            }
        }

        if (!isTouched[i] && wasTouched[i]) {
            unsigned long touchDuration = currentMillis - touchStartTime[i];
            if (touchDuration < SHORT_TOUCH_MAX) {
                tapCount[i]++;
            }
            longTouchTriggered[i] = false;
        }
        wasTouched[i] = isTouched[i];
    }

    for (int i = 0; i < 4; i++) {
        if (isTouched[i] && !longTouchTriggered[i]) {
            if (currentMillis - longTouchStart[i] >= LONG_TOUCH_MIN) {
                longTouchTriggered[i] = true;
                result = "LT" + String(i + 1);
                processGesture(result);
                gestureEventCounter++;
                resetStates();
                touchActivityFlag = true;
                suppressUntilRelease = true;
                return;
            }
        }
    }

    if (activeTouches == 0 && onSequence.length() > 0 && (currentMillis - globalGestureTimer > MULTI_TAP_GAP)) {
        bool isL2R = false;
        bool isR2L = false;
        int targetL2R = 1;
        for (unsigned int i = 0; i < onSequence.length(); i++) {
            if (onSequence.charAt(i) == '0' + targetL2R) targetL2R++;
        }
        if (targetL2R > 4) isL2R = true;

        int targetR2L = 4;
        for (unsigned int i = 0; i < onSequence.length(); i++) {
            if (onSequence.charAt(i) == '0' + targetR2L) targetR2L--;
        }
        if (targetR2L < 1) isR2L = true;

        bool isRLLR = (isL2R && isR2L);

        unsigned long timeDiff = 0;
        if (touchStartTime[0] > touchStartTime[3]) {
            timeDiff = touchStartTime[0] - touchStartTime[3];
        } else {
            timeDiff = touchStartTime[3] - touchStartTime[0];
        }

        if (timeDiff > 60) {
            if (isRLLR) result = "RLLR";
            else if (isL2R) result = "L2R";
            else if (isR2L) result = "R2L";
        }

        if (result == "") {
            int multiFingerCount = 0;
            int maxTaps = 0;
            int lastTappedPin = -1;
            for (int i = 0; i < 4; i++) {
                if (tapCount[i] > 0) {
                    multiFingerCount++;
                    lastTappedPin = i + 1;
                    if (tapCount[i] > maxTaps) maxTaps = tapCount[i];
                }
            }

            if (multiFingerCount == 1) {
                result = String(maxTaps) + "TS" + String(lastTappedPin);
            } else if (multiFingerCount > 1) {
                result = String(maxTaps) + "T" + String(multiFingerCount) + "F";
            }
        }

        // ============================================================
        //  دیباگ: چاپ نتیجه تشخیص داده شده
        // ============================================================
        if (result != "") {
            Serial.print("Gesture detected: ");
            Serial.println(result);
        }

        if (result != "") {
            processGesture(result);
            gestureEventCounter++;
            touchActivityFlag = true;
        }

        resetStates();
    }
}

// ============================================================
//  توابع مدیریت حالت‌های خاص
// ============================================================

void setShowBattery(bool state) { showBattery = state; }
bool getShowBattery() { return showBattery; }
void setConfigMode(bool state) { configMode = state; }
bool getConfigMode() { return configMode; }

// ============================================================
//  توابع مدیریت پرچم فعالیت
// ============================================================

bool isTouchActivityDetected() {
    return touchActivityFlag;
}

void clearTouchActivityFlag() {
    touchActivityFlag = false;
}

void suspendTouchUntilRelease() {
    suppressUntilRelease = true;
    touchActivityFlag = false;
    resetStates();
}

uint32_t getGestureEventCounter() {
    return gestureEventCounter;
}

// ============================================================
//  توابع کمکی داخلی
// ============================================================

void resetStates() {
    for (int i = 0; i < 4; i++) {
        tapCount[i] = 0;
    }
    onSequence = "";
    globalGestureTimer = 0;
}

void processGesture(String gesture) {
    Serial.print("Processing gesture: ");
    Serial.println(gesture);

    if (gesture != "L2R" && gesture != "R2L") {
        setHeadSpecialMotion(0);
    }

    if (gesture == "L2R") {
        currentFace = FACE_NORMAL;
        setHeadSpecialMotion(HEAD_MOTION_SLOW_RIGHT);
    } 
    else if (gesture == "R2L") {
        currentFace = FACE_NORMAL;
        setHeadSpecialMotion(HEAD_MOTION_SLOW_LEFT);
    }
    else if (gesture == "RLLR") {
        currentFace = FACE_ECSTASY;
    }
    else if (gesture == "1TS1") {
        currentFace = FACE_HAPPY;
    }
    else if (gesture == "3TS1") {
        currentFace = FACE_ANGRY;
    }
    else if (gesture == "LT1") {
        currentFace = FACE_THINKING;
    }
    else if (gesture == "LT2") {
        currentFace = FACE_SLEEPY;
    }
    else if (gesture == "2TS3") {
        currentFace = FACE_SURPRISED;
    }
    else if (gesture == "3TS3") {
        currentFace = FACE_SAD;
    }
    else if (gesture == "2TS1") {
        currentFace = FACE_WINK;
    }
    else if (gesture == "3TS2") {
        currentFace = FACE_FEAR;
    }
    else if (gesture == "LT3") {
        currentFace = FACE_EATING;
    }
    else if (gesture == "1TS2") {
        currentFace = FACE_RAPID_BLINK;
    }
    else if (gesture == "1TS3") {
        currentFace = FACE_SICK;
    }
    else if (gesture == "2TS2") {
        currentFace = FACE_HUNGRY;
    }
    else if (gesture == "1TS4") {
        currentFace = FACE_XO;
    }
    else if (gesture == "2TS4") {
        // دو بار لمس سنسور 4 -> نمایش ساعت اینترنتی
        Serial.println("Double tap on sensor 4 detected! Showing time.");
        showInternetTime();
    }
    else if (gesture == "LT4") {
        currentFace = FACE_NORMAL;
        setShowBattery(true);
        Serial.println("Battery display activated");
    }
    else if (gesture == "3TS4") {
        previousFace = currentFace;
        currentFace = FACE_NORMAL;
        setConfigMode(true);
        Serial.println("Setup mode activated");
    }
    else {
        Serial.println("Unknown gesture, ignored.");
    }
}
