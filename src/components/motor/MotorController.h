#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class MotionController;

    class MotorController {
    public:
      explicit MotorController(Pinetime::Controllers::MotionController& motionController);

      enum class Intensity : uint8_t { Light, Medium, Strong };

      void Init();
      void Buzz(Intensity intensity);
      void Ring(Intensity intensity, TickType_t ringPeriod, uint16_t numBuzzes = 0);
      void StopRinging();
      bool IsRinging();

    private:
      enum class State : uint8_t { Idle, Buzz, RingOn, RingOff };
      enum class RequestEvt : uint8_t { RequestBuzz, RequestRingOn, RequestRingOff };

      struct Request {
        TickType_t ringOnTicks;
        TickType_t ringPeriod;
        uint16_t buzzCount;
      };

      Request nextOperation;
      State state;
      uint16_t ringOnTicks;
      TickType_t ringPeriod;
      TickType_t nextEvtTime;
      TimerHandle_t nextEvt;
      uint16_t remainingBuzzes;
      MotionController& motionController;
      uint16_t CalculateLength(Intensity intensity);
      static void NextEvent(TimerHandle_t timer);
      static void RunEvent(void* motorControllerPtr, uint32_t requestedStateInt);
      static void SetMotorRunning(bool active);
      void PushEvent(RequestEvt state);
    };
  }
}
