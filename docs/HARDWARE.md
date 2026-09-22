# Hardware, parts and wiring

## Bill of materials

| Component | Qty | Notes |
|---|---:|---|
| Wemos D1 mini or compatible ESP8266 board | 1 | 3.3 V logic |
| SSD1306 128×64 I2C OLED | 1 | Usually address `0x3C` |
| PCF8574 I2C I/O expander | 1 | Address must be `0x20` |
| TTP223 capacitive touch module | 4 | Active-high output expected |
| DFPlayer Mini MP3 module | 1 | Use a genuine or compatible module |
| microSD card | 1 | FAT32, MP3 files in `/MP3` |
| Speaker | 1 | Match the DFPlayer output specification |
| SG90 or similar hobby servo | 1 | Separate regulated power recommended |
| Resistor for DFPlayer RX line | 1 | Typically 1 kΩ series resistor |
| Battery voltage divider | 1 | Values must suit the board's A0 limit |
| 100 nF ceramic capacitors | Several | Local decoupling near modules |
| 470–1000 µF electrolytic capacitor | 1 | Recommended near servo/audio 5 V rail |
| Regulated supply, wiring and common ground | — | Size for servo stall current and audio peaks |

## Complete connection table

| Device pin | Connect to | Notes |
|---|---|---|
| OLED VCC | 3.3 V | Confirm module voltage rating |
| OLED GND | GND | Common ground |
| OLED SDA | D2 / GPIO4 | Shared I2C bus |
| OLED SCL | D1 / GPIO5 | Shared I2C bus |
| PCF8574 VCC | 3.3 V | Keeps I2C and touch logic at 3.3 V |
| PCF8574 GND | GND | Common ground |
| PCF8574 SDA | D2 / GPIO4 | Shared with OLED |
| PCF8574 SCL | D1 / GPIO5 | Shared with OLED |
| PCF8574 A0, A1, A2 | GND | Selects address `0x20` |
| TTP223 sensor 1 OUT | PCF8574 P0 | Gesture input S1 |
| TTP223 sensor 2 OUT | PCF8574 P1 | Gesture input S2 |
| TTP223 sensor 3 OUT | PCF8574 P2 | Gesture input S3 |
| TTP223 sensor 4 OUT | PCF8574 P3 | Gesture input S4 |
| All TTP223 VCC | 3.3 V | Active-high 3.3 V output |
| All TTP223 GND | GND | Common ground |
| Servo signal | D0 / GPIO16 | Head position command |
| Servo power | Regulated servo supply | Do not power a loaded servo from ESP8266 3.3 V |
| Servo GND | GND | Must be common with ESP8266 |
| DFPlayer TX | D7 / GPIO13 | ESP8266 software-serial RX |
| DFPlayer RX | D8 / GPIO15 | ESP8266 TX through about 1 kΩ series resistor |
| DFPlayer VCC | Stable supply per module | Commonly 5 V |
| DFPlayer GND | GND | Common ground |
| DFPlayer SPK1/SPK2 | Speaker | Do not connect either speaker lead to ground |
| Battery divider output | A0 | Never exceed the A0 limit of the exact board |

## Power and boot warnings

1. Servo and DFPlayer current peaks can reset the ESP8266. Use a supply with adequate peak current, short wiring, common ground and bulk capacitance near the load.
2. GPIO15/D8 is an ESP8266 boot-strap pin and must be LOW during boot. Keep the DFPlayer connection high impedance at reset and use the series resistor shown above.
3. ESP8266 GPIO pins are not 5 V tolerant. Power the PCF8574 and TTP223 signal chain from 3.3 V unless proper level shifting is used.
4. Different D1 mini revisions use different A0 input scaling. Do not connect a 4.2 V lithium cell directly to A0. Calculate and verify the divider with a multimeter.
5. The firmware constant `BATTERY_FACTOR` in `Config.h` must be calibrated for the actual board and divider.

## I2C addresses

| Device | Expected address |
|---|---|
| PCF8574 | `0x20` |
| SSD1306 OLED | Commonly `0x3C` |

If the display or touch input does not respond, run an I2C scanner and confirm that both addresses appear without conflict.
