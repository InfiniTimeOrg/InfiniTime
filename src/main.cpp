#include <FreeRTOS.h>
#include <task.h>
#include <legacy/nrf_drv_clock.h>
#include <libraries/timer/app_timer.h>
#include <libraries/gpiote/app_gpiote.h>
#include <DisplayApp/DisplayApp.h>
#include <softdevice/common/nrf_sdh.h>
#include <hal/nrf_rtc.h>
#include <timers.h>
#include <ble/ble_services/ble_cts_c/ble_cts_c.h>
#include <Components/DateTime/DateTimeController.h>
#include "BLE/BleManager.h"
#include "Components/Battery/BatteryController.h"
#include "Components/Ble/BleController.h"
#include <drivers/St7789.h>
#include <drivers/SpiMaster.h>
#include <DisplayApp/LittleVgl.h>
#include <SystemTask/SystemTask.h>

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

void OnNewTime(current_time_char_t* currentTime) {
  auto dayOfWeek = currentTime->exact_time_256.day_date_time.day_of_week;
  auto year = currentTime->exact_time_256.day_date_time.date_time.year;
  auto month = currentTime->exact_time_256.day_date_time.date_time.month;
  auto day = currentTime->exact_time_256.day_date_time.date_time.day;
  auto hour = currentTime->exact_time_256.day_date_time.date_time.hours;
  auto minute = currentTime->exact_time_256.day_date_time.date_time.minutes;
  auto second = currentTime->exact_time_256.day_date_time.date_time.seconds;

  dateTimeController.SetTime(year, month, day,
                             dayOfWeek, hour, minute, second, nrf_rtc_counter_get(portNRF_RTC_REG));
}

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

int main(void) {
  logger.Init();

  nrf_gpio_cfg_output(27);
  nrf_gpio_pin_clear(27);
  nrf_gpio_cfg_output(29);
  nrf_gpio_pin_clear(29);

  nrf_gpio_cfg_output(20);
  nrf_gpio_pin_clear(20);

  nrf_gpio_cfg_output(17);
  nrf_gpio_pin_clear(17);

  nrf_gpio_cfg_output(11);
  nrf_gpio_pin_clear(11);



  nrf_drv_clock_init();

  debounceTimer = xTimerCreate ("debounceTimer", 200, pdFALSE, (void *) 0, DebounceTimerCallback);

  systemTask.reset(new Pinetime::System::SystemTask(spi, lcd, touchPanel, lvgl, batteryController, bleController, dateTimeController));
  systemTask->Start();

  ble_manager_init();
  ble_manager_set_new_time_callback(OnNewTime);
  ble_manager_set_ble_connection_callback(OnBleConnection);
  ble_manager_set_ble_disconnection_callback(OnBleDisconnection);

  vTaskStartScheduler();

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }
}




