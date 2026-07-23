#include "displayapp/screens/settings/SettingLanguage.h"

#include <array>
#include <functional>
#include <lvgl/lvgl.h>

#include "displayapp/localization/Localization.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::Applications::Localization;

namespace {
  struct Option {
    Language language;
    StringId name;
  };

  constexpr std::array<Option, 11> options {{
    {Language::English, StringId::English},
    {Language::Spanish, StringId::Spanish},
    {Language::Portuguese, StringId::Portuguese},
    {Language::Russian, StringId::Russian},
    {Language::French, StringId::French},
    {Language::German, StringId::German},
    {Language::Italian, StringId::Italian},
    {Language::Turkish, StringId::Turkish},
    {Language::Polish, StringId::Polish},
    {Language::Catalan, StringId::Catalan},
    {Language::Basque, StringId::Basque},
  }};

  std::array<CheckboxList::Item, CheckboxList::MaxItems> CreateOptionArray(Language language, uint8_t screenId) {
    std::array<CheckboxList::Item, CheckboxList::MaxItems> optionArray;
    for (size_t i = 0; i < CheckboxList::MaxItems; i++) {
      const auto optionIndex = i + (screenId * CheckboxList::MaxItems);
      if (optionIndex >= options.size()) {
        optionArray[i].name = "";
        optionArray[i].enabled = false;
      } else {
        optionArray[i].name = Translate(language, options[optionIndex].name);
        optionArray[i].enabled = true;
      }
    }
    return optionArray;
  }

  uint32_t GetDefaultOption(Language currentOption) {
    for (size_t i = 0; i < options.size(); i++) {
      if (options[i].language == currentOption) {
        return i;
      }
    }
    return 0;
  }

  class LanguagePage : public Screen {
  public:
    LanguagePage(uint8_t screenNum, uint8_t numScreens, Pinetime::Controllers::Settings& settingsController)
      : settingsController {settingsController},
        screenNum {screenNum},
        checkboxList(
          screenNum,
          numScreens,
          Translate(settingsController.GetLanguage(), StringId::Language),
          Symbols::map,
          GetDefaultOption(settingsController.GetLanguage()),
          [this](uint32_t index) {
            if (index >= options.size()) {
              return;
            }
            const auto language = options[index].language;
            this->settingsController.SetLanguage(language);
            this->settingsController.SaveSettings();
            UpdateScreen(language);
          },
          CreateOptionArray(settingsController.GetLanguage(), screenNum)) {
    }

    ~LanguagePage() override {
      lv_obj_clean(lv_scr_act());
    }

  private:
    void UpdateScreen(Language language) {
      checkboxList.SetTitle(Translate(language, StringId::Language));
      checkboxList.SetSymbol(Symbols::map);
      checkboxList.SetOptions(CreateOptionArray(language, screenNum));
    }

    Pinetime::Controllers::Settings& settingsController;
    uint8_t screenNum;
    CheckboxList checkboxList;
  };
}

auto SettingLanguage::CreateScreenList() const {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screenList;
  for (size_t i = 0; i < screenList.size(); i++) {
    screenList[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screenList;
}

std::unique_ptr<Screen> SettingLanguage::CreateScreen(unsigned int screenNum) const {
  return std::make_unique<LanguagePage>(screenNum, nScreens, settingsController);
}

SettingLanguage::SettingLanguage(DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : settingsController {settingsController},
    screens {app, static_cast<uint8_t>(GetDefaultOption(settingsController.GetLanguage()) / CheckboxList::MaxItems), CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingLanguage::~SettingLanguage() {
  lv_obj_clean(lv_scr_act());
}

bool SettingLanguage::OnTouchEvent(TouchEvents event) {
  return screens.OnTouchEvent(event);
}
