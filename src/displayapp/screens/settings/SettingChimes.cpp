#include "displayapp/screens/settings/SettingChimes.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <array>

using namespace Pinetime::Applications::Screens;

namespace {
  struct Option {
    Pinetime::Controllers::Settings::ChimesOption chimesOption;
    const char* name;
  };

  constexpr std::array<Option, 3> options = {{
    {Pinetime::Controllers::Settings::ChimesOption::None, "Off"},
    {Pinetime::Controllers::Settings::ChimesOption::Hours, "Every hour"},
    {Pinetime::Controllers::Settings::ChimesOption::HalfHours, "Every 30 mins"},
  }};

  std::array<CheckboxList::Item, CheckboxList::MaxItems> CreateOptionArray() {
    std::array<Pinetime::Applications::Screens::CheckboxList::Item, CheckboxList::MaxItems> optionArray;
    for (size_t i = 0; i < CheckboxList::MaxItems; i++) {
      if (i >= options.size()) {
        optionArray[i].name = "";
        optionArray[i].enabled = false;
      } else {
        optionArray[i].name = options[i].name;
        optionArray[i].enabled = true;
      }
    }
    return optionArray;
  }
}

constexpr std::array<SettingChimes::Option, 4> SettingChimes::options;

SettingChimes::SettingChimes(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController} {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Chimes");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 10, 15);

  uint32_t GetDefaultOption(Pinetime::Controllers::Settings::ChimesOption currentOption) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].chimesOption == currentOption) {
        return i;
      }
    }
    return 0;
  }
}

SettingChimes::SettingChimes(Pinetime::Controllers::Settings& settingsController)
  : checkboxList(
      0,
      1,
      "Chimes",
      Symbols::clock,
      GetDefaultOption(settingsController.GetChimeOption()),
      [&settings = settingsController](uint32_t index) {
        settings.SetChimeOption(options[index].chimesOption);
        settings.SaveSettings();
      },
      CreateOptionArray()) {
}

SettingChimes::~SettingChimes() {
  lv_obj_clean(lv_scr_act());
}
