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
#pragma once

#include <FreeRTOS.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include <string>
#include "Screen.h"
#include <array>
#include <lvgl/src/lv_draw/lv_img_decoder.h>

#include "displayapp/icons/navigation/arrive-left.c"
#include "displayapp/icons/navigation/arrive-right.c"
#include "displayapp/icons/navigation/arrive-straight.c"
#include "displayapp/icons/navigation/arrive.c"
#include "displayapp/icons/navigation/close.c"
#include "displayapp/icons/navigation/continue-left.c"
#include "displayapp/icons/navigation/continue-right.c"
#include "displayapp/icons/navigation/continue-slight-left.c"
#include "displayapp/icons/navigation/continue-slight-right.c"
#include "displayapp/icons/navigation/continue-straight.c"
#include "displayapp/icons/navigation/continue-uturn.c"
#include "displayapp/icons/navigation/continue.c"
#include "displayapp/icons/navigation/depart-left.c"
#include "displayapp/icons/navigation/depart-right.c"
#include "displayapp/icons/navigation/depart-straight.c"
#include "displayapp/icons/navigation/end-of-road-left.c"
#include "displayapp/icons/navigation/end-of-road-right.c"
#include "displayapp/icons/navigation/ferry.c"
#include "displayapp/icons/navigation/flag.c"
#include "displayapp/icons/navigation/fork-left.c"
#include "displayapp/icons/navigation/fork-right.c"
#include "displayapp/icons/navigation/fork-slight-left.c"
#include "displayapp/icons/navigation/fork-slight-right.c"
#include "displayapp/icons/navigation/fork-straight.c"
#include "displayapp/icons/navigation/invalid.c"
#include "displayapp/icons/navigation/invalid-left.c"
#include "displayapp/icons/navigation/invalid-right.c"
#include "displayapp/icons/navigation/invalid-slight-left.c"
#include "displayapp/icons/navigation/invalid-slight-right.c"
#include "displayapp/icons/navigation/invalid-straight.c"
#include "displayapp/icons/navigation/invalid-uturn.c"
#include "displayapp/icons/navigation/merge-left.c"
#include "displayapp/icons/navigation/merge-right.c"
#include "displayapp/icons/navigation/merge-slight-left.c"
#include "displayapp/icons/navigation/merge-slight-right.c"
#include "displayapp/icons/navigation/merge-straight.c"
#include "displayapp/icons/navigation/new-name-left.c"
#include "displayapp/icons/navigation/new-name-right.c"
#include "displayapp/icons/navigation/new-name-sharp-left.c"
#include "displayapp/icons/navigation/new-name-sharp-right.c"
#include "displayapp/icons/navigation/new-name-slight-left.c"
#include "displayapp/icons/navigation/new-name-slight-right.c"
#include "displayapp/icons/navigation/new-name-straight.c"
#include "displayapp/icons/navigation/notification-left.c"
#include "displayapp/icons/navigation/notification-right.c"
#include "displayapp/icons/navigation/notification-sharp-left.c"
#include "displayapp/icons/navigation/notification-sharp-right.c"
#include "displayapp/icons/navigation/notification-slight-left.c"
#include "displayapp/icons/navigation/notification-slight-right.c"
#include "displayapp/icons/navigation/notification-straight.c"
#include "displayapp/icons/navigation/off-ramp-left.c"
#include "displayapp/icons/navigation/off-ramp-right.c"
#include "displayapp/icons/navigation/off-ramp-slight-left.c"
#include "displayapp/icons/navigation/off-ramp-slight-right.c"
#include "displayapp/icons/navigation/on-ramp-left.c"
#include "displayapp/icons/navigation/on-ramp-right.c"
#include "displayapp/icons/navigation/on-ramp-sharp-left.c"
#include "displayapp/icons/navigation/on-ramp-sharp-right.c"
#include "displayapp/icons/navigation/on-ramp-slight-left.c"
#include "displayapp/icons/navigation/on-ramp-slight-right.c"
#include "displayapp/icons/navigation/on-ramp-straight.c"
#include "displayapp/icons/navigation/rotary.c"
#include "displayapp/icons/navigation/rotary-left.c"
#include "displayapp/icons/navigation/rotary-right.c"
#include "displayapp/icons/navigation/rotary-sharp-left.c"
#include "displayapp/icons/navigation/rotary-sharp-right.c"
#include "displayapp/icons/navigation/rotary-slight-left.c"
#include "displayapp/icons/navigation/rotary-slight-right.c"
#include "displayapp/icons/navigation/rotary-straight.c"
#include "displayapp/icons/navigation/roundabout.c"
#include "displayapp/icons/navigation/roundabout-left.c"
#include "displayapp/icons/navigation/roundabout-right.c"
#include "displayapp/icons/navigation/roundabout-sharp-left.c"
#include "displayapp/icons/navigation/roundabout-sharp-right.c"
#include "displayapp/icons/navigation/roundabout-slight-left.c"
#include "displayapp/icons/navigation/roundabout-slight-right.c"
#include "displayapp/icons/navigation/roundabout-straight.c"
#include "displayapp/icons/navigation/turn-left.c"
#include "displayapp/icons/navigation/turn-right.c"
#include "displayapp/icons/navigation/turn-sharp-left.c"
#include "displayapp/icons/navigation/turn-sharp-right.c"
#include "displayapp/icons/navigation/turn-slight-left.c"
#include "displayapp/icons/navigation/turn-slight-right.c"
#include "displayapp/icons/navigation/turn-straight.c"
#include "displayapp/icons/navigation/updown.c"
#include "displayapp/icons/navigation/uturn.c"

namespace Pinetime {
  namespace Controllers {
    class NavigationService;
  }

  namespace Applications {
    namespace Screens {
      class Navigation : public Screen {
      public:
        Navigation(DisplayApp *app, Pinetime::Controllers::NavigationService &nav);
        ~Navigation() override;

        bool Refresh() override;
        bool OnButtonPushed() override;

      private:

        lv_obj_t *imgFlag;
        lv_obj_t *txtNarrative;
        lv_obj_t *txtManDist;
        lv_obj_t *barProgress;

        Pinetime::Controllers::NavigationService &navService;

        std::string m_flag;
        std::string m_narrative;
        std::string m_manDist;
        int m_progress;

        /** Watchapp */
        bool running = true;

        const lv_img_dsc_t* iconForName(std::string icon);

        std::array<std::pair<std::string, const lv_img_dsc_t*>, 89 > m_iconMap = { {
            {"arrive-left", &arrive_left},
            {"arrive-right", &arrive_right},
            {"arrive-straight", &arrive_straight},
            {"arrive", &arrive},
            {"close", &close},
            {"continue-left", &continue_left},
            {"continue-right", &continue_right},
            {"continue-slight-left", &continue_slight_left},
            {"continue-slight-right", &continue_slight_right},
            {"continue-straight", &continue_straight},
            {"continue-uturn", &continue_uturn},
            {"continue", &continue_icon},
            {"depart-left", &depart_left},
            {"depart-right", &depart_right},
            {"depart-straight", &depart_straight},
            {"end-of-road-left", &end_of_road_left},
            {"end-of-road-right", &end_of_road_right},
            {"ferry", &ferry},
            {"flag", &flag},
            {"fork-left", &fork_left},
            {"fork-right", &fork_right},
            {"fork-slight-left", &fork_slight_left},
            {"fork-slight-right", &fork_slight_right},
            {"fork-straight", &fork_straight},
            {"invalid", &invalid},
            {"invalid-left", &invalid_left},
            {"invalid-right", &invalid_right},
            {"invalid-slight-left", &invalid_slight_left},
            {"invalid-slight-right", &invalid_slight_right},
            {"invalid-straight", &invalid_straight},
            {"invalid-uturn", &invalid_uturn},
            {"merge-left", &merge_left},
            {"merge-right", &merge_right},
            {"merge-slight-left", &merge_slight_left},
            {"merge-slight-right", &merge_slight_right},
            {"merge-straight", &merge_straight},
            {"new-name-left", &new_name_left},
            {"new-name-right", &new_name_right},
            {"new-name-sharp-left", &new_name_sharp_left},
            {"new-name-sharp-right", &new_name_sharp_right},
            {"new-name-slight-left", &new_name_slight_left},
            {"new-name-slight-right", &new_name_slight_right},
            {"new-name-straight", &new_name_straight},
            {"notification-left", &notification_left},
            {"notification-right", &notification_right},
            {"notification-sharp-left", &notification_sharp_left},
            {"notification-sharp-right", &notification_sharp_right},
            {"notification-slight-left", &notification_slight_left},
            {"notification-slight-right", &notification_slight_right},
            {"notification-straight", &notification_straight},
            {"off-ramp-left", &off_ramp_left},
            {"off-ramp-right", &off_ramp_right},
            {"off-ramp-slight-left", &off_ramp_slight_left},
            {"off-ramp-slight-right", &off_ramp_slight_right},
            {"on-ramp-left", &on_ramp_left},
            {"on-ramp-right", &on_ramp_right},
            {"on-ramp-sharp-left", &on_ramp_sharp_left},
            {"on-ramp-sharp-right", &on_ramp_sharp_right},
            {"on-ramp-slight-left", &on_ramp_slight_left},
            {"on-ramp-slight-right", &on_ramp_slight_right},
            {"on-ramp-straight", &on_ramp_straight},
            {"rotary", &rotary},
            {"rotary-left", &rotary_left},
            {"rotary-right", &rotary_right},
            {"rotary-sharp-left", &rotary_sharp_left},
            {"rotary-sharp-right", &rotary_sharp_right},
            {"rotary-slight-left", &rotary_slight_left},
            {"rotary-slight-right", &rotary_slight_right},
            {"rotary-straight", &rotary_straight},
            {"roundabout", &roundabout},
            {"roundabout-left", &roundabout_left},
            {"roundabout-right", &roundabout_right},
            {"roundabout-sharp-left", &roundabout_sharp_left},
            {"roundabout-sharp-right", &roundabout_sharp_right},
            {"roundabout-slight-left", &roundabout_slight_left},
            {"roundabout-slight-right", &roundabout_slight_right},
            {"roundabout-straight", &roundabout_straight},
            {"turn-left", &turn_left},
            {"turn-right", &turn_right},
            {"turn-sharp-left", &turn_sharp_left},
            {"turn-sharp-right", &turn_sharp_right},
            {"turn-slight-left", &turn_slight_left},
            {"turn-slight-right", &turn_slight_right},
            {"turn-straight", &turn_straight},
            {"updown", &updown},
            {"uturn", &uturn} } };
      };
    }
  }
}
