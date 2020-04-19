#include <FreeRTOS.h>
#include <task.h>
#include <legacy/nrf_drv_clock.h>
#include <libraries/timer/app_timer.h>
#include <libraries/gpiote/app_gpiote.h>
#include <DisplayApp/DisplayApp.h>
#include <softdevice/common/nrf_sdh.h>
#include <hal/nrf_rtc.h>
#include <timers.h>
//#include <ble/ble_services/ble_cts_c/ble_cts_c.h>
#include <Components/DateTime/DateTimeController.h>
#include "BLE/BleManager.h"
#include "Components/Battery/BatteryController.h"
#include "Components/Ble/BleController.h"
#include <drivers/St7789.h>
#include <drivers/SpiMaster.h>
#include <DisplayApp/LittleVgl.h>
#include <SystemTask/SystemTask.h>
#include <Components/Ble/NotificationManager.h>
#include <nimble/nimble_port_freertos.h>
#include <nimble/npl_freertos.h>
#include <nimble/nimble_port.h>
#include <host/ble_hs.h>
#include <controller/ble_ll.h>
#include <os/os_cputime.h>
#include <transport/ram/ble_hci_ram.h>
#include <hal/nrf_wdt.h>
#include <host/util/util.h>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>


#if NRF_LOG_ENABLED
#include "Logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
#include "Logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

static constexpr uint8_t pinSpiSck = 2;
static constexpr uint8_t pinSpiMosi = 3;
static constexpr uint8_t pinSpiMiso = 4;
static constexpr uint8_t pinSpiCsn = 25;
static constexpr uint8_t pinLcdDataCommand = 18;

Pinetime::Drivers::SpiMaster spi{Pinetime::Drivers::SpiMaster::SpiModule::SPI0, {
        Pinetime::Drivers::SpiMaster::BitOrder::Msb_Lsb,
        Pinetime::Drivers::SpiMaster::Modes::Mode3,
        Pinetime::Drivers::SpiMaster::Frequencies::Freq8Mhz,
        pinSpiSck,
        pinSpiMosi,
        pinSpiMiso,
        pinSpiCsn
  }
};
Pinetime::Drivers::St7789 lcd {spi, pinLcdDataCommand};
Pinetime::Drivers::Cst816S touchPanel {};
Pinetime::Components::LittleVgl lvgl {lcd, touchPanel};


TimerHandle_t debounceTimer;
Pinetime::Controllers::Battery batteryController;
Pinetime::Controllers::Ble bleController;
Pinetime::Controllers::DateTime dateTimeController;
void ble_manager_set_ble_connection_callback(void (*connection)());
void ble_manager_set_ble_disconnection_callback(void (*disconnection)());
static constexpr uint8_t pinTouchIrq = 28;
std::unique_ptr<Pinetime::System::SystemTask> systemTask;

Pinetime::Controllers::NotificationManager notificationManager;

void nrfx_gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  if(pin == pinTouchIrq) {
    systemTask->OnTouchEvent();
    return ;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTimerStartFromISR(debounceTimer, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

extern "C" {
  void vApplicationIdleHook(void) {
    lv_tick_inc(1);
  }
}

void DebounceTimerCallback(TimerHandle_t xTimer) {
  xTimerStop(xTimer, 0);
  systemTask->OnButtonPushed();
}

void OnBleConnection() {
  bleController.Connect();
}

void OnBleDisconnection() {
  bleController.Disconnect();
}

void OnNewNotification(const char* message, uint8_t size) {
  notificationManager.Push(Pinetime::Controllers::NotificationManager::Categories::SimpleAlert, message, size);
  systemTask->PushMessage(Pinetime::System::SystemTask::Messages::OnNewNotification);
}

//void OnNewTime(current_time_char_t* currentTime) {
//  auto dayOfWeek = currentTime->exact_time_256.day_date_time.day_of_week;
//  auto year = currentTime->exact_time_256.day_date_time.date_time.year;
//  auto month = currentTime->exact_time_256.day_date_time.date_time.month;
//  auto day = currentTime->exact_time_256.day_date_time.date_time.day;
//  auto hour = currentTime->exact_time_256.day_date_time.date_time.hours;
//  auto minute = currentTime->exact_time_256.day_date_time.date_time.minutes;
//  auto second = currentTime->exact_time_256.day_date_time.date_time.seconds;
//
//  dateTimeController.SetTime(year, month, day,
//                             dayOfWeek, hour, minute, second, nrf_rtc_counter_get(portNRF_RTC_REG));
//
//  systemTask->PushMessage(Pinetime::System::SystemTask::Messages::OnNewTime);
//}

void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler(void) {
  if(((NRF_SPIM0->INTENSET & (1<<6)) != 0) && NRF_SPIM0->EVENTS_END == 1) {
    NRF_SPIM0->EVENTS_END = 0;
    spi.OnEndEvent();
  }

  if(((NRF_SPIM0->INTENSET & (1<<19)) != 0) && NRF_SPIM0->EVENTS_STARTED == 1) {
    NRF_SPIM0->EVENTS_STARTED = 0;
    spi.OnStartedEvent();
  }

  if(((NRF_SPIM0->INTENSET & (1<<1)) != 0) && NRF_SPIM0->EVENTS_STOPPED == 1) {
    NRF_SPIM0->EVENTS_STOPPED = 0;
  }
}

static void (*radio_isr_addr)(void) ;
static void (*rng_isr_addr)(void) ;
static void (*rtc0_isr_addr)(void) ;


/* Some interrupt handlers required for NimBLE radio driver */
extern "C" {
void RADIO_IRQHandler(void) {
  ((void (*)(void)) radio_isr_addr)();
}

void RNG_IRQHandler(void) {
  ((void (*)(void)) rng_isr_addr)();
}

void RTC0_IRQHandler(void) {
  ((void (*)(void)) rtc0_isr_addr)();
}

void WDT_IRQHandler(void) {
  nrf_wdt_event_clear(NRF_WDT_EVENT_TIMEOUT);
}

void npl_freertos_hw_set_isr(int irqn, void (*addr)(void)) {
  switch (irqn) {
    case RADIO_IRQn:
      radio_isr_addr = addr;
      break;
    case RNG_IRQn:
      rng_isr_addr = addr;
      break;
    case RTC0_IRQn:
      rtc0_isr_addr = addr;
      break;
  }
}

uint32_t
npl_freertos_hw_enter_critical(void) {
  uint32_t ctx = __get_PRIMASK();
  __disable_irq();
  return (ctx & 0x01);
}

void npl_freertos_hw_exit_critical(uint32_t ctx) {
  if (!ctx) {
    __enable_irq();
  }
}


static struct ble_npl_eventq g_eventq_dflt;

struct ble_npl_eventq *
nimble_port_get_dflt_eventq(void) {
  return &g_eventq_dflt;
}

void nimble_port_run(void) {
  struct ble_npl_event *ev;

  while (1) {
    ev = ble_npl_eventq_get(&g_eventq_dflt, BLE_NPL_TIME_FOREVER);
    ble_npl_event_run(ev);
  }
}

void BleHost(void *) {
  nimble_port_run();
}

void nimble_port_init(void) {
  void os_msys_init(void);
  void ble_store_ram_init(void);
  ble_npl_eventq_init(&g_eventq_dflt);
  os_msys_init();
  ble_hs_init();
  ble_store_ram_init();

  hal_timer_init(5, NULL);
  os_cputime_init(32768);
  ble_ll_init();
  ble_hci_ram_init();
  nimble_port_freertos_init(BleHost);


}

void nimble_port_ll_task_func(void *args) {
//  extern void ble_ll_task(void *arg);
  ble_ll_task(args);
}
}

int main(void) {
  logger.Init();

  nrf_drv_clock_init();

  debounceTimer = xTimerCreate ("debounceTimer", 200, pdFALSE, (void *) 0, DebounceTimerCallback);

  systemTask.reset(new Pinetime::System::SystemTask(spi, lcd, touchPanel, lvgl, batteryController, bleController,
                                                    dateTimeController, notificationManager));
  systemTask->Start();

  nimble_port_init();
  ble_svc_gap_init();
  ble_svc_gatt_init();

//  ble_manager_init();
//  ble_manager_set_new_time_callback(OnNewTime);
//  ble_manager_set_ble_connection_callback(OnBleConnection);
//  ble_manager_set_ble_disconnection_callback(OnBleDisconnection);
//  ble_manager_set_new_notification_callback(OnNewNotification);

  vTaskStartScheduler();

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }
}




