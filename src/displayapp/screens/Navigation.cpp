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
#include "displayapp/screens/Navigation.h"
#include <cstdint>
#include "displayapp/DisplayApp.h"
#include "components/ble/NavigationService.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  struct Icon {
    const char* fileName;
    int16_t offset;
  };

  constexpr uint16_t iconHeight = -80;
  constexpr uint8_t flagIndex = 18;
  constexpr uint8_t maxIconsPerFile = 25;
  const char* iconsFile0 = "F:/images/navigation0.bin";
  const char* iconsFile1 = "F:/images/navigation1.bin";

  constexpr std::array<std::pair<const char*, uint8_t>, 86> iconMap = {{
    {"arrive-left", 1},
    {"arrive-right", 2},
    {"arrive-straight", 0},
    {"arrive", 0},
    {"close", 3},
    {"continue-left", 5},
    {"continue-right", 6},
    {"continue-slight-left", 7},
    {"continue-slight-right", 8},
    {"continue-straight", 4},
    {"continue-uturn", 9},
    {"continue", 4},
    {"depart-left", 11},
    {"depart-right", 12},
    {"depart-straight", 10},
    {"end-of-road-left", 13},
    {"end-of-road-right", 14},
    {"ferry", 15},
    {"flag", 16},
    {"fork-left", 18},
    {"fork-right", 19},
    {"fork-slight-left", 20},
    {"fork-slight-right", 21},
    {"fork-straight", 22},
    {"invalid", 4},
    {"invalid-left", 5},
    {"invalid-right", 6},
    {"invalid-slight-left", 7},
    {"invalid-slight-right", 8},
    {"invalid-straight", 4},
    {"invalid-uturn", 9},
    {"merge-left", 23},
    {"merge-right", 24},
    {"merge-slight-left", 25},
    {"merge-slight-right", 26},
    {"merge-straight", 4},
    {"new-name-left", 5},
    {"new-name-right", 6},
    {"new-name-sharp-left", 27},
    {"new-name-sharp-right", 28},
    {"new-name-slight-left", 7},
    {"new-name-slight-right", 8},
    {"new-name-straight", 4},
    {"notification-left", 5},
    {"notification-right", 6},
    {"notification-sharp-left", 27},
    {"notification-sharp-right", 37},
    {"notification-slight-left", 7},
    {"notification-slight-right", 8},
    {"notification-straight", 4},
    {"off-ramp-left", 29},
    {"off-ramp-right", 30},
    {"off-ramp-slight-left", 31},
    {"off-ramp-slight-right", 32},
    {"on-ramp-left", 5},
    {"on-ramp-right", 6},
    {"on-ramp-sharp-left", 27},
    {"on-ramp-sharp-right", 37},
    {"on-ramp-slight-left", 7},
    {"on-ramp-slight-right", 8},
    {"on-ramp-straight", 4},
    {"rotary", 33},
    {"rotary-left", 34},
    {"rotary-right", 35},
    {"rotary-sharp-left", 36},
    {"rotary-sharp-right", 37},
    {"rotary-slight-left", 38},
    {"rotary-slight-right", 39},
    {"rotary-straight", 40},
    {"roundabout", 33},
    {"roundabout-left", 34},
    {"roundabout-right", 35},
    {"roundabout-sharp-left", 36},
    {"roundabout-sharp-right", 37},
    {"roundabout-slight-left", 38},
    {"roundabout-slight-right", 39},
    {"roundabout-straight", 40},
    {"turn-left", 5},
    {"turn-right", 6},
    {"turn-sharp-left", 27},
    {"turn-sharp-right", 37},
    {"turn-slight-left", 7},
    {"turn-slight-right", 8},
    {"turn-straight", 4},
    {"updown", 41},
    {"uturn", 9},
  }};

  Icon GetIcon(uint8_t index) {
    if(index < maxIconsPerFile) {
      return {iconsFile0, static_cast<int16_t>(iconHeight * index)};
    }
    return {iconsFile1, static_cast<int16_t>(iconHeight * (index - maxIconsPerFile))};
  }

  Icon GetIcon(const std::string& icon) {
    uint8_t index = 0;
    for (const auto& iter : iconMap) {
      if (iter.first == icon) {
        return GetIcon(iter.second);
      }
      index++;
    }
    return GetIcon(flagIndex);
  }
}

/**
 * Navigation watchapp
 *
 */
Navigation::Navigation(Pinetime::Controllers::NavigationService& nav) : navService(nav) {
  const auto& image = GetIcon("flag");
  imgFlag = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_auto_size(imgFlag, false);
  lv_obj_set_size(imgFlag, 80, 80);
  lv_img_set_src(imgFlag, image.fileName);
  lv_img_set_offset_x(imgFlag, 0);
  lv_img_set_offset_y(imgFlag, image.offset);
  lv_obj_set_style_local_image_recolor_opa(imgFlag, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_image_recolor(imgFlag, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  lv_obj_align(imgFlag, nullptr, LV_ALIGN_CENTER, 0, -60);

  txtNarrative = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtNarrative, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(txtNarrative, LV_HOR_RES);
  lv_label_set_text_static(txtNarrative, "Navigation");
  lv_label_set_align(txtNarrative, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(txtNarrative, nullptr, LV_ALIGN_CENTER, 0, 10);

  txtManDist = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(txtManDist, LV_LABEL_LONG_BREAK);
  lv_obj_set_style_local_text_color(txtManDist, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  lv_obj_set_width(txtManDist, LV_HOR_RES);
  lv_label_set_text_static(txtManDist, "--M");
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

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Navigation::~Navigation() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Navigation::Refresh() {
  if (flag != navService.getFlag()) {
    flag = navService.getFlag();
    const auto& image = GetIcon(flag);
    lv_img_set_src(imgFlag, image.fileName);
    lv_obj_set_style_local_image_recolor_opa(imgFlag, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_obj_set_style_local_image_recolor(imgFlag, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
    lv_img_set_offset_y(imgFlag, image.offset);
  }

  if (narrative != navService.getNarrative()) {
    narrative = navService.getNarrative();
    lv_label_set_text(txtNarrative, narrative.data());
  }

  if (manDist != navService.getManDist()) {
    manDist = navService.getManDist();
    lv_label_set_text(txtManDist, manDist.data());
  }

  if (progress != navService.getProgress()) {
    progress = navService.getProgress();
    lv_bar_set_value(barProgress, progress, LV_ANIM_OFF);
    if (progress > 90) {
      lv_obj_set_style_local_bg_color(barProgress, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_RED);
    } else {
      lv_obj_set_style_local_bg_color(barProgress, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::orange);
    }
  }
}
