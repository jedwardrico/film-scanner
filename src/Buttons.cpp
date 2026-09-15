#include "Buttons.h"
#include "config.h"

void Buttons::begin() {
  buttons_[0].pin = PIN_BTN_NEXT;
  buttons_[0].event = InputEvent::NEXT;
  buttons_[1].pin = PIN_BTN_SELECT;
  buttons_[1].event = InputEvent::SELECT;
  buttons_[2].pin = PIN_BTN_FORWARD;
  buttons_[2].event = InputEvent::FORWARD;

  for (auto &b : buttons_) {
    pinMode(b.pin, INPUT_PULLUP);
    b.lastReading = digitalRead(b.pin);
    b.debounced = b.lastReading;
    b.lastChangeMs = millis();
  }
}

InputEvent Buttons::update() {
  uint32_t now = millis();

  for (auto &b : buttons_) {
    bool reading = digitalRead(b.pin);

    if (reading != b.lastReading) {
      b.lastChangeMs = now;
      b.lastReading = reading;
    }

    if ((now - b.lastChangeMs) >= BUTTON_DEBOUNCE_MS && reading != b.debounced) {
      b.debounced = reading;
      // Active LOW: a transition to LOW is a press.
      if (b.debounced == LOW) {
        return b.event;
      }
    }
  }

  return InputEvent::NONE;
}
