# Configuration guide

## Wi-Fi portal

When there are no valid stored credentials, or when connection retries are exhausted, ADA starts a local access point:

| Setting | Default |
|---|---|
| SSID | `ADA-Config` |
| Password | `12345678` |
| Address | `http://192.168.4.1` |

The web page scans nearby networks, accepts a manual SSID and supports open networks with an empty password. SSID and password lengths are validated before being written to EEPROM.

The portal can also be opened from the OLED Setup menu using `WiFi setup`. Starting the portal disconnects the current station connection so the device can accept new credentials.

## Setup menu

Enter Setup with `3TS4`.

| Sensor | Browse mode | Edit mode |
|---|---|---|
| S1 | Previous item | Increase value |
| S2 | Next item | Decrease value |
| S3 | Select | Confirm and save |
| S4 | Exit Setup | Cancel current edit |

Menu items:

1. Volume, range 0–30
2. Dim time, 5–60 seconds
3. Half-closed time, 10–120 seconds
4. Sleep time, 30–600 seconds
5. Wi-Fi setup portal
6. Exit

The firmware enforces a valid order: dim time < half-closed time < sleep time. Confirmed values are stored with CRC protection. Unconfirmed changes are discarded after the ten-second Setup timeout.

## microSD and audio files

Use a FAT32 microSD card. Create an uppercase or lowercase `MP3` directory supported by the card filesystem and use four-digit filenames:

```text
/MP3/0001.mp3
/MP3/0002.mp3
...
/MP3/0014.mp3
/MP3/0020.mp3
```

Tracks 1–14 are reactions and track 20 is the startup sound. The firmware uses `playMp3Folder()` so selection follows the filename, not the order in which files were copied to the card.

## Battery calibration

`BATTERY_FACTOR` converts ADC counts to battery volts. To calibrate:

1. Measure the battery with a trusted multimeter.
2. Open Serial Monitor or temporarily print `analogRead(A0)`.
3. Calculate `factor = measured_battery_voltage / ADC_count`.
4. Replace `BATTERY_FACTOR` in `Config.h`.
5. Verify at both a high and a low battery voltage.

The percentage display is a simple linear estimate between `BATTERY_MIN_VOLTAGE` and `BATTERY_MAX_VOLTAGE`; it is not a fuel-gauge measurement.
