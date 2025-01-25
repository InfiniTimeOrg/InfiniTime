#include "displayapp/LittleVgl.h"

namespace Pinetime {
  namespace Components {
    namespace FastFont {
      lv_font_t* LoadFont(Pinetime::Controllers::FS& filesystem, const char* fontPath);
    }
  }
}
