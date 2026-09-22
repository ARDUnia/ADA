# Touch gestures and reactions

Sensors are numbered from left to right as S1, S2, S3 and S4.

Notation:

- `1TS1`: one short tap on sensor 1
- `3TS2`: three short taps on sensor 2
- `LT4`: long touch on sensor 4
- `L2R`: swipe from left to right
- `R2L`: swipe from right to left
- `RLLR`: left-to-right followed by right-to-left

## Complete mapping

| Gesture | Result | Audio file |
|---|---|---|
| `L2R` | Slow head movement to the right | — |
| `R2L` | Slow head movement to the left | — |
| `RLLR` | Ecstasy face | `/MP3/0009.mp3` |
| `1TS1` | Happy | `/MP3/0001.mp3` |
| `3TS1` | Angry | `/MP3/0002.mp3` |
| `LT1` | Thinking | `/MP3/0003.mp3` |
| `LT2` | Sleepy | `/MP3/0004.mp3` |
| `2TS3` | Surprised | `/MP3/0005.mp3` |
| `3TS3` | Sad | `/MP3/0006.mp3` |
| `2TS1` | Wink | `/MP3/0007.mp3` |
| `3TS2` | Fear | `/MP3/0008.mp3` |
| `LT3` | Eating | `/MP3/0010.mp3` |
| `1TS2` | Rapid blink | `/MP3/0011.mp3` |
| `1TS3` | Sick | `/MP3/0012.mp3` |
| `2TS2` | Hungry | `/MP3/0013.mp3` |
| `1TS4` | XO face | `/MP3/0014.mp3` |
| `2TS4` | Show internet time for five seconds | — |
| `LT4` | Show battery status for three seconds | — |
| `3TS4` | Open Setup menu | — |

## Timing constants

The defaults in `Touch.cpp` are:

- Short touch maximum: 700 ms
- Long touch threshold: 1000 ms
- Multi-tap/gesture completion gap: 600 ms

A recognized long touch is locked until all sensors are released. This prevents the command from repeating while the finger remains on the sensor.
