/*  Copyright (C) 2024 thnikk, Boteium, JustScott

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

#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include <lvgl/lvgl.h>

#include "Symbols.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
  }

  namespace Applications {
    namespace Screens {
      class Calendar : public Screen {
      public:
        Calendar(Controllers::DateTime& dateTimeController);
        ~Calendar() override;

      private:
        bool OnTouchEvent(TouchEvents event);
        Controllers::DateTime& dateTimeController;
        lv_obj_t* calendar;
        lv_calendar_date_t current;
      };
    }

    template <>
    struct AppTraits<Apps::Calendar> {
      static constexpr Apps app = Apps::Calendar;
      static constexpr const char* icon = Screens::Symbols::calendar;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Calendar(controllers.dateTimeController);
      };
    };
  }
}
