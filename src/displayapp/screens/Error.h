#pragma once

#include "displayapp/screens/Screen.h"
#include "BootErrors.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Error : public Screen {
      public:
        Error(System::BootErrors error);
        ~Error() override;

        void ButtonEventHandler();

      private:
        lv_obj_t* btnOk;
      };
    }
  }
}
