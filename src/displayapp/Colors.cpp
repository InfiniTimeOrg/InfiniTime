#include "Colors.h"

using namespace Pinetime::Applications;
using namespace Pinetime::Controllers;

lv_color_t Pinetime::Applications::Convert(Pinetime::Controllers::Settings::Colors color) {
  switch (color) {
    case Pinetime::Controllers::Settings::Colors::White: return LV_COLOR_WHITE;
    case Pinetime::Controllers::Settings::Colors::Silver: return LV_COLOR_SILVER;
    case Pinetime::Controllers::Settings::Colors::Gray: return LV_COLOR_GRAY;
    case Pinetime::Controllers::Settings::Colors::Black: return LV_COLOR_BLACK;
    case Pinetime::Controllers::Settings::Colors::Red: return LV_COLOR_RED;
    case Pinetime::Controllers::Settings::Colors::Maroon: return LV_COLOR_MAROON;
    case Pinetime::Controllers::Settings::Colors::Yellow: return LV_COLOR_YELLOW;
    case Pinetime::Controllers::Settings::Colors::Olive: return LV_COLOR_OLIVE;
    case Pinetime::Controllers::Settings::Colors::Lime: return LV_COLOR_LIME;
    case Pinetime::Controllers::Settings::Colors::Green: return LV_COLOR_GREEN;
    case Pinetime::Controllers::Settings::Colors::Cyan: return LV_COLOR_CYAN;
    case Pinetime::Controllers::Settings::Colors::Teal: return LV_COLOR_TEAL;
    case Pinetime::Controllers::Settings::Colors::Blue: return LV_COLOR_BLUE;
    case Pinetime::Controllers::Settings::Colors::Navy: return LV_COLOR_NAVY;
    case Pinetime::Controllers::Settings::Colors::Magenta: return LV_COLOR_MAGENTA;
    case Pinetime::Controllers::Settings::Colors::Purple: return LV_COLOR_PURPLE;
    case Pinetime::Controllers::Settings::Colors::Orange: return LV_COLOR_ORANGE;
    default: return LV_COLOR_WHITE;
  }
}
