#pragma once

// Logical input events, decoupled from the physical control that produced
// them. Buttons.h emits these today; a rotary encoder or additional buttons
// added later can emit the same events without touching the state machine
// in main.cpp.
enum class InputEvent {
  NONE,
  NEXT,             // move selection forward / increment
  PREV,             // move selection backward / decrement
  SELECT,           // confirm current selection / save
  BACK,             // cancel / return to menu
  FRAME_FORWARD,    // advance one whole frame
  FRAME_BACKWARD,   // back up one whole frame
  JOG_FORWARD,      // move forward by the configured jog step count
  JOG_BACKWARD,     // move backward by the configured jog step count
  SHUTTER,          // fire the shutter
};
