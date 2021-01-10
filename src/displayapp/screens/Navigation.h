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
        lv_obj_t *txtFlag; //TODO make graphic
        lv_obj_t *txtNarrative;
        lv_obj_t *txtManDist;
        lv_obj_t *barProgress;

        Pinetime::Controllers::NavigationService &navService;

        /* Valid flags:
         * arrive
         * arrive-left
         * arrive-right
         * arrive-straight
         * close
         * continue
         * continue-left
         * continue-right
         * continue-slight-left
         * continue-slight-right
         * continue-straight
         * continue-uturn
         * depart
         * depart-left
         * depart-right
         * depart-straight
         * end-of-road-left
         * end-of-road-right
         * ferry
         * flag
         * fork
         * fork-left
         * fork-right
         * fork-slight-left
         * fork-slight-right
         * fork-straight
         * invalid
         * invalid-left
         * invalid-right
         * invalid-slight-left
         * invalid-slight-right
         * invalid-straight
         * invalid-uturn
         * merge-left
         * merge-right
         * merge-slight-left
         * merge-slight-right
         * merge-straight
         * new-name-left
         * new-name-right
         * new-name-sharp-left
         * new-name-sharp-right
         * new-name-slight-left
         * new-name-slight-right
         * new-name-straight
         * notification-left
         * notification-right
         * notification-sharp-left
         * notification-sharp-right
         * notification-slight-left
         * notification-slight-right
         * notification-straight
         * off-ramp-left
         * off-ramp-right
         * off-ramp-sharp-left
         * off-ramp-sharp-right
         * off-ramp-slight-left
         * off-ramp-slight-right
         * off-ramp-straight
         * on-ramp-left
         * on-ramp-right
         * on-ramp-sharp-left
         * on-ramp-sharp-right
         * on-ramp-slight-left
         * on-ramp-slight-right
         * on-ramp-straight
         * rotary
         * rotary-left
         * rotary-right
         * rotary-sharp-left
         * rotary-sharp-right
         * rotary-slight-left
         * rotary-slight-right
         * rotary-straight
         * roundabout
         * roundabout-left
         * roundabout-right
         * roundabout-sharp-left
         * roundabout-sharp-right
         * roundabout-slight-left
         * roundabout-slight-right
         * roundabout-straight
         * turn-left
         * turn-right
         * turn-sharp-left
         * turn-sharp-right
         * turn-slight-left
         * turn-slight-right
         * turn-stright
         * updown
         * uturn
         */
        std::string m_flag;
        std::string m_narrative;
        std::string m_manDist;
        int m_progress;

        /** Watchapp */
        bool running = true;
      };
    }
  }
}
