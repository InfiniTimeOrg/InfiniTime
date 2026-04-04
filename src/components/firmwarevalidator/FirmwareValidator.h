#pragma once

#include <cstdint>
#include <string>

#include "components/fs/FS.h"
#include "Version.h"

namespace Pinetime {
  namespace Controllers {
    class FirmwareValidator {
    public:
      FirmwareValidator(Pinetime::Controllers::FS& fs);

      void Validate();
      void AutoValidate();
      bool IsValidated() const;

      void Reset();

    private:
      Pinetime::Controllers::FS& fs;

      static constexpr uint32_t validBitAdress {0x7BFE8};
      static constexpr uint32_t validBitValue {1};
      static constexpr uint32_t commitHashBufferLength = (uint32_t) std::char_traits<char>::length(Version::GitCommitHash()) + 1;
    };
  }
}
