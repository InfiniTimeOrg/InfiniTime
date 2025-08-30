#pragma once

#include <cstdint>
#include <components/ble/HeartRateService.h>
#include <optional>

namespace Pinetime {
  namespace Applications {
    class HeartRateTask;
  }

  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class HeartRateController {
    public:
      enum class States { Stopped, NotEnoughData, NoTouch, Running };

      HeartRateController() = default;
      void Start();
      void Stop();
      void Update(States newState, std::optional<uint8_t> heartRate);

      void SetHeartRateTask(Applications::HeartRateTask* task);

      States State() const {
        return state;
      }

      std::optional<uint8_t> HeartRate() const {
        return heartRate;
      }

      void SetService(Pinetime::Controllers::HeartRateService* service);

    private:
      Applications::HeartRateTask* task = nullptr;
      States state = States::Stopped;
      std::optional<uint8_t> heartRate = std::nullopt;
      Pinetime::Controllers::HeartRateService* service = nullptr;
    };
  }
}
