#include "components/ble/ServiceDiscovery.h"
#include <libraries/log/nrf_log.h>
#include "components/ble/BleClient.h"

using namespace Pinetime::Controllers;

ServiceDiscovery::ServiceDiscovery(std::array<BleClient*, 3>&& clients) : clients {clients} {
}

void ServiceDiscovery::StartDiscovery(uint16_t connectionHandle) {
  NRF_LOG_INFO("[Discovery] Starting discovery");
  clientIterator = clients.begin();
  DiscoverNextService(connectionHandle);
}

void ServiceDiscovery::OnServiceDiscovered(uint16_t connectionHandle) {
  clientIterator++;
  if (clientIterator != clients.end()) {
    DiscoverNextService(connectionHandle);
  } else {
    NRF_LOG_INFO("End of service discovery");
  }
}

void ServiceDiscovery::DiscoverNextService(uint16_t connectionHandle) {
  NRF_LOG_INFO("[Discovery] Discover next service");

  auto discoverNextService = [this](uint16_t connectionHandle) {
    this->OnServiceDiscovered(connectionHandle);
  };
  (*clientIterator)->Discover(connectionHandle, discoverNextService);
}
