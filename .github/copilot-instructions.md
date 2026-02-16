# Copilot Instructions for `attiny-simon`

## Current repository state
- The repository currently contains only `README.md` and Git metadata.
- `README.md` defines the product intent: a Simon Says game on **ATtiny85** using:
  - 4 buttons
  - 4 LEDs
  - piezo buzzer
- No source tree, build scripts, dependency manifests, or test setup are present yet.

## What this means for AI coding agents
- Treat this repo as an **early scaffold**.
- Use **PlatformIO** as the expected firmware toolchain for build/upload workflows.
- Do not assume an existing source layout until files are added, but prefer PlatformIO defaults when scaffolding (for example: `platformio.ini`, `src/`).
- Ground every implementation decision in explicit user requests or newly added files.

## Proven project context (from `README.md`)
- Target domain is embedded firmware/game logic for ATtiny85 hardware.
- Core behavior is Simon sequence generation, user input checking, LED feedback, and buzzer sound.

## Workflow guidance for this repo
- No build/test files are committed yet, but the intended workflow is PlatformIO.
- For firmware tasks, prefer PlatformIO commands and configuration over ad-hoc AVR tool invocations.
- If adding initial firmware code, include `platformio.ini` plus minimal build/upload usage notes.
- Flashing is done with an external programmer to ATtiny85; define `upload_protocol` and related programmer settings in `platformio.ini` instead of hard-coding assumptions in source.

## Conventions to follow right now
- Keep changes minimal and repository-specific.
- Prefer adding foundational files incrementally (for example: `platformio.ini`, `src/main.cpp`, flashing notes) only when requested.
- When introducing structure, explain why (e.g., small AVR firmware footprint, deterministic timing for game loop).

## Key references
- Project intent: `README.md`

## Maintenance note
- Update this file as soon as toolchain config, hardware pin mappings, or programmer settings are added.
- Once `platformio.ini` exists, replace high-level workflow text with exact project commands (for example build/upload targets used by this repo).