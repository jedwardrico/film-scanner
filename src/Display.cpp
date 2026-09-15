#include <Wire.h>
#include "Display.h"
#include "config.h"
#include "FilmTypes.h"

void Display::begin() {
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  u8g2_.begin();
  u8g2_.setFont(u8g2_font_6x12_tr);
}

void Display::render(ScannerState state, size_t selectedFilmIndex, uint32_t frameCount) {
  u8g2_.clearBuffer();

  switch (state) {
    case ScannerState::MENU: {
      u8g2_.drawStr(0, 10, "Select film type:");
      for (size_t i = 0; i < FILM_TYPE_COUNT; i++) {
        int y = 24 + static_cast<int>(i) * 12;
        if (i == selectedFilmIndex) {
          u8g2_.drawStr(0, y, ">");
        }
        u8g2_.drawStr(10, y, FILM_TYPES[i].name);
      }
      u8g2_.drawStr(0, 62, "NEXT / SELECT");
      break;
    }
    case ScannerState::READY: {
      u8g2_.drawStr(0, 10, "Ready");
      u8g2_.drawStr(0, 24, FILM_TYPES[selectedFilmIndex].name);

      char buf[24];
      snprintf(buf, sizeof(buf), "Frame: %lu", static_cast<unsigned long>(frameCount));
      u8g2_.drawStr(0, 38, buf);

      u8g2_.drawStr(0, 62, "FORWARD to advance");
      break;
    }
    case ScannerState::ADVANCING:
      u8g2_.drawStr(0, 10, "Ready");
      u8g2_.drawStr(0, 32, "Advancing...");
      break;
    case ScannerState::SHUTTER:
      u8g2_.drawStr(0, 10, "Ready");
      u8g2_.drawStr(0, 32, "Shutter!");
      break;
  }

  u8g2_.sendBuffer();
}
