#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include <array>
#include <cstdint>
#include <functional>
#include <lvgl/lvgl.h>
#include <memory>
#include "displayapp/widgets/PageIndicator.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class CheckboxList : public Screen {
      public:
        static constexpr size_t MaxItems = 4;

        struct Item {
          const char* name;
          bool enabled;
        };

        CheckboxList(const uint8_t screenID,
                     const uint8_t numScreens,
                     const char* optionsTitle,
                     const char* optionsSymbol,
                     uint32_t originalValue,
                     std::function<void(uint32_t)> OnValueChanged,
                     std::array<Item, MaxItems> options);
        ~CheckboxList() override;
        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        const uint8_t screenID;
        std::function<void(uint32_t)> OnValueChanged;
        std::array<Item, MaxItems> options;
        std::array<lv_obj_t*, MaxItems> cbOption;
        uint32_t value;

        Widgets::PageIndicator pageIndicator;
      };
    }
  }
}
