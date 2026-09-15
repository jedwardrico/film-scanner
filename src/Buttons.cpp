#include "Buttons.h"
#include "config.h"

void Buttons::begin() {
  buttons_[0].pin = PIN_BTN_NEXT;
  buttons_[0].event = InputEvent::NEXT;
  buttons_[1].pin = PIN_BTN_SELECT;
  buttons_[1].event = InputEvent::SELECT;
  buttons_[2].pin = PIN_BTN_FRAME_FWD;
  buttons_[2].event = InputEvent::FRAME_FORWARD;
  buttons_[3].pin = PIN_BTN_FRAME_BACK;
  buttons_[3].event = InputEvent::FRAME_BACKWARD;
  buttons_[4].pin = PIN_BTN_JOG_FWD;
  buttons_[4].event = InputEvent::JOG_FORWARD;
  buttons_[5].pin = PIN_BTN_JOG_BACK;
  buttons_[5].event = InputEvent::JOG_BACKWARD;
  buttons_[6].pin = PIN_BTN_SHUTTER;
  buttons_[6].event = InputEvent::SHUTTER;

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
