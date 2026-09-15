#include <Arduino.h>
#include "config.h"
#include "FilmTypes.h"
#include "InputEvent.h"
#include "Buttons.h"
#include "StepperControl.h"
#include "ShutterControl.h"
#include "Display.h"

namespace {

Buttons buttons;
StepperControl stepper;
ShutterControl shutter;
Display display;

ScannerState state = ScannerState::MENU;
size_t selectedFilmIndex = 0;
uint32_t frameCount = 0;
bool displayDirty = true;

void handleMenuInput(InputEvent event) {
  switch (event) {
    case InputEvent::NEXT:
      selectedFilmIndex = (selectedFilmIndex + 1) % FILM_TYPE_COUNT;
      displayDirty = true;
      break;
    case InputEvent::SELECT:
      frameCount = 0;
      state = ScannerState::READY;
      displayDirty = true;
      break;
    default:
      break;
  }
}

void handleReadyInput(InputEvent event) {
  switch (event) {
    case InputEvent::FORWARD:
      stepper.startFrameAdvance(FILM_TYPES[selectedFilmIndex].framePitchMm);
      state = ScannerState::ADVANCING;
      displayDirty = true;
      break;
    case InputEvent::BACK:
      state = ScannerState::MENU;
      displayDirty = true;
      break;
    default:
      break;
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);

  buttons.begin();
  display.begin();
  stepper.begin();
  shutter.begin();

  display.render(state, selectedFilmIndex, frameCount);
}

void loop() {
  InputEvent event = buttons.update();

  switch (state) {
    case ScannerState::MENU:
      handleMenuInput(event);
      break;

    case ScannerState::READY:
      handleReadyInput(event);
      break;

    case ScannerState::ADVANCING:
      stepper.update();
      if (!stepper.isMoving()) {
        shutter.trigger();
        state = ScannerState::SHUTTER;
        displayDirty = true;
      }
      break;

    case ScannerState::SHUTTER:
      shutter.update();
      if (!shutter.isActive()) {
        frameCount++;
        state = ScannerState::READY;
        displayDirty = true;
      }
      break;
  }

  static uint32_t lastRenderMs = 0;
  uint32_t now = millis();
  if (displayDirty || (now - lastRenderMs) >= DISPLAY_REFRESH_MS) {
    display.render(state, selectedFilmIndex, frameCount);
    displayDirty = false;
    lastRenderMs = now;
  }
}
