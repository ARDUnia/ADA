# Software architecture

The main loop is cooperative and avoids delays during normal operation. Each module owns one responsibility:

| Module | Responsibility |
|---|---|
| `Ada.ino` | Initialization, main scheduler, reaction timer and inactivity state machine |
| `Config.h` | Pin map, defaults, enums and shared declarations |
| `Touch.cpp/.h` | PCF8574 sampling, gesture recognition and gesture event counter |
| `Display.cpp/.h` | OLED rendering, face animation, time and battery screens |
| `Audio.cpp/.h` | Face-to-track mapping and DFPlayer volume/playback |
| `Head.cpp/.h` | Non-blocking servo motion state machines |
| `Setup.cpp/.h` | Paged OLED menu and persistent user settings |
| `internet.cpp/.h` | Wi-Fi station/AP modes, configuration server, EEPROM credentials and NTP |

## Main execution order

1. If Setup is open, only the Setup state machine runs.
2. Touch input is sampled and activity is recorded.
3. Wi-Fi and NTP state are serviced.
4. Reaction duration and special displays are updated.
5. The inactivity state machine selects active, dimmed, half-closed or sleeping mode.
6. Audio, head motion and OLED rendering are updated.

## Important design choices

- Unsigned `millis()` subtraction is used for rollover-safe timing.
- Long-touch recognition locks until release.
- A monotonic gesture counter allows the same reaction to restart and replay.
- Wi-Fi and settings use separate EEPROM regions and independent CRC records.
- DFPlayer filenames are addressed through `/MP3`, avoiding FAT copy-order dependence.
- Setup suppresses the exit touch until release so it cannot leak into normal gesture recognition.
