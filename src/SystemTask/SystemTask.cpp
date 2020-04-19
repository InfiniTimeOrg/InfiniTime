#include <libraries/log/nrf_log.h>
#include <libraries/gpiote/app_gpiote.h>
#include <drivers/Cst816s.h>
#include <DisplayApp/LittleVgl.h>
#include <hal/nrf_rtc.h>
#include <BLE/BleManager.h>
#include <Components/Ble/NotificationManager.h>
#include <services/gap/ble_svc_gap.h>
#include <host/ble_gatt.h>
#include <host/ble_hs_adv.h>
#include "SystemTask.h"
#include <nimble/hci_common.h>
#include <host/ble_gap.h>
#include <host/util/util.h>
#include "../main.h"
using namespace Pinetime::System;

SystemTask::SystemTask(Drivers::SpiMaster &spi, Drivers::St7789 &lcd, Drivers::Cst816S &touchPanel,
                       Components::LittleVgl &lvgl,
                       Controllers::Battery &batteryController, Controllers::Ble &bleController,
                       Controllers::DateTime &dateTimeController,
                       Pinetime::Controllers::NotificationManager& notificationManager) :
                       spi{spi}, lcd{lcd}, touchPanel{touchPanel}, lvgl{lvgl}, batteryController{batteryController},
                       bleController{bleController}, dateTimeController{dateTimeController},
                       watchdog{}, watchdogView{watchdog}, notificationManager{notificationManager} {
  systemTaksMsgQueue = xQueueCreate(10, 1);
}

void SystemTask::Start() {
  if (pdPASS != xTaskCreate(SystemTask::Process, "MAIN", 350, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void SystemTask::Process(void *instance) {
  auto *app = static_cast<SystemTask *>(instance);
  NRF_LOG_INFO("SystemTask task started!");
  app->Work();
}

static int _gap_event_cb(struct ble_gap_event *event, void *arg)
{
  return 0;
}

static int
adv_event(struct ble_gap_event *event, void *arg)
{
  switch (event->type) {
    case BLE_GAP_EVENT_ADV_COMPLETE:
      return 0;
    case BLE_GAP_EVENT_CONNECT:
      return 0;
    case BLE_GAP_EVENT_DISCONNECT:
      return 0;
    default:
      return 0;
  }
}

void SystemTask::Work() {
//  watchdog.Setup(7);
//  watchdog.Start();
  NRF_LOG_INFO("Last reset reason : %s", Pinetime::Drivers::Watchdog::ResetReasonToString(watchdog.ResetReason()));
  APP_GPIOTE_INIT(2);
//  bool erase_bonds=true;
//  ble_manager_init_peer_manager();
//  nrf_sdh_freertos_init(ble_manager_start_advertising, &erase_bonds);
/* BLE */
  while (!ble_hs_synced()) {}

  int res;
  res = ble_hs_util_ensure_addr(0);
  assert(res == 0);
  uint8_t addrType;
  res = ble_hs_id_infer_auto(0, &addrType);
  assert(res == 0);

  res = ble_svc_gap_device_name_set("Pinetime-JF");
  assert(res == 0);


  /* set adv parameters */
  struct ble_gap_adv_params adv_params;
  struct ble_hs_adv_fields fields;
  /* advertising payload is split into advertising data and advertising
     response, because all data cannot fit into single packet; name of device
     is sent as response to scan request */
  struct ble_hs_adv_fields rsp_fields;

  /* fill all fields and parameters with zeros */
  memset(&adv_params, 0, sizeof(adv_params));
  memset(&fields, 0, sizeof(fields));
  memset(&rsp_fields, 0, sizeof(rsp_fields));

  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

  fields.flags = BLE_HS_ADV_F_DISC_GEN |
                 BLE_HS_ADV_F_BREDR_UNSUP;
//  fields.uuids128 = BLE_UUID128(BLE_UUID128_DECLARE(
//          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
//          0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff));
  fields.num_uuids128 = 0;
  fields.uuids128_is_complete = 0;;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  rsp_fields.name = (uint8_t *)"Pinetime-JF";
  rsp_fields.name_len = strlen("Pinetime-JF");
  rsp_fields.name_is_complete = 1;

  res = ble_gap_adv_set_fields(&fields);
  assert(res == 0);

  res = ble_gap_adv_rsp_set_fields(&rsp_fields);

  res = ble_gap_adv_start(addrType, NULL, 36000,
                         &adv_params, adv_event, NULL);
  assert(res == 0);

/* /BLE*/

  spi.Init();
  lcd.Init();
  touchPanel.Init();
  batteryController.Init();

  displayApp.reset(new Pinetime::Applications::DisplayApp(lcd, lvgl, touchPanel, batteryController, bleController,
                                                          dateTimeController, watchdogView, *this, notificationManager));
  displayApp->Start();

  batteryController.Update();
  displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateBatteryLevel);

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


  while(true) {
    uint8_t msg;
    if (xQueueReceive(systemTaksMsgQueue, &msg, isSleeping?2500 : 1000)) {
      Messages message = static_cast<Messages >(msg);
      switch(message) {
        case Messages::GoToRunning: isSleeping = false; break;
        case Messages::GoToSleep:
          NRF_LOG_INFO("[SystemTask] Going to sleep");
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::GoToSleep);
          isSleeping = true; break;
        case Messages::OnNewTime:
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateDateTime);
          break;
        case Messages::OnNewNotification:
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::NewNotification);
          break;
        default: break;
      }
    }
    uint32_t systick_counter = nrf_rtc_counter_get(portNRF_RTC_REG);
    dateTimeController.UpdateTime(systick_counter);
    batteryController.Update();

    if(!nrf_gpio_pin_read(pinButton))
      watchdog.Kick();
  }
}

void SystemTask::OnButtonPushed() {

  if(!isSleeping) {
    NRF_LOG_INFO("[SystemTask] Button pushed");
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::ButtonPushed);
  }
  else {
    NRF_LOG_INFO("[SystemTask] Button pushed, waking up");
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::GoToRunning);
    isSleeping = false;
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateBatteryLevel);
  }
}

void SystemTask::OnTouchEvent() {
  NRF_LOG_INFO("[SystemTask] Touch event");
  displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::TouchEvent);
}

void SystemTask::PushMessage(SystemTask::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(systemTaksMsgQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}
