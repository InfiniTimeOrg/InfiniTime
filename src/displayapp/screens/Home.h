#pragma once

#include <FreeRTOS.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include <string>
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "components/ble/HomeService.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Home : public Screen {
      public:
        Home(Pinetime::Controllers::HomeService& home, DisplayApp* app);

        ~Home() override;

        void Refresh() override;
        void OnObjectEvent(lv_obj_t* obj, lv_event_t event);
        bool OnTouchEvent(TouchEvents event) override;

        void RequestUpdate();

      private:
        Pinetime::Controllers::HomeService& homeService;
        DisplayApp* app;

        lv_task_t *taskRefresh, *taskRequestUpdate;

        bool is_connected = false;
        TickType_t updated_at = 0;

        lv_obj_t *label_status, *container;

        uint8_t current_screen = 0;

        void ShowScreen(const Pinetime::Controllers::HomeService::Screen& screen);

        /** Watchapp */
      };
    }

    template <>
    struct AppTraits<Apps::Home> {
      static constexpr Apps app = Apps::Home;
      static constexpr const char* icon = "H";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Home(*controllers.homeService, controllers.displayApp);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
