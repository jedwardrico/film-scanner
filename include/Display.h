#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

enum class ScannerState {
  MENU,       // choosing film type
  READY,      // film type selected, waiting for FORWARD
  ADVANCING,  // stepper is moving to the next frame
  SHUTTER,    // shutter trigger pulse is active
};

// Renders the current UI screen. Owns the U8g2 display driver instance;
// swap the constructor in Display.cpp if you use a different panel.
class Display {
 public:
  void begin();
  void render(ScannerState state, size_t selectedFilmIndex, uint32_t frameCount);

 private:
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2_{U8G2_R0, U8X8_PIN_NONE};
};
