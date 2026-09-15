#include <cmath>
#include <cstdio>
#include <Preferences.h>
#include "FilmCalibration.h"
#include "FilmTypes.h"
#include "config.h"

namespace {

Preferences prefs;
constexpr char kNamespace[] = "filmcal";

// Keys are just the film's index into FILM_TYPES ("0", "1", ...) — stable
// as long as entries aren't reordered/removed in FilmTypes.h.
void keyFor(size_t index, char *buf, size_t bufLen) {
  snprintf(buf, bufLen, "%u", static_cast<unsigned>(index));
}

}  // namespace

void FilmCalibration::begin() { prefs.begin(kNamespace, /*readOnly=*/false); }

long FilmCalibration::defaultStepsForFilm(size_t index) {
  return lroundf(FILM_TYPES[index].framePitchMm * STEPS_PER_MM);
}

bool FilmCalibration::isCalibrated(size_t index) const {
  char key[8];
  keyFor(index, key, sizeof(key));
  return prefs.isKey(key);
}

long FilmCalibration::stepsForFilm(size_t index) const {
  char key[8];
  keyFor(index, key, sizeof(key));
  return prefs.getLong(key, defaultStepsForFilm(index));
}

void FilmCalibration::setStepsForFilm(size_t index, long steps) {
  char key[8];
  keyFor(index, key, sizeof(key));
  prefs.putLong(key, steps);
}
