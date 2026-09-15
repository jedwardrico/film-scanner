#pragma once

#include <cstddef>

// Persists per-film-type frame-advance step counts in NVS (flash), so a
// calibration survives reboots and each film type can be independently
// recalibrated later if the transport, gearing, or stock changes.
//
// Until a film type is calibrated, stepsForFilm() falls back to the
// nominal value derived from FILM_TYPES[index].framePitchMm and
// STEPS_PER_MM (see FilmTypes.h / config.h).
class FilmCalibration {
 public:
  void begin();

  bool isCalibrated(size_t index) const;

  // Steps needed to advance one frame of the film type at `index`.
  long stepsForFilm(size_t index) const;

  // Saves `steps` as the frame-advance distance for the film type at
  // `index`, overriding the nominal value. Calling this again for the
  // same index recalibrates it.
  void setStepsForFilm(size_t index, long steps);

 private:
  static long defaultStepsForFilm(size_t index);
};
