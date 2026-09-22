#include <iostream>
#include <stdexcept>
#include <string>
#include "Arduino.h"
#include "Wire.h"
#include "EEPROM.h"
#include "ESP8266WiFi.h"
#include "DFRobotDFPlayerMini.h"
#include "Servo.h"
#include "U8g2lib.h"
#include "Config.h"
#include "Touch.h"
#include "Display.h"
#include "Setup.h"
#include "internet.h"

void setup();
void loop();
void readCredentialsFromEEPROM(String&, String&);
void saveCredentialsToEEPROM(const String&, const String&);
extern FaceState previousFace;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

static int checks = 0;

void expect(bool condition, const std::string& message) {
    checks++;
    if (!condition) throw std::runtime_error(message);
}

size_t occurrences(const std::string& text, const std::string& needle) {
    size_t count = 0, pos = 0;
    while ((pos = text.find(needle, pos)) != std::string::npos) { count++; pos += needle.size(); }
    return count;
}

void tick(unsigned long ms = 1, uint8_t mask = 0) {
    mockMillis += ms;
    Wire.input = mask;
    loop();
}

void tapSensor(int sensor, int count) {
    uint8_t mask = 1U << (sensor - 1);
    for (int i = 0; i < count; i++) {
        tick(50, mask);
        tick(120, 0);
    }
    tick(650, 0);
}

void longSensor(int sensor, unsigned long heldExtra = 1600) {
    uint8_t mask = 1U << (sensor - 1);
    tick(20, mask);
    tick(1050, mask);
    for (unsigned long elapsed = 0; elapsed < heldExtra; elapsed += 100) tick(100, mask);
    tick(20, 0);
    tick(650, 0);
}

void swipe(const std::vector<int>& sensors) {
    for (int sensor : sensors) {
        tick(30, 1U << (sensor - 1));
        tick(80, 0);
    }
    tick(650, 0);
}

void finishReaction() {
    for (int i = 0; i < 330; i++) tick(10, 0);
    expect(currentFace == FACE_NORMAL, "reaction did not return to normal");
}

void expectTapFace(int sensor, int count, FaceState expected, const char* name) {
    tapSensor(sensor, count);
    expect(currentFace == expected, std::string(name) + " gesture mapped to wrong face");
    finishReaction();
}

void expectLongFace(int sensor, FaceState expected, const char* token) {
    size_t before = occurrences(Serial.log, token);
    longSensor(sensor);
    expect(currentFace == expected || currentFace == FACE_NORMAL,
           std::string(token) + " gesture mapped to wrong face");
    size_t after = occurrences(Serial.log, token);
    expect(after == before + 1, std::string(token) + " repeated while held");
    if (currentFace != FACE_NORMAL) finishReaction();
}

void menuPress(int sensor) {
    tick(20, 1U << (sensor - 1));
    tick(20, 0);
}

int main() {
    try {
        EEPROM.begin(EEPROM_SIZE);
        saveCredentialsToEEPROM("OpenNetwork", "");

        setup();
        expect(AUDIO_VOLUME == 25, "default volume not loaded");
        expect(DFRobotDFPlayerMini::lastVolume == 25, "DFPlayer startup volume differs from setting");
        expect(!DFRobotDFPlayerMini::mp3Tracks.empty() && DFRobotDFPlayerMini::mp3Tracks.front() == 20,
               "startup track was not played from /MP3");
        expect(WiFi.beginCalls == 1, "blank password/open WiFi credentials were rejected");

        WiFi.statusValue = WL_CONNECTED;
        tick(1, 0);
        expect(isWiFiConnected(), "WiFi connection was not detected");
        expect(getFormattedTime() == "12:34:56", "clock format is not HH:MM:SS");
        WiFi.statusValue = WL_DISCONNECTED;
        tick(1, 0);
        expect(!isWiFiConnected(), "WiFi loss was not detected");

        expectTapFace(1, 1, FACE_HAPPY, "1TS1");
        expectTapFace(1, 3, FACE_ANGRY, "3TS1");
        expectLongFace(1, FACE_THINKING, "Processing gesture: LT1");
        expectLongFace(2, FACE_SLEEPY, "Processing gesture: LT2");
        expectTapFace(3, 2, FACE_SURPRISED, "2TS3");
        expectTapFace(3, 3, FACE_SAD, "3TS3");
        expectTapFace(1, 2, FACE_WINK, "2TS1");
        expectTapFace(2, 3, FACE_FEAR, "3TS2");
        expectLongFace(3, FACE_EATING, "Processing gesture: LT3");
        expectTapFace(2, 1, FACE_RAPID_BLINK, "1TS2");
        expectTapFace(3, 1, FACE_SICK, "1TS3");
        expectTapFace(2, 2, FACE_HUNGRY, "2TS2");
        expectTapFace(4, 1, FACE_XO, "1TS4");

        auto track1Before = std::count(DFRobotDFPlayerMini::mp3Tracks.begin(),
                                       DFRobotDFPlayerMini::mp3Tracks.end(), 1);
        tapSensor(1, 1);
        tick(400, 0);
        tapSensor(1, 1);
        expect(std::count(DFRobotDFPlayerMini::mp3Tracks.begin(),
                          DFRobotDFPlayerMini::mp3Tracks.end(), 1) == track1Before + 2,
               "same face gesture did not replay audio");
        for (int i = 0; i < 250; i++) tick(10, 0);
        expect(currentFace == FACE_HAPPY, "same face gesture did not restart reaction timer");
        for (int i = 0; i < 60; i++) tick(10, 0);
        expect(currentFace == FACE_NORMAL, "retriggered reaction did not finish");

        tapSensor(4, 2);
        expect(isTimeDisplayActive(), "2TS4 clock closed in the activation loop");
        tick(20, 0x01);
        expect(!isTimeDisplayActive(), "active clock did not close on touch press");
        tick(20, 0);
        tick(650, 0);
        finishReaction();

        size_t lt4Before = occurrences(Serial.log, "Processing gesture: LT4");
        longSensor(4, 400);
        expect(occurrences(Serial.log, "Processing gesture: LT4") == lt4Before + 1,
               "LT4 repeated while held");
        expect(getShowBattery(), "LT4 did not activate battery view");
        tick(3100, 0);
        expect(!getShowBattery(), "battery view did not time out");

        size_t l2rBefore = occurrences(Serial.log, "Processing gesture: L2R");
        swipe({1,2,3,4});
        expect(occurrences(Serial.log, "Processing gesture: L2R") == l2rBefore + 1, "L2R not recognized");
        for (int i = 0; i < 330; i++) tick(10, 0);
        size_t r2lBefore = occurrences(Serial.log, "Processing gesture: R2L");
        swipe({4,3,2,1});
        expect(occurrences(Serial.log, "Processing gesture: R2L") == r2lBefore + 1, "R2L not recognized");
        for (int i = 0; i < 330; i++) tick(10, 0);
        size_t rllrBefore = occurrences(Serial.log, "Processing gesture: RLLR");
        swipe({1,2,3,4,3,2,1});
        expect(occurrences(Serial.log, "Processing gesture: RLLR") == rllrBefore + 1, "RLLR not recognized");
        finishReaction();

        tapSensor(4, 3);
        expect(getConfigMode(), "3TS4 did not enter Setup");
        tick(1, 0); // initialize Setup and edge state
        u8g2.outOfBoundsDraws = 0;
        menuPress(3); // edit Volume
        menuPress(1); // 25 -> 26
        menuPress(3); // confirm and persist
        expect(AUDIO_VOLUME == 26, "volume edit was not applied");
        for (int i = 0; i < 4; i++) menuPress(2); // Render second menu page.
        expect(u8g2.outOfBoundsDraws == 0, "Setup menu drew outside 128x64 OLED");
        menuPress(4); // exit Setup
        expect(!getConfigMode(), "S4 did not exit Setup");
        tick(20, 0); // release suppression
        tick(650, 0);
        expect(currentFace != FACE_XO, "Setup exit touch leaked into normal gesture handling");

        AUDIO_VOLUME = 1;
        INACTIVITY_DIM_START = 5;
        INACTIVITY_HALF_CLOSE = 6;
        INACTIVITY_SLEEP_START = 7;
        initSetup();
        expect(AUDIO_VOLUME == 26, "persisted volume did not reload");
        expect(INACTIVITY_DIM_START == 30000 && INACTIVITY_HALF_CLOSE == 60000 &&
               INACTIVITY_SLEEP_START == 300000, "persisted time settings did not reload");

        EEPROM.write(146, EEPROM.read(146) ^ 0x01); // corrupt settings CRC
        AUDIO_VOLUME = 1;
        initSetup();
        expect(AUDIO_VOLUME == 25 && INACTIVITY_DIM_START == 30000 &&
               INACTIVITY_HALF_CLOSE == 60000 && INACTIVITY_SLEEP_START == 300000,
               "corrupt settings CRC was accepted");

        String ssid, pass;
        readCredentialsFromEEPROM(ssid, pass);
        expect(ssid == "OpenNetwork" && pass == "", "credential record failed round-trip");
        EEPROM.write(102, EEPROM.read(102) ^ 0x01); // corrupt credential CRC
        readCredentialsFromEEPROM(ssid, pass);
        expect(ssid.length() == 0 && pass.length() == 0, "corrupt credential CRC was accepted");

        for (int i = 0; i <= 103; i++) EEPROM.write(i, 0xFF);
        const char legacy[] = "LegacySSID\0legacy-pass\0";
        for (size_t i = 0; i < sizeof(legacy); i++) EEPROM.write((int)i, legacy[i]);
        readCredentialsFromEEPROM(ssid, pass);
        expect(ssid == "LegacySSID" && pass == "legacy-pass", "legacy credentials did not migrate");
        expect(EEPROM.read(0) == 'A' && EEPROM.read(1) == 'D' && EEPROM.read(2) == 'A' && EEPROM.read(3) == '2',
               "legacy credential migration did not write the new record");

        String maxSsid(std::string(32, 'S'));
        String maxPass(std::string(64, 'P'));
        saveCredentialsToEEPROM(maxSsid, maxPass);
        readCredentialsFromEEPROM(ssid, pass);
        expect(ssid == maxSsid && pass == maxPass, "maximum credential lengths failed round-trip");

        INACTIVITY_DIM_START = 100;
        INACTIVITY_HALF_CLOSE = 200;
        INACTIVITY_SLEEP_START = 300;
        tick(1, 0x01);
        tick(1, 0);
        expect(currentSystemState == STATE_ACTIVE, "precondition wake for sleep test failed");
        tick(101, 0);
        expect(currentSystemState == STATE_DIMMED, "DIMMED transition failed");
        tick(100, 0);
        expect(currentSystemState == STATE_HALF_CLOSED, "HALF_CLOSED transition failed");
        tick(100, 0);
        expect(currentSystemState == STATE_SLEEPING, "SLEEPING transition failed");
        tick(1, 0x01);
        expect(currentSystemState == STATE_ACTIVE, "touch did not wake system immediately");
        tick(20, 0);

        expect(Servo::minWritten >= 0 && Servo::maxWritten <= 180, "servo command exceeded safe angle range");
        for (int track = 1; track <= 14; track++) {
            expect(std::find(DFRobotDFPlayerMini::mp3Tracks.begin(), DFRobotDFPlayerMini::mp3Tracks.end(), track) !=
                   DFRobotDFPlayerMini::mp3Tracks.end(), "audio mapping missing track " + std::to_string(track));
        }
        expect(std::find(DFRobotDFPlayerMini::mp3Tracks.begin(), DFRobotDFPlayerMini::mp3Tracks.end(), -1) ==
               DFRobotDFPlayerMini::mp3Tracks.end(), "non-deterministic DFPlayer play() was used");

        std::cout << "ADA host hardware simulation PASS\n";
        std::cout << "Assertions: " << checks << "\n";
        std::cout << "Recognized gesture events: 19/19\n";
        std::cout << "Servo range: " << Servo::minWritten << ".." << Servo::maxWritten << " degrees\n";
        std::cout << "DFPlayer MP3-folder commands: " << DFRobotDFPlayerMini::mp3Tracks.size() << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "ADA host hardware simulation FAIL: " << error.what() << "\n";
        return 1;
    }
}
