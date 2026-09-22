/*
 * Config.h - تنظیمات مرجع و زیرساخت اصلی پروژه آدا
 * نسخه: ۱.۵.۰ (ذخیره پایدار تنظیمات و اصلاح زمان شبکه)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
//  تنظیمات سریال
// ============================================================
#define SERIAL_BAUD 115200

// ============================================================
//  تنظیمات اینترنت
// ============================================================
#define AP_SSID "ADA-Config"
#define AP_PASSWORD "12345678"
#define EEPROM_SIZE 512
#define NTP_UTC_OFFSET_SECONDS 12600L       // Iran Standard Time: UTC+03:30
#define NTP_UPDATE_INTERVAL_MS 3600000UL    // 1 hour (NTPClient expects ms)

// ============================================================
//  پین‌های سخت‌افزاری
// ============================================================
#define I2C_SCL_PIN 5   // D1 (GPIO5)
#define I2C_SDA_PIN 4   // D2 (GPIO4)
#define PCF8574_ADDRESS 0x20
#define SERVO_PIN 16    // D0 (GPIO16)

// پین‌های DFPlayer
#define DFPLAYER_TX_PIN 15  // D8 (GPIO15)
#define DFPLAYER_RX_PIN 13  // D7 (GPIO13)

// پین سنسورهای لمسی روی PCF8574
#define PCF_TOUCH_1 0
#define PCF_TOUCH_2 1
#define PCF_TOUCH_3 2
#define PCF_TOUCH_4 3

// پین آنالوگ باتری
#define BATTERY_PIN A0

// ============================================================
//  تنظیمات باتری
// ============================================================
#define BATTERY_MIN_VOLTAGE 3.0
#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_FACTOR 0.00710526

// ============================================================
//  سطوح روشنایی OLED
// ============================================================
#define BRIGHTNESS_NORMAL 255
#define BRIGHTNESS_DIMMED 128
#define BRIGHTNESS_SLEEP 64

// ============================================================
//  وضعیت‌های سیستم (فقط یک بار تعریف شده)
// ============================================================
enum SystemState {
    STATE_ACTIVE,
    STATE_DIMMED,
    STATE_HALF_CLOSED,
    STATE_SLEEPING
};

// ============================================================
//  وضعیت‌های چهره
// ============================================================
enum FaceState {
    FACE_NORMAL,
    FACE_HAPPY,
    FACE_ANGRY,
    FACE_THINKING,
    FACE_SLEEPY,
    FACE_SURPRISED,
    FACE_SAD,
    FACE_WINK,
    FACE_FEAR,
    FACE_ECSTASY,
    FACE_EATING,
    FACE_RAPID_BLINK,
    FACE_SICK,
    FACE_HUNGRY,
    FACE_XO
};

enum RobotMode {
    MODE_INTERACTIVE,
    MODE_STANDBY,
    MODE_CONFIG
};

// ============================================================
//  متغیرهای قابل تنظیم (با مقادیر پیش‌فرض - فقط extern)
// ============================================================
extern uint32_t INACTIVITY_DIM_START;
extern uint32_t INACTIVITY_HALF_CLOSE;
extern uint32_t INACTIVITY_SLEEP_START;
extern uint8_t AUDIO_VOLUME;

// ============================================================
//  متغیرهای عمومی سیستم
// ============================================================
extern FaceState currentFace;
extern RobotMode currentMode;
extern bool isNetworkConnected;
extern SystemState currentSystemState;

#endif // CONFIG_H
