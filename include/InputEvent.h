#pragma once

// Logical input events, decoupled from the physical control that produced
// them. Buttons.h emits these today; a rotary encoder or additional buttons
// added later can emit the same events without touching the state machine
// in main.cpp.
enum class InputEvent {
  NONE,
  NEXT,     // move selection forward / increment
  PREV,     // move selection backward / decrement
  SELECT,   // confirm current selection
  BACK,     // cancel / return to menu
  FORWARD,  // advance one frame and fire the shutter
};
