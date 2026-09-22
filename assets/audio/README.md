# ADA sound effects

This directory contains the 15 sound effects used by ADA. The `MP3` folder is ready to copy to the root of a FAT32-formatted microSD card for DFPlayer Mini.

## Installation

1. Format the microSD card as FAT32.
2. Copy this directory's `MP3` folder to the root of the card.
3. Confirm that the resulting paths are `/MP3/0001.mp3` through `/MP3/0014.mp3`, plus `/MP3/0020.mp3`.
4. Insert the card into DFPlayer Mini while power is off, then power the project on.

The firmware calls `playMp3Folder()`, so the four-digit filename and the `MP3` directory are significant.

## Track map

| Files | Purpose |
|---|---|
| `0001.mp3`–`0014.mp3` | Touch-gesture reaction sounds; see [`docs/GESTURES.md`](../../docs/GESTURES.md) |
| `0020.mp3` | Startup sound |

All supplied files were decode-checked after extraction. They are mono MP3 audio at 11025 Hz.

## Rights

These audio assets are provided as part of the ADA project by the project owner. The repository currently has no open-source license; reuse or redistribution requires permission until an explicit license is added.

Project websites: [ardunia.ir](https://ardunia.ir) · [hrmnia.com](https://hrmnia.com)
