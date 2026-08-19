#pragma once

#include <array>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class BleClient;

    class ServiceDiscovery {
    public:
      ServiceDiscovery(std::array<BleClient*, 3>&& bleClients);

      void StartDiscovery(uint16_t connectionHandle);

    private:
      BleClient** clientIterator;
      std::array<BleClient*, 3> clients;
      void OnServiceDiscovered(uint16_t connectionHandle);
      void DiscoverNextService(uint16_t connectionHandle);
    };
  }
}
