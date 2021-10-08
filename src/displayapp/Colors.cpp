#include "Colors.h"

using namespace Pinetime::Applications;
using namespace Pinetime::Controllers;

lv_color_t Pinetime::Applications::Convert(Pinetime::Controllers::Settings::Colors color) {
  switch (color) {
    case Pinetime::Controllers::Settings::Colors::White: return lv_color_white();
    case Pinetime::Controllers::Settings::Colors::Silver: return lv_color_hex(0xC0C0C0);
    case Pinetime::Controllers::Settings::Colors::Gray: return lv_palette_main(LV_PALETTE_GREY);
    case Pinetime::Controllers::Settings::Colors::Black: return lv_color_black();
    case Pinetime::Controllers::Settings::Colors::Red: return lv_palette_main(LV_PALETTE_RED);
    case Pinetime::Controllers::Settings::Colors::Maroon: return lv_color_hex(0x800000);
    case Pinetime::Controllers::Settings::Colors::Yellow: return lv_palette_main(LV_PALETTE_YELLOW);
    case Pinetime::Controllers::Settings::Colors::Olive: return lv_color_hex(0x808000);
    case Pinetime::Controllers::Settings::Colors::Lime: return lv_color_hex(0x00FF00);
    case Pinetime::Controllers::Settings::Colors::Green: return lv_palette_main(LV_PALETTE_GREEN);
    case Pinetime::Controllers::Settings::Colors::Cyan: return lv_color_hex(0x00FFFF);
    case Pinetime::Controllers::Settings::Colors::Teal: return lv_color_hex(0x008080);
    case Pinetime::Controllers::Settings::Colors::Blue: return lv_palette_main(LV_PALETTE_BLUE);
    case Pinetime::Controllers::Settings::Colors::Navy: return lv_color_hex(0x000080);
    case Pinetime::Controllers::Settings::Colors::Magenta: return lv_color_hex(0xFF00FF);
    case Pinetime::Controllers::Settings::Colors::Purple: return lv_color_hex(0x800080);
    case Pinetime::Controllers::Settings::Colors::Orange: return lv_palette_main(LV_PALETTE_ORANGE);
    default: return lv_color_white();
  }
}
