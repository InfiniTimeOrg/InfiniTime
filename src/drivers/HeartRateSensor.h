#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>

namespace Pinetime {
  namespace Drivers {
    template <typename HeartRateSensorImpl>
    concept IsHeartRateSensor = requires(HeartRateSensorImpl sensor, uint8_t gain, uint8_t drive) {
                               { sensor.Init() };
                               { sensor.Enable() };
                               { sensor.Disable() };
                               { sensor.ReadHrs() } -> std::same_as<uint32_t>;
                               { sensor.ReadAls() } -> std::same_as<uint32_t>;
                               { sensor.SetGain(gain) };
                               { sensor.SetDrive(drive) };
                           };

    namespace Interface {
      template <class T>
        requires IsHeartRateSensor<T>
      class HeartRateSensor {
      public:
        explicit HeartRateSensor(T& impl) : impl {impl} {}
        HeartRateSensor(const HeartRateSensor&) = delete;
        HeartRateSensor& operator=(const HeartRateSensor&) = delete;
        HeartRateSensor(HeartRateSensor&&) = delete;
        HeartRateSensor& operator=(HeartRateSensor&&) = delete;

        void Init() {
          impl.Init();
        }

        void Enable() {
          impl.Enable();
        }

        void Disable() {
          impl.Disable();
        }

        uint32_t ReadHrs() {
          return impl.ReadHrs();
        }

        uint32_t ReadAls() {
          return impl.ReadAls();
        }

        void SetGain(uint8_t gain) {
          impl.SetGain(gain);
        }

        void SetDrive(uint8_t drive) {
          impl.SetDrive(drive);
        }

      private:
        T& impl;
      };
    }
  }
}
