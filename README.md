# film-scanner

Firmware for a motorized film-scanning transport: an ESP32-S (S2/S3) drives a
NEMA 17 stepper through a TMC2209 driver to advance film by exactly one frame
at a time, and fires an opto-isolated trigger into a camera's shutter contact
to capture each frame. Film type (Super 8, Standard 8, 16mm, 35mm) is chosen
on an OLED menu; a single "forward" button does the advance-then-shoot cycle.

## Hardware

| Signal                     | ESP32-S3 pin (default, see `include/config.h`) |
|-----------------------------|-------------------------------------------------|
| TMC2209 STEP                | 4                                                 |
| TMC2209 DIR                 | 5                                                 |
| TMC2209 ENABLE (active LOW) | 6                                                 |
| TMC2209 UART TX             | 17 (through ~1k resistor to PDN_UART)             |
| TMC2209 UART RX             | 18 (direct to PDN_UART)                           |
| OLED SDA (SSD1306, I2C)     | 8                                                 |
| OLED SCL                    | 9                                                 |
| Button: NEXT                | 10 (to GND, internal pull-up)                     |
| Button: SELECT              | 11 (to GND, internal pull-up)                     |
| Button: FORWARD             | 12 (to GND, internal pull-up)                     |
| Shutter trigger out          | 13                                                |

All pins are defined once in `include/config.h` — change them there to match
your wiring; nothing else in the code needs to know pin numbers.

**TMC2209 UART**: single-wire (PDN_UART) mode is used for configuring current,
microstepping and StealthChop, so no MS1/MS2 microstep-select wiring is
needed. Two ESP32 GPIOs (RX/TX) share the one PDN_UART net; put a resistor
(~1k) between the TX pin and the net so the driver's own reply doesn't fight
the ESP32's TX driver. Set the driver address via MS1/MS2 strapping if you
run more than one driver on the same bus, and update `STEPPER_UART_ADDRESS`.

**Shutter trigger**: `PIN_SHUTTER_TRIGGER` should drive an opto-isolator
(e.g. PC817) wired across the camera's shutter-release contacts, not the
contacts directly — this keeps the ESP32 and camera electrically isolated,
which matters for cameras with a hot shoe/flash sync or a shutter contact
referenced to a different ground than your scanner electronics.

**Motor/drivetrain**: `MM_PER_MOTOR_REV` in `config.h` must match your actual
mechanism (capstan/roller diameter, pulley+belt ratio, or sprocket pitch
circle) — it's a placeholder. Calibrate it by commanding a known number of
motor revolutions and measuring actual film travel, then adjust until the
computed `STEPS_PER_MM` produces accurate frame spacing.

## Firmware architecture

- `include/config.h` — all pins, motor/driver tuning, and timing constants.
- `include/FilmTypes.h` — table of film formats and their frame pitch (mm).
- `include/InputEvent.h` — logical input events (`NEXT`/`PREV`/`SELECT`/
  `BACK`/`FORWARD`), decoupled from whatever physical control produces them.
- `Buttons` — debounces the three momentary buttons and emits `InputEvent`s.
  A rotary encoder or extra buttons can be added later as another producer
  of the same `InputEvent` enum, with no changes to the state machine.
- `StepperControl` — configures the TMC2209 over UART (current, microsteps,
  StealthChop) and drives step/dir pulses via AccelStepper for acceleration-
  limited, non-blocking frame moves.
- `ShutterControl` — fires a timed, non-blocking pulse on the trigger pin.
- `Display` — renders the menu/ready/advancing/shutter screens on an SSD1306
  OLED via U8g2 (swap the panel driver in `Display.h`/`.cpp` if you use a
  different screen).
- `src/main.cpp` — the state machine: `MENU -> READY -> ADVANCING -> SHUTTER
  -> READY -> ...`. Everything is polled non-blockingly in `loop()` (no
  `delay()`), so buttons stay responsive while the motor is moving.

## Build

This is a [PlatformIO](https://platformio.org/) project.

```sh
pio run                       # build (default env: esp32-s3-devkitc-1)
pio run -e esp32-s2-saola-1    # or build for an ESP32-S2 board
pio run -t upload             # flash
pio device monitor             # serial monitor
```

Add a board-specific `[env:...]` section in `platformio.ini` if you're using
a different ESP32-S board than the two provided.

> Note: this firmware has been reviewed for correctness but not build-tested
> in this environment (no network access to the PlatformIO package registry
> here). Run `pio run` on your machine before flashing to catch anything a
> manual review could miss.

## Extending

- **Rotary encoder**: add an `Encoder` producer that emits `InputEvent::NEXT`/
  `PREV`/`SELECT` the same way `Buttons` does, and poll it alongside
  `buttons.update()` in `loop()`.
- **More film types**: add entries to `FILM_TYPES` in `include/FilmTypes.h`.
- **Reverse/jog**: `StepperControl` can be extended with a signed move helper
  built on the same `AccelStepper` instance for manual positioning.
