#pragma once

#include <cstdint>
#include <cstddef>

namespace Pinetime {
  class BootloaderVersion {
  public:
    static uint32_t Major();
    static uint32_t Minor();
    static uint32_t Patch();
    static const char* VersionString();
    static bool IsValid();
    static void SetVersion(uint32_t v);

  private:
    static uint32_t version;
    static constexpr size_t VERSION_STR_LEN = 12;
    static char versionString[VERSION_STR_LEN];
  };
}
