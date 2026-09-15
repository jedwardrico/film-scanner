# film-scanner

Firmware for a motorized film-scanning transport: an ESP32-S (S2/S3) drives a
NEMA 17 stepper through a TMC2209 driver to advance film by exactly one frame
at a time, and fires an opto-isolated trigger into a camera's shutter contact
to capture each frame. Film type (35mm, 6x9, 6x6, 6x4.5) is chosen on an
OLED menu. Seven buttons handle input: two for menu navigation, four for
transport movement (whole-frame forward/backward and small jog
forward/backward), and one dedicated shutter button.

## Hardware

| Signal                       | ESP32-S3 pin (default, see `include/config.h`) |
|-------------------------------|-------------------------------------------------|
| TMC2209 STEP                  | 4                                                 |
| TMC2209 DIR                   | 5                                                 |
| TMC2209 ENABLE (active LOW)   | 6                                                 |
| TMC2209 UART TX                | 17 (through ~1k resistor to PDN_UART)             |
| TMC2209 UART RX                | 18 (direct to PDN_UART)                           |
| OLED SDA (SSD1306, I2C)       | 8                                                 |
| OLED SCL                      | 9                                                 |
| Button: NEXT                  | 10 (to GND, internal pull-up)                     |
| Button: SELECT                | 11 (to GND, internal pull-up)                     |
| Button: FRAME FORWARD         | 12 (to GND, internal pull-up)                     |
| Button: FRAME BACKWARD        | 14 (to GND, internal pull-up)                     |
| Button: JOG FORWARD           | 15 (to GND, internal pull-up)                     |
| Button: JOG BACKWARD          | 16 (to GND, internal pull-up)                     |
| Button: SHUTTER               | 21 (to GND, internal pull-up)                     |
| Shutter trigger out            | 13                                                |

Button pins are placeholder assignments — adjust them in `include/config.h`
once your buttons are physically wired up; nothing else needs to change.

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
circle) — it's a placeholder. Get it roughly right by commanding a known
number of motor revolutions and measuring actual film travel; per-film-type
frame spacing is then fine-tuned with the on-device calibration menu below,
so `MM_PER_MOTOR_REV` only needs to be close, not exact.

## Calibration

`FILM_TYPES` in `include/FilmTypes.h` gives each film format a nominal frame
pitch, but real transports (belt slip, roller diameter tolerance, backlash)
drift from nominal. Rather than editing and reflashing that table, each film
type's actual frame-advance step count can be calibrated on-device and is
stored in flash (NVS), independent of the firmware image:

1. From the film-type menu, cycle with NEXT past the last film type to the
   **Calibrate...** entry and press SELECT.
2. Pick the film type to (re)calibrate with NEXT, then SELECT.
3. Load a strip of that film and press FRAME FORWARD repeatedly — each press
   jogs the transport forward by `CALIBRATION_JOG_STEPS` (see `config.h`).
   Watch the film and stop once exactly one frame has advanced.
4. Press SELECT to save the accumulated step count for that film type, or
   BACK to cancel without saving.

The saved value overrides the nominal `framePitchMm`-derived default for
that film type until it's recalibrated again; other film types are
unaffected. This is handled by `FilmCalibration` (`include/FilmCalibration.h`,
`src/FilmCalibration.cpp`), which wraps the ESP32 `Preferences` (NVS) API.

## Ready screen controls

Once a film type is selected (`READY` state):

- **FRAME FORWARD** / **FRAME BACKWARD** — move the transport exactly one
  calibrated frame in that direction.
- **JOG FORWARD** / **JOG BACKWARD** — nudge the transport by the configured
  jog step count (see Settings below), for fine manual positioning.
- **SHUTTER** — fires the shutter immediately, without moving the transport.
- **BACK** — return to the film-type menu.

By default, moving the film never fires the shutter on its own — SHUTTER is
always a separate press. Enable "Auto-fire" in Settings to restore the
original combined behavior, where FRAME FORWARD advances a frame and then
fires the shutter automatically once the move completes; FRAME BACKWARD and
both JOG buttons never auto-fire, regardless of that setting.

## Settings

From the film-type menu, cycle with NEXT past **Calibrate...** to the
**Settings...** entry and press SELECT. On the Settings screen:

- **NEXT** moves the cursor between the "Jog steps" and "Auto-fire" fields.
- **JOG FORWARD** / **JOG BACKWARD** adjust the highlighted field (the jog
  step count up/down by `JOG_STEP_ADJUST_INCREMENT`, or toggle auto-fire).
- **SELECT** saves both fields and returns to the menu; **BACK** discards
  any changes.

Both settings are persisted in flash (NVS) by `Settings`
(`include/Settings.h`, `src/Settings.cpp`) and survive reboots.

## Firmware architecture

- `include/config.h` — all pins, motor/driver tuning, and timing constants.
- `include/FilmTypes.h` — table of film formats and their nominal frame
  pitch (mm), used as the calibration default.
- `FilmCalibration` — persists each film type's calibrated frame-advance
  step count in flash (NVS) and falls back to the nominal value from
  `FilmTypes.h` until that film type has been calibrated. See "Calibration"
  below.
- `Settings` — persists the jog step count and the auto-fire-on-advance
  toggle in flash (NVS). See "Settings" above.
- `include/InputEvent.h` — logical input events (`NEXT`/`PREV`/`SELECT`/
  `BACK`/`FRAME_FORWARD`/`FRAME_BACKWARD`/`JOG_FORWARD`/`JOG_BACKWARD`/
  `SHUTTER`), decoupled from whatever physical control produces them.
- `Buttons` — debounces the seven momentary buttons and emits `InputEvent`s.
  A rotary encoder or extra buttons can be added later as another producer
  of the same `InputEvent` enum, with no changes to the state machine.
- `StepperControl` — configures the TMC2209 over UART (current, microsteps,
  StealthChop) and drives step/dir pulses via AccelStepper for acceleration-
  limited, non-blocking moves; `startAdvanceSteps()` takes a signed step
  count, so the same call drives forward frame advances, backward moves,
  and jogs in either direction.
- `ShutterControl` — fires a timed, non-blocking pulse on the trigger pin.
- `Display` — renders the menu/ready/advancing/shutter/settings screens on
  an SSD1306 OLED via U8g2 (swap the panel driver in `Display.h`/`.cpp` if
  you use a different screen).
- `src/main.cpp` — the state machine: `MENU -> READY -> ADVANCING -> (SHUTTER
  if auto-fire) -> READY -> ...`, with side branches `MENU ->
  CALIBRATE_SELECT -> CALIBRATE_JOG -> MENU` for calibration and `MENU ->
  SETTINGS -> MENU` for settings. Everything is polled non-blockingly in
  `loop()` (no `delay()`), so buttons stay responsive while the motor is
  moving.

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
