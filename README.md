# GDI8001

A PC-8001/PC-8801 emulator using Windows GDI for graphics.

## Quick start

1. Download and extract the repository ZIP.
2. Choose the executable for your system, for example `amd64/GDI8001.exe` for 64-bit Windows.
3. Put the ROM files below **beside that executable**, then start `GDI8001.exe` from the same folder. ROMs must be supplied separately.

## ROM setup

Add `font.rom` (2 KiB character font) for your chosen machine, plus the BASIC ROM files below. Use a separate folder for each machine model.

| Machine | BASIC ROM files |
| --- | --- |
| PC-8001 | `n80basic.rom` (24 KiB) |
| PC-8001mkII, combined ROM | `nbasic.rom` (32 KiB) |
| PC-8001mkII, split ROMs | `nbasic.rom` (24 KiB) **and** `n80.rom` (8 KiB) |
| PC-8801, separate ROMs | `n88basic.rom` (32 KiB), `n80basic.rom` (24 KiB) and `n88_0.rom` (8 KiB extension ROM) |

For PC-8001mkII, choose **one** of the two layouts above; the 32 KiB file already contains the extra 8 KiB. The presence of `n88basic.rom` or a supported combined `pc88.rom` selects PC-8801 mode automatically.

Optional ROMs go in the same folder:

- `n80s31.rom` (2 KiB): PC-80S31 or PC-8801 floppy controller firmware for low-level disk emulation.
- `kfont1.rom` and `kfont2.rom` (128 KiB each): level 1 and level 2 Kanji fonts.
- `n88_1.rom` through `n88_3.rom` (8 KiB each): additional PC-8801 extension ROMs.

Example: PC-8001mkII with a 32 KiB BASIC ROM and floppy firmware:

```text
amd64/
  GDI8001.exe
  font.rom
  nbasic.rom
  n80s31.rom
```

## Controls

- **F9:** select a CMT tape image, then use the emulated machine's tape-loading command.
- **F11:** reset the emulated machine.
- **F12:** toggle full screen.

The original builds target Windows Vista or later, or Wine on Unix-like systems/Android. Legacy builds are in `GDI8001(LegacyPC).7z`.
