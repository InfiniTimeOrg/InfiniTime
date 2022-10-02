#pragma once

#include "displayapp/Apps.h"
#include "displayapp/screens/Screen.h"
#include <array>
#include <cstdint>
#include <functional>
#include <lvgl/lvgl.h>
#include <memory>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class CheckboxList : public Screen {
      public:
        static constexpr size_t MaxItems = 4;
        CheckboxList(const uint8_t screenID,
                     const uint8_t numScreens,
                     DisplayApp* app,
                     const char* optionsTitle,
                     const char* optionsSymbol,
                     uint32_t originalValue,
                     std::function<void(uint32_t)>OnValueChanged,
                     std::array<const char*, MaxItems> options);
        ~CheckboxList() override;
        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        const uint8_t screenID;
        std::function<void(uint32_t)>OnValueChanged;
        std::array<const char*, MaxItems> options;
        std::array<lv_obj_t*, MaxItems> cbOption;
        std::array<lv_point_t, 2> pageIndicatorBasePoints;
        std::array<lv_point_t, 2> pageIndicatorPoints;
        lv_obj_t* pageIndicatorBase;
        lv_obj_t* pageIndicator;
        uint32_t newValue;
      };
    }
  }
}
