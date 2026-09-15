#include <cmath>
#include "StepperControl.h"

void StepperControl::begin() {
  pinMode(PIN_STEPPER_ENABLE, OUTPUT);
  digitalWrite(PIN_STEPPER_ENABLE, HIGH);  // driver disabled until we're configured

  uart_.begin(115200, SERIAL_8N1, PIN_STEPPER_UART_RX, PIN_STEPPER_UART_TX);
  driver_.begin();
  driver_.toff(4);
  driver_.rms_current(TMC_RMS_CURRENT_MA);
  driver_.microsteps(TMC_MICROSTEPS);
  driver_.pwm_autoscale(true);
  driver_.en_spreadCycle(!TMC_USE_STEALTHCHOP);

  stepper_.setMaxSpeed(STEPPER_MAX_SPEED_SPS);
  stepper_.setAcceleration(STEPPER_ACCEL_SPS2);
  stepper_.setCurrentPosition(0);

  digitalWrite(PIN_STEPPER_ENABLE, LOW);  // active LOW enable
}

void StepperControl::startFrameAdvance(float pitchMm) {
  long steps = lroundf(pitchMm * STEPS_PER_MM);
  stepper_.move(steps);
}

void StepperControl::update() { stepper_.run(); }

bool StepperControl::isMoving() const {
  return const_cast<AccelStepper &>(stepper_).distanceToGo() != 0;
}
