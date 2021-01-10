#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Applications {
    class HeartRateTask;
  }
  namespace Controllers {
    class HeartRateController {
    public:
      enum class States { NotEnoughData, NoTouch, Running};

      void Start();
      void Stop();
      void Update(States newState, uint8_t heartRate);

      void SetHeartRateTask(Applications::HeartRateTask* task);
      States State() const { return state; }
      uint8_t HeartRate() const { return heartRate; }

    private:
      Applications::HeartRateTask* task = nullptr;
      States state = States::NotEnoughData;
      uint8_t heartRate = 0;
    };
  }
}