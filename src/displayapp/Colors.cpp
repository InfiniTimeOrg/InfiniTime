#include "Colors.h"

using namespace Pinetime::Applications;

lv_color_t Pinetime::Applications::Convert(Colors color) {
  switch (color) {
    case Colors::White:
      return LV_COLOR_WHITE;
    case Colors::Silver:
      return LV_COLOR_SILVER;
    case Colors::Gray:
      return LV_COLOR_GRAY;
    case Colors::Black:
      return LV_COLOR_BLACK;
    case Colors::Red:
      return LV_COLOR_RED;
    case Colors::Maroon:
      return LV_COLOR_MAROON;
    case Colors::Yellow:
      return LV_COLOR_YELLOW;
    case Colors::Olive:
      return LV_COLOR_OLIVE;
    case Colors::Lime:
      return LV_COLOR_LIME;
    case Colors::Green:
      return LV_COLOR_GREEN;
    case Colors::Cyan:
      return LV_COLOR_CYAN;
    case Colors::Teal:
      return LV_COLOR_TEAL;
    case Colors::Blue:
      return LV_COLOR_BLUE;
    case Colors::Navy:
      return LV_COLOR_NAVY;
    case Colors::Magenta:
      return LV_COLOR_MAGENTA;
    case Colors::Purple:
      return LV_COLOR_PURPLE;
    case Colors::Orange:
      return LV_COLOR_ORANGE;
    default:
      return LV_COLOR_WHITE;
  }
}
