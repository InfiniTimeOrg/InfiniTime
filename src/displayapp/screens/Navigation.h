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
        
        const char* iconForName(std::string icon);
        
        std::array<std::pair<std::string, const char*>, 89 > m_iconMap = { {
            {"arrive-left"                ,"\xEE\xA4\x81" },
            {"arrive-right"               ,"\xEE\xA4\x82" },
            {"arrive-straight"            ,"\xEE\xA4\x80" },
            {"arrive"                     ,"\xEE\xA4\x80" },
            {"close"                      ,"\xEE\xA4\x83" },
            {"continue-left"              ,"\xEE\xA4\x85" },
            {"continue-right"             ,"\xEE\xA4\x86" },
            {"continue-slight-left"       ,"\xEE\xA4\x87" },
            {"continue-slight-right"      ,"\xEE\xA4\x88" },
            {"continue-straight"          ,"\xEE\xA4\x84" },
            {"continue-uturn"             ,"\xEE\xA4\x89" },
            {"continue"                   ,"\xEE\xA4\x84" },
            {"depart-left"                ,"\xEE\xA4\x8B" },
            {"depart-right"               ,"\xEE\xA4\x8C" },
            {"depart-straight"            ,"\xEE\xA4\x8A" },
            {"end-of-road-left"           ,"\xEE\xA4\x8D" },
            {"end-of-road-right"          ,"\xEE\xA4\x8E" },
            {"ferry"                      ,"\xEE\xA4\x8F" },
            {"flag"                       ,"\xEE\xA4\x90" },
            {"fork-left"                  ,"\xEE\xA4\x92" },
            {"fork-right"                 ,"\xEE\xA4\x93" },
            {"fork-slight-left"           ,"\xEE\xA4\x94" },
            {"fork-slight-right"          ,"\xEE\xA4\x95" },
            {"fork-straight"              ,"\xEE\xA4\x96" },
            {"invalid"                    ,"\xEE\xA4\x84" },
            {"invalid-left"               ,"\xEE\xA4\x85" },
            {"invalid-right"              ,"\xEE\xA4\x86" },
            {"invalid-slight-left"        ,"\xEE\xA4\x87" },
            {"invalid-slight-right"       ,"\xEE\xA4\x88" },
            {"invalid-straight"           ,"\xEE\xA4\x84" },
            {"invalid-uturn"              ,"\xEE\xA4\x89" },
            {"merge-left"                 ,"\xEE\xA4\x97" },
            {"merge-right"                ,"\xEE\xA4\x98" },
            {"merge-slight-left"          ,"\xEE\xA4\x99" },
            {"merge-slight-right"         ,"\xEE\xA4\x9A" },
            {"merge-straight"             ,"\xEE\xA4\x84" },
            {"new-name-left"              ,"\xEE\xA4\x85" },
            {"new-name-right"             ,"\xEE\xA4\x86" },
            {"new-name-sharp-left"        ,"\xEE\xA4\x9B" },
            {"new-name-sharp-right"       ,"\xEE\xA4\x9C" },
            {"new-name-slight-left"       ,"\xEE\xA4\x87" },
            {"new-name-slight-right"      ,"\xEE\xA4\x88" },
            {"new-name-straight"          ,"\xEE\xA4\x84" },
            {"notification-left"          ,"\xEE\xA4\x85" },
            {"notification-right"         ,"\xEE\xA4\x86" },
            {"notification-sharp-left"    ,"\xEE\xA4\x9B" },
            {"notification-sharp-right"   ,"\xEE\xA4\xA5" },
            {"notification-slight-left"   ,"\xEE\xA4\x87" },
            {"notification-slight-right"  ,"\xEE\xA4\x88" },
            {"notification-straight"      ,"\xEE\xA4\x84" },
            {"off-ramp-left"              ,"\xEE\xA4\x9D" },
            {"off-ramp-right"             ,"\xEE\xA4\x9E" },
            {"off-ramp-slight-left"       ,"\xEE\xA4\x9F" },
            {"off-ramp-slight-right"      ,"\xEE\xA4\xA0" },
            {"on-ramp-left"               ,"\xEE\xA4\x85" },
            {"on-ramp-right"              ,"\xEE\xA4\x86" },
            {"on-ramp-sharp-left"         ,"\xEE\xA4\x9B" },
            {"on-ramp-sharp-right"        ,"\xEE\xA4\xA5" },
            {"on-ramp-slight-left"        ,"\xEE\xA4\x87" },
            {"on-ramp-slight-right"       ,"\xEE\xA4\x88" },
            {"on-ramp-straight"           ,"\xEE\xA4\x84" },
            {"rotary"                     ,"\xEE\xA4\xA1" },
            {"rotary-left"                ,"\xEE\xA4\xA2" },
            {"rotary-right"               ,"\xEE\xA4\xA3" },
            {"rotary-sharp-left"          ,"\xEE\xA4\xA4" },
            {"rotary-sharp-right"         ,"\xEE\xA4\xA5" },
            {"rotary-slight-left"         ,"\xEE\xA4\xA6" },
            {"rotary-slight-right"        ,"\xEE\xA4\xA7" },
            {"rotary-straight"            ,"\xEE\xA4\xA8" },
            {"roundabout"                 ,"\xEE\xA4\xA1" },
            {"roundabout-left"            ,"\xEE\xA4\xA2" },
            {"roundabout-right"           ,"\xEE\xA4\xA3" },
            {"roundabout-sharp-left"      ,"\xEE\xA4\xA4" },
            {"roundabout-sharp-right"     ,"\xEE\xA4\xA5" },
            {"roundabout-slight-left"     ,"\xEE\xA4\xA6" },
            {"roundabout-slight-right"    ,"\xEE\xA4\xA7" },
            {"roundabout-straight"        ,"\xEE\xA4\xA8" },
            {"turn-left"                  ,"\xEE\xA4\x85" },
            {"turn-right"                 ,"\xEE\xA4\x86" },
            {"turn-sharp-left"            ,"\xEE\xA4\x9B" },
            {"turn-sharp-right"           ,"\xEE\xA4\xA5" },
            {"turn-slight-left"           ,"\xEE\xA4\x87" },
            {"turn-slight-right"          ,"\xEE\xA4\x88" },
            {"turn-straight"              ,"\xEE\xA4\x84" },
            {"updown"                     ,"\xEE\xA4\xA9" },
            {"uturn"                      ,"\xEE\xA4\x89" },
        } };
      };
    }
  }
}
