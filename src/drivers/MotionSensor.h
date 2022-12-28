#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>

namespace Pinetime {
  namespace Drivers {
    namespace MotionSensors {
      enum class DeviceTypes : uint8_t { Unknown, BMA421, BMA425 };
      struct Values {
        uint32_t steps;
        int16_t x;
        int16_t y;
        int16_t z;
      };
    }

    template <typename motionSensorImpl>
    concept IsMotionSensor = requires(motionSensorImpl sensor, uint8_t registerAddress, uint8_t* data, uint8_t* constData, size_t size) {
                               { sensor.SoftReset() };
                               { sensor.Init() };
                               { sensor.Process() };
                               { sensor.ResetStepCounter() };
                               { sensor.IsOk() };
                               { sensor.DeviceType() };
                               { sensor.Read(registerAddress, data, size) };
                               { sensor.Write(registerAddress, constData, size) };
                           };

    namespace Interface {
      template <class T>
        requires IsMotionSensor<T>
      class MotionSensor {
      public:
        explicit MotionSensor(T& impl) : impl {impl} {}
        MotionSensor(const MotionSensor&) = delete;
        MotionSensor& operator=(const MotionSensor&) = delete;
        MotionSensor(MotionSensor&&) = delete;
        MotionSensor& operator=(MotionSensor&&) = delete;

        void SoftReset() {
          return impl.SoftReset();
        }

        void Init() {
          impl.Init();
        }

        MotionSensors::Values Process() {
          return impl.Process();
        }

        void ResetStepCounter() {
          return impl.ResetStepCounter();
        }

        void Read(uint8_t registerAddress, uint8_t* buffer, size_t size) {
          impl.Read(registerAddress, buffer, size);
        }

        void Write(uint8_t registerAddress, const uint8_t* data, size_t size) {
          impl.Write(registerAddress, data, size);
        }

        bool IsOk() const {
          return impl.IsOk();
        }

        MotionSensors::DeviceTypes DeviceType() const {
          return impl.DeviceType();
        }

      private:
        T& impl;
      };
    }
  }
}
