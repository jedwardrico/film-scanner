#pragma once

// Frame advance distances per film format, in millimeters. For 35mm this is
// perforation pitch times perforations-per-frame; medium format (120/220
// roll film) has no perforations, so its pitch is the nominal frame length
// along the film plus a typical inter-frame gap. These are nominal values —
// verify against your actual stock/camera gate and adjust if frames drift
// over a reel.

struct FilmType {
  const char *name;
  float framePitchMm;
};

constexpr FilmType FILM_TYPES[] = {
    {"35mm",   18.96f},   // 4 perf/frame, 4.74mm perf pitch
    {"6x9",    90.00f},   // 56x84mm frame + gap, 8 exp/roll (120)
    {"6x6",    62.00f},   // 56x56mm frame + gap, 12 exp/roll (120)
    {"6x4.5",  43.00f},   // 56x41.5mm frame + gap, 16 exp/roll (120)
};

constexpr size_t FILM_TYPE_COUNT = sizeof(FILM_TYPES) / sizeof(FILM_TYPES[0]);
