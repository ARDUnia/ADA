# Testing and hardware simulation

The repository contains a host-side simulator under `tests/host`. It compiles the actual firmware modules against mocks for:

- PCF8574 / Wire touch input
- SSD1306 / U8g2 drawing bounds
- DFPlayer commands and volume
- Servo angle commands
- EEPROM persistence and corruption
- ESP8266 Wi-Fi state
- NTP formatted time
- Arduino timing and ADC input

The same test command also checks that the 15 required DFPlayer files exist under `assets/audio/MP3` and are non-empty.

## Run locally

Requirements: Bash and a C++17 compiler with AddressSanitizer and UndefinedBehaviorSanitizer.

```bash
tests/host/run_host_tests.sh
```

Validated result:

```text
ADA host hardware simulation PASS
Assertions: 83
Recognized gesture events: 19/19
Servo range: 45..134 degrees
DFPlayer MP3-folder commands: 18
Audio assets: 15/15
```

## Covered scenarios

- All 19 gesture patterns and their mappings
- Held long touch triggers exactly once
- Clock activation is not cancelled in the same loop
- Any new touch closes an existing clock screen
- Repeating the same face restarts the reaction timer and sound
- Reaction returns to normal after three seconds
- Battery view timeout
- Setup entry, editing, persistence, second-page layout and clean exit
- OLED Setup coordinates remain inside 128×64 bounds
- Settings CRC rejection and default recovery
- Wi-Fi credential CRC rejection
- Migration from the previous EEPROM credential format
- Maximum 32-byte SSID and 64-byte password round-trip
- Open Wi-Fi network credentials
- Wi-Fi connection and loss detection
- NTP `HH:MM:SS` formatting
- Dimmed, half-closed, sleeping and wake transitions
- Servo commands stay in the safe 0°–180° range
- All face audio tracks use deterministic MP3-folder commands
- All required audio assets use four-digit filenames and are present

## Limitation

Host simulation validates code logic but cannot reproduce supply noise, I2C wiring faults, touch sensitivity, servo stall current, speaker load or behavior differences between clone DFPlayer modules. A final physical smoke test is required after assembly.
