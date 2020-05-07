#pragma once
#include <cstddef>

namespace Pinetime {
  namespace Drivers {
    class Spi;
    class SpiNorFlash {
      public:
        explicit SpiNorFlash(Spi& spi);
        SpiNorFlash(const SpiNorFlash&) = delete;
        SpiNorFlash& operator=(const SpiNorFlash&) = delete;
        SpiNorFlash(SpiNorFlash&&) = delete;
        SpiNorFlash& operator=(SpiNorFlash&&) = delete;

        void Init();
        void Uninit();


        void Sleep();
        void Wakeup();
      private:
        Spi& spi;

    };
  }
}


