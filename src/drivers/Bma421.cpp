#include "drivers/Bma421.h"
#include <libraries/delay/nrf_delay.h>
#include <libraries/log/nrf_log.h>
#include "drivers/TwiMaster.h"
#include <drivers/Bma421_C/bma423.h>

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

  void user_delay(uint32_t period_us, void* /*intf_ptr*/) {
    nrf_delay_us(period_us);
  }

  // Scale factors to convert accelerometer counts to milli-g
  // from datasheet: https://files.pine64.org/doc/datasheet/pinetime/BST-BMA421-FL000.pdf
  // The array index to use is stored in accel_conf.range
  constexpr int16_t accelScaleFactors[] = {
    [BMA4_ACCEL_RANGE_2G] = 1024, // LSB/g +/- 2g range
    [BMA4_ACCEL_RANGE_4G] = 512,  // LSB/g +/- 4g range
    [BMA4_ACCEL_RANGE_8G] = 256,  // LSB/g +/- 8g range
    [BMA4_ACCEL_RANGE_16G] = 128  // LSB/g +/- 16g range
  };
}

Bma421::Bma421(TwiMaster& twiMaster, uint8_t twiAddress) : twiMaster {twiMaster}, deviceAddress {twiAddress} {
  bma.intf = BMA4_I2C_INTF;
  bma.bus_read = user_i2c_read;
  bma.bus_write = user_i2c_write;
  bma.variant = BMA42X_VARIANT;
  bma.intf_ptr = this;
  bma.delay_us = user_delay;
  bma.read_write_len = 16;
  bma.resolution = 12;
}

void Bma421::Init() {
  if (!isResetOk)
    return; // Call SoftReset (and reset TWI device) first!

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

  ret = bma423_feature_enable(BMA423_STEP_CNTR, 1, &bma);
  if (ret != BMA4_OK)
    return;

  ret = bma423_step_detector_enable(0, &bma);
  if (ret != BMA4_OK)
    return;

  ret = bma4_set_accel_enable(1, &bma);
  if (ret != BMA4_OK)
    return;

  ret = bma4_set_fifo_config(BMA4_FIFO_ACCEL, 1, &bma);
  if (ret != BMA4_OK)
    return;
  fifo_frame.data = (uint8_t*) &fifo;
  fifo_frame.length = sizeof(fifo);
  fifo_frame.fifo_data_enable = BMA4_FIFO_A_ENABLE;
  fifo_frame.fifo_header_enable = 0;

  struct bma4_accel_config accel_conf;
  accel_conf.odr = BMA4_OUTPUT_DATA_RATE_200HZ;
  accel_conf.range = BMA4_ACCEL_RANGE_2G;
  accel_conf.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
  accel_conf.perf_mode = BMA4_CONTINUOUS_MODE;
  ret = bma4_set_accel_config(&accel_conf, &bma);
  if (ret != BMA4_OK)
    return;

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
  if (not isOk)
    return {};

  bma4_read_fifo_data(&fifo_frame, &bma);
  uint16_t length = 32; // Should be about 20 (200 Hz ODR / 10 Hz main loop)
  bma4_extract_accel((bma4_accel*) fifo, &length, &fifo_frame, &bma);

  uint32_t steps = 0;
  bma423_step_counter_output(&steps, &bma);

  int32_t temperature = 0;
  bma4_get_temperature(&temperature, BMA4_DEG, &bma);
  temperature = temperature / 1000;

  uint8_t activity = 0;
  bma423_activity_output(&activity, &bma);

  int16_t avgs[3] = {0};
  for (uint8_t i = 0; i < length; i++) {
    // X and Y axis are swapped because of the way the sensor is mounted in the PineTime
    int16_t swap = fifo[i][0];
    fifo[i][0] = fifo[i][1];
    fifo[i][1] = swap;
    for (uint8_t j = 0; j < 3; j++)
      avgs[j] += fifo[i][j];
  }

  for (uint8_t j = 0; j < 3; j++)
  {
    avgs[j] /= length;
    // Scale the measured ADC counts to units of 'binary milli-g'
    // where 1g = 1024 'binary milli-g' units.
    // See https://github.com/InfiniTimeOrg/InfiniTime/pull/1950 for
    // discussion of why we opted for scaling to 1024 rather than 1000.
    avgs[j] = 1024 * avgs[j] / accelScaleFactors[accel_conf.range];
  }

  return {steps, avgs[0], avgs[1], avgs[2]};
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
