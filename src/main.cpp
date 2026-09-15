#include <Arduino.h>
#include "config.h"
#include "FilmTypes.h"
#include "FilmCalibration.h"
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
FilmCalibration filmCalibration;

ScannerState state = ScannerState::MENU;
size_t selectedFilmIndex = 0;
uint32_t frameCount = 0;
bool displayDirty = true;

// Only meaningful while state is CALIBRATE_SELECT / CALIBRATE_JOG.
size_t calibFilmIndex = 0;
long calibSteps = 0;

void handleMenuInput(InputEvent event) {
  switch (event) {
    case InputEvent::NEXT:
      // One extra entry beyond the film types: "Calibrate...".
      selectedFilmIndex = (selectedFilmIndex + 1) % (FILM_TYPE_COUNT + 1);
      displayDirty = true;
      break;
    case InputEvent::SELECT:
      if (selectedFilmIndex < FILM_TYPE_COUNT) {
        frameCount = 0;
        state = ScannerState::READY;
      } else {
        calibFilmIndex = 0;
        state = ScannerState::CALIBRATE_SELECT;
      }
      displayDirty = true;
      break;
    default:
      break;
  }
}

void handleReadyInput(InputEvent event) {
  switch (event) {
    case InputEvent::FORWARD:
      stepper.startAdvanceSteps(filmCalibration.stepsForFilm(selectedFilmIndex));
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

void handleCalibrateSelectInput(InputEvent event) {
  switch (event) {
    case InputEvent::NEXT:
      calibFilmIndex = (calibFilmIndex + 1) % FILM_TYPE_COUNT;
      displayDirty = true;
      break;
    case InputEvent::SELECT:
      calibSteps = 0;
      state = ScannerState::CALIBRATE_JOG;
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

void handleCalibrateJogInput(InputEvent event) {
  switch (event) {
    case InputEvent::FORWARD:
      // Ignore repeat presses until the previous jog has finished, so
      // presses map 1:1 to CALIBRATION_JOG_STEPS increments.
      if (!stepper.isMoving()) {
        stepper.startAdvanceSteps(CALIBRATION_JOG_STEPS);
        calibSteps += CALIBRATION_JOG_STEPS;
        displayDirty = true;
      }
      break;
    case InputEvent::SELECT:
      // Require at least one jog so a stray SELECT can't save a zero-step
      // (no-op) calibration.
      if (calibSteps > 0) {
        filmCalibration.setStepsForFilm(calibFilmIndex, calibSteps);
        state = ScannerState::MENU;
        displayDirty = true;
      }
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
  filmCalibration.begin();

  display.render(state, selectedFilmIndex, frameCount, calibFilmIndex, calibSteps);
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

    case ScannerState::CALIBRATE_SELECT:
      handleCalibrateSelectInput(event);
      break;

    case ScannerState::CALIBRATE_JOG:
      stepper.update();
      handleCalibrateJogInput(event);
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
    display.render(state, selectedFilmIndex, frameCount, calibFilmIndex, calibSteps);
    displayDirty = false;
    lastRenderMs = now;
  }
}
