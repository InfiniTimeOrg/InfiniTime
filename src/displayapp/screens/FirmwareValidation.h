#pragma once

#include "Screen.h"
#include <lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {
  namespace Controllers {
    class FirmwareValidator;
  }

  namespace Applications {
    namespace Screens {

      class FirmwareValidation : public Screen {
      public:
        FirmwareValidation(DisplayApp* app, Pinetime::Controllers::FirmwareValidator& validator);
        ~FirmwareValidation() override;

        bool Refresh() override;

        void OnButtonEvent(lv_obj_t* object, lv_event_t event);

      private:
        Pinetime::Controllers::FirmwareValidator& validator;

        lv_obj_t* labelVersionInfo;
        lv_obj_t* labelVersionValue;
        lv_obj_t* labelShortRefInfo;
        lv_obj_t* labelShortRefValue;
        char version[9];
        char shortref[9];
        lv_obj_t* labelIsValidated;
        lv_obj_t* buttonValidate;
        lv_obj_t* labelButtonValidate;
        lv_obj_t* buttonReset;
        lv_obj_t* labelButtonReset;
      };
    }
  }
}
