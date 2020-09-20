#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class HidService;
    class BleMouse {
      public:
        explicit BleMouse(HidService& hidService);

        void Move(uint8_t x, uint8_t y);

      private:
        HidService& hidService;
    };
  }
}


