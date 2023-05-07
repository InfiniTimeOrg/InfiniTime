/*
  SPDX-License-Identifier: LGPL-3.0-or-later
  Original work Copyright (C) 2020 Daniel Thompson
  C++ port Copyright (C) 2021 Jean-François Milants
*/

#include "drivers/Hrs3300.h"
#include <algorithm>
#include <nrf_gpio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <nrf_log.h>

using namespace Pinetime::Drivers;

namespace {
  static constexpr uint8_t ledDriveCurrentValue = 0x2f;
}

/** Driver for the HRS3300 heart rate sensor.
 * Original implementation from wasp-os : https://github.com/daniel-thompson/wasp-os/blob/master/wasp/drivers/hrs3300.py
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

uint32_t Hrs3300::ReadHrs() {
  auto m = ReadRegister(static_cast<uint8_t>(Registers::C0DataM));
  auto h = ReadRegister(static_cast<uint8_t>(Registers::C0DataH));
  auto l = ReadRegister(static_cast<uint8_t>(Registers::C0dataL));
  return ((l & 0x30) << 12) | (m << 8) | ((h & 0x0f) << 4) | (l & 0x0f);
}

uint32_t Hrs3300::ReadAls() {
  auto m = ReadRegister(static_cast<uint8_t>(Registers::C1dataM));
  auto h = ReadRegister(static_cast<uint8_t>(Registers::C1dataH));
  auto l = ReadRegister(static_cast<uint8_t>(Registers::C1dataL));
  return ((h & 0x3f) << 11) | (m << 3) | (l & 0x07);
}

void Hrs3300::SetGain(uint8_t gain) {
  constexpr uint8_t maxGain = 64U;
  gain = std::min(gain, maxGain);
  uint8_t hgain = 0;
  while ((1 << hgain) < gain) {
    ++hgain;
  }

  WriteRegister(static_cast<uint8_t>(Registers::Hgain), hgain << 2);
}

void Hrs3300::SetDrive(uint8_t drive) {
  auto en = ReadRegister(static_cast<uint8_t>(Registers::Enable));
  auto pd = ReadRegister(static_cast<uint8_t>(Registers::PDriver));

  en = (en & 0xf7) | ((drive & 2) << 2);
  pd = (pd & 0xbf) | ((drive & 1) << 6);

  WriteRegister(static_cast<uint8_t>(Registers::Enable), en);
  WriteRegister(static_cast<uint8_t>(Registers::PDriver), pd);
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
