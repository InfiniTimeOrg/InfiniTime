#pragma once

#include <FreeRTOS.h>
#include <queue.h>

namespace Pinetime {
  namespace Controllers {
    class Ble {
      public:
        enum class FirmwareUpdateStates {Idle, Running, Validated, Error};

        Ble() = default;
        bool IsConnected() const {return isConnected;}
        void Connect();
        void Disconnect();

        void StartFirmwareUpdate();
        void StopFirmwareUpdate();
        void FirmwareUpdateTotalBytes(uint32_t totalBytes);
        void FirmwareUpdateCurrentBytes(uint32_t currentBytes);
        void State(FirmwareUpdateStates state) { firmwareUpdateState = state; }

        bool IsFirmwareUpdating() const { return isFirmwareUpdating; }
        uint32_t FirmwareUpdateTotalBytes() const { return firmwareUpdateTotalBytes; }
        uint32_t FirmwareUpdateCurrentBytes() const { return firmwareUpdateCurrentBytes; }
        FirmwareUpdateStates State() const { return firmwareUpdateState; }
      private:
        bool isConnected = false;
        bool isFirmwareUpdating = false;
        uint32_t firmwareUpdateTotalBytes = 0;
        uint32_t firmwareUpdateCurrentBytes = 0;
        FirmwareUpdateStates firmwareUpdateState = FirmwareUpdateStates::Idle;

    };
  }
}