#pragma once

#include "Screen.h"
#include "systemtask/SystemTask.h"
#include "../LittleVgl.h"
#include "components/alarm/AlarmController.h"

namespace Pinetime::Applications::Screens {
  class Alarm : public Screen {
  public:
    Alarm(DisplayApp* app, Controllers::AlarmController& alarmController);
    ~Alarm() override;
    void SetAlerting();
    void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

  private:
    bool running;
    uint8_t alarmHours = 0;
    uint8_t alarmMinutes = 0;
    Controllers::AlarmController& alarmController;

    lv_obj_t *time, *btnEnable, *txtEnable, *btnMinutesUp, *btnMinutesDown, *btnHoursUp, *btnHoursDown, *txtMinUp, *txtMinDown, *txtHrUp,
      *txtHrDown, *btnRecur, *txtRecur, *btnMessage, *txtMessage, *btnInfo, *txtInfo;

    enum class EnableButtonState { On, Off, Alerting };
    void setEnableButtonState();
    void setRecurButtonState();
    void setAlarm();
    void showInfo();
  };
}