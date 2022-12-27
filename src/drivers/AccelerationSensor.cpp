#include "AccelerationSensor.h"

namespace Pinetime {
  namespace Drivers {

    AccelerationSensor::AccelerationSensor(TwiMaster& twiMaster, uint8_t twiAddress) : twiMaster(twiMaster), deviceAddress(twiAddress) {
    }

    void AccelerationSensor::SoftReset() {
    }

    void AccelerationSensor::Init() {
    }

    AccelerationValues AccelerationSensor::Process() {
      return {0};
    }

    void AccelerationSensor::ResetStepCounter() {
    }

    void AccelerationSensor::Read(uint8_t registerAddress, uint8_t* buffer, size_t size) {
      twiMaster.Read(deviceAddress, registerAddress, buffer, size);
    }

    void AccelerationSensor::Write(uint8_t registerAddress, const uint8_t* data, size_t size) {
      twiMaster.Write(deviceAddress, registerAddress, data, size);
    }

    AccelerationDeviceTypes AccelerationSensor::DeviceType() const {
      return deviceType;
    }

    bool AccelerationSensor::IsInitialized() const {
      return isInitialized;
    }

  }
}
