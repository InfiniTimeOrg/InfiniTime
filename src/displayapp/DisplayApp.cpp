#include <string>

#include "DisplayApp.h"
#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include <nrf_font.h>
#include <queue.h>
#include "components/datetime/DateTimeController.h"
#include <drivers/Cst816s.h>
#include "displayapp/screens/Tile.h"
#include "displayapp/screens/Meter.h"
#include "displayapp/screens/Gauge.h"
#include "displayapp/screens/Brightness.h"
#include "displayapp/screens/SystemInfo.h"
#include "displayapp/screens/Music.h"
#include "components/ble/NotificationManager.h"
#include "displayapp/screens/FirmwareUpdate.h"
#include "displayapp/screens/ApplicationList.h"
#include "displayapp/screens/FirmwareValidation.h"
#include "displayapp/screens/InfiniPaint.h"
#include "systemtask/SystemTask.h"

// most people are right-handed and wear the watch on left land
#define DEFAULT_LEFT_HAND_ORIENTATION true

using namespace Pinetime::Applications;

DisplayApp::DisplayApp(Drivers::St7789 &lcd, Components::LittleVgl &lvgl, Drivers::Cst816S &touchPanel,
                       Controllers::Battery &batteryController, Controllers::Ble &bleController,
                       Controllers::DateTime &dateTimeController, Drivers::WatchdogView &watchdog,
                       System::SystemTask &systemTask,
                       Pinetime::Controllers::NotificationManager& notificationManager) :
        lcd{lcd},
        lvgl{lvgl},
        batteryController{batteryController},
        bleController{bleController},
        dateTimeController{dateTimeController},
        watchdog{watchdog},
        touchPanel{touchPanel},
        currentScreen{new Screens::Clock(this, dateTimeController, batteryController, bleController) },
        systemTask{systemTask},
        notificationManager{notificationManager} {
  msgQueue = xQueueCreate(queueSize, itemSize);
  onClockApp = true;
  modal.reset(new Screens::Modal(this));
}

void DisplayApp::Start() {
  if (pdPASS != xTaskCreate(DisplayApp::Process, "displayapp", 512, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void DisplayApp::Process(void *instance) {
  auto *app = static_cast<DisplayApp *>(instance);
  NRF_LOG_INFO("displayapp task started!");
  app->InitHw();

  // Send a dummy notification to unlock the lvgl display driver for the first iteration
  xTaskNotifyGive(xTaskGetCurrentTaskHandle());

  while (1) {

    app->Refresh();

  }
}

void DisplayApp::InitHw() {
  SetHandOrientation(DEFAULT_LEFT_HAND_ORIENTATION);
  brightnessController.Init();
}

uint32_t acc = 0;
uint32_t count = 0;
bool toggle = true;
void DisplayApp::Refresh() {
  TickType_t queueTimeout;
  switch (state) {
    case States::Idle:
      IdleState();
      queueTimeout = portMAX_DELAY;
      break;
    case States::Running:
      RunningState();
      queueTimeout = 20;
      break;
    default:
      queueTimeout = portMAX_DELAY;
      break;
  }

  Messages msg;
  if (xQueueReceive(msgQueue, &msg, queueTimeout)) {
    switch (msg) {
      case Messages::GoToSleep:
        brightnessController.Backup();
        while(brightnessController.Level() != Controllers::BrightnessController::Levels::Off) {
          brightnessController.Lower();
          vTaskDelay(100);
        }
        lcd.DisplayOff();
        systemTask.PushMessage(System::SystemTask::Messages::OnDisplayTaskSleeping);
        state = States::Idle;
        break;
      case Messages::GoToRunning:
        lcd.DisplayOn();
        brightnessController.Restore();
        state = States::Running;
        break;
      case Messages::UpdateDateTime:
//        modal->Show();
        break;
      case Messages::UpdateBleConnection:
//        clockScreen.SetBleConnectionState(bleController.IsConnected() ? Screens::Clock::BleConnectionStates::Connected : Screens::Clock::BleConnectionStates::NotConnected);
        break;
      case Messages::UpdateBatteryLevel:
//        clockScreen.SetBatteryPercentRemaining(batteryController.PercentRemaining());
        break;
      case Messages::NewNotification: {
        auto notification = notificationManager.Pop();
        modal->Show(notification.message.data());
      }
        break;
      case Messages::TouchEvent: {
        if (state != States::Running) break;
        auto gesture = OnTouchEvent();
        if(!currentScreen->OnTouchEvent(gesture)) {
          switch (gesture) {
            case TouchEvents::SwipeUp:
              currentScreen->OnButtonPushed();
              lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Up);
              break;
            case TouchEvents::SwipeDown:
              currentScreen->OnButtonPushed();
              lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Down);
              break;
            default:
              break;
          }
        }
      }
        break;
      case Messages::ButtonPushed:
        if(onClockApp)
            systemTask.PushMessage(System::SystemTask::Messages::GoToSleep);
          else {
            auto buttonUsedByApp = currentScreen->OnButtonPushed();
            if (!buttonUsedByApp) {
              systemTask.PushMessage(System::SystemTask::Messages::GoToSleep);
            } else {
              lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Up);
          }
        }

//        lvgl.SetFullRefresh(components::LittleVgl::FullRefreshDirections::Down);
//        currentScreen.reset(nullptr);
//        if(toggle) {
//          currentScreen.reset(new Screens::Tile(this));
//          toggle = false;
//        } else {
//          currentScreen.reset(new Screens::Clock(this, dateTimeController, batteryController, bleController));
//          toggle = true;
//        }

        break;
      case Messages::BleFirmwareUpdateStarted:
        lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Down);
        currentScreen.reset(nullptr);
        currentScreen.reset(new Screens::FirmwareUpdate(this, bleController));
        onClockApp = false;

        break;
    }
  }

  if(state != States::Idle && touchMode == TouchModes::Polling) {
    auto info = touchPanel.GetTouchInfo();
    if(info.action == 2) {// 2 = contact
      if(!currentScreen->OnTouchEvent(info.x, info.y)) {
        SetTapCoordinates(info.x, info.y);
      }
    }
  }
}

void DisplayApp::RunningState() {
//  clockScreen.SetCurrentDateTime(dateTimeController.CurrentDateTime());

  if(!currentScreen->Refresh()) {
    currentScreen.reset(nullptr);
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Up);
    onClockApp = false;
    switch(nextApp) {
      case Apps::None:
      case Apps::Launcher: currentScreen.reset(new Screens::ApplicationList(this)); break;
      case Apps::Clock:
        currentScreen.reset(new Screens::Clock(this, dateTimeController, batteryController, bleController));
        onClockApp = true;
        break;
//      case Apps::Test: currentScreen.reset(new Screens::Message(this)); break;
      case Apps::SysInfo: currentScreen.reset(new Screens::SystemInfo(this, dateTimeController, batteryController, brightnessController, bleController, watchdog)); break;
      case Apps::Meter: currentScreen.reset(new Screens::Meter(this)); break;
      case Apps::Gauge: currentScreen.reset(new Screens::Gauge(this)); break;
      case Apps::Paint: currentScreen.reset(new Screens::InfiniPaint(this, lvgl)); break;
      case Apps::Brightness : currentScreen.reset(new Screens::Brightness(this, brightnessController)); break;
      case Apps::Music : currentScreen.reset(new Screens::Music(this, systemTask.nimble().music())); break;
      case Apps::FirmwareValidation: currentScreen.reset(new Screens::FirmwareValidation(this, validator)); break;
    }
    nextApp = Apps::None;
  }
  lv_task_handler();
}

void DisplayApp::IdleState() {

}

void DisplayApp::PushMessage(DisplayApp::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(msgQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}

TouchEvents DisplayApp::OnTouchEvent() {
  auto info = touchPanel.GetTouchInfo();
  if(info.isTouch) {
    switch(info.gesture) {
      case Pinetime::Drivers::Cst816S::Gestures::SingleTap:
        if(touchMode == TouchModes::Gestures) SetTapCoordinates(info.x, info.y);
        return TouchEvents::Tap;
      case Pinetime::Drivers::Cst816S::Gestures::LongPress:
        return TouchEvents::LongTap;
      case Pinetime::Drivers::Cst816S::Gestures::DoubleTap:
        return TouchEvents::DoubleTap;
      case Pinetime::Drivers::Cst816S::Gestures::SlideRight:
        return (isLeftHandWorn ? TouchEvents::SwipeLeft : TouchEvents::SwipeRight);
      case Pinetime::Drivers::Cst816S::Gestures::SlideLeft:
        return (isLeftHandWorn ? TouchEvents::SwipeRight : TouchEvents::SwipeLeft);
      case Pinetime::Drivers::Cst816S::Gestures::SlideDown:
        return (isLeftHandWorn ? TouchEvents::SwipeUp : TouchEvents::SwipeDown);
      case Pinetime::Drivers::Cst816S::Gestures::SlideUp:
        return (isLeftHandWorn ? TouchEvents::SwipeDown : TouchEvents::SwipeUp);
      case Pinetime::Drivers::Cst816S::Gestures::None:
      default:
        return TouchEvents::None;
    }
  }
  return TouchEvents::None;
}

void DisplayApp::StartApp(Apps app) {
  nextApp = app;
}

void DisplayApp::SetFullRefresh(DisplayApp::FullRefreshDirections direction) {
  switch(direction){
    case DisplayApp::FullRefreshDirections::Down:
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Down);
      break;
    case DisplayApp::FullRefreshDirections::Up:
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Up);
      break;
    default: break;
  }

}

void DisplayApp::SetTouchMode(DisplayApp::TouchModes mode) {
  touchMode = mode;
}


void DisplayApp::SetHandOrientation(bool left_hand)
{
  isLeftHandWorn = left_hand;

  // LCD: mirror x/y memory positions
  lcd.SetOrientation(left_hand ? lcd.Orientation_180 : lcd.Orientation_0);
}


void DisplayApp::SetTapCoordinates(uint16_t x, uint16_t y)
{
  // mirror x/y tap coordinates
  if (isLeftHandWorn) {
    x = 240 - x;
    y = 240 - y;
  }

  lvgl.SetNewTapEvent(x, y);
}
