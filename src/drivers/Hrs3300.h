#pragma once

#include "drivers/TwiMaster.h"
#include <components/heartrate/Ppg.h>
#include <cstdint>

namespace Pinetime {
  namespace Drivers {
    class Hrs3300 {
    public:
      enum class Registers : uint8_t {
        Id = 0x00,
        Enable = 0x01,
        C1DataM = 0x08,
        C0DataM = 0x09,
        C0DataH = 0x0a,
        PDriver = 0x0c,
        C1DataH = 0x0d,
        C1DataL = 0x0e,
        C0DataL = 0x0f,
        Res = 0x16,
        HGain = 0x17
      };
      // Reset is not used internally, it exists to inform the heart
      // rate task that it should reset its state
      enum class PPGState : uint8_t { Off, Running, NoTouch, Reset };

      struct PackedHrsAls {
        uint16_t hrs;
        uint16_t als;
      };

      Hrs3300(TwiMaster& twiMaster, uint8_t twiAddress);
      Hrs3300(const Hrs3300&) = delete;
      Hrs3300& operator=(const Hrs3300&) = delete;
      Hrs3300(Hrs3300&&) = delete;
      Hrs3300& operator=(Hrs3300&&) = delete;

      void Init();
      void Enable();
      void Disable();
      PackedHrsAls ReadHrsAls();
      PPGState AutoGain(uint16_t hrsAvg, uint16_t currentAls);

    private:
      enum class PPGDrive : uint8_t {
        NoTouchPowerSave,
        // Special registers set
        Current13mALow,
        Current13mA,
        Current20mA,
        Current30mA,
        Current40mA,
        // Special registers set
        Current40mAHigh,
      };
      enum class PPGGain : uint8_t { Gain1x, Gain2x, Gain4x, Gain8x };
      TwiMaster& twiMaster;
      uint8_t twiAddress;

      uint8_t r7f;
      uint8_t r80;
      uint8_t r81;
      uint8_t r82;
      PPGDrive ppgDrive;
      PPGGain ppgGain;
      PPGState ppgState;

      uint8_t actionDelay;

      uint8_t hrsLowCount;
      uint8_t hrsHighCount;
      uint16_t alsLowCount;
      uint8_t alsHighCount;

      void WriteRegister(uint8_t reg, uint8_t data);
      uint8_t ReadRegister(uint8_t reg);
      void SetPower();
    };
  }
}
