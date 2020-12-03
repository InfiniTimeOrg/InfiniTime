#include <cstdint>
#include "BootloaderVersion.h"

using namespace Pinetime;

// NOTE : current bootloader does not export its version to the application firmware.

uint32_t BootloaderVersion::Major() {
  return 0;
}

uint32_t BootloaderVersion::Minor() {
  return 0;
}

uint32_t BootloaderVersion::Patch() {
  return 0;
}

const char *BootloaderVersion::VersionString() {
  return "0.0.0";
}

bool BootloaderVersion::IsValid() {
  return false;
}
