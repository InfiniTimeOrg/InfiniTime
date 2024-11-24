/*
  SPDX-License-Identifier: LGPL-3.0-or-later
  Original work Copyright (C) 2020 Daniel Thompson
  C++ port Copyright (C) 2021 Jean-François Milants
*/

#include "drivers/Hrs3300.h"
#include <algorithm>
#include <iterator>
#include <nrf_gpio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <nrf_log.h>

using namespace Pinetime::Drivers;

namespace {
  static constexpr uint8_t ledDriveCurrentValue = 0x2f;
}

/** Driver for the HRS3300 heart rate sensor.
 * Original implementation from wasp-os : https://github.com/wasp-os/wasp-os/blob/master/wasp/drivers/hrs3300.py
 *
 * Experimentaly derived changes to improve signal/noise (see comments below) - Ceimour
 */
Hrs3300::Hrs3300(TwiMaster& twiMaster, uint8_t twiAddress) : twiMaster {twiMaster}, twiAddress {twiAddress} {
}

void Hrs3300::Init() {
  nrf_gpio_cfg_input(30, NRF_GPIO_PIN_NOPULL);

  Disable();
  vTaskDelay(100);

  // HRS disabled, 50ms wait time between ADC conversion period, current 12.5mA
  WriteRegister(static_cast<uint8_t>(Registers::Enable), 0x50);

  // Current 12.5mA and low nibble 0xF.
  // Note: Setting low nibble to 0x8 per the datasheet results in
  // modulated LED driver output. Setting to 0xF results in clean,
  // steady output during the ADC conversion period.
  WriteRegister(static_cast<uint8_t>(Registers::PDriver), ledDriveCurrentValue);

  // HRS and ALS both in 15-bit mode results in ~50ms LED drive period
  // and presumably ~50ms ADC conversion period.
  WriteRegister(static_cast<uint8_t>(Registers::Res), 0x77);

  // Gain set to 1x
  WriteRegister(static_cast<uint8_t>(Registers::Hgain), 0x00);
}

void Hrs3300::Enable() {
  NRF_LOG_INFO("ENABLE");
  auto value = ReadRegister(static_cast<uint8_t>(Registers::Enable));
  value |= 0x80;
  WriteRegister(static_cast<uint8_t>(Registers::Enable), value);

  WriteRegister(static_cast<uint8_t>(Registers::PDriver), ledDriveCurrentValue);
}

void Hrs3300::Disable() {
  NRF_LOG_INFO("DISABLE");
  auto value = ReadRegister(static_cast<uint8_t>(Registers::Enable));
  value &= ~0x80;
  WriteRegister(static_cast<uint8_t>(Registers::Enable), value);

  WriteRegister(static_cast<uint8_t>(Registers::PDriver), 0);
}

Hrs3300::PackedHrsAls Hrs3300::ReadHrsAls() {
  constexpr Registers dataRegisters[] =
    {Registers::C1dataM, Registers::C0DataM, Registers::C0DataH, Registers::C1dataH, Registers::C1dataL, Registers::C0dataL};
  // Calculate smallest register address
  constexpr uint8_t baseOffset = static_cast<uint8_t>(*std::min_element(std::begin(dataRegisters), std::end(dataRegisters)));
  // Calculate largest address to determine length of read needed
  // Add one to largest relative index to find the length
  constexpr uint8_t length = static_cast<uint8_t>(*std::max_element(std::begin(dataRegisters), std::end(dataRegisters))) - baseOffset + 1;

  Hrs3300::PackedHrsAls res;
  uint8_t buf[length];
  auto ret = twiMaster.Read(twiAddress, baseOffset, buf, length);
  if (ret != TwiMaster::ErrorCodes::NoError) {
    NRF_LOG_INFO("READ ERROR");
  }
  // hrs
  uint8_t m = static_cast<uint8_t>(Registers::C0DataM) - baseOffset;
  uint8_t h = static_cast<uint8_t>(Registers::C0DataH) - baseOffset;
  uint8_t l = static_cast<uint8_t>(Registers::C0dataL) - baseOffset;
  // There are two extra bits (17 and 18) but they are not read here
  // as resolutions >16bit aren't practically useful (too slow) and
  // all hrs values throughout InfiniTime are 16bit
  res.hrs = (buf[m] << 8) | ((buf[h] & 0x0f) << 4) | (buf[l] & 0x0f);

  // als
  m = static_cast<uint8_t>(Registers::C1dataM) - baseOffset;
  h = static_cast<uint8_t>(Registers::C1dataH) - baseOffset;
  l = static_cast<uint8_t>(Registers::C1dataL) - baseOffset;
  res.als = ((buf[h] & 0x3f) << 11) | (buf[m] << 3) | (buf[l] & 0x07);

  return res;
}

void Hrs3300::WriteRegister(uint8_t reg, uint8_t data) {
  auto ret = twiMaster.Write(twiAddress, reg, &data, 1);
  if (ret != TwiMaster::ErrorCodes::NoError)
    NRF_LOG_INFO("WRITE ERROR");
}

uint8_t Hrs3300::ReadRegister(uint8_t reg) {
  uint8_t value;
  auto ret = twiMaster.Read(twiAddress, reg, &value, 1);
  if (ret != TwiMaster::ErrorCodes::NoError)
    NRF_LOG_INFO("READ ERROR");
  return value;
}
