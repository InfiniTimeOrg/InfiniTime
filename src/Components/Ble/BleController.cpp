#include "BleController.h"

using namespace Pinetime::Controllers;

void Ble::Connect() {
  isConnected = true;
}

void Ble::Disconnect() {
  isConnected = false;
}
