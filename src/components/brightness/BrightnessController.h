#pragma once

#include <cstdint>

#include "nrf_ppi.h"
#include "nrfx_timer.h"
#include "nrfx_gpiote.h"

namespace Pinetime {
  namespace Controllers {
    class BrightnessController {
    public:
      enum class Levels { Off, AlwaysOn, Low, Medium, High };
      void Init();

      void Set(Levels level);
      Levels Level() const;
      void Lower();
      void Higher();
      void Step();

      const char* GetIcon();
      const char* ToString();

    private:
      Levels level = Levels::High;
      static constexpr uint8_t UNSET = UINT8_MAX;
      uint8_t lastPin = UNSET;
      nrfx_timer_t timer;
      // Warning: nimble reserves some PPIs
      // https://github.com/InfiniTimeOrg/InfiniTime/blob/034d83fe6baf1ab3875a34f8cee387e24410a824/src/libs/mynewt-nimble/nimble/drivers/nrf52/src/ble_phy.c#L53
      // SpiMaster uses PPI 0 for an erratum workaround
      // Channel 1, 2 should be free to use
      static constexpr nrf_ppi_channel_t ppiBacklightOn = NRF_PPI_CHANNEL1;
      static constexpr nrf_ppi_channel_t ppiBacklightOff = NRF_PPI_CHANNEL2;

      void applyBrightness(uint16_t val);
    };
  }
}
