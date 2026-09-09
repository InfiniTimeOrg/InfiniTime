#pragma once

#include "displayapp/screens/Screen.h"
#include "components/settings/Settings.h"
#include <lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {
  namespace Controllers {
    class FirmwareValidator;
  }

  namespace Applications {
    namespace Screens {

      class FirmwareValidation : public Screen {
      public:
        FirmwareValidation(Pinetime::Controllers::FirmwareValidator& validator, Pinetime::Controllers::Settings& settingsController);
        ~FirmwareValidation() override;

        void OnButtonEvent(lv_obj_t* object, lv_event_t event);

      private:
        Pinetime::Controllers::FirmwareValidator& validator;
        Pinetime::Controllers::Settings& settingsController;

        lv_obj_t* labelVersion;
        lv_obj_t* labelIsValidated;
        lv_obj_t* buttonValidate;
        lv_obj_t* labelButtonValidate;
        lv_obj_t* buttonReset;
        lv_obj_t* labelButtonReset;
      };
    }
  }
}
