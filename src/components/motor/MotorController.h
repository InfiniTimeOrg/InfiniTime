#pragma once

#include <cstdint>
#include "app_timer.h"
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Controllers {
    static constexpr uint8_t pinMotor = 16;
    
    
    class MotorController {
    

    public:
      enum TuneType : uint8_t {
        NOTIFICATION,
        SHORT,
        RING,
        STOP
      };

      MotorController(Controllers::Settings& settingsController);
      void Init();
      void RunForDuration(uint8_t motorDuration);
      void StartRinging();
      static void StopRinging();
      void VibrateTune(TuneType tune);

    private:

    private:
      struct Tune { 
        uint8_t tune;
        uint8_t length;
        uint8_t tempo;
      };
      Controllers::Settings& settingsController;
      static TuneType runningTune;
      static uint8_t step;

      static constexpr Tune tunes[] =  {
        [TuneType::NOTIFICATION] = {.tune = 0x29, .length = 6, .tempo = 50},
        [TuneType::SHORT] = {.tune = 0x01, .length = 2, .tempo = 35},
        [TuneType::RING] = {.tune = 0x0f, .length = 8, .tempo = 50},
        [TuneType::STOP] = {.tune = 0x00, .length = 0, .tempo = 0},
      };

      static void Vibrate(void* p_context);
      static void Ring(void* p_context);
    
    };
  }
}
