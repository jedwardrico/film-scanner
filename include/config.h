#pragma once

#include <Arduino.h>

// ---------------------------------------------------------------------------
// PIN MAP — adjust to match your wiring. Defaults assume an ESP32-S3 DevKitC-1.
// ---------------------------------------------------------------------------

// TMC2209 step/dir/enable (motion control pins, driven directly by the MCU)
constexpr uint8_t PIN_STEPPER_STEP    = 4;
constexpr uint8_t PIN_STEPPER_DIR     = 5;
constexpr uint8_t PIN_STEPPER_ENABLE  = 6;   // active LOW on most TMC2209 boards

// TMC2209 UART (single-wire, half-duplex). Put a ~1k resistor between the
// ESP32 TX pin and the driver's PDN_UART pin; tie RX to the same node.
constexpr uint8_t PIN_STEPPER_UART_TX = 17;
constexpr uint8_t PIN_STEPPER_UART_RX = 18;
constexpr uint8_t STEPPER_UART_ADDRESS = 0;  // set by MS1/MS2 strapping, 0 if both tied low

// SSD1306 128x64 I2C OLED
constexpr uint8_t PIN_OLED_SDA = 8;
constexpr uint8_t PIN_OLED_SCL = 9;

// Buttons (active LOW, wired to GND with internal pull-ups enabled)
constexpr uint8_t PIN_BTN_NEXT    = 10;  // cycle film type in the menu
constexpr uint8_t PIN_BTN_SELECT  = 11;  // confirm film type / return to menu
constexpr uint8_t PIN_BTN_FORWARD = 12;  // advance one frame + fire shutter

// Shutter trigger output. Do NOT wire this straight into a camera's shutter
// contacts — opto-isolate it (e.g. PC817) so the ESP32 and camera share no
// ground reference. This pin drives the opto's LED.
constexpr uint8_t PIN_SHUTTER_TRIGGER = 13;
constexpr bool SHUTTER_ACTIVE_HIGH = true;   // set false if your opto is wired inverted
constexpr uint32_t SHUTTER_PULSE_MS = 150;   // how long the trigger line stays active

// ---------------------------------------------------------------------------
// MOTOR / DRIVER CONFIGURATION
// ---------------------------------------------------------------------------

constexpr float TMC_R_SENSE = 0.11f;      // sense resistor on most generic TMC2209 boards
constexpr uint16_t TMC_RMS_CURRENT_MA = 800;  // set to your NEMA 17's rated current (or a bit under)
constexpr uint16_t TMC_MICROSTEPS = 16;
constexpr bool TMC_USE_STEALTHCHOP = true;    // quiet, lower max torque; set false for spreadCycle

constexpr uint16_t MOTOR_FULL_STEPS_PER_REV = 200;  // 1.8 deg/step NEMA 17

// Mechanical advance per motor revolution, in millimeters. This depends on
// your drivetrain (capstan/roller diameter, pulley + belt pitch, or sprocket
// pitch circle) — measure or calculate it for your build, then recalibrate
// by jogging a known number of revolutions and measuring actual film travel.
constexpr float MM_PER_MOTOR_REV = 25.0f;

constexpr float STEPS_PER_MM =
    (MOTOR_FULL_STEPS_PER_REV * TMC_MICROSTEPS) / MM_PER_MOTOR_REV;

// Motion profile for a single frame advance.
constexpr float STEPPER_MAX_SPEED_SPS = 4000.0f;      // steps/sec
constexpr float STEPPER_ACCEL_SPS2    = 8000.0f;      // steps/sec^2

// ---------------------------------------------------------------------------
// MISC
// ---------------------------------------------------------------------------

constexpr uint32_t BUTTON_DEBOUNCE_MS = 30;
constexpr uint32_t DISPLAY_REFRESH_MS = 100;
