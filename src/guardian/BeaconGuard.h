#pragma once
#include <cstdint>

// Minimalny interfejs – wołasz Start() raz, potem Update() co jakiś czas.
namespace Guardian {
class BeaconGuard {
public:
  static void Start();
  static void Update(uint8_t hr, uint8_t batt);
};
} // namespace Guardian
