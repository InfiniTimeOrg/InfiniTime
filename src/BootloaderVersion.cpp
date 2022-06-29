#include <cstdint>
#include <cstdio>
#include "BootloaderVersion.h"

using namespace Pinetime;

// NOTE : version < 1.0.0 of bootloader does not export its version to the application firmware.

uint32_t BootloaderVersion::version = 0;
char BootloaderVersion::versionString[BootloaderVersion::VERSION_STR_LEN] = "0.0.0";

uint32_t BootloaderVersion::Major() {
  return (BootloaderVersion::version >> 16u) & 0xff;
}

uint32_t BootloaderVersion::Minor() {
  return (BootloaderVersion::version >> 8u) & 0xff;
}

uint32_t BootloaderVersion::Patch() {
  return BootloaderVersion::version & 0xff;
}

const char* BootloaderVersion::VersionString() {
  return BootloaderVersion::versionString;
}

bool BootloaderVersion::IsValid() {
  return BootloaderVersion::version >= 0x00010000;
}

void BootloaderVersion::SetVersion(uint32_t v) {
  BootloaderVersion::version = v;
  snprintf(BootloaderVersion::versionString,
           BootloaderVersion::VERSION_STR_LEN,
           "%ld.%ld.%ld",
           BootloaderVersion::Major(),
           BootloaderVersion::Minor(),
           BootloaderVersion::Patch());
}
