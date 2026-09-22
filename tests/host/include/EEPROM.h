#ifndef MOCK_EEPROM_H
#define MOCK_EEPROM_H
#include "Arduino.h"
class EEPROMClass {
public:
    std::vector<uint8_t> bytes;
    int commits = 0;
    EEPROMClass() : bytes(512, 0xFF) {}
    bool begin(size_t size) { if (bytes.size() < size) bytes.resize(size, 0xFF); return true; }
    uint8_t read(int address) const { return bytes.at(address); }
    void write(int address, uint8_t value) { bytes.at(address) = value; }
    bool commit() { commits++; return true; }
};
extern EEPROMClass EEPROM;
#endif
