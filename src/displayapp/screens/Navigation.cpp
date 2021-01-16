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

extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;

/**
 * Set the pixel array to display by the image
 * This just calls lv_img_set_src but adds type safety
 *
 * @param img pointer to an image object
 * @param data the image array
 */
inline void lv_img_set_src_arr(lv_obj_t *img, const lv_img_dsc_t *src_img) {
  lv_img_set_src(img, src_img);
}

/**
 * Navigation watchapp
 *
 */
Navigation::Navigation(Pinetime::Applications::DisplayApp *app, Pinetime::Controllers::NavigationService &nav) : Screen(app), navService(nav) {

  constexpr uint8_t FONT_HEIGHT = 12;
  constexpr uint8_t LINE_PAD = 15;
  constexpr int8_t MIDDLE_OFFSET = -25;

  imgFlag = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src_arr(imgFlag, &flag);
  lv_obj_align(imgFlag, nullptr, LV_ALIGN_IN_TOP_MID, 0, 15);

  txtNarrative = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtNarrative, LV_LABEL_LONG_SROLL);
  lv_obj_align(txtNarrative, nullptr, LV_ALIGN_IN_LEFT_MID, 12, MIDDLE_OFFSET + 1 * FONT_HEIGHT);
  lv_label_set_text(txtNarrative, "Narrative");
  lv_label_set_align(txtNarrative, LV_LABEL_ALIGN_CENTER);
  lv_label_set_anim_speed(txtNarrative, 15);
  lv_obj_set_width(txtNarrative, LV_HOR_RES);

  txtManDist = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtManDist, LV_LABEL_LONG_SROLL);
  lv_obj_align(txtManDist, nullptr, LV_ALIGN_IN_LEFT_MID, 12, MIDDLE_OFFSET + 2 * FONT_HEIGHT + LINE_PAD);
  lv_label_set_text(txtManDist, "0M");
  lv_label_set_align(txtManDist, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_width(txtManDist, LV_HOR_RES);

  //Route Progress
  barProgress = lv_bar_create(lv_scr_act(), NULL);
  lv_obj_set_size(barProgress, 200, 20);
  lv_obj_align(barProgress, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_bar_set_anim_time(barProgress, 500);
  lv_bar_set_range(barProgress, 0, 100);
  lv_bar_set_value(barProgress, 0, LV_ANIM_ON);
}

Navigation::~Navigation() {
  lv_obj_clean(lv_scr_act());
}

bool Navigation::Refresh() {

  if (m_flag != navService.getFlag()) {
    m_flag = navService.getFlag();

    lv_img_set_src_arr(imgFlag, iconForName(m_flag));
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
    lv_bar_set_value(barProgress, m_progress, LV_ANIM_ON);
  }

  return running;
}

bool Navigation::OnButtonPushed() {
  running = false;
  return true;
}

const lv_img_dsc_t* Navigation::iconForName(std::string icon)
{
    for (auto iter : m_iconMap) {
        if (iter.first == icon) {
            return iter.second;
        }
    }
    return &invalid;
}
