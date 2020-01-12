#include <FreeRTOS.h>
#include <task.h>
#include <libraries/bsp/bsp.h>
#include <legacy/nrf_drv_clock.h>
#include <libraries/timer/app_timer.h>
#include <libraries/gpiote/app_gpiote.h>
#include <DisplayApp/DisplayApp.h>
#include <softdevice/common/nrf_sdh.h>
#include <softdevice/common/nrf_sdh_freertos.h>
#include <hal/nrf_rtc.h>
#include <timers.h>
#include <libraries/log/nrf_log.h>
#include <ble/ble_services/ble_cts_c/ble_cts_c.h>
#include <Components/DateTime/DateTimeController.h>
#include "BLE/BleManager.h"
#include "Components/Battery/BatteryController.h"
#include "Components/Ble/BleController.h"

#if NRF_LOG_ENABLED
#include "Logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
#include "Logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

std::unique_ptr<Pinetime::Applications::DisplayApp> displayApp;
TaskHandle_t systemThread;
bool isSleeping = false;
TimerHandle_t debounceTimer;
Pinetime::Controllers::Battery batteryController;
Pinetime::Controllers::Ble bleController;
Pinetime::Controllers::DateTime dateTimeController;


void ble_manager_set_ble_connection_callback(void (*connection)());
void ble_manager_set_ble_disconnection_callback(void (*disconnection)());
static constexpr uint8_t pinButton = 13;
static constexpr uint8_t pinTouchIrq = 28;

void nrfx_gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  if(pin == pinTouchIrq) {
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::TouchEvent);
    if(!isSleeping) return;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTimerStartFromISR(debounceTimer, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void DebounceTimerCallback(TimerHandle_t xTimer) {
  xTimerStop(xTimer, 0);
  if(isSleeping) {
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::GoToRunning);
    isSleeping = false;
    batteryController.Update();
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateBatteryLevel);
  }
  else {
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::GoToSleep);
    isSleeping = true;
  }
}

void SystemTask(void *) {
  APP_GPIOTE_INIT(2);
  bool erase_bonds=false;
  nrf_sdh_freertos_init(ble_manager_start_advertising, &erase_bonds);
  displayApp->Start();

  batteryController.Init();
  batteryController.Update();
  displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateBatteryLevel);

  debounceTimer = xTimerCreate ("debounceTimer", 200, pdFALSE, (void *) 0, DebounceTimerCallback);

  nrf_gpio_cfg_sense_input(pinButton, (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pulldown, (nrf_gpio_pin_sense_t)GPIO_PIN_CNF_SENSE_High);
  nrf_gpio_cfg_output(15);
  nrf_gpio_pin_set(15);

  nrfx_gpiote_in_config_t pinConfig;
  pinConfig.skip_gpio_setup = true;
  pinConfig.hi_accuracy = false;
  pinConfig.is_watcher = false;
  pinConfig.sense = (nrf_gpiote_polarity_t)NRF_GPIOTE_POLARITY_HITOLO;
  pinConfig.pull = (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pulldown;

  nrfx_gpiote_in_init(pinButton, &pinConfig, nrfx_gpiote_evt_handler);

  nrf_gpio_cfg_sense_input(pinTouchIrq, (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pullup, (nrf_gpio_pin_sense_t)GPIO_PIN_CNF_SENSE_Low);

  pinConfig.skip_gpio_setup = true;
  pinConfig.hi_accuracy = false;
  pinConfig.is_watcher = false;
  pinConfig.sense = (nrf_gpiote_polarity_t)NRF_GPIOTE_POLARITY_HITOLO;
  pinConfig.pull = (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pullup;

  nrfx_gpiote_in_init(pinTouchIrq, &pinConfig, nrfx_gpiote_evt_handler);
  vTaskSuspend(nullptr);
}

void OnBleConnection() {
  bleController.Connect();
  displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateBleConnection);
}

void OnBleDisconnection() {
  bleController.Disconnect();
  displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateBleConnection);
}

void OnNewTime(current_time_char_t* currentTime) {
  auto dayOfWeek = currentTime->exact_time_256.day_date_time.day_of_week;
  auto year = currentTime->exact_time_256.day_date_time.date_time.year;
  auto month = currentTime->exact_time_256.day_date_time.date_time.month;
  auto day = currentTime->exact_time_256.day_date_time.date_time.day;
  auto hour = currentTime->exact_time_256.day_date_time.date_time.hours;
  auto minute = currentTime->exact_time_256.day_date_time.date_time.minutes;
  auto second = currentTime->exact_time_256.day_date_time.date_time.seconds;
  dateTimeController.UpdateTime(year, static_cast<Pinetime::Controllers::DateTime::Months >(month), day, static_cast<Pinetime::Controllers::DateTime::Days>(dayOfWeek), hour, minute, second);
  displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateDateTime);
}

int main(void) {
  displayApp.reset(new Pinetime::Applications::DisplayApp(batteryController, bleController, dateTimeController));
  logger.Init();
  nrf_drv_clock_init();

  if (pdPASS != xTaskCreate(SystemTask, "MAIN", 256, nullptr, 0, &systemThread))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);

  ble_manager_init();
  ble_manager_set_new_time_callback(OnNewTime);
  ble_manager_set_ble_connection_callback(OnBleConnection);
  ble_manager_set_ble_disconnection_callback(OnBleDisconnection);

  vTaskStartScheduler();

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }
}




