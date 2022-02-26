#include "components/ble/BleController.h"

using namespace Pinetime::Controllers;

bool Ble::IsConnected() const {
  return isConnected;
}

void Ble::Connect() {
  isConnected = true;
}

void Ble::Disconnect() {
  isConnected = false;
}

bool Ble::IsRadioEnabled() const {
  return isRadioEnabled;
}

void Ble::EnableRadio() {
  isRadioEnabled = true;
}

void Ble::DisableRadio() {
  isRadioEnabled = false;
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
