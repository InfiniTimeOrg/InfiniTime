#pragma once

#include <cstdint>
#include <chrono>
#include <array>
#include "displayapp/screens/Screen.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"
#include <lvgl/src/lv_core/lv_style.h>
#include <lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {
  namespace Controllers {
    class HeartRateController;
  }

  namespace Applications {
    namespace Screens {

      class HeartRateZone : public Screen {
      public:
        HeartRateZone(Controllers::HeartRateController& HeartRateController, System::SystemTask& systemTask);
        ~HeartRateZone() override;

        void Refresh() override;

      private:
        Controllers::HeartRateController& heartRateController;
        Pinetime::System::WakeLock wakeLock;

        std::array<lv_obj_t*, 5> zone_bar = {};
        std::array<lv_obj_t*, 5> label_time = {};
        lv_obj_t* total_bar = nullptr;
        lv_obj_t* total_label = nullptr;
        lv_obj_t* title = nullptr;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::HeartRateZone> {
      static constexpr Apps app = Apps::HeartRateZone;
      static constexpr const char* icon = Screens::Symbols::heartBeat;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::HeartRateZone(controllers.heartRateController, *controllers.systemTask);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
