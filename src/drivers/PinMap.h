#pragma once

#ifdef WATCH_P8

// BatteryController.h
#define PINMAP_CHARGING_PIN 19

// Cst816s.h
#define PINMAP_CST816S_RESET_PIN 13

// SystemTask.h
#define PINMAP_BUTTON_PIN 17

#else

// BatteryController.h
#define PINMAP_CHARGING_PIN 12

// Cst816s.h
#define PINMAP_CST816S_RESET_PIN 10

// SystemTask.h
#define PINMAP_BUTTON_PIN 13

#endif