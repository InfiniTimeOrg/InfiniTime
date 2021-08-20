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
        RING
      };

      MotorController(Controllers::Settings& settingsController);
      void Init();
      void RunForDuration(uint8_t motorDuration);
      void StartRinging();
      static void StopRinging();
      void VibrateTune(TuneType tune);

    private:
      struct Tune { 
        uint8_t tune;
        uint8_t length;
        uint8_t tempo;
      };
      Controllers::Settings& settingsController;
      TuneType runningTune = TuneType::SHORT;
      uint8_t step = 255;

      static constexpr Tune tunes[] =  {
        [TuneType::NOTIFICATION] = {.tune = 0b00101001, .length = 6, .tempo = 50},
        [TuneType::SHORT] =        {.tune = 0b00000001, .length = 2, .tempo = 35},
        [TuneType::RING] =         {.tune = 0b00001111, .length = 8, .tempo = 50},
      };
    
      static void Vibrate(void* p_context);
      static void Ring(void* p_context);
      void ScheduleVibrateTimer(uint8_t motorDuration, bool vibrate);
      void StopTune();
      void ScheduleTune(TuneType tune);
    };
  }
}
