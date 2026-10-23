#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include <atomic>
#include <memory>
#include <vector>

namespace Pinetime {
  namespace Controllers {
    class NimbleController;

    class HomeService {
    public:
      enum ComponentType : uint8_t {
        Button,
        Label,
      };

      struct Component {
        ComponentType type;
        uint8_t x, y, w, h;
        std::unique_ptr<char[]> label;
      };

      struct Screen {
        uint8_t index, rows, cols;
        std::vector<Component> components {};
      };

      HomeService(NimbleController& nimble);
      void Init();

      int OnCommand(struct ble_gatt_access_ctxt* ctxt);

      bool OnOpened();
      void OnViewScreen(uint8_t n);
      void OnClosed();
      void OnPressed(uint8_t screen, uint8_t componentId);

      TickType_t DataUpdateTime() {
        return dataUpdateTime;
      }

      const Screen& CurrentScreen() {
        return *currentScreen.get();
      }

      uint8_t NumScreens() {
        return numScreens;
      }

    private:
      NimbleController& nimble;

      uint16_t eventOpenedHandle {}, eventPressedHandle;

      std::unique_ptr<Screen> currentScreen;
      TickType_t dataUpdateTime = 0;
      uint8_t numScreens;

      struct ble_gatt_chr_def characteristicDefinition[4];
      struct ble_gatt_svc_def serviceDefinition[2];

      bool NotifyOpened(int8_t screenIndex);
    };
  }
}
