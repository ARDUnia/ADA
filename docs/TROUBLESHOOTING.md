# Troubleshooting

## OLED is blank

- Confirm 3.3 V and common ground.
- Scan the I2C bus; SSD1306 is commonly `0x3C`.
- Check SDA = GPIO4/D2 and SCL = GPIO5/D1.
- Make sure the OLED is 128×64, not 128×32.

## Touch sensors do not respond

- Confirm PCF8574 appears at `0x20`.
- Connect address pins A0/A1/A2 to ground.
- Confirm TTP223 outputs are active high and limited to 3.3 V.
- Verify the order P0=S1, P1=S2, P2=S3, P3=S4.
- Keep touch wiring away from servo and speaker power leads.

## Device resets when the servo or speaker works

- Use a regulated supply with enough peak current.
- Do not power the servo from the ESP8266 3.3 V pin.
- Connect grounds together.
- Add a 470–1000 µF capacitor near the servo/DFPlayer supply and 100 nF local decoupling.
- Shorten high-current wiring.

## DFPlayer has no sound or plays the wrong file

- Format the card as FAT32.
- Put files in `/MP3`, not only in the root.
- Use exactly `0001.mp3` through `0014.mp3` and `0020.mp3`.
- Cross serial lines: DFPlayer TX → GPIO13 and ESP GPIO15 → DFPlayer RX.
- Add a series resistor on the DFPlayer RX line.
- Confirm speaker wiring uses SPK1 and SPK2.

## ESP8266 does not boot

- Disconnect peripherals and test USB power first.
- GPIO15/D8 must remain LOW at boot; inspect the DFPlayer RX connection.
- Check for a weak supply or servo inrush.

## Wi-Fi portal does not appear

- Look for `ADA-Config` after the retry sequence finishes.
- Open `http://192.168.4.1` directly; mobile data may need to be disabled temporarily.
- Use `3TS4`, select `WiFi setup`, and press S3.
- If credentials fail CRC validation, firmware intentionally discards them and starts the portal.

## Clock displays `--:--:--`

- Confirm Wi-Fi is connected.
- Confirm the network allows UDP NTP traffic.
- Wait for the first successful NTP update.
- Check `NTP_UTC_OFFSET_SECONDS` for the intended time zone.

## Battery reading is wrong

- Never assume A0 scaling is identical across boards.
- Measure the divider output and battery voltage.
- Recalculate `BATTERY_FACTOR` as described in `CONFIGURATION.md`.
