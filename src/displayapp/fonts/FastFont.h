#pragma once

#include "displayapp/LittleVgl.h"
#include <memory>

namespace Pinetime {
  namespace Components {
    namespace FastFont {
      // Since the font pointer is actually a u8 array containing all loaded font data
      // we need to use the deleter for the true datatype rather than the single struct
      struct CastingDeleter {
        void operator()(lv_font_t* fontData) const {
          auto* fontDataReal = reinterpret_cast<uint8_t*>(fontData);
          std::default_delete<uint8_t[]> {}(fontDataReal);
        }
      };

      using Font = std::unique_ptr<lv_font_t, CastingDeleter>;
      Font LoadFont(Pinetime::Controllers::FS& filesystem, const char* fontPath);
    }
  }
}
