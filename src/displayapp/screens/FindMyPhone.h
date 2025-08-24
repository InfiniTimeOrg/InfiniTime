#pragma once

#include <cstdint>
#include <chrono>
#include <optional>
#include "displayapp/screens/Screen.h"
#include "Symbols.h"
#include "systemtask/SystemTask.h"
#include "components/ble/ImmediateAlertClient.h"

#include <lvgl/src/lv_core/lv_style.h>
#include <lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {

  namespace Controllers {
    class ImmediateAlertClient;
  }

  namespace Applications {
    namespace Screens {

      class FindMyPhone : public Screen {
      public:
        explicit FindMyPhone(Pinetime::Controllers::ImmediateAlertClient& immediateAlertClient);
        ~FindMyPhone() override;

        void OnImmediateAlertEvent(lv_obj_t* obj, lv_event_t event);

        void StopRestoreLabelTask();
        void RestoreLabelText();
        void Refresh() override;

      private:
        Pinetime::Controllers::ImmediateAlertClient& immediateAlertClient;

        struct LabelState {
          const char* text;
          lv_color_t color;
        };

        static const LabelState stoppedLabelState;
        static const LabelState noConnectionLabelState;
        static const LabelState noServiceLabelState;
        static const LabelState defaultLabelState;
        static const LabelState alertingLabelState;
        static const LabelState sendFailedLabelState;
        const LabelState& GetLabelState() const;

        lv_obj_t* container;
        lv_obj_t* lblTitle;
        lv_obj_t* btnStop;
        lv_obj_t* btnRing;
        lv_obj_t* lblStop;
        lv_obj_t* lblRing;
        lv_task_t* refreshTask = nullptr;

        bool lastSendFailed = false;
        std::optional<Pinetime::Controllers::ImmediateAlertClient::Levels> lastUserInitiatedLevel;
      };
    }

    template <>
    struct AppTraits<Apps::FindMyPhone> {
      static constexpr Apps app = Apps::FindMyPhone;
      static constexpr const char* icon = Screens::Symbols::magnifyingGlass;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::FindMyPhone(controllers.systemTask->nimble().immediateAlertClient());
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
