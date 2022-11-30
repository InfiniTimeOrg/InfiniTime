#pragma once

#include "drivers/TwiMaster.h"

namespace Pinetime {
  namespace Drivers {

    enum class AccelerationDeviceTypes : uint8_t { Unknown, BMA421, BMA425, SC7A20 };

    struct AccelerationValues {
      uint32_t steps;
      int16_t x;
      int16_t y;
      int16_t z;
      int16_t* samples = nullptr;
      uint16_t samples_length = 0;
    };

    class AccelerationSensor {
    public:
      AccelerationSensor(TwiMaster& twiMaster, uint8_t twiAddress);
      AccelerationSensor(const AccelerationSensor&) = delete;
      AccelerationSensor& operator=(const AccelerationSensor&) = delete;
      AccelerationSensor(AccelerationSensor&&) = delete;
      AccelerationSensor& operator=(AccelerationSensor&&) = delete;

      virtual void SoftReset();
      virtual void Init();
      virtual AccelerationValues Process();
      virtual void ResetStepCounter();

      void Read(uint8_t registerAddress, uint8_t* buffer, size_t size);
      void Write(uint8_t registerAddress, const uint8_t* data, size_t size);

      bool IsInitialized() const;
      AccelerationDeviceTypes DeviceType() const;

    protected:
      TwiMaster& twiMaster;
      uint8_t deviceAddress;
      bool isInitialized = false;
      AccelerationDeviceTypes deviceType = AccelerationDeviceTypes::Unknown;
      int16_t fifo[32][3] = {0};
    };

  }
}
