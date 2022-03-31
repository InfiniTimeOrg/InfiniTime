#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingColor : public Screen{
        public:
          SettingColor(DisplayApp* app, Pinetime::Controllers::Settings &settingsController);
          ~SettingColor() override;

          void UpdateSelected(lv_obj_t *object, lv_event_t event);
         
        private:          
          Controllers::Settings& settingsController;

          enum class Page: uint8_t {
            Primary = 0,
            Secondary = 1,
            Surface = 2,
            Background = 3
          };

          void setPage(Page newPage);

          Page currentPage = Page::Primary;
        
          Pinetime::Controllers::Settings::Colors getCurrentColor();
          void setCurrentColor(Pinetime::Controllers::Settings::Colors color);
          void nextColor();
          void prevColor();

          uint8_t getCurrentTint();
          void setCurrentTint(uint8_t tint);
          void nextTint();
          void prevTint();

          Pinetime::Controllers::Settings::ColorScheme colorScheme;

          void updateUI();

          lv_obj_t * btnNextColor;
          lv_obj_t * btnPrevColor;
          lv_obj_t * btnNextTint;
          lv_obj_t * btnPrevTint;
          lv_obj_t * btnOpacity;
          lv_obj_t * btnReset;
          lv_obj_t * elementColor;
          lv_obj_t * elementTint;
          lv_obj_t * labelColor;
          lv_obj_t * labelTint;
          lv_obj_t * btnNextPage;
          lv_obj_t * btnPrevPage;
          // lv_obj_t * backgroundLabel;
      };
    }
  }
}