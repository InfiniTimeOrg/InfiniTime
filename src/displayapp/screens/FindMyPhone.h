#pragma once

#include <cstdint>
#include <chrono>
#include "displayapp/screens/Screen.h"
#include "Symbols.h"
#include "systemtask/SystemTask.h"
#include "components/ble/ImmediateAlertService.h"

#include <lvgl/src/lv_core/lv_style.h>
#include <lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {

  namespace Controllers {
    class ImmediateAlertService;
  }

  namespace Applications {
    namespace Screens {

      class FindMyPhone : public Screen {
      public:
        FindMyPhone(Pinetime::Controllers::ImmediateAlertService& immediateAlertService);
        ~FindMyPhone() override;

        void OnImmediateAlertEvent(lv_obj_t* obj, lv_event_t event);

      private:
        Pinetime::Controllers::ImmediateAlertService& immediateAlertService;

        void UpdateImmediateAlerts();

        lv_obj_t* container;
        lv_obj_t* label_title;
        lv_obj_t* bt_none;
        lv_obj_t* bt_high;
        lv_obj_t* bt_mild;
        lv_obj_t* label_none;
        lv_obj_t* label_high;
        lv_obj_t* label_mild;


        Pinetime::Controllers::ImmediateAlertService::Levels last_level;
      };
    }

    template <>
    struct AppTraits<Apps::FindMyPhone> {
      static constexpr Apps app = Apps::FindMyPhone;
      static constexpr const char* icon = Screens::Symbols::magnifyingGlass ;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::FindMyPhone(controllers.systemTask->nimble().immediateAlertClient());
      };
    };
  }
}
