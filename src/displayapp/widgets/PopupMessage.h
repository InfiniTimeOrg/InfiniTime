#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class PopupMessage {
      public:
        PopupMessage();
        void Create();
        void SetHidden(bool hidden);
        bool IsHidden();

      private:
        lv_obj_t* popup = nullptr;
        bool isHidden = true;
      };
    }
  }
}
