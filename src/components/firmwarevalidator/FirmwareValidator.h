#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class FirmwareValidator {
    public:
      void Validate();
      bool IsValidated() const;

      void Reset();

    private:
      static constexpr uint32_t validBitAdress {0x7BFE8};
      static constexpr uint32_t validBitValue {1};
    };
  }
}
