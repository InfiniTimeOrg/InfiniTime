#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class PopupMessage {
      public:
        // The caller owns the message string, it is not copied.
        PopupMessage(const char* msg);
        void Create();
        void SetHidden(bool hidden);
        bool IsHidden();

      private:
        const char* message;
        lv_obj_t* btnPopup = nullptr;
        bool isHidden = true;
      };
    }
  }
}
