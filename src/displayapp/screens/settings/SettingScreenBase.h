#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SettingScreenBase {
      public:
        struct Entry {
          const char* title;
          bool enabled;
        };

        SettingScreenBase(const char* title,
                          const char* icon,
                          Entry entries[],
                          size_t nEntries,
                          lv_obj_t*(*checkboxObjectArray),
                          bool radioButtonStyle);

        void UpdateSelected(lv_obj_t* object);

      private:
        lv_obj_t*(*checkboxObjectArray);
        size_t nEntries;
      };
    }
  }
}
