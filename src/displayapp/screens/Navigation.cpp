/*  Copyright (C) 2021  Adam Pigg

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "Navigation.h"
#include <cstdint>
#include "../DisplayApp.h"
#include "components/ble/NavigationService.h"

using namespace Pinetime::Applications::Screens;

LV_FONT_DECLARE(lv_font_navi_80)

/**
 * Navigation watchapp
 *
 */
Navigation::Navigation(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::NavigationService& nav)
  : Screen(app), navService(nav) {

  imgFlag = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(imgFlag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_navi_80);
  lv_obj_set_style_local_text_color(imgFlag, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  lv_label_set_text(imgFlag, iconForName("flag"));
  lv_obj_align(imgFlag, nullptr, LV_ALIGN_CENTER, 0, -60);

  txtNarrative = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtNarrative, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(txtNarrative, LV_HOR_RES);
  lv_label_set_text(txtNarrative, "Navigation");
  lv_label_set_align(txtNarrative, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(txtNarrative, nullptr, LV_ALIGN_CENTER, 0, 10);

  txtManDist = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtManDist, LV_LABEL_LONG_BREAK);
  lv_obj_set_style_local_text_color(txtManDist, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_obj_set_width(txtManDist, LV_HOR_RES);
  lv_label_set_text(txtManDist, "--M");
  lv_label_set_align(txtManDist, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(txtManDist, nullptr, LV_ALIGN_CENTER, 0, 60);

  // Route Progress
  barProgress = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(barProgress, 200, 20);
  lv_obj_align(barProgress, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_local_bg_color(barProgress, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x222222));
  lv_obj_set_style_local_bg_color(barProgress, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_bar_set_anim_time(barProgress, 500);
  lv_bar_set_range(barProgress, 0, 100);
  lv_bar_set_value(barProgress, 0, LV_ANIM_OFF);
}

Navigation::~Navigation() {
  lv_obj_clean(lv_scr_act());
}

bool Navigation::Refresh() {

  if (m_flag != navService.getFlag()) {
    m_flag = navService.getFlag();
    lv_label_set_text(imgFlag, iconForName(m_flag));
    // lv_img_set_src_arr(imgFlag, iconForName(m_flag));
  }

  if (m_narrative != navService.getNarrative()) {
    m_narrative = navService.getNarrative();
    lv_label_set_text(txtNarrative, m_narrative.data());
  }

  if (m_manDist != navService.getManDist()) {
    m_manDist = navService.getManDist();
    lv_label_set_text(txtManDist, m_manDist.data());
  }

  if (m_progress != navService.getProgress()) {
    m_progress = navService.getProgress();
    lv_bar_set_value(barProgress, m_progress, LV_ANIM_OFF);
    if (m_progress > 90) {
      lv_obj_set_style_local_bg_color(barProgress, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_RED);
    } else {
      lv_obj_set_style_local_bg_color(barProgress, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    }
  }

  return running;
}

const char* Navigation::iconForName(std::string icon) {
  for (auto iter : m_iconMap) {
    if (iter.first == icon) {
      return iter.second;
    }
  }
  return "\xEE\xA4\x90";
}
