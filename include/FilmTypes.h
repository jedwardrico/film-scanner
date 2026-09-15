#pragma once

// Frame advance distances per film format, in millimeters (perforation pitch
// times perforations-per-frame). These are nominal values — verify against
// your actual stock/camera gate and adjust if frames drift over a reel.

struct FilmType {
  const char *name;
  float framePitchMm;
};

constexpr FilmType FILM_TYPES[] = {
    {"Super 8",    4.23f},   // 1 perf/frame
    {"Std 8mm",    3.81f},   // 1 perf/frame (double-8, pre-split pitch)
    {"16mm",       7.62f},   // 1 perf/frame, Type II/A perf pitch
    {"35mm (4p)", 18.96f},   // 4 perf/frame, 4.74mm perf pitch
};

constexpr size_t FILM_TYPE_COUNT = sizeof(FILM_TYPES) / sizeof(FILM_TYPES[0]);
