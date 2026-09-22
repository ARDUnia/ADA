/*
 * Setup.cpp - persistent, paged Setup menu for ADA
 * Activated by three taps on sensor 4.
 * Version 2.0
 */

#include "Setup.h"
#include "Touch.h"
#include "Audio.h"
#include "Display.h"
#include "internet.h"
#include "Config.h"
#include <Wire.h>
#include <EEPROM.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern FaceState previousFace;

static bool setupActive = false;
static unsigned long setupStartTime = 0;
const unsigned long SETUP_TIMEOUT = 10000;

enum MenuItem {
    MENU_VOLUME,
    MENU_DIM_TIME,
    MENU_HALF_TIME,
    MENU_SLEEP_TIME,
    MENU_WIFI_SETUP,
    MENU_EXIT,
    MENU_ITEM_COUNT
};

static MenuItem currentMenuItem = MENU_VOLUME;
static int menuPosition = 0;
static bool editMode = false;

static uint8_t tempVolume = 25;
static uint32_t tempDimTime = 30000;
static uint32_t tempHalfTime = 60000;
static uint32_t tempSleepTime = 300000;

static bool lastS1 = false;
static bool lastS2 = false;
static bool lastS3 = false;
static bool lastS4 = false;

const int SETTINGS_MAGIC_ADDR = 128;
const int SETTINGS_VERSION_ADDR = 132;
const int SETTINGS_VOLUME_ADDR = 133;
const int SETTINGS_DIM_ADDR = 134;
const int SETTINGS_HALF_ADDR = 138;
const int SETTINGS_SLEEP_ADDR = 142;
const int SETTINGS_CRC_ADDR = 146;
const uint8_t SETTINGS_VERSION = 1;

void drawSetupMenu();
void handleSetupTouch();
void applySettings();
void loadSettings();
void saveSettings();
void normalizeSettings();
void cancelCurrentEdit();
void exitSetup();
void readSetupSensors(bool &s1, bool &s2, bool &s3, bool &s4);
void writeUint32(int address, uint32_t value);
uint32_t readUint32(int address);
uint16_t settingsCrc();

void initSetup() {
    EEPROM.begin(EEPROM_SIZE);
    loadSettings();
    tempVolume = AUDIO_VOLUME;
    tempDimTime = INACTIVITY_DIM_START;
    tempHalfTime = INACTIVITY_HALF_CLOSE;
    tempSleepTime = INACTIVITY_SLEEP_START;
    setupActive = false;
}

void updateSetup() {
    if (!getConfigMode()) {
        setupActive = false;
        return;
    }

    if (!setupActive) {
        setupActive = true;
        setupStartTime = millis();
        menuPosition = 0;
        currentMenuItem = MENU_VOLUME;
        editMode = false;
        tempVolume = AUDIO_VOLUME;
        tempDimTime = INACTIVITY_DIM_START;
        tempHalfTime = INACTIVITY_HALF_CLOSE;
        tempSleepTime = INACTIVITY_SLEEP_START;
        readSetupSensors(lastS1, lastS2, lastS3, lastS4);
    }

    if (millis() - setupStartTime > SETUP_TIMEOUT) {
        Serial.println("Setup timeout - unconfirmed edit cancelled");
        cancelCurrentEdit();
        exitSetup();
        return;
    }

    handleSetupTouch();
    if (getConfigMode()) drawSetupMenu();
}

bool isSetupActive() {
    return setupActive;
}

void readSetupSensors(bool &s1, bool &s2, bool &s3, bool &s4) {
    Wire.requestFrom(PCF8574_ADDRESS, 1);
    if (!Wire.available()) {
        s1 = s2 = s3 = s4 = false;
        return;
    }
    uint8_t data = Wire.read();
    s1 = data & 0x01;
    s2 = data & 0x02;
    s3 = data & 0x04;
    s4 = data & 0x08;
}

void handleSetupTouch() {
    bool s1, s2, s3, s4;
    readSetupSensors(s1, s2, s3, s4);

    bool press1 = s1 && !lastS1;
    bool press2 = s2 && !lastS2;
    bool press3 = s3 && !lastS3;
    bool press4 = s4 && !lastS4;
    lastS1 = s1;
    lastS2 = s2;
    lastS3 = s3;
    lastS4 = s4;

    if (editMode) {
        if (press1) {
            switch (currentMenuItem) {
                case MENU_VOLUME:     if (tempVolume < 30) tempVolume++; setAudioVolume(tempVolume); break;
                case MENU_DIM_TIME:   if (tempDimTime < 60000) tempDimTime += 5000; break;
                case MENU_HALF_TIME:  if (tempHalfTime < 120000) tempHalfTime += 10000; break;
                case MENU_SLEEP_TIME: if (tempSleepTime < 600000) tempSleepTime += 30000; break;
                default: break;
            }
            setupStartTime = millis();
        }
        if (press2) {
            switch (currentMenuItem) {
                case MENU_VOLUME:     if (tempVolume > 0) tempVolume--; setAudioVolume(tempVolume); break;
                case MENU_DIM_TIME:   tempDimTime = tempDimTime > 5000 ? tempDimTime - 5000 : 5000; break;
                case MENU_HALF_TIME:  tempHalfTime = tempHalfTime > 10000 ? tempHalfTime - 10000 : 10000; break;
                case MENU_SLEEP_TIME: tempSleepTime = tempSleepTime > 30000 ? tempSleepTime - 30000 : 30000; break;
                default: break;
            }
            setupStartTime = millis();
        }
        if (press3) {
            editMode = false;
            applySettings();
            Serial.println("Settings saved");
            setupStartTime = millis();
        }
        if (press4) {
            cancelCurrentEdit();
            editMode = false;
            Serial.println("Edit cancelled");
            setupStartTime = millis();
        }
        return;
    }

    if (press1) {
        menuPosition--;
        if (menuPosition < 0) menuPosition = MENU_ITEM_COUNT - 1;
        currentMenuItem = (MenuItem)menuPosition;
        setupStartTime = millis();
    }
    if (press2) {
        menuPosition++;
        if (menuPosition >= MENU_ITEM_COUNT) menuPosition = 0;
        currentMenuItem = (MenuItem)menuPosition;
        setupStartTime = millis();
    }
    if (press3) {
        setupStartTime = millis();
        if (currentMenuItem == MENU_EXIT) {
            exitSetup();
        } else if (currentMenuItem == MENU_WIFI_SETUP) {
            exitSetup();
            startWiFiConfigPortal();
            Serial.println("WiFi setup portal activated");
        } else {
            editMode = true;
        }
    }
    if (press4) exitSetup();
}

void drawSetupMenu() {
    const char* menuItems[MENU_ITEM_COUNT] = {
        "Volume", "Dim time", "Half time", "Sleep time", "WiFi setup", "Exit"
    };

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(2, 8, "SETUP");

    int first = (menuPosition / 4) * 4;
    int last = first + 4;
    if (last > MENU_ITEM_COUNT) last = MENU_ITEM_COUNT;
    const char* page = first == 0 ? "1/2" : "2/2";
    u8g2.drawStr(110, 8, page);

    for (int i = first; i < last; i++) {
        int y = 19 + (i - first) * 11;
        if (i == menuPosition) u8g2.drawStr(1, y, ">");
        u8g2.drawStr(8, y, menuItems[i]);

        char value[12] = "";
        if (i == menuPosition && editMode) {
            snprintf(value, sizeof(value), "EDIT");
        } else {
            switch (i) {
                case MENU_VOLUME: snprintf(value, sizeof(value), "%u", tempVolume); break;
                case MENU_DIM_TIME: snprintf(value, sizeof(value), "%lus", (unsigned long)(tempDimTime / 1000)); break;
                case MENU_HALF_TIME: snprintf(value, sizeof(value), "%lus", (unsigned long)(tempHalfTime / 1000)); break;
                case MENU_SLEEP_TIME: snprintf(value, sizeof(value), "%lus", (unsigned long)(tempSleepTime / 1000)); break;
                case MENU_WIFI_SETUP: snprintf(value, sizeof(value), "OPEN"); break;
                default: break;
            }
        }
        if (value[0]) u8g2.drawStr(98, y, value);
    }

    u8g2.drawStr(2, 63, "S1:UP S2:DN S3:OK S4:X");
    u8g2.sendBuffer();
}

void normalizeSettings() {
    if (tempVolume > 30) tempVolume = 30;
    tempDimTime = constrain(tempDimTime, (uint32_t)5000, (uint32_t)60000);
    tempHalfTime = constrain(tempHalfTime, (uint32_t)10000, (uint32_t)120000);
    tempSleepTime = constrain(tempSleepTime, (uint32_t)30000, (uint32_t)600000);
    if (tempHalfTime <= tempDimTime) tempHalfTime = tempDimTime + 5000UL;
    if (tempSleepTime <= tempHalfTime) tempSleepTime = tempHalfTime + 10000UL;
}

void applySettings() {
    normalizeSettings();
    AUDIO_VOLUME = tempVolume;
    INACTIVITY_DIM_START = tempDimTime;
    INACTIVITY_HALF_CLOSE = tempHalfTime;
    INACTIVITY_SLEEP_START = tempSleepTime;
    setAudioVolume(AUDIO_VOLUME);
    saveSettings();

    Serial.println("Settings applied and persisted:");
    Serial.print("  Volume: "); Serial.println(AUDIO_VOLUME);
    Serial.print("  Dim Time: "); Serial.println(INACTIVITY_DIM_START / 1000);
    Serial.print("  Half Time: "); Serial.println(INACTIVITY_HALF_CLOSE / 1000);
    Serial.print("  Sleep Time: "); Serial.println(INACTIVITY_SLEEP_START / 1000);
}

void cancelCurrentEdit() {
    switch (currentMenuItem) {
        case MENU_VOLUME: tempVolume = AUDIO_VOLUME; setAudioVolume(AUDIO_VOLUME); break;
        case MENU_DIM_TIME: tempDimTime = INACTIVITY_DIM_START; break;
        case MENU_HALF_TIME: tempHalfTime = INACTIVITY_HALF_CLOSE; break;
        case MENU_SLEEP_TIME: tempSleepTime = INACTIVITY_SLEEP_START; break;
        default: break;
    }
}

void exitSetup() {
    setConfigMode(false);
    setupActive = false;
    editMode = false;
    currentFace = previousFace;
    suspendTouchUntilRelease();
    Serial.println("Exiting setup mode");
}

void loadSettings() {
    const char magic[4] = {'C', 'F', 'G', '2'};
    bool valid = true;
    for (int i = 0; i < 4; i++) {
        if (EEPROM.read(SETTINGS_MAGIC_ADDR + i) != (uint8_t)magic[i]) valid = false;
    }
    if (EEPROM.read(SETTINGS_VERSION_ADDR) != SETTINGS_VERSION) valid = false;
    uint16_t storedCrc = (uint16_t)EEPROM.read(SETTINGS_CRC_ADDR) |
                         ((uint16_t)EEPROM.read(SETTINGS_CRC_ADDR + 1) << 8);
    if (storedCrc != settingsCrc()) valid = false;

    if (valid) {
        tempVolume = EEPROM.read(SETTINGS_VOLUME_ADDR);
        tempDimTime = readUint32(SETTINGS_DIM_ADDR);
        tempHalfTime = readUint32(SETTINGS_HALF_ADDR);
        tempSleepTime = readUint32(SETTINGS_SLEEP_ADDR);
        uint8_t oldVolume = tempVolume;
        uint32_t oldDim = tempDimTime, oldHalf = tempHalfTime, oldSleep = tempSleepTime;
        normalizeSettings();
        valid = oldVolume == tempVolume && oldDim == tempDimTime &&
                oldHalf == tempHalfTime && oldSleep == tempSleepTime;
    }

    if (!valid) {
        tempVolume = 25;
        tempDimTime = 30000;
        tempHalfTime = 60000;
        tempSleepTime = 300000;
        Serial.println("No valid saved settings; defaults loaded.");
    } else {
        Serial.println("Saved settings loaded.");
    }

    AUDIO_VOLUME = tempVolume;
    INACTIVITY_DIM_START = tempDimTime;
    INACTIVITY_HALF_CLOSE = tempHalfTime;
    INACTIVITY_SLEEP_START = tempSleepTime;
}

void saveSettings() {
    const char magic[4] = {'C', 'F', 'G', '2'};
    for (int i = 0; i < 4; i++) EEPROM.write(SETTINGS_MAGIC_ADDR + i, magic[i]);
    EEPROM.write(SETTINGS_VERSION_ADDR, SETTINGS_VERSION);
    EEPROM.write(SETTINGS_VOLUME_ADDR, AUDIO_VOLUME);
    writeUint32(SETTINGS_DIM_ADDR, INACTIVITY_DIM_START);
    writeUint32(SETTINGS_HALF_ADDR, INACTIVITY_HALF_CLOSE);
    writeUint32(SETTINGS_SLEEP_ADDR, INACTIVITY_SLEEP_START);
    uint16_t crc = settingsCrc();
    EEPROM.write(SETTINGS_CRC_ADDR, crc & 0xFF);
    EEPROM.write(SETTINGS_CRC_ADDR + 1, crc >> 8);
    EEPROM.commit();
}

void writeUint32(int address, uint32_t value) {
    for (int i = 0; i < 4; i++) EEPROM.write(address + i, (value >> (8 * i)) & 0xFF);
}

uint32_t readUint32(int address) {
    uint32_t value = 0;
    for (int i = 0; i < 4; i++) value |= (uint32_t)EEPROM.read(address + i) << (8 * i);
    return value;
}

uint16_t settingsCrc() {
    uint16_t crc = 0xA55A;
    for (int address = SETTINGS_VERSION_ADDR; address < SETTINGS_CRC_ADDR; address++) {
        crc ^= EEPROM.read(address);
        crc = (crc >> 1) | (crc << 15);
    }
    return crc;
}
