# ADA — Interactive ESP8266 Robot

[![Host simulation](https://github.com/ARDUnia/ADA/actions/workflows/host-tests.yml/badge.svg)](https://github.com/ARDUnia/ADA/actions/workflows/host-tests.yml)
[![Platform](https://img.shields.io/badge/platform-ESP8266-00979D)](https://www.espressif.com/en/products/socs/esp8266)
[![Documentation](https://img.shields.io/badge/docs-English%20%7C%20فارسی-0B2C47)](#documentation)

ADA is an interactive desktop robot built around the Wemos D1 mini / ESP8266. It combines animated OLED eyes, four capacitive touch inputs, expressive servo head motion, DFPlayer audio feedback, battery monitoring, Wi-Fi configuration and an NTP clock.

> مستندات کامل فارسی: [docs/README_FA.md](docs/README_FA.md)

## Project links

- ARDUnia projects and technical articles: [ardunia.ir](https://ardunia.ir)
- Hamidreza Milaninia: [hrmnia.com](https://hrmnia.com)
- GitHub organization: [github.com/ARDUnia](https://github.com/ARDUnia)

## Features

- 15 animated face states on a 128×64 SSD1306 OLED
- 19 mapped touch gestures using four TTP223 sensors through a PCF8574
- Non-blocking servo animations and left/right swipe movements
- 14 reaction tracks plus a startup track on DFPlayer Mini
- Local Wi-Fi setup portal with scanned network selection
- NTP clock configured for Iran Standard Time by default
- Battery voltage and percentage display
- Automatic dim, half-closed and sleep states
- Persistent, CRC-protected settings and Wi-Fi credentials
- Host-side hardware simulation with AddressSanitizer and UndefinedBehaviorSanitizer

## Quick start

1. Build the circuit according to [docs/HARDWARE.md](docs/HARDWARE.md).
2. Install the libraries listed in [docs/SOFTWARE.md](docs/SOFTWARE.md).
3. Open `firmware/Ada/Ada.ino` in Arduino IDE, or open the repository in PlatformIO.
4. Format the microSD card as FAT32.
5. Copy the included [`assets/audio/MP3`](assets/audio/MP3) directory to the root of the card. It already contains `/MP3/0001.mp3` through `/MP3/0014.mp3` and the startup sound `/MP3/0020.mp3`.
6. Upload the firmware to a Wemos D1 mini / ESP8266.
7. If no saved network exists, connect to `ADA-Config` using password `12345678`, then open `http://192.168.4.1`.

## Hardware overview

| Part | Quantity | Purpose |
|---|---:|---|
| Wemos D1 mini / ESP8266 | 1 | Main controller and Wi-Fi |
| SSD1306 OLED 128×64, I2C | 1 | Animated face and menus |
| PCF8574 I/O expander | 1 | Four touch inputs |
| TTP223 capacitive touch module | 4 | Gesture sensors S1–S4 |
| DFPlayer Mini | 1 | MP3 reaction audio |
| microSD card, FAT32 | 1 | Audio storage |
| Small speaker, suitable for DFPlayer | 1 | Audio output |
| Hobby servo such as SG90 | 1 | Head movement |
| Battery measurement divider | 1 | Safe A0 voltage sensing |
| Regulated power supply | 1 | Stable controller, audio and servo power |

See the complete BOM, voltage warnings and wiring table in [docs/HARDWARE.md](docs/HARDWARE.md).

## Pin summary

| ESP8266 pin | D1 mini label | Connected device |
|---|---|---|
| GPIO4 | D2 | I2C SDA: OLED + PCF8574 |
| GPIO5 | D1 | I2C SCL: OLED + PCF8574 |
| GPIO16 | D0 | Servo signal |
| GPIO13 | D7 | ESP RX ← DFPlayer TX |
| GPIO15 | D8 | ESP TX → DFPlayer RX through a series resistor |
| A0 | A0 | Battery divider output |

## Documentation

- [Persian overview and operating guide](docs/README_FA.md)
- [Hardware, BOM and wiring](docs/HARDWARE.md)
- [Libraries, build and upload](docs/SOFTWARE.md)
- [Touch gestures and reactions](docs/GESTURES.md)
- [Wi-Fi, Setup menu and SD card](docs/CONFIGURATION.md)
- [Software architecture](docs/ARCHITECTURE.md)
- [Testing and simulation](docs/TESTING.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)
- [Contributing](CONTRIBUTING.md)
- [Security policy](SECURITY.md)
- [Change log](CHANGELOG.md)

## Repository structure

```text
firmware/Ada/        Arduino sketch and modules
assets/audio/MP3/    Ready-to-copy DFPlayer sound effects
docs/                Hardware and software documentation
tests/host/          ESP8266 peripheral simulation and regression tests
.github/workflows/   Automated host test workflow
platformio.ini       Optional PlatformIO environment
```

## Test status

The current build passes 83 host assertions, recognizes all 19 gesture patterns and verifies that all 15 required audio tracks are present. The simulated servo command range is 45°–134°. See [docs/TESTING.md](docs/TESTING.md) for scope and limitations.

## License

No open-source license has been selected yet. Copyright remains with the project owner; reuse or redistribution requires permission until a license is added.

## Author

**Hamidreza Milaninia — ARDUnia**  
[ardunia.ir](https://ardunia.ir) · [hrmnia.com](https://hrmnia.com)
