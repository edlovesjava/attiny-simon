# ATtiny85 Simon – Hardware Wiring

## Pin Mapping

| ATtiny85 Pin | Physical Pin | Function          |
|--------------|--------------|-------------------|
| PB0          | 5            | LED 0 + Button 0  |
| PB1          | 6            | LED 1 + Button 1  |
| PB2          | 7            | LED 2 + Button 2  |
| PB3          | 2            | LED 3 + Button 3  |
| PB4          | 3            | Buzzer             |
| PB5 (RESET)  | 1            | Not used (keep as reset) |
| VCC          | 8            | +5V / +3.3V       |
| GND          | 4            | Ground             |

## ATtiny85 Pinout Reference

```
              ┌──────┐
  (RESET) PB5 │1    8│ VCC
          PB3 │2    7│ PB2
          PB4 │3    6│ PB1
          GND │4    5│ PB0
              └──────┘
```

## Wiring Per Shared Pin (PB0–PB3)

Each of the 4 game channels shares one pin for both the LED and the button:

```
            ┌─── [220R] ── LED ── GND
  PBx ──────┤
            └─── [Button] ─────── GND
```

- **220 ohm resistor** in series with each LED limits current and prevents the LED from interfering with button reads.
- **Button** connects the pin directly to GND when pressed.
- The firmware switches the pin between OUTPUT (to drive the LED) and INPUT_PULLUP (to read the button). The internal pull-up is too weak to visibly light the LED.

## Buzzer (PB4)

```
  PB4 ── [Piezo Buzzer] ── GND
```

A passive piezo buzzer is driven directly from PB4 using `tone()`.

## Power

- **3x AA batteries** (4.5V) or **USB 5V** to VCC/GND.
- Add a **100nF bypass cap** between VCC and GND close to the chip.

## Full Schematic

```
                        VCC
                         │
                    ┌────┤ 100nF
                    │    │
              ┌─────┼────┘
              │     │
         ┌────┴──┐  │
    ┌────│1  VCC8│──┘
    │    │       │
    │  ┌─│2   PB2│─7─┬──[220R]──LED2──GND
    │  │ │       │   └──[BTN2]─────────GND
    │  │ │       │
    │  │ │3   PB1│─6─┬──[220R]──LED1──GND
    │  │ │       │   └──[BTN1]─────────GND
    │  └─│4      │
    │    │  GND  │─5─┬──[220R]──LED0──GND
    │    └───┬───┘   └──[BTN0]─────────GND
    │        │
    │       GND
    │
    └── (RESET, leave unconnected or tie to VCC with 10K)

  Pin 2 (PB3) ─┬──[220R]──LED3──GND
                └──[BTN3]─────────GND

  Pin 3 (PB4) ──[Piezo Buzzer]──GND
```

## BOM (Bill of Materials)

| Qty | Component              |
|-----|------------------------|
| 1   | ATtiny85 (DIP-8)       |
| 4   | LEDs (any color)       |
| 4   | 220 ohm resistors      |
| 4   | Tactile push buttons   |
| 1   | Passive piezo buzzer   |
| 1   | 100nF ceramic capacitor|
| 1   | 8-pin DIP socket (optional) |
| 1   | Battery holder (3xAA) or USB power |
