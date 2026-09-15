#include <Arduino.h>
#include "config.h"
#include "FilmTypes.h"
#include "FilmCalibration.h"
#include "Settings.h"
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
Settings settings;

ScannerState state = ScannerState::MENU;
size_t selectedFilmIndex = 0;
uint32_t frameCount = 0;
bool displayDirty = true;

// Only meaningful while state is CALIBRATE_SELECT / CALIBRATE_JOG.
size_t calibFilmIndex = 0;
long calibSteps = 0;

// Only meaningful while state is ADVANCING: whether the move in progress
// should auto-fire the shutter once it completes.
bool advancingWillAutoFire = false;

// Only meaningful while state is SETTINGS: working copies of the settings
// being edited, applied to `settings` only on SELECT (save).
int settingsCursor = 0;
long settingsJogSteps = 0;
bool settingsAutoFire = false;

void handleMenuInput(InputEvent event) {
  switch (event) {
    case InputEvent::NEXT:
      // Two extra entries beyond the film types: "Calibrate..." and "Settings...".
      selectedFilmIndex = (selectedFilmIndex + 1) % (FILM_TYPE_COUNT + 2);
      displayDirty = true;
      break;
    case InputEvent::SELECT:
      if (selectedFilmIndex < FILM_TYPE_COUNT) {
        frameCount = 0;
        state = ScannerState::READY;
      } else if (selectedFilmIndex == FILM_TYPE_COUNT) {
        calibFilmIndex = 0;
        state = ScannerState::CALIBRATE_SELECT;
      } else {
        settingsJogSteps = settings.jogSteps();
        settingsAutoFire = settings.autoFireOnAdvance();
        settingsCursor = 0;
        state = ScannerState::SETTINGS;
      }
      displayDirty = true;
      break;
    default:
      break;
  }
}

void handleReadyInput(InputEvent event) {
  switch (event) {
    case InputEvent::FRAME_FORWARD:
      advancingWillAutoFire = settings.autoFireOnAdvance();
      stepper.startAdvanceSteps(filmCalibration.stepsForFilm(selectedFilmIndex));
      state = ScannerState::ADVANCING;
      displayDirty = true;
      break;
    case InputEvent::FRAME_BACKWARD:
      advancingWillAutoFire = false;
      stepper.startAdvanceSteps(-filmCalibration.stepsForFilm(selectedFilmIndex));
      state = ScannerState::ADVANCING;
      displayDirty = true;
      break;
    case InputEvent::JOG_FORWARD:
      advancingWillAutoFire = false;
      stepper.startAdvanceSteps(settings.jogSteps());
      state = ScannerState::ADVANCING;
      displayDirty = true;
      break;
    case InputEvent::JOG_BACKWARD:
      advancingWillAutoFire = false;
      stepper.startAdvanceSteps(-settings.jogSteps());
      state = ScannerState::ADVANCING;
      displayDirty = true;
      break;
    case InputEvent::SHUTTER:
      shutter.trigger();
      state = ScannerState::SHUTTER;
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

void handleSettingsInput(InputEvent event) {
  switch (event) {
    case InputEvent::NEXT:
      settingsCursor = (settingsCursor + 1) % 2;
      displayDirty = true;
      break;
    case InputEvent::JOG_FORWARD:
      if (settingsCursor == 0) {
        settingsJogSteps += JOG_STEP_ADJUST_INCREMENT;
      } else {
        settingsAutoFire = true;
      }
      displayDirty = true;
      break;
    case InputEvent::JOG_BACKWARD:
      if (settingsCursor == 0) {
        settingsJogSteps = max(settingsJogSteps - JOG_STEP_ADJUST_INCREMENT,
                                JOG_STEP_ADJUST_INCREMENT);
      } else {
        settingsAutoFire = false;
      }
      displayDirty = true;
      break;
    case InputEvent::SELECT:
      settings.setJogSteps(settingsJogSteps);
      settings.setAutoFireOnAdvance(settingsAutoFire);
      state = ScannerState::MENU;
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
    case InputEvent::FRAME_FORWARD:
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
  settings.begin();

  display.render(state, selectedFilmIndex, frameCount, calibFilmIndex, calibSteps,
                  settingsCursor, settingsJogSteps, settingsAutoFire);
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

    case ScannerState::SETTINGS:
      handleSettingsInput(event);
      break;

    case ScannerState::ADVANCING:
      stepper.update();
      if (!stepper.isMoving()) {
        if (advancingWillAutoFire) {
          shutter.trigger();
          state = ScannerState::SHUTTER;
        } else {
          state = ScannerState::READY;
        }
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
    display.render(state, selectedFilmIndex, frameCount, calibFilmIndex, calibSteps,
                    settingsCursor, settingsJogSteps, settingsAutoFire);
    displayDirty = false;
    lastRenderMs = now;
  }
}
