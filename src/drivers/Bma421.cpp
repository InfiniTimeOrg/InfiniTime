#include "drivers/Bma421.h"
#include "drivers/Bma421_C/bma4.h"
#include <libraries/delay/nrf_delay.h>
#include <libraries/log/nrf_log.h>
#include "drivers/TwiMaster.h"
#include <drivers/Bma421_C/bma423.h>
#include <cstdint>

using namespace Pinetime::Drivers;

namespace {
  int8_t user_i2c_read(uint8_t reg_addr, uint8_t* reg_data, uint32_t length, void* intf_ptr) {
    auto* bma421 = static_cast<Bma421*>(intf_ptr);
    bma421->Read(reg_addr, reg_data, length);
    return 0;
  }

  int8_t user_i2c_write(uint8_t reg_addr, const uint8_t* reg_data, uint32_t length, void* intf_ptr) {
    auto* bma421 = static_cast<Bma421*>(intf_ptr);
    bma421->Write(reg_addr, reg_data, length);
    return 0;
  }

  void user_delay(uint32_t period_us, void* intf_ptr) {
    nrf_delay_us(period_us);
  }
}

Bma421::Bma421(TwiMaster& twiMaster, uint8_t twiAddress) : twiMaster {twiMaster}, deviceAddress {twiAddress} {
  bma.intf = BMA4_I2C_INTF;
  bma.bus_read = user_i2c_read;
  bma.bus_write = user_i2c_write;
  bma.variant = BMA42X_VARIANT;
  bma.intf_ptr = this;
  bma.delay_us = user_delay;
  bma.read_write_len = 16;
}

void Bma421::Init() {
  if (not isResetOk) {
    return; // Call SoftReset (and reset TWI device) first!
  }

  // Use a uint8_t to hold the feature flags, and enable all at once.
  uint8_t features_to_enable = BMA423_STEP_CNTR;
  auto ret = bma423_init(&bma);
  if (ret != BMA4_OK)
    return;

  switch (bma.chip_id) {
    case BMA423_CHIP_ID:
      deviceType = DeviceTypes::BMA421;
      break;
    case BMA425_CHIP_ID:
      deviceType = DeviceTypes::BMA425;
      break;
    default:
      deviceType = DeviceTypes::Unknown;
      break;
  }

  ret = bma423_write_config_file(&bma);
  if (ret != BMA4_OK)
    return;

  ret = bma4_set_interrupt_mode(BMA4_LATCH_MODE, &bma);
  if (ret != BMA4_OK)
    return;

  ret = bma4_set_accel_enable(1, &bma);
  if (ret != BMA4_OK)
    return;

  struct bma4_accel_config accel_conf {};
  accel_conf.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
  accel_conf.range = BMA4_ACCEL_RANGE_2G;
  accel_conf.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
  accel_conf.perf_mode = BMA4_CIC_AVG_MODE;
  ret = bma4_set_accel_config(&accel_conf, &bma);
  if (ret != BMA4_OK)
    return;

  // Remap the axes to the proper orientation. See datasheet for BMA425 for a diagram and more information.
  // Remap depending on right or left hand. Currently set to left hand (which is {1, 0, 2, 1, 1, 1}).
  // remap_data will be used for the BMA425's raise-to-wake interrupt, and for this driver's axes remap
  remap_data = {1, 0, 2, 1, 1, 1};
  ret = bma423_set_remap_axes(&remap_data, &bma);
  if (ret != BMA4_OK) {
    return;
  }

  // Enable the raise-to-wake interrupt on supported devices
  if (deviceType == DeviceTypes::BMA425) {
    features_to_enable |= BMA423_WRIST_WEAR;
  }

  // Enable all features at once
  ret = bma423_feature_enable(features_to_enable, 1, &bma);
  if (ret != BMA4_OK)
    return;

  // The interrupt map is needed for the interrupt flag to be set
  if (deviceType == DeviceTypes::BMA425) {
    ret = bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_WRIST_WEAR_INT, BMA4_ENABLE, &bma);
    if (ret != BMA4_OK)
      return;
  }

  isOk = true;
}

void Bma421::Reset() {
  uint8_t data = 0xb6;
  twiMaster.Write(deviceAddress, 0x7E, &data, 1);
}

void Bma421::Read(uint8_t registerAddress, uint8_t* buffer, size_t size) {
  twiMaster.Read(deviceAddress, registerAddress, buffer, size);
}

void Bma421::Write(uint8_t registerAddress, const uint8_t* data, size_t size) {
  twiMaster.Write(deviceAddress, registerAddress, data, size);
}

Bma421::Values Bma421::Process() {
  if (not isOk) {
    return {};
  }
  struct bma4_accel data {};
  bma4_read_accel_xyz(&data, &bma);

  uint32_t steps = 0;
  bma423_step_counter_output(&steps, &bma);

  int32_t temperature = 0;
  bma4_get_temperature(&temperature, BMA4_DEG, &bma);
  temperature = temperature / 1000;

  uint8_t activity = 0;
  bma423_activity_output(&activity, &bma);

  if (deviceType == DeviceTypes::BMA425) {
    uint16_t status = 0;

    // bma4_read_int_status_0 clears all the interrupts, if any other interrupts are needed, this will have to be refactored
    bma423_read_int_status(&status, &bma);
    if (((status & BMA423_WRIST_WEAR_INT) != 0) && !wristTiltInterrupt) {
      wristTiltInterrupt = true;
    }
  }

  // Format the data for remapping
  int16_t accel_data[3];
  accel_data[0] = data.x;
  accel_data[1] = data.y;
  accel_data[2] = data.z;

  // Use the remap data to remap the axes and remap the signs
  return {steps,
          static_cast<int16_t>(accel_data[remap_data.x_axis] * (-1 * remap_data.x_axis_sign)),
          static_cast<int16_t>(accel_data[remap_data.y_axis] * (-1 * remap_data.x_axis_sign)),
          static_cast<int16_t>(accel_data[remap_data.z_axis] * (-1 * remap_data.x_axis_sign))};
}

bool Bma421::IsOk() const {
  return isOk;
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
Bma421::DeviceTypes Bma421::DeviceType() const {
  return deviceType;
}
