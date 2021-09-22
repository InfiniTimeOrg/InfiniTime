/*  Copyright (C) 2021 mruss77, Florian

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

#include "Screen.h"
#include "systemtask/SystemTask.h"
#include "../LittleVgl.h"
#include "components/alarm/AlarmController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Alarm : public Screen {
      public:
        Alarm(DisplayApp* app, Controllers::AlarmController& alarmController);
        ~Alarm() override;
        void SetAlerting();
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

      private:
        bool running;
        uint8_t alarmHours;
        uint8_t alarmMinutes;
        Controllers::AlarmController& alarmController;

        lv_obj_t *time, *btnEnable, *txtEnable, *btnMinutesUp, *btnMinutesDown, *btnHoursUp, *btnHoursDown, *txtMinUp, *txtMinDown,
          *txtHrUp, *txtHrDown, *btnRecur, *txtRecur, *btnMessage, *txtMessage, *btnInfo, *txtInfo;

        enum class EnableButtonState { On, Off, Alerting };
        void SetEnableButtonState();
        void SetRecurButtonState();
        void SetAlarm();
        void ShowInfo();
        void ToggleRecurrence();
        void UpdateAlarmTime();
      };
    };
  };
}
