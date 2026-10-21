#pragma once

#include "displayapp/LittleVgl.h"
#include <memory>

namespace Pinetime {
  namespace Components {
    namespace FastFont {
      static constexpr const lv_font_t* FallbackFont = &jetbrains_mono_42;

      // Since the font pointer is actually a u8 array containing all loaded font data
      // we need to use the deleter for the true datatype rather than the single struct
      struct CastingDeleter {
        void operator()(const lv_font_t* fontData) const {
          // Don't try to free the fallback!
          if (fontData == FallbackFont) {
            return;
          }
          const auto* fontDataReal = reinterpret_cast<const uint8_t*>(fontData);
          std::default_delete<const uint8_t[]> {}(fontDataReal);
        }
      };

      using Font = std::unique_ptr<const lv_font_t, CastingDeleter>;
      // Returns a fallback if loading fails
      Font LoadFont(Pinetime::Controllers::FS& filesystem, const char* fontPath);
    }
  }
}
