#pragma once

#include <array>
#include <cstdint>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/ble/NimbleController.h"
#include "systemtask/SystemTask.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Trackers : public Screen {
      public:
        explicit Trackers(Controllers::NimbleController& nimble);
        ~Trackers() override;

        void Refresh() override;
        bool OnTouchEvent(TouchEvents event) override;
        void OnButtonEvent(lv_obj_t* object, lv_event_t event);

      private:
        static constexpr uint8_t perPage = 3;
        static constexpr uint8_t liveScanRenewSeconds = 10;

        void UpdateToggle();
        void ShowList();

        Controllers::NimbleController& nimble;
        std::array<Controllers::TrackerDetector::Tracker, Controllers::NimbleController::MaxTrackers> trackers {};
        size_t count = 0;
        uint8_t page = 0;
        uint8_t ticks = 0;

        lv_obj_t* labelScanning;
        lv_obj_t* buttonToggle;
        lv_obj_t* labelToggle;
        lv_obj_t* labelEmpty;
        lv_obj_t* labelPage;
        std::array<lv_obj_t*, perPage> labelNames;
        std::array<lv_obj_t*, perPage> labelDetails;
        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Trackers> {
      static constexpr Apps app = Apps::Trackers;
      static constexpr const char* icon = Screens::Symbols::crosshairs;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Trackers(controllers.systemTask->nimble());
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
