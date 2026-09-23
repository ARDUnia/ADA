/*
 * Ada.ino - نسخه پایدار با مدیریت خواب، تنظیمات و واکنش ۳ ثانیه‌ای
 * بهینه‌سازی شده برای سرعت پاسخ‌دهی
 */

#include "Config.h"
#include "Version.h"
#include "Touch.h"
#include "Display.h"
#include "Audio.h"
#include "Head.h"
#include "Setup.h"
#include "internet.h"

// ============================================================
//  تنظیمات اصلی
// ============================================================
#define FACE_DIAGNOSTIC_MODE 0
#define REACTION_DURATION 3000   // ۳ ثانیه

String getFaceName(FaceState face);

// ============================================================
//  تعریف متغیرهای قابل تنظیم (مقادیر پیش‌فرض)
// ============================================================
uint32_t INACTIVITY_DIM_START = 30000;      // ۳۰ ثانیه
uint32_t INACTIVITY_HALF_CLOSE = 60000;     // ۱ دقیقه
uint32_t INACTIVITY_SLEEP_START = 300000;   // ۵ دقیقه
uint8_t AUDIO_VOLUME = 25;                  // ۰ تا ۳۰

// ============================================================
//  متغیرهای عمومی سیستم
// ============================================================
FaceState currentFace = FACE_NORMAL;
RobotMode currentMode = MODE_INTERACTIVE;
bool isNetworkConnected = false;
SystemState currentSystemState = STATE_ACTIVE;

static unsigned long reactionStartTime = 0;
static bool inReaction = false;
static FaceState lastFace = FACE_NORMAL;
static unsigned long lastActivityTime = 0;
static uint32_t lastGestureEventCounter = 0;

// متغیرهای نمایش باتری
static unsigned long batteryDisplayStartTime = 0;
static bool batteryDisplayActive = false;
const unsigned long BATTERY_DISPLAY_DURATION = 3000;

// متغیر برای نگهداری چهره قبلی (برای بازگشت از تنظیمات)
FaceState previousFace = FACE_NORMAL;

#if FACE_DIAGNOSTIC_MODE == 1
unsigned long lastFaceChangeTime = 0;
const unsigned long FACE_DEMO_INTERVAL = 3000;
int demoFaceIndex = 0;
#endif

// ============================================================
//  setup
// ============================================================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(500);

    Serial.println("=====================================");
    Serial.print("           ADA ROBOT v");
    Serial.println(ADA_VERSION);
    Serial.println("=====================================");

    initDisplay();
    initTouch();
    initHead();
    initSetup();       // Load persisted settings before initializing audio.
    initAudio();
    initInternet();
    
    delay(200);
    playStartupSound();
    playStartupAnimation();

    currentSystemState = STATE_ACTIVE;
    setSystemState(STATE_ACTIVE);
    lastActivityTime = millis();

    Serial.print("Reaction duration: ");
    Serial.print(REACTION_DURATION / 1000);
    Serial.println(" seconds, then back to NORMAL.");
    Serial.println("Auto-sleep features active:");
    Serial.println("  - 30s: 50% brightness");
    Serial.println("  - 1min: Half-closed eyes, 50% brightness");
    Serial.println("  - 5min: Sleeping, 25% brightness");
}

// ============================================================
//  loop
// ============================================================
void loop() {
    unsigned long currentTime = millis();

    // ============================================================
    //  ۰. اگر حالت تنظیمات فعال است، فقط Setup را اجرا کن
    // ============================================================
    if (getConfigMode()) {
        updateSetup();
        // بدون delay - سریع‌ترین اجرا
        return;
    }

    // ============================================================
    //  ۱. پردازش لمس و تشخیص فعالیت (اولویت اول)
    // ============================================================
    lastFace = currentFace;
    const bool timeWasActiveBeforeTouch = isTimeDisplayActive();
    updateTouch();  // این تابع سریعاً اجرا می‌شود
    uint32_t gestureEventCounter = getGestureEventCounter();
    bool newGestureEvent = gestureEventCounter != lastGestureEventCounter;
    if (newGestureEvent) lastGestureEventCounter = gestureEventCounter;
    
    // اگر نمایش زمان فعال است، با هر لمس آن را خاموش کن
    if (timeWasActiveBeforeTouch && isTouchActivityDetected()) {
        deactivateTimeDisplay();
    }
    
    if (isTouchActivityDetected()) {
        lastActivityTime = currentTime;
        clearTouchActivityFlag();
        if (currentSystemState != STATE_ACTIVE) {
            currentSystemState = STATE_ACTIVE;
            setSystemState(STATE_ACTIVE);
            if (!inReaction && !getShowBattery() && !getConfigMode()) {
                currentFace = FACE_NORMAL;
            }
            Serial.println("Activity detected! Returning to ACTIVE state.");
        }
    }

    // ============================================================
    //  ۲. مدیریت اینترنت (غیرمسدود)
    // ============================================================
    handleInternet();

    // ============================================================
    //  ۳. مدیریت واکنش‌های عادی (تغییر چهره بر اثر لمس)
    // ============================================================
    if ((currentFace != lastFace || newGestureEvent) && currentFace != FACE_NORMAL) {
        reactionStartTime = currentTime;
        inReaction = true;
        lastActivityTime = currentTime;
        Serial.print("Reaction: ");
        Serial.println(getFaceName(currentFace));
    }

    // بازگشت به حالت عادی پس از REACTION_DURATION (۳ ثانیه)
    if (inReaction && (currentTime - reactionStartTime >= REACTION_DURATION)) {
        if (!getShowBattery() && !getConfigMode()) {
            currentFace = FACE_NORMAL;
            setHeadSpecialMotion(0);
        }
        inReaction = false;
        Serial.println("Returned to NORMAL");
    }

    // ============================================================
    //  ۴. مدیریت نمایش باتری
    // ============================================================
    if (getShowBattery()) {
        if (!batteryDisplayActive) {
            batteryDisplayActive = true;
            batteryDisplayStartTime = currentTime;
            Serial.println("Battery display started");
        }
        if (currentTime - batteryDisplayStartTime >= BATTERY_DISPLAY_DURATION) {
            setShowBattery(false);
            batteryDisplayActive = false;
            Serial.println("Battery display off");
            lastActivityTime = currentTime;
        }
    } else {
        if (batteryDisplayActive) {
            batteryDisplayActive = false;
        }
    }

    // ============================================================
    //  ۵. مدیریت خواب خودکار (فقط در صورت عدم فعالیت)
    // ============================================================
    unsigned long inactivityTime = currentTime - lastActivityTime;

    if (currentSystemState != STATE_ACTIVE || inactivityTime > INACTIVITY_DIM_START) {
        SystemState newState = currentSystemState;

        if (inactivityTime >= INACTIVITY_SLEEP_START) {
            newState = STATE_SLEEPING;
        } else if (inactivityTime >= INACTIVITY_HALF_CLOSE) {
            newState = STATE_HALF_CLOSED;
        } else if (inactivityTime >= INACTIVITY_DIM_START) {
            newState = STATE_DIMMED;
        }

        if (newState != currentSystemState) {
            currentSystemState = newState;
            setSystemState(newState);
            Serial.print("System state changed to: ");
            switch (newState) {
                case STATE_DIMMED:      Serial.println("DIMMED (50%)"); break;
                case STATE_HALF_CLOSED: Serial.println("HALF_CLOSED"); break;
                case STATE_SLEEPING:    Serial.println("SLEEPING"); break;
                default: break;
            }
        }
    }

    // ============================================================
    //  ۶. حالت دیاگنوستیک (اختیاری)
    // ============================================================
    #if FACE_DIAGNOSTIC_MODE == 1
    {
        if (currentTime - lastFaceChangeTime >= FACE_DEMO_INTERVAL) {
            lastFaceChangeTime = currentTime;
            switch (demoFaceIndex) {
                case 0:  currentFace = FACE_NORMAL;      break;
                case 1:  currentFace = FACE_HAPPY;       break;
                case 2:  currentFace = FACE_ANGRY;       break;
                case 3:  currentFace = FACE_THINKING;    break;
                case 4:  currentFace = FACE_SLEEPY;      break;
                case 5:  currentFace = FACE_SURPRISED;   break;
                case 6:  currentFace = FACE_SAD;         break;
                case 7:  currentFace = FACE_WINK;        break;
                case 8:  currentFace = FACE_FEAR;        break;
                case 9:  currentFace = FACE_ECSTASY;     break;
                case 10: currentFace = FACE_EATING;      break;
                case 11: currentFace = FACE_RAPID_BLINK; break;
                case 12: currentFace = FACE_SICK;        break;
                case 13: currentFace = FACE_HUNGRY;      break;
                case 14: currentFace = FACE_XO;          break;
            }
            demoFaceIndex++;
            if (demoFaceIndex > 14) demoFaceIndex = 0;
        }
    }
    #endif

    // ============================================================
    //  ۷. بروزرسانی خروجی‌ها (صدا، حرکت، نمایشگر)
    // ============================================================
    updateAudio();
    updateHead();
    updateDisplay();

    // ============================================================
    //  حذف delay(5) برای افزایش سرعت پاسخ‌دهی
    // ============================================================
    // دیگر هیچ delay در حلقه اصلی وجود ندارد
}

// ============================================================
//  تابع کمکی برای نمایش نام چهره
// ============================================================
String getFaceName(FaceState face) {
    switch (face) {
        case FACE_NORMAL: return "NORMAL";
        case FACE_HAPPY: return "HAPPY";
        case FACE_ANGRY: return "ANGRY";
        case FACE_THINKING: return "THINKING";
        case FACE_SLEEPY: return "SLEEPY";
        case FACE_SURPRISED: return "SURPRISED";
        case FACE_SAD: return "SAD";
        case FACE_WINK: return "WINK";
        case FACE_FEAR: return "FEAR";
        case FACE_ECSTASY: return "ECSTASY";
        case FACE_EATING: return "EATING";
        case FACE_RAPID_BLINK: return "RAPID BLINK";
        case FACE_SICK: return "SICK";
        case FACE_HUNGRY: return "HUNGRY";
        case FACE_XO: return "XO STATE";
        default: return "UNKNOWN";
    }
}

