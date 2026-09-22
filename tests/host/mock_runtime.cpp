#include "Arduino.h"
#include "Wire.h"
#include "U8g2lib.h"
#include "Servo.h"
#include "DFRobotDFPlayerMini.h"
#include "EEPROM.h"
#include "ESP8266WiFi.h"

unsigned long mockMillis = 0;
int mockAnalogValue = 600;
SerialMock Serial;
TwoWire Wire;
EEPROMClass EEPROM;
ESP8266WiFiClass WiFi;
ESPClass ESP;
int Servo::minWritten = 181;
int Servo::maxWritten = -1;
int DFRobotDFPlayerMini::lastVolume = -1;
std::vector<int> DFRobotDFPlayerMini::mp3Tracks;

static int fontIds[6];
const void* u8g2_font_6x10_tf = &fontIds[0];
const void* u8g2_font_5x8_tf = &fontIds[1];
const void* u8g2_font_fub20_tf = &fontIds[2];
const void* u8g2_font_fub30_tf = &fontIds[3];
const void* u8g2_font_helvB14_tf = &fontIds[4];
const void* u8g2_font_6x12_tf = &fontIds[5];

unsigned long millis() { return mockMillis; }
void delay(unsigned long ms) { mockMillis += ms; }
void delayMicroseconds(unsigned int) {}
long random(long maxValue) { return maxValue > 0 ? std::rand() % maxValue : 0; }
long random(long minValue, long maxValue) { return maxValue > minValue ? minValue + std::rand() % (maxValue - minValue) : minValue; }
int analogRead(int) { return mockAnalogValue; }
