/*
 * Head.cpp - مدیریت حرکت سر با حرکات پویا
 * نویسنده: حمیدرضا میلانی نیا (ARDUnia)
 * نسخه: ۲.۱ (اصلاح ترتیب توابع)
 */

#include "Head.h"
#include <Servo.h>

// ============================================================
//  پرتوتایپ توابع کمکی (برای جلوگیری از خطای کامپایلر)
// ============================================================
void initMotionForFace(FaceState face);
void updateDynamicMotion(unsigned long now);
void updateShakeMotion(unsigned long now);
void updateSwingMotion(unsigned long now);
void updateSequenceMotion(unsigned long now);
void updateSpecialMotion(unsigned long now);

// ============================================================
//  متغیرهای سراسری ماژول
// ============================================================
Servo headServo;
int currentAngle = 90;
int targetAngle = 90;
unsigned long lastMoveTime = 0;
const int MOVE_STEP = 1;
const unsigned long MOVE_INTERVAL = 12;

enum MotionState {
    MOTION_IDLE,
    MOTION_SHAKE,
    MOTION_SWING,
    MOTION_SEQUENCE
};

struct DynamicMotion {
    MotionState state;
    int step;
    int phase;
    unsigned long phaseStartTime;
    int startAngle;
    int endAngle;
    int duration;
    int repeatCount;
    int maxRepeats;
};

static DynamicMotion motion = {MOTION_IDLE, 0, 0, 0, 90, 90, 0, 0, 0};
static FaceState lastFace = FACE_NORMAL;
static int specialMotion = 0; // برای حرکات ویژه سوایپ
static int specialPhase = 0;
static unsigned long specialStart = 0;

// ============================================================
//  توابع اصلی (در هدر اعلان شده‌اند)
// ============================================================

void initHead() {
    headServo.attach(SERVO_PIN, 500, 2400);
    headServo.write(90);
    currentAngle = 90;
    targetAngle = 90;
    lastMoveTime = millis();
    Serial.println("Head Servo Initialized");
}

void updateHead() {
    unsigned long now = millis();

    // اگر حرکت ویژه (سوایپ) فعال است، آن را اجرا کن
    if (specialMotion != 0) {
        updateSpecialMotion(now);
        return;
    }

    // حرکات داینامیک بر اساس currentFace
    if (currentFace != lastFace) {
        lastFace = currentFace;
        motion.state = MOTION_IDLE;
        motion.step = 0;
        motion.phase = 0;
        motion.repeatCount = 0;
        initMotionForFace(currentFace);
    }

    // اجرای حرکت داینامیک
    updateDynamicMotion(now);

    // حرکت نرم به سمت هدف (برای حالت‌های ساکن)
    if (motion.state == MOTION_IDLE && now - lastMoveTime >= MOVE_INTERVAL) {
        lastMoveTime = now;
        if (currentAngle < targetAngle) {
            currentAngle += MOVE_STEP;
            if (currentAngle > targetAngle) currentAngle = targetAngle;
        } else if (currentAngle > targetAngle) {
            currentAngle -= MOVE_STEP;
            if (currentAngle < targetAngle) currentAngle = targetAngle;
        }
        headServo.write(currentAngle);
    }
}

void setHeadSpecialMotion(int motionType) {
    specialMotion = motionType;
    specialPhase = 0;
    specialStart = 0;
    if (motionType != 0) {
        // قطع حرکت داینامیک
        motion.state = MOTION_IDLE;
    }
}

// ============================================================
//  توابع کمکی (پیاده‌سازی)
// ============================================================

void initMotionForFace(FaceState face) {
    switch (face) {
        case FACE_HAPPY:
            motion.state = MOTION_SWING;
            motion.startAngle = 90;
            motion.endAngle = 70;
            motion.duration = 200;
            motion.maxRepeats = 2;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_ANGRY:
            motion.state = MOTION_SHAKE;
            motion.startAngle = 90;
            motion.endAngle = 60;
            motion.duration = 300;
            motion.maxRepeats = 1;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_THINKING:
            motion.state = MOTION_SHAKE;
            motion.startAngle = 90;
            motion.endAngle = 120;
            motion.duration = 800;
            motion.maxRepeats = 1;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_SLEEPY:
            motion.state = MOTION_SWING;
            motion.startAngle = 90;
            motion.endAngle = 80;
            motion.duration = 1000;
            motion.maxRepeats = 3;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_SURPRISED:
            motion.state = MOTION_SHAKE;
            motion.startAngle = 90;
            motion.endAngle = 85;
            motion.duration = 100;
            motion.maxRepeats = 10;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_SAD:
            motion.state = MOTION_SHAKE;
            motion.startAngle = 90;
            motion.endAngle = 135;
            motion.duration = 500;
            motion.maxRepeats = 1;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_WINK:
            motion.state = MOTION_SWING;
            motion.startAngle = 90;
            motion.endAngle = 75;
            motion.duration = 150;
            motion.maxRepeats = 3;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_FEAR:
            motion.state = MOTION_SHAKE;
            motion.startAngle = 90;
            motion.endAngle = 80;
            motion.duration = 30;
            motion.maxRepeats = 30;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_ECSTASY:
            motion.state = MOTION_SWING;
            motion.startAngle = 90;
            motion.endAngle = 60;
            motion.duration = 150;
            motion.maxRepeats = 4;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_EATING:
            motion.state = MOTION_SEQUENCE;
            motion.startAngle = 90;
            motion.endAngle = 60;
            motion.duration = 400;
            motion.maxRepeats = 1;
            motion.repeatCount = 0;
            motion.phase = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_RAPID_BLINK:
            motion.state = MOTION_SEQUENCE;
            motion.startAngle = 90;
            motion.endAngle = 120;
            motion.duration = 300;
            motion.maxRepeats = 1;
            motion.repeatCount = 0;
            motion.phase = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_SICK:
            motion.state = MOTION_SHAKE;
            motion.startAngle = 90;
            motion.endAngle = 120;
            motion.duration = 200;
            motion.maxRepeats = 1;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_HUNGRY:
            motion.state = MOTION_SWING;
            motion.startAngle = 90;
            motion.endAngle = 65;
            motion.duration = 150;
            motion.maxRepeats = 5;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        case FACE_XO:
            motion.state = MOTION_SHAKE;
            motion.startAngle = 90;
            motion.endAngle = 120;
            motion.duration = 800;
            motion.maxRepeats = 1;
            motion.repeatCount = 0;
            motion.phaseStartTime = millis();
            break;
        default:
            motion.state = MOTION_IDLE;
            targetAngle = 90;
            break;
    }
}

void updateDynamicMotion(unsigned long now) {
    if (motion.state == MOTION_IDLE) return;

    switch (motion.state) {
        case MOTION_SHAKE:
            updateShakeMotion(now);
            break;
        case MOTION_SWING:
            updateSwingMotion(now);
            break;
        case MOTION_SEQUENCE:
            updateSequenceMotion(now);
            break;
        default:
            break;
    }
}

void updateShakeMotion(unsigned long now) {
    if (motion.repeatCount >= motion.maxRepeats) {
        motion.state = MOTION_IDLE;
        targetAngle = 90;
        return;
    }

    if (now - motion.phaseStartTime < (unsigned long)motion.duration) {
        float progress = (float)(now - motion.phaseStartTime) / motion.duration;
        int angle = motion.startAngle + (motion.endAngle - motion.startAngle) * progress;
        headServo.write(angle);
        currentAngle = angle;
    } else {
        motion.repeatCount++;
        if (motion.repeatCount < motion.maxRepeats) {
            // برای لرزش، به سمت دیگر می‌رویم
            int temp = motion.startAngle;
            motion.startAngle = motion.endAngle;
            motion.endAngle = temp;
            motion.phaseStartTime = now;
        } else {
            motion.state = MOTION_IDLE;
            targetAngle = 90;
        }
    }
}

void updateSwingMotion(unsigned long now) {
    if (motion.repeatCount >= motion.maxRepeats) {
        motion.state = MOTION_IDLE;
        targetAngle = 90;
        return;
    }

    if (now - motion.phaseStartTime < (unsigned long)motion.duration) {
        float progress = (float)(now - motion.phaseStartTime) / motion.duration;
        int angle = motion.startAngle + (motion.endAngle - motion.startAngle) * progress;
        headServo.write(angle);
        currentAngle = angle;
    } else {
        motion.repeatCount++;
        motion.phaseStartTime = now;
        // معکوس کردن جهت
        int temp = motion.startAngle;
        motion.startAngle = motion.endAngle;
        motion.endAngle = temp;
    }
}

void updateSequenceMotion(unsigned long now) {
    // حرکت مرحله‌ای برای EATING و RAPID_BLINK
    switch (motion.phase) {
        case 0: {
            // حرکت اصلی به چپ یا راست
            if (now - motion.phaseStartTime < (unsigned long)motion.duration) {
                float progress = (float)(now - motion.phaseStartTime) / motion.duration;
                int angle = motion.startAngle + (motion.endAngle - motion.startAngle) * progress;
                headServo.write(angle);
                currentAngle = angle;
            } else {
                motion.phase = 1;
                motion.phaseStartTime = now;
                if (currentFace == FACE_EATING) {
                    motion.startAngle = motion.endAngle; // 60
                    motion.endAngle = 70; // 10 درجه به راست
                    motion.duration = 200;
                } else if (currentFace == FACE_RAPID_BLINK) {
                    motion.startAngle = motion.endAngle; // 120
                    motion.endAngle = 110; // 10 درجه به چپ
                    motion.duration = 150;
                }
            }
            break;
        }
        case 1:
        case 2: {
            // نوسان‌های ثانویه
            if (now - motion.phaseStartTime < (unsigned long)motion.duration) {
                float progress = (float)(now - motion.phaseStartTime) / motion.duration;
                int angle = motion.startAngle + (motion.endAngle - motion.startAngle) * progress;
                headServo.write(angle);
                currentAngle = angle;
            } else {
                motion.phase++;
                if (motion.phase > 3) {
                    motion.state = MOTION_IDLE;
                    targetAngle = 90;
                    return;
                }
                motion.phaseStartTime = now;
                // معکوس کردن جهت
                int temp = motion.startAngle;
                motion.startAngle = motion.endAngle;
                motion.endAngle = temp;
            }
            break;
        }
        default:
            motion.state = MOTION_IDLE;
            targetAngle = 90;
            break;
    }
}

void updateSpecialMotion(unsigned long now) {
    if (specialPhase == 0) {
        specialStart = now;
        specialPhase = 1;
    }

    if (specialPhase == 1) {
        unsigned long elapsed = now - specialStart;
        int target;
        if (specialMotion == HEAD_MOTION_SLOW_RIGHT) target = 135;
        else target = 45;
        int duration = 1500;

        if (elapsed < (unsigned long)duration) {
            float progress = (float)elapsed / duration;
            int angle = 90 + (target - 90) * progress;
            headServo.write(angle);
            currentAngle = angle;
        } else {
            specialPhase = 2;
            specialStart = now;
        }
    } else if (specialPhase == 2) {
        if (now - specialStart > 1500) {
            specialPhase = 0;
            specialMotion = 0;
            targetAngle = 90;
        }
    }
}
