#pragma once

#include <Arduino.h>

// Drives the shutter trigger output as a timed, non-blocking pulse.
class ShutterControl {
 public:
  void begin();

  // Starts a pulse of SHUTTER_PULSE_MS on the trigger pin. No-op if a pulse
  // is already in progress.
  void trigger();

  // Must be called every loop() iteration to end the pulse on time.
  void update();

  bool isActive() const { return active_; }

 private:
  bool active_ = false;
  uint32_t pulseStartMs_ = 0;

  void setLine(bool active);
};
