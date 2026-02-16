# attiny-simon
Simon says game using ATtiny85 with four buttons, four LEDs and piezo buzzer for sound.

## Toolchain
This project uses **PlatformIO** for firmware build and flashing.

## Project layout
- `platformio.ini` - PlatformIO environment and upload settings
- `src/main.cpp` - firmware entrypoint and game logic

## Build and upload
1. Install PlatformIO Core (`pio`) or use VS Code + PlatformIO IDE extension.
2. Build firmware:
	- `pio run`
3. Flash using your external programmer:
	- `pio run -t upload`

## Programmer configuration
Default settings are in `platformio.ini`.
Update these values for your hardware if needed:
- `upload_protocol` (example: `usbasp`, `stk500v1`, `avrisp`)
- `board_build.f_cpu` (example: `1000000L` for internal 1 MHz, `8000000L` for internal 8 MHz)
