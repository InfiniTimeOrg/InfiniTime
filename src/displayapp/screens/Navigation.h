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
#include "displayapp/screens/Screen.h"
#include <array>
#include "displayapp/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Controllers {
    class NavigationService;
    class FS;
  }

  namespace Applications {
    namespace Screens {
      class Navigation : public Screen {
      public:
        explicit Navigation(Pinetime::Controllers::NavigationService& nav);
        ~Navigation() override;

        void Refresh() override;
        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

      private:
        lv_obj_t* imgFlag;
        lv_obj_t* txtNarrative;
        lv_obj_t* txtManDist;
        lv_obj_t* barProgress;

        Pinetime::Controllers::NavigationService& navService;

        std::string flag;
        std::string narrative;
        std::string manDist;
        int progress = 0;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Navigation> {
      static constexpr Apps app = Apps::Navigation;
      static constexpr const char* icon = Screens::Symbols::map;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Navigation(*controllers.navigationService);
      };
    };
  }
}
