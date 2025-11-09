/*
  SPDX-License-Identifier: LGPL-3.0-or-later
  Original work Copyright (C) 2020 Daniel Thompson
  C++ port Copyright (C) 2021 Jean-François Milants
*/

#include "drivers/Hrs3300.h"
#include <algorithm>
#include <cstdint>
#include <nrf_gpio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <nrf_log.h>

using namespace Pinetime::Drivers;

namespace {
  constexpr uint16_t hrsHighThresh = 15000;
  constexpr uint16_t hrsLowThresh = 3000;
  constexpr uint16_t alsHighThresh = 1500;
  constexpr uint16_t alsLowThresh = 300;
  constexpr uint16_t touchRegainedThresh = 70;
  constexpr uint16_t touchLostThresh = 1000;
  // The proprietary driver uses this and it seems to be required to re-initialise the sensor
  constexpr std::array<std::array<uint8_t, 2>, 18> bootupRegisters = {{
    {0x01, 0xd0},
    {0x0c, 0x4e},
    {0x16, 0x78},
    {0x17, 0x0d},
    {0x02, 0x80},
    {0x03, 0x00},
    {0x04, 0x00},
    {0x05, 0x00},
    {0x06, 0x00},
    {0x07, 0x00},
    {0x08, 0x74},
    {0x09, 0x00},
    {0x0a, 0x08},
    {0x0b, 0x00},
    {0x0c, 0x6e},
    {0x0d, 0x02},
    {0x0e, 0x07},
    {0x0f, 0x0f},
  }};

  constexpr uint8_t powerChangeDelay = .25f / Pinetime::Controllers::Ppg::sampleDuration;
  constexpr uint8_t noTouchDelay = 2 / Pinetime::Controllers::Ppg::sampleDuration;
  // Duration ambient light must be low for gain up to trigger
  // Long to prevent gaining up when high ambient light only occurs occasionally
  // Much better to be in too low a gain than too high and saturate the ADC
  constexpr uint16_t gainUpWaitTime = 30 / Pinetime::Controllers::Ppg::sampleDuration;
  // If just gained up, wait at least this long before considering another gain up
  constexpr uint16_t gainUpCascadeWaitTime = 5 / Pinetime::Controllers::Ppg::sampleDuration;
}

/** Driver for the HRS3300 heart rate sensor.
 * Original implementation from wasp-os : https://github.com/wasp-os/wasp-os/blob/master/wasp/drivers/hrs3300.py
 *
 * Experimentaly derived changes to improve signal/noise (see comments below) - Ceimour
 *
 * Refactored to include automatic drive and gain control using proprietary driver techniques - mark
 */
Hrs3300::Hrs3300(TwiMaster& twiMaster, uint8_t twiAddress) : twiMaster {twiMaster}, twiAddress {twiAddress} {
}

void Hrs3300::Init() {
  for (auto bootupRegister : bootupRegisters) {
    WriteRegister(bootupRegister[0], bootupRegister[1]);
  }
  r7f = ReadRegister(0x7f);
  r80 = ReadRegister(0x80);
  r81 = ReadRegister(0x81);
  r82 = ReadRegister(0x82);
  // Top 4 bits of r80 clear and bottom 3 bits of r81 clear
  if (((r80 & 0xf0) | (r81 & 0x07)) == 0) {
    r81 |= 0x04;
  }
  WriteRegister(static_cast<uint8_t>(Registers::Res), 0x78);
  Disable();
}

void Hrs3300::Enable() {
  NRF_LOG_INFO("ENABLE");

  ppgDrive = PPGDrive::Current13mALow;
  ppgGain = PPGGain::Gain8x;
  ppgState = PPGState::Running;
  actionDelay = powerChangeDelay;
  hrsLowCount = 0;
  hrsHighCount = 0;
  alsLowCount = 0;
  alsHighCount = 0;

  // Needs to be initialised twice or sensor gets stuck in weird state
  SetPower();
  SetPower();
}

void Hrs3300::Disable() {
  NRF_LOG_INFO("DISABLE");
  WriteRegister(static_cast<uint8_t>(Registers::Enable), 0x00);
  WriteRegister(static_cast<uint8_t>(Registers::PDriver), 0x00);
  ppgState = PPGState::Off;
}

Hrs3300::PackedHrsAls Hrs3300::ReadHrsAls() {
  static constexpr Registers dataRegisters[] =
    {Registers::C1DataM, Registers::C0DataM, Registers::C0DataH, Registers::C1DataH, Registers::C1DataL, Registers::C0DataL};

  // Calculate smallest/largest register address
  constexpr auto bounds = std::ranges::minmax(dataRegisters);
  constexpr uint8_t baseOffset = static_cast<uint8_t>(bounds.min);
  constexpr uint8_t length = static_cast<uint8_t>(bounds.max) - baseOffset + 1;

  PackedHrsAls res;
  uint8_t buf[length];
  auto ret = twiMaster.Read(twiAddress, baseOffset, buf, length);
  if (ret != TwiMaster::ErrorCodes::NoError) {
    NRF_LOG_INFO("READ ERROR");
  }
  // hrs
  uint8_t m = static_cast<uint8_t>(Registers::C0DataM) - baseOffset;
  uint8_t h = static_cast<uint8_t>(Registers::C0DataH) - baseOffset;
  uint8_t l = static_cast<uint8_t>(Registers::C0DataL) - baseOffset;
  // There are two extra bits (17 and 18) but they are not read here
  // as resolutions >16bit aren't practically useful (too slow) and
  // all hrs values throughout InfiniTime are 16bit
  res.hrs = (buf[m] << 8) | ((buf[h] & 0x0f) << 4) | (buf[l] & 0x0f);

  // als
  m = static_cast<uint8_t>(Registers::C1DataM) - baseOffset;
  h = static_cast<uint8_t>(Registers::C1DataH) - baseOffset;
  l = static_cast<uint8_t>(Registers::C1DataL) - baseOffset;
  res.als = ((buf[h] & 0x3f) << 11) | (buf[m] << 3) | (buf[l] & 0x07);

  return res;
}

void Hrs3300::WriteRegister(uint8_t reg, uint8_t data) {
  auto ret = twiMaster.Write(twiAddress, reg, &data, 1);
  if (ret != TwiMaster::ErrorCodes::NoError) {
    NRF_LOG_INFO("WRITE ERROR");
  }
}

uint8_t Hrs3300::ReadRegister(uint8_t reg) {
  uint8_t value;
  auto ret = twiMaster.Read(twiAddress, reg, &value, 1);
  if (ret != TwiMaster::ErrorCodes::NoError) {
    NRF_LOG_INFO("READ ERROR");
  }
  return value;
}

void Hrs3300::SetPower() {
  uint8_t r80Temp = r80;
  uint8_t r81Temp = r81;
  uint8_t r7aTemp = 0x00;

  // All in MSB->LSB order
  // It's just what the proprietary driver does
  if (ppgDrive == PPGDrive::Current13mALow) {
    r80Temp = (r81 & 0x01) << 7 | (r80 & 0xe0) >> 1 | (r80 & 0x0f);
    r81Temp = (r81 & 0xf8) | (r81 & 0x06) >> 1;
  } else if (ppgDrive == PPGDrive::Current40mAHigh) {
    r80Temp = r80 | 0xf0;
    r81Temp = r81 | 0x07;
  } else if (ppgDrive == PPGDrive::NoTouchPowerSave) {
    r80Temp = (r80 & 0x8f) | 0x30;
    r81Temp = r81 & 0xf8;
    r7aTemp = 0x02;
  }
  // Set special power registers
  WriteRegister(0x85, 0x20);
  WriteRegister(0x7f, r7f);
  WriteRegister(0x80, r80Temp);
  WriteRegister(0x81, r81Temp);
  WriteRegister(0x82, r82);
  WriteRegister(0x85, 0x00);
  WriteRegister(0x7a, r7aTemp);

  uint8_t gain = 0x01;
  switch (ppgGain) {
    case PPGGain::Gain1x:
      break;
    case PPGGain::Gain2x:
      gain |= 0x04;
      break;
    case PPGGain::Gain4x:
      gain |= 0x08;
      break;
    case PPGGain::Gain8x:
      gain |= 0x0c;
      break;
  }

  // HRS enabled, 800ms wait
  uint8_t enable = 0x80;
  // OSC on, 50% duty cycle (last 4 bits)
  uint8_t drive = 0x2e;
  // See https://github.com/wasp-os/wasp-os/pull/363#issuecomment-1279733038 for acquisition period details
  switch (ppgDrive) {
    // 800ms, 13mA (exact current unknown due to special power registers)
    case PPGDrive::NoTouchPowerSave:
      break;
    // 48ms, 13mA/Lower
    case PPGDrive::Current13mALow:
    case PPGDrive::Current13mA:
      enable |= 0x70;
      break;
    // 48ms, 20mA
    case PPGDrive::Current20mA:
      enable |= 0x70;
      drive |= 0x40;
      break;
    // 48ms, 30mA
    case PPGDrive::Current30mA:
      enable |= 0x78;
      break;
    // 48ms, 40mA/Higher
    case PPGDrive::Current40mA:
    case PPGDrive::Current40mAHigh:
      enable |= 0x78;
      drive |= 0x40;
      break;
  }
  WriteRegister(static_cast<uint8_t>(Registers::HGain), gain);
  WriteRegister(static_cast<uint8_t>(Registers::Enable), enable);
  WriteRegister(static_cast<uint8_t>(Registers::PDriver), drive);
}

Hrs3300::PPGState Hrs3300::AutoGain(uint16_t hrs, uint16_t als) {
  PPGDrive previousPpgDrive = ppgDrive;
  PPGGain previousPpgGain = ppgGain;

  if (hrs > hrsHighThresh) {
    hrsHighCount = std::max(hrsHighCount, static_cast<uint8_t>(hrsHighCount + 1));
  } else {
    hrsHighCount = 0;
  }
  if (hrs < hrsLowThresh) {
    hrsLowCount = std::max(hrsLowCount, static_cast<uint8_t>(hrsLowCount + 1));
  } else {
    hrsLowCount = 0;
  }

  if (als > alsHighThresh) {
    alsHighCount = std::max(alsHighCount, static_cast<uint8_t>(alsHighCount + 1));
  } else {
    alsHighCount = 0;
  }
  if (als < alsLowThresh) {
    alsLowCount = std::max(alsLowCount, static_cast<uint16_t>(alsLowCount + 1));
  } else {
    alsLowCount = 0;
  }

  if (actionDelay > 0) {
    actionDelay--;
    return ppgState;
  }

  // No touch handling
  if (ppgState == PPGState::NoTouch) {
    // If touch regained, reinitialise
    if (hrs >= touchRegainedThresh) {
      // Disable then enable to avoid waiting for the 800ms inter-sample delay in no touch mode
      Disable();
      Enable();
      return PPGState::Reset;
    }
    // Otherwise return early as waiting until touch is regained
    return ppgState;
  }

  // Drive changes
  if (hrsHighCount > powerChangeDelay) {
    if (ppgDrive == PPGDrive::Current40mAHigh) {
      ppgDrive = PPGDrive::Current40mA;
    } else if (ppgDrive == PPGDrive::Current40mA) {
      ppgDrive = PPGDrive::Current30mA;
    } else if (ppgDrive == PPGDrive::Current30mA) {
      ppgDrive = PPGDrive::Current20mA;
    } else if (ppgDrive == PPGDrive::Current20mA) {
      ppgDrive = PPGDrive::Current13mA;
    } else if (ppgDrive == PPGDrive::Current13mA) {
      ppgDrive = PPGDrive::Current13mALow;
    }
  } else if (hrsLowCount > powerChangeDelay) {
    if (ppgDrive == PPGDrive::Current13mALow) {
      ppgDrive = PPGDrive::Current13mA;
    } else if (ppgDrive == PPGDrive::Current13mA) {
      ppgDrive = PPGDrive::Current20mA;
    } else if (ppgDrive == PPGDrive::Current20mA) {
      ppgDrive = PPGDrive::Current30mA;
    } else if (ppgDrive == PPGDrive::Current30mA) {
      ppgDrive = PPGDrive::Current40mA;
    } else if (ppgDrive == PPGDrive::Current40mA) {
      ppgDrive = PPGDrive::Current40mAHigh;
    }
  }

  // Gain changes
  if (alsHighCount > powerChangeDelay) {
    if (ppgGain == PPGGain::Gain8x) {
      ppgGain = PPGGain::Gain4x;
    } else if (ppgGain == PPGGain::Gain4x) {
      ppgGain = PPGGain::Gain2x;
    } else if (ppgGain == PPGGain::Gain2x) {
      ppgGain = PPGGain::Gain1x;
    }
  } else if (alsLowCount > gainUpWaitTime) {
    if (ppgGain == PPGGain::Gain1x) {
      ppgGain = PPGGain::Gain2x;
    } else if (ppgGain == PPGGain::Gain2x) {
      ppgGain = PPGGain::Gain4x;
    } else if (ppgGain == PPGGain::Gain4x) {
      ppgGain = PPGGain::Gain8x;
    }
    if (ppgGain != previousPpgGain) {
      alsLowCount = gainUpWaitTime - gainUpCascadeWaitTime;
    }
  }

  // Transition into no touch if maximum drive and hrs low
  // Otherwise apply any gain/drive changes
  if (ppgDrive == PPGDrive::Current40mAHigh && previousPpgDrive == PPGDrive::Current40mAHigh && hrs < touchLostThresh) {
    actionDelay = noTouchDelay;
    ppgState = PPGState::NoTouch;
    ppgDrive = PPGDrive::NoTouchPowerSave;
    ppgGain = PPGGain::Gain8x;
    SetPower();
  } else if (ppgDrive != previousPpgDrive || ppgGain != previousPpgGain) {
    actionDelay = powerChangeDelay;
    SetPower();
  }
  return ppgState;
}
