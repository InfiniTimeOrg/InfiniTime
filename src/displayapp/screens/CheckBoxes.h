#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"

namespace Pinetime::Applications::Screens {
  class CheckBoxes : public Screen {
  public:
    struct Options {
      bool state;
      const char* title;
    };
    CheckBoxes(const char* symbol, const char* titleText, Options *options, DisplayApp* app);
    ~CheckBoxes() override;

    bool Refresh() override;
    void UpdateSelected(lv_obj_t* object, lv_event_t event);

  private:
    struct Options *options;

    uint8_t optionsTotal;
    lv_obj_t* buttons[6];
    lv_style_t buttonStyle;
  };
}
