#pragma once

// On hardware, LVGL is only ever touched by the display task, so no locking is
// needed and this is a no-op. InfiniSim shadows this header (sim/displayapp/
// LvglGuard.h) with a real mutex: the simulator's main thread also drives LVGL
// while the display task sleeps (the "Screen is OFF" overlay), and the
// wake/sleep handoff races the display task without it.
#define LVGL_GUARD() (void) 0
