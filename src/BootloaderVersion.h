#pragma once

namespace Pinetime {
  class BootloaderVersion {
      public:
    static uint32_t Major();
    static uint32_t Minor();
    static uint32_t Patch();
    static const char* VersionString();
    static bool IsValid();
  };
}