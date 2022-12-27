#include "drivers/Bma421.h"
#include "drivers/TwiMaster.h"
#include <drivers/Bma421_C/bma423.h>
#include <libraries/delay/nrf_delay.h>

using namespace Pinetime::Drivers;
namespace {
  int8_t user_i2c_read(uint8_t reg_addr, uint8_t* reg_data, uint32_t length, void* intf_ptr) {
    auto bma421 = static_cast<Bma421*>(intf_ptr);
    bma421->Read(reg_addr, reg_data, length);
    return 0;
  }

  int8_t user_i2c_write(uint8_t reg_addr, const uint8_t* reg_data, uint32_t length, void* intf_ptr) {
    auto bma421 = static_cast<Bma421*>(intf_ptr);
    bma421->Write(reg_addr, reg_data, length);
    return 0;
  }

  void user_delay(uint32_t period_us, void* intf_ptr) {
    nrf_delay_us(period_us);
  }
}

Bma421::Bma421(TwiMaster& twiMaster, uint8_t twiAddress) : AccelerationSensor(twiMaster, twiAddress) {
  bma.intf = BMA4_I2C_INTF;
  bma.bus_read = user_i2c_read;
  bma.bus_write = user_i2c_write;
  bma.variant = BMA42X_VARIANT;
  bma.intf_ptr = this;
  bma.delay_us = user_delay;
  bma.read_write_len = 16;
}

void Bma421::Init() {
  if (!isResetOk)
    return; // Call SoftReset (and reset TWI device) first!

  // Initialize interface
  auto ret = bma423_init(&bma);
  if (ret != BMA4_OK)
    return;

  // Identify chip by ID. The driver code has been modified to handle BMA421 as BMA423
  switch (bma.chip_id) {
    case BMA423_CHIP_ID:
      deviceType = AccelerationDeviceTypes::BMA421;
      break;
    case BMA425_CHIP_ID:
      deviceType = AccelerationDeviceTypes::BMA425;
      break;
    default:
      deviceType = AccelerationDeviceTypes::Unknown;
      break;
  }

  // Load proprietary firmware blob required for step counting engine
  ret = bma423_write_config_file(&bma);
  if (ret != BMA4_OK)
    return;

  // Enable step counter and accelerometer, disable step detector
  ret = bma423_feature_enable(BMA423_STEP_CNTR, 1, &bma);
  if (ret != BMA4_OK)
    return;
  ret = bma423_step_detector_enable(0, &bma);
  if (ret != BMA4_OK)
    return;
  ret = bma4_set_accel_enable(1, &bma);
  if (ret != BMA4_OK)
    return;

  // Configure accelerometer
  struct bma4_accel_config accel_conf;
  accel_conf.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
  accel_conf.range = BMA4_ACCEL_RANGE_2G;
  accel_conf.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
  accel_conf.perf_mode = BMA4_CIC_AVG_MODE;
  ret = bma4_set_accel_config(&accel_conf, &bma);
  if (ret != BMA4_OK)
    return;

  isInitialized = true;
}

AccelerationValues Bma421::Process() {
  if (!isInitialized)
    return {};

  struct bma4_accel data;
  bma4_read_accel_xyz(&data, &bma);

  uint32_t steps = 0;
  bma423_step_counter_output(&steps, &bma);

  int32_t temperature;
  bma4_get_temperature(&temperature, BMA4_DEG, &bma);
  temperature = temperature / 1000;

  uint8_t activity = 0;
  bma423_activity_output(&activity, &bma);

  // X and Y axis are swapped because of the way the sensor is mounted in the PineTime
  return {steps, data.y, data.x, data.z, (int16_t*) fifo, 0};
}

void Bma421::ResetStepCounter() {
  bma423_reset_step_counter(&bma);
}

void Bma421::SoftReset() {
  auto ret = bma4_soft_reset(&bma);
  if (ret == BMA4_OK) {
    isResetOk = true;
    nrf_delay_ms(1);
  }
}
