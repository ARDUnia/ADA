/*
 * Audio.cpp - پیادهسازی لایه صوتی غیرمسدودکننده ربات آدا
 * هماهنگ با فایل moods.txt (۰۰۰۱.mp3 تا ۰۰۱۴.mp3)
 * نویسنده: حمیدرضا میلانی نیا (ARDUnia)
 * نسخه: ۲.۱ (ولوم هماهنگ و پخش قطعی بر اساس نام فایل)
 */

#include "Audio.h"
#include "Touch.h"
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ============================================================
//  تنظیمات سختافزاری (طبق Config.h)
// ============================================================
SoftwareSerial audioSerial(DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
DFRobotDFPlayerMini myDFPlayer;

// ============================================================
//  جدول نگاشت حالت چهره به شماره ترک صوتی (بر اساس moods.txt)
// ============================================================
struct AudioMapping {
    FaceState face;
    int trackNumber;
    const char* description;
};

const AudioMapping audioMap[] = {
    { FACE_HAPPY,       1, "HAPPY" },
    { FACE_ANGRY,       2, "ANGRY" },
    { FACE_THINKING,    3, "THINKING" },
    { FACE_SLEEPY,      4, "SLEEPING" },
    { FACE_SURPRISED,   5, "SURPRISED" },
    { FACE_SAD,         6, "SAD" },
    { FACE_WINK,        7, "WINK" },
    { FACE_FEAR,        8, "FEAR" },
    { FACE_ECSTASY,     9, "ECSTASY" },
    { FACE_EATING,      10, "EATING" },
    { FACE_RAPID_BLINK, 11, "RAPID BLINK" },
    { FACE_SICK,        12, "SICK" },
    { FACE_HUNGRY,      13, "HUNGRY" },
    { FACE_XO,          14, "XO STATE" },
    // FACE_NORMAL بدون صدا است (در این جدول نیست)
};

const int audioMapSize = sizeof(audioMap) / sizeof(audioMap[0]);

// ============================================================
//  متغیرهای مدیریت پخش (غیرمسدودکننده)
// ============================================================
static FaceState lastAudioFace = FACE_SLEEPY;
static unsigned long lastPlayTime = 0;
static bool audioReady = false;
static uint32_t lastAudioGestureEvent = 0;
const unsigned long MIN_PLAY_INTERVAL = 150; // جلوگیری از پخش مکرر در صورت تغییر سریع چهره

// ============================================================
//  توابع جدید برای کنترل ولوم
// ============================================================

void setAudioVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    AUDIO_VOLUME = volume;
    if (audioReady) myDFPlayer.volume(volume);
    Serial.print("Volume set to: ");
    Serial.println(volume);
}

uint8_t getAudioVolume() {
    return AUDIO_VOLUME;
}


// ============================================================
//  توابع عمومی
// ============================================================

void initAudio() {
    audioSerial.begin(9600);
    Serial.println("Initializing DFPlayer Mini Audio System...");

    if (!myDFPlayer.begin(audioSerial)) {
        Serial.println("Error: Unable to begin DFPlayer Mini. Check SD Card or Pin Connections.");
        audioReady = false;
    } else {
        Serial.println("DFPlayer Mini Online and Ready.");
        audioReady = true;
        myDFPlayer.volume(AUDIO_VOLUME); // 0-30
    }
    lastAudioFace = currentFace;
    lastAudioGestureEvent = getGestureEventCounter();
    lastPlayTime = millis();
}

void updateAudio() {
    if (!audioReady) return;
    // اگر چهره تغییری نکرده باشد، خروج
    uint32_t gestureEvent = getGestureEventCounter();
    if (currentFace == lastAudioFace && gestureEvent == lastAudioGestureEvent) {
        return;
    }

    // جلوگیری از پخش مکرر در بازهٔ زمانی کوتاه (جهت جلوگیری از قطع و وصل ناخواسته)
    if (millis() - lastPlayTime < MIN_PLAY_INTERVAL) {
        return;
    }

    // جستجوی شماره ترک متناسب با چهرهٔ فعلی
    int trackNumber = 0;
    for (int i = 0; i < audioMapSize; i++) {
        if (audioMap[i].face == currentFace) {
            trackNumber = audioMap[i].trackNumber;
            break;
        }
    }

    // اگر ترک معتبری یافت شد، پخش کن
    if (trackNumber > 0) {
        Serial.print("Audio Sync: Playing track ");
        Serial.print(trackNumber);
        Serial.print(" (");
        // نمایش نام حالت (برای دیباگ)
        for (int i = 0; i < audioMapSize; i++) {
            if (audioMap[i].face == currentFace) {
                Serial.print(audioMap[i].description);
                break;
            }
        }
        Serial.println(")");
        
        // Deterministic filename lookup: /MP3/0001.mp3 ... /MP3/0014.mp3
        myDFPlayer.playMp3Folder(trackNumber);
        lastPlayTime = millis();
    } else {
        // حالت NORMAL یا هر حالت بدون صدا
        Serial.println("Audio Sync: No track for this face (NORMAL or undefined)");
    }

    // بهروزرسانی آخرین وضعیت ثبتشده
    lastAudioFace = currentFace;
    lastAudioGestureEvent = gestureEvent;
}

void playStartupSound() {
    Serial.println("Audio: Playing startup track 0020.mp3");
    if (audioReady) myDFPlayer.playMp3Folder(20);
}
