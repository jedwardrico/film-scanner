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
                      size_t calibFilmIndex, long calibSteps, int settingsCursor,
                      long settingsJogSteps, bool settingsAutoFire) {
  u8g2_.clearBuffer();

  switch (state) {
    case ScannerState::MENU: {
      // FILM_TYPE_COUNT film rows plus "Calibrate..." and "Settings..." rows,
      // so rows run tighter (9px, starting right under the header) than
      // elsewhere to still fit the 64px-tall panel.
      u8g2_.drawStr(0, 8, "Select film type:");
      for (size_t i = 0; i <= FILM_TYPE_COUNT + 1; i++) {
        int y = 18 + static_cast<int>(i) * 9;
        const char *label = (i < FILM_TYPE_COUNT)      ? FILM_TYPES[i].name
                             : (i == FILM_TYPE_COUNT)   ? "Calibrate..."
                                                         : "Settings...";
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

      u8g2_.drawStr(0, 46, "FRAME FWD=jog");
      u8g2_.drawStr(0, 56, "SELECT=save BACK=cancel");
      break;
    }
    case ScannerState::READY: {
      u8g2_.drawStr(0, 10, "Ready");
      u8g2_.drawStr(0, 24, FILM_TYPES[selectedFilmIndex].name);

      char buf[24];
      snprintf(buf, sizeof(buf), "Frame: %lu", static_cast<unsigned long>(frameCount));
      u8g2_.drawStr(0, 38, buf);

      u8g2_.drawStr(0, 50, "FWD/BACK=frame JOG=nudge");
      u8g2_.drawStr(0, 62, "SHUTTER=fire  BACK=menu");
      break;
    }
    case ScannerState::SETTINGS: {
      u8g2_.drawStr(0, 8, "Settings");

      char buf[24];
      snprintf(buf, sizeof(buf), "Jog steps: %ld", settingsJogSteps);
      drawRow(24, settingsCursor == 0, buf);

      snprintf(buf, sizeof(buf), "Auto-fire: %s", settingsAutoFire ? "ON" : "OFF");
      drawRow(36, settingsCursor == 1, buf);

      u8g2_.drawStr(0, 50, "NEXT=field JOG=adjust");
      u8g2_.drawStr(0, 62, "SELECT=save BACK=cancel");
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
