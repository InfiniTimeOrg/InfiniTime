#pragma once

#include <vector>
#include "Screen.h"

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
    }
  }
}