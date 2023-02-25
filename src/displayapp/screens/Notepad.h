#pragma once

#include "Screen.h"
#include <string>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Notepad : public Screen {
      public:
        ~Notepad() override;

        Notepad(DisplayApp* app, char* textptr);
        void update_textarea();
        void next_guess();
        void t9_enter();
        void on_input_len_change();
        void toggle_edit();
        bool c_in_str(char c, const char* s);
        void on_touch_event(lv_obj_t* object, lv_event_t event);

      private:
        lv_obj_t *result, *buttonMatrix, *bspButton, *bspLabel;
        char* text;
        char guess[7] = "";
        char raw_input[7] = "";
        uint8_t tpos = 0, gpos = 0, rpos = 0;
        int guess_idx = 0;
        char eol = '_';
        bool in_edit_mode = true;
        bool ignore_repeat = false;
      };

    }
  }
}
