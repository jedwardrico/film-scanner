#include "ShutterControl.h"
#include "config.h"

void ShutterControl::begin() {
  pinMode(PIN_SHUTTER_TRIGGER, OUTPUT);
  setLine(false);
}

void ShutterControl::trigger() {
  if (active_) return;
  active_ = true;
  pulseStartMs_ = millis();
  setLine(true);
}

void ShutterControl::update() {
  if (active_ && (millis() - pulseStartMs_) >= SHUTTER_PULSE_MS) {
    setLine(false);
    active_ = false;
  }
}

void ShutterControl::setLine(bool active) {
  bool level = SHUTTER_ACTIVE_HIGH ? active : !active;
  digitalWrite(PIN_SHUTTER_TRIGGER, level ? HIGH : LOW);
}
