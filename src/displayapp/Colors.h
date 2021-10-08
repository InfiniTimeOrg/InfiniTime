#pragma once

#include <lvgl/src/lv_misc/lv_color.h>
//#include <components/settings/Settings.h>

namespace Pinetime {
  namespace Applications {

    enum class Colors : uint8_t {
      White,
      Silver,
      Gray,
      Black,
      Red,
      Maroon,
      Yellow,
      Olive,
      Lime,
      Green,
      Cyan,
      Teal,
      Blue,
      Navy,
      Magenta,
      Purple,
      Orange
    };

    lv_color_t Convert(Colors color);
  }
}