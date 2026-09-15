#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

enum class ScannerState {
  MENU,             // choosing film type, or the "Calibrate..."/"Settings..." entries
  READY,            // film type selected, waiting for a movement/shutter button
  ADVANCING,        // stepper is moving (frame advance/back-up or a jog)
  SHUTTER,          // shutter trigger pulse is active
  CALIBRATE_SELECT, // choosing which film type to (re)calibrate
  CALIBRATE_JOG,    // jogging the stepper to find that film's frame pitch
  SETTINGS,         // adjusting the jog step count / auto-fire-on-advance
};

// Renders the current UI screen. Owns the U8g2 display driver instance;
// swap the constructor in Display.cpp if you use a different panel.
class Display {
 public:
  void begin();

  // `calibFilmIndex` and `calibSteps` are only meaningful in the
  // CALIBRATE_SELECT / CALIBRATE_JOG states. `settingsCursor`,
  // `settingsJogSteps` and `settingsAutoFire` are only meaningful in the
  // SETTINGS state.
  void render(ScannerState state, size_t selectedFilmIndex, uint32_t frameCount,
              size_t calibFilmIndex, long calibSteps, int settingsCursor,
              long settingsJogSteps, bool settingsAutoFire);

 private:
  void drawRow(int y, bool selected, const char *label);

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2_{U8G2_R0, U8X8_PIN_NONE};
};
