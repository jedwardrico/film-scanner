#pragma once

// Persists user-adjustable runtime settings in NVS (flash), so they survive
// reboots: the jog step count used by JOG_FORWARD/JOG_BACKWARD, and whether
// a FRAME_FORWARD advance automatically fires the shutter once the move
// completes. Adjustable on-device from the Settings menu (see main.cpp).
class Settings {
 public:
  void begin();

  long jogSteps() const;
  void setJogSteps(long steps);

  bool autoFireOnAdvance() const;
  void setAutoFireOnAdvance(bool enabled);
};
