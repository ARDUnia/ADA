#ifndef MOCK_DFPLAYER_H
#define MOCK_DFPLAYER_H
#include "Arduino.h"
#include "SoftwareSerial.h"
class DFRobotDFPlayerMini {
public:
    static int lastVolume;
    static std::vector<int> mp3Tracks;
    bool begin(SoftwareSerial&) { return true; }
    void volume(int value) { lastVolume = value; }
    void play(int track) { mp3Tracks.push_back(-track); }
    void playMp3Folder(int track) { mp3Tracks.push_back(track); }
};
#endif
