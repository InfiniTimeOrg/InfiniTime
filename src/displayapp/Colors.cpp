#include "displayapp/Colors.h"

using namespace Pinetime::Applications;
using namespace Pinetime::Controllers;

lv_color_t Pinetime::Applications::Convert(Pinetime::Controllers::Settings::Colors color) {
  switch (color) {
    case Pinetime::Controllers::Settings::Colors::White:
      return LV_COLOR_MAKE(0xfb, 0xf1, 0xc7);
    case Pinetime::Controllers::Settings::Colors::Silver:
      return LV_COLOR_MAKE(0xa8, 0x99, 0x84);
    case Pinetime::Controllers::Settings::Colors::Gray:
      return LV_COLOR_MAKE(0x92, 0x83, 0x74);
    case Pinetime::Controllers::Settings::Colors::Black:
      return LV_COLOR_MAKE(0x1d, 0x20, 0x21);
    case Pinetime::Controllers::Settings::Colors::Red:
      return LV_COLOR_MAKE(0xfb, 0x49, 0x34);
    case Pinetime::Controllers::Settings::Colors::Maroon:
      return LV_COLOR_MAKE(0xcc, 0x24, 0x1d);
    case Pinetime::Controllers::Settings::Colors::Yellow:
      return LV_COLOR_MAKE(0xfa, 0xbd, 0x2f);
    case Pinetime::Controllers::Settings::Colors::Olive:
      return LV_COLOR_MAKE(0xd7, 0x99, 0x21);
    case Pinetime::Controllers::Settings::Colors::Lime:
      return LV_COLOR_MAKE(0xb8, 0xbb, 0x26);
    case Pinetime::Controllers::Settings::Colors::Green:
      return LV_COLOR_MAKE(0x98, 0x97, 0x1a);
    case Pinetime::Controllers::Settings::Colors::Cyan:
      return LV_COLOR_MAKE(0x8e, 0xc0, 0x7c);
    case Pinetime::Controllers::Settings::Colors::Teal:
      return LV_COLOR_MAKE(0x68, 0x9d, 0x6a);
    case Pinetime::Controllers::Settings::Colors::Blue:
      return LV_COLOR_MAKE(0x83, 0xa5, 0x98);
    case Pinetime::Controllers::Settings::Colors::Navy:
      return LV_COLOR_MAKE(0x45, 0x85, 0x88);
    case Pinetime::Controllers::Settings::Colors::Magenta:
      return LV_COLOR_MAKE(0xd3, 0x86, 0x9b);
    case Pinetime::Controllers::Settings::Colors::Purple:
      return LV_COLOR_MAKE(0xb1, 0x62, 0x86);
    case Pinetime::Controllers::Settings::Colors::Orange:
      return LV_COLOR_MAKE(0xfe, 0x80, 0x19);
    case Pinetime::Controllers::Settings::Colors::Pink:
      return LV_COLOR_MAKE(0xd6, 0x5d, 0x0e);
    default:
      return LV_COLOR_WHITE;
  }
}
