#pragma once

#include "Screen.h"
#include "BootErrors.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Error : public Screen {
      public:
        Error(DisplayApp* app, System::BootErrors error);
        ~Error() override;

        bool Refresh() override;
        void ButtonEventHandler();
      private:
        lv_obj_t* btnOk;
      };
    }
  }
}
