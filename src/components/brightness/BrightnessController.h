#pragma once

#include <cstdint>
#include <cmath>
#include <FreeRTOS.h>
#include <task.h>

namespace Pinetime {
  namespace Controllers {
    class BrightnessController {
    public:
      enum class Levels { Off, Low, Medium, High };
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
      uint16_t pwmVal;
      uint16_t getPwm(Levels level);
      void setPwm(uint16_t val);
      uint16_t pwmSequence[1] = {10000};
    };
  }
}
