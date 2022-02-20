#pragma once

#include <array>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class Ble {
    public:
      using BleAddress = std::array<uint8_t, 6>;
      enum class FirmwareUpdateStates { Idle, Running, Validated, Error };
      enum class AddressTypes { Public, Random, RPA_Public, RPA_Random };
      enum class ConnectStates { Disconnected, Connected, Airplane };

      Ble() = default;
      bool IsConnected() const {
        return (connectionState == ConnectStates::Connected);
      }
      void SetConnectState(ConnectStates newState);
      ConnectStates GetConnectState() const;

      void StartFirmwareUpdate();
      void StopFirmwareUpdate();
      void FirmwareUpdateTotalBytes(uint32_t totalBytes);
      void FirmwareUpdateCurrentBytes(uint32_t currentBytes);
      void State(FirmwareUpdateStates state) {
        firmwareUpdateState = state;
      }

      bool IsFirmwareUpdating() const {
        return isFirmwareUpdating;
      }
      uint32_t FirmwareUpdateTotalBytes() const {
        return firmwareUpdateTotalBytes;
      }
      uint32_t FirmwareUpdateCurrentBytes() const {
        return firmwareUpdateCurrentBytes;
      }
      FirmwareUpdateStates State() const {
        return firmwareUpdateState;
      }

      void Address(BleAddress&& addr) {
        address = addr;
      }
      const BleAddress& Address() const {
        return address;
      }
      void AddressType(AddressTypes t) {
        addressType = t;
      }
      void SetPairingKey(uint32_t k) {
        pairingKey = k;
      }
      uint32_t GetPairingKey() const {
        return pairingKey;
      }

    private:
      ConnectStates connectionState = ConnectStates::Disconnected;
      bool isFirmwareUpdating = false;
      uint32_t firmwareUpdateTotalBytes = 0;
      uint32_t firmwareUpdateCurrentBytes = 0;
      FirmwareUpdateStates firmwareUpdateState = FirmwareUpdateStates::Idle;
      BleAddress address;
      AddressTypes addressType;
      uint32_t pairingKey = 0;
    };
  }
}
