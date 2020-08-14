#pragma once

#include <vector>
#include "Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Label {
        public:
          Label() = default;
          explicit Label(const char* text);
          ~Label();
          void Refresh();

          void Hide();
          void Show();
        private:
          lv_obj_t * label = nullptr;
          const char* text = nullptr;
      };

      class Label2 : public Screen {
        public:
          Label2(DisplayApp* app, const char* text) : Screen(app), text{text} {
            label = lv_label_create(lv_scr_act(), NULL);
            lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
            lv_obj_set_size(label, 240, 240);
            lv_label_set_text(label, text);
          }

          ~Label2() override {
            lv_obj_clean(lv_scr_act());
          }

          bool Refresh() override {return false;}

        private:
          lv_obj_t * label = nullptr;
          const char* text = nullptr;
      };
    }
  }
}