# Software, libraries and build instructions

## Board support

Install **ESP8266 by ESP8266 Community** in Arduino IDE Boards Manager, then select:

- Board: `LOLIN(WEMOS) D1 R2 & mini`
- Upload speed: start with `115200` if a faster setting is unreliable
- Serial monitor: `115200 baud`

## External libraries

Install these through Arduino Library Manager:

| Library Manager name | Header used | Purpose |
|---|---|---|
| U8g2 | `U8g2lib.h` | OLED graphics and fonts |
| DFRobotDFPlayerMini | `DFRobotDFPlayerMini.h` | DFPlayer control |
| NTPClient | `NTPClient.h` | Network time |
| Time | `TimeLib.h` | Time utilities |

The following are supplied by the ESP8266 Arduino core and should not be installed as unrelated third-party replacements:

- `Arduino.h`
- `Wire.h`
- `EEPROM.h`
- `ESP8266WiFi.h`
- `ESP8266WebServer.h`
- `WiFiUdp.h`
- `SoftwareSerial.h`
- `Servo.h`

## Arduino IDE build

1. Clone or download the repository.
2. Open `firmware/Ada/Ada.ino`.
3. Arduino IDE loads the adjacent `.cpp` and `.h` modules automatically.
4. Select the D1 mini board and the correct serial port.
5. Compile before connecting high-current peripherals.
6. Upload, then open Serial Monitor at 115200 baud.

## PlatformIO build

The root `platformio.ini` defines a `d1_mini` environment and the required dependencies.

```bash
pio run
pio run -t upload
pio device monitor
```

## Main configurable constants

`firmware/Ada/Config.h` contains:

- Wi-Fi AP name and password
- I2C and peripheral pin assignments
- battery voltage range and conversion factor
- OLED brightness levels
- NTP UTC offset and update interval

The default NTP offset is `12600` seconds, equal to UTC+03:30. Change `NTP_UTC_OFFSET_SECONDS` if the device is used in another time zone.

## Memory layout

The firmware uses 512 bytes of EEPROM emulation:

- Wi-Fi credentials start at address 0 and include magic data, lengths and CRC.
- User settings start at address 128 and include a format version and CRC.

Do not add new EEPROM data inside these ranges without updating the documented layout and migration logic.
