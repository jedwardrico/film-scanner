#pragma once

#include <Arduino.h>
#include <AccelStepper.h>
#include <TMCStepper.h>
#include "config.h"

// Wraps TMC2209 UART configuration and AccelStepper-based step/dir motion
// generation behind a small frame-advance-oriented API.
class StepperControl {
 public:
  void begin();

  // Kicks off a non-blocking move of `pitchMm` millimeters. Call update()
  // every loop() iteration until isMoving() returns false.
  void startFrameAdvance(float pitchMm);

  // Pumps the step generator. Must be called frequently (every loop()
  // iteration) while a move is in progress.
  void update();

  bool isMoving() const;

 private:
  HardwareSerial uart_{1};
  TMC2209Stepper driver_{&uart_, TMC_R_SENSE, STEPPER_UART_ADDRESS};
  AccelStepper stepper_{AccelStepper::DRIVER, PIN_STEPPER_STEP, PIN_STEPPER_DIR};
};
