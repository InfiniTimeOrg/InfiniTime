#pragma once

#include "drivers/TwiMaster.h"

namespace Pinetime {
  namespace Drivers {
    class Hrs3300 {
    public:
      enum class Registers : uint8_t {
        Id = 0x00,
        Enable = 0x01,
        EnableHen = 0x80,
        C1dataM = 0x08,
        C0DataM = 0x09,
        C0DataH = 0x0a,
        PDriver = 0x0c,
        C1dataH = 0x0d,
        C1dataL = 0x0e,
        C0dataL = 0x0f,
        Res = 0x16,
        Hgain = 0x17
      };

      Hrs3300(TwiMaster& twiMaster, uint8_t twiAddress);
      Hrs3300(const Hrs3300&) = delete;
      Hrs3300& operator=(const Hrs3300&) = delete;
      Hrs3300(Hrs3300&&) = delete;
      Hrs3300& operator=(Hrs3300&&) = delete;

      void Init();
      void Enable();
      void Disable();
      uint16_t ReadHrs();
      uint16_t ReadAls();
      void SetGain(uint8_t gain);
      void SetDrive(uint8_t drive);

    private:
      TwiMaster& twiMaster;
      uint8_t twiAddress;

      void WriteRegister(uint8_t reg, uint8_t data);
      uint8_t ReadRegister(uint8_t reg);
    };
  }
}
