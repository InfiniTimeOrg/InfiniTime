#include "displayapp/screens/settings/SettingDateFormat.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  struct DateOption {
    Pinetime::Controllers::Settings::DateFormat dateFormat;
    const char* name;
  };

  constexpr std::array<DateOption, 4> dateOptions = {{
    {Pinetime::Controllers::Settings::DateFormat::YYYYMMDD, "yyyy-mm-dd"},
    {Pinetime::Controllers::Settings::DateFormat::DDMMYYYY, "dd/mm/yyyy"},
    {Pinetime::Controllers::Settings::DateFormat::MMDDYYYY, "mm/dd/yyyy"},
    {Pinetime::Controllers::Settings::DateFormat::DayDDMonthYYYY, "Day DD Month YYYY"},
  }};

  std::array<CheckboxList::Item, CheckboxList::MaxItems> CreateDateOptionArray() {
    std::array<Pinetime::Applications::Screens::CheckboxList::Item, CheckboxList::MaxItems> dateOptionArray;
    for (size_t i = 0; i < CheckboxList::MaxItems; i++) {
      if (i >= dateOptions.size()) {
        dateOptionArray[i].name = "";
        dateOptionArray[i].enabled = false;
      } else {
        dateOptionArray[i].name = dateOptions[i].name;
        dateOptionArray[i].enabled = true;
      }
    }
    return dateOptionArray;
  }

  uint32_t GetDefaultDateOption(Pinetime::Controllers::Settings::DateFormat currentOption) {
    for (size_t i = 0; i < dateOptions.size(); i++) {
      if (dateOptions[i].dateFormat == currentOption) {
        return i;
      }
    }
    return 0;
  }
}

SettingDateFormat::SettingDateFormat(Pinetime::Controllers::Settings& settingsController)
  : dateCheckboxList(
      0,
      1,
      "Date format",
      Symbols::clock,
      GetDefaultDateOption(settingsController.GetDateFormat()),
      [&settings = settingsController](uint32_t index) {
        settings.SetDateFormat(dateOptions[index].dateFormat);
        settings.SaveSettings();
      },
      CreateDateOptionArray()) {
}

SettingDateFormat::~SettingDateFormat() {
  lv_obj_clean(lv_scr_act());
}
