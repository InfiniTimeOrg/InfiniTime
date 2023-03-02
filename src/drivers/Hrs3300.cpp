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

/** Driver for the HRS3300 heart rate sensor.
 * Original implementation from wasp-os : https://github.com/daniel-thompson/wasp-os/blob/master/wasp/drivers/hrs3300.py
 */
Hrs3300::Hrs3300(TwiMaster& twiMaster, uint8_t twiAddress) : twiMaster {twiMaster}, twiAddress {twiAddress} {
}

void Hrs3300::Init() {
  nrf_gpio_cfg_input(30, NRF_GPIO_PIN_NOPULL);

  Disable();
  vTaskDelay(100);

  // HRS disabled, 12.5 ms wait time between cycles, (partly) 20mA drive
  WriteRegister(static_cast<uint8_t>(Registers::Enable), 0x60);

  // (partly) 20mA drive, power on, "magic" (datasheet says both
  // "reserved" and "set low nibble to 8" but 0xe gives better results
  // and is used by at least two other HRS3300 drivers
  WriteRegister(static_cast<uint8_t>(Registers::PDriver), 0x6E);

  // HRS and ALS both in 16-bit mode
  WriteRegister(static_cast<uint8_t>(Registers::Res), 0x88);

  // 8x gain, non default, reduced value for better readings
  WriteRegister(static_cast<uint8_t>(Registers::Hgain), 0xc);
}

void Hrs3300::Enable() {
  NRF_LOG_INFO("ENABLE");
  auto value = ReadRegister(static_cast<uint8_t>(Registers::Enable));
  value |= 0x80;
  WriteRegister(static_cast<uint8_t>(Registers::Enable), value);
}

void Hrs3300::Disable() {
  NRF_LOG_INFO("DISABLE");
  auto value = ReadRegister(static_cast<uint8_t>(Registers::Enable));
  value &= ~0x80;
  WriteRegister(static_cast<uint8_t>(Registers::Enable), value);
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
