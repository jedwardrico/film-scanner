#include <Wire.h>
#include "Display.h"
#include "config.h"
#include "FilmTypes.h"

void Display::begin() {
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  u8g2_.begin();
  u8g2_.setFont(u8g2_font_6x12_tr);
}

void Display::drawRow(int y, bool selected, const char *label) {
  if (selected) {
    u8g2_.drawStr(0, y, ">");
  }
  u8g2_.drawStr(10, y, label);
}

void Display::render(ScannerState state, size_t selectedFilmIndex, uint32_t frameCount,
                      size_t calibFilmIndex, long calibSteps) {
  u8g2_.clearBuffer();

  switch (state) {
    case ScannerState::MENU: {
      // FILM_TYPE_COUNT film rows plus one "Calibrate..." row, so rows run
      // tighter (10px) than elsewhere to still fit the 64px-tall panel.
      u8g2_.drawStr(0, 10, "Select film type:");
      for (size_t i = 0; i <= FILM_TYPE_COUNT; i++) {
        int y = 20 + static_cast<int>(i) * 10;
        const char *label = (i < FILM_TYPE_COUNT) ? FILM_TYPES[i].name : "Calibrate...";
        drawRow(y, i == selectedFilmIndex, label);
      }
      break;
    }
    case ScannerState::CALIBRATE_SELECT: {
      u8g2_.drawStr(0, 10, "Calibrate which film?");
      for (size_t i = 0; i < FILM_TYPE_COUNT; i++) {
        int y = 20 + static_cast<int>(i) * 10;
        drawRow(y, i == calibFilmIndex, FILM_TYPES[i].name);
      }
      u8g2_.drawStr(0, 62, "SELECT=go BACK=cancel");
      break;
    }
    case ScannerState::CALIBRATE_JOG: {
      u8g2_.drawStr(0, 10, "Calibrating:");
      u8g2_.drawStr(70, 10, FILM_TYPES[calibFilmIndex].name);

      char buf[24];
      snprintf(buf, sizeof(buf), "Steps: %ld", calibSteps);
      u8g2_.drawStr(0, 30, buf);

      u8g2_.drawStr(0, 46, "FORWARD=jog");
      u8g2_.drawStr(0, 56, "SELECT=save BACK=cancel");
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
