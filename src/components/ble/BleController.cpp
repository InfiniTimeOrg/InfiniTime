#include "components/ble/BleController.h"

using namespace Pinetime::Controllers;

void Ble::SetConnectState(Ble::ConnectStates newState) {
  connectionState = newState;
}

Ble::ConnectStates Ble::GetConnectState() const {
  return connectionState;
}

void Ble::StartFirmwareUpdate() {
  isFirmwareUpdating = true;
}

void Ble::StopFirmwareUpdate() {
  isFirmwareUpdating = false;
}

void Ble::FirmwareUpdateTotalBytes(uint32_t totalBytes) {
  firmwareUpdateTotalBytes = totalBytes;
}

void Ble::FirmwareUpdateCurrentBytes(uint32_t currentBytes) {
  firmwareUpdateCurrentBytes = currentBytes;
}
