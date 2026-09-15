#pragma once

#include <Arduino.h>
#include "InputEvent.h"

// Polls a set of active-LOW, pulled-up momentary buttons and reports one
// debounced press event per call to update(). Call update() every loop()
// iteration; it does not block.
class Buttons {
 public:
  void begin();
  InputEvent update();

 private:
  struct ButtonState {
    uint8_t pin;
    InputEvent event;
    bool lastReading = true;   // idle (pulled up) = HIGH
    bool debounced = true;
    uint32_t lastChangeMs = 0;
  };

  static constexpr int kCount = 7;
  ButtonState buttons_[kCount];
};
