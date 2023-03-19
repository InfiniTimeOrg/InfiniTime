// nrf
#include <hal/nrf_wdt.h>
#include <legacy/nrf_drv_clock.h>
#include <libraries/gpiote/app_gpiote.h>
#include <softdevice/common/nrf_sdh.h>
#include <nrf_delay.h>

// nimble
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <controller/ble_ll.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <nimble/npl_freertos.h>
#include <os/os_cputime.h>
#include <services/gap/ble_svc_gap.h>
#include <transport/ram/ble_hci_ram.h>
#undef max
#undef min

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <drivers/Hrs3300.h>
#include <drivers/Bma421.h>

#include "BootloaderVersion.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/brightness/BrightnessController.h"
#include "components/motor/MotorController.h"
#include "components/datetime/DateTimeController.h"
#include "components/heartrate/HeartRateController.h"
#include "components/fs/FS.h"
#include "drivers/Spi.h"
#include "drivers/SpiMaster.h"
#include "drivers/SpiNorFlash.h"
#include "drivers/St7789.h"
#include "drivers/TwiMaster.h"
#include "drivers/Cst816s.h"
#include "drivers/PinMap.h"
#include "systemtask/SystemTask.h"
#include "touchhandler/TouchHandler.h"
#include "buttonhandler/ButtonHandler.h"

#if NRF_LOG_ENABLED
  #include "logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
  #include "logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

static constexpr uint8_t touchPanelTwiAddress = 0x15;
static constexpr uint8_t motionSensorTwiAddress = 0x18;
static constexpr uint8_t heartRateSensorTwiAddress = 0x44;

Pinetime::Drivers::SpiMaster spi {Pinetime::Drivers::SpiMaster::SpiModule::SPI0,
                                  {Pinetime::Drivers::SpiMaster::BitOrder::Msb_Lsb,
                                   Pinetime::Drivers::SpiMaster::Modes::Mode3,
                                   Pinetime::Drivers::SpiMaster::Frequencies::Freq8Mhz,
                                   Pinetime::PinMap::SpiSck,
                                   Pinetime::PinMap::SpiMosi,
                                   Pinetime::PinMap::SpiMiso}};

Pinetime::Drivers::Spi lcdSpi {spi, Pinetime::PinMap::SpiLcdCsn};
Pinetime::Drivers::St7789 lcd {lcdSpi, Pinetime::PinMap::LcdDataCommand};

Pinetime::Drivers::Spi flashSpi {spi, Pinetime::PinMap::SpiFlashCsn};
Pinetime::Drivers::SpiNorFlash spiNorFlash {flashSpi};

// The TWI device should work @ up to 400Khz but there is a HW bug which prevent it from
// respecting correct timings. According to erratas heet, this magic value makes it run
// at ~390Khz with correct timings.
static constexpr uint32_t MaxTwiFrequencyWithoutHardwareBug {0x06200000};
Pinetime::Drivers::TwiMaster twiMaster {NRF_TWIM1, MaxTwiFrequencyWithoutHardwareBug, Pinetime::PinMap::TwiSda, Pinetime::PinMap::TwiScl};
Pinetime::Drivers::Cst816S touchPanel {twiMaster, touchPanelTwiAddress};
#ifdef PINETIME_IS_RECOVERY
  #include "displayapp/DisplayAppRecovery.h"
#else
  #include "displayapp/DisplayApp.h"
#endif
Pinetime::Drivers::Bma421 motionSensor {twiMaster, motionSensorTwiAddress};
Pinetime::Drivers::Hrs3300 heartRateSensor {twiMaster, heartRateSensorTwiAddress};

TimerHandle_t debounceTimer;
TimerHandle_t debounceChargeTimer;
Pinetime::Controllers::Battery batteryController;
Pinetime::Controllers::Ble bleController;

Pinetime::Controllers::HeartRateController heartRateController;
Pinetime::Applications::HeartRateTask heartRateApp(heartRateSensor, heartRateController);

Pinetime::Controllers::FS fs {spiNorFlash};
Pinetime::Controllers::Settings settingsController {fs};
Pinetime::Controllers::MotorController motorController {};

Pinetime::Controllers::DateTime dateTimeController {settingsController};
Pinetime::Drivers::Watchdog watchdog;
Pinetime::Controllers::NotificationManager notificationManager;
Pinetime::Controllers::MotionController motionController;
Pinetime::Controllers::TimerController timerController;
Pinetime::Controllers::AlarmController alarmController {dateTimeController};
Pinetime::Controllers::TouchHandler touchHandler;
Pinetime::Controllers::ButtonHandler buttonHandler;
Pinetime::Controllers::BrightnessController brightnessController {};

Pinetime::Applications::DisplayApp displayApp(lcd,
                                              touchPanel,
                                              batteryController,
                                              bleController,
                                              dateTimeController,
                                              watchdog,
                                              notificationManager,
                                              heartRateController,
                                              settingsController,
                                              motorController,
                                              motionController,
                                              timerController,
                                              alarmController,
                                              brightnessController,
                                              touchHandler,
                                              fs);

Pinetime::System::SystemTask systemTask(spi,
                                        spiNorFlash,
                                        twiMaster,
                                        touchPanel,
                                        batteryController,
                                        bleController,
                                        dateTimeController,
                                        timerController,
                                        alarmController,
                                        watchdog,
                                        notificationManager,
                                        heartRateSensor,
                                        motionController,
                                        motionSensor,
                                        settingsController,
                                        heartRateController,
                                        displayApp,
                                        heartRateApp,
                                        fs,
                                        touchHandler,
                                        buttonHandler);

/* Variable Declarations for variables in noinit SRAM
   Increment NoInit_MagicValue upon adding variables to this area
*/
extern uint32_t __start_noinit_data;
extern uint32_t __stop_noinit_data;
static constexpr uint32_t NoInit_MagicValue = 0xDEAD0000;
uint32_t NoInit_MagicWord __attribute__((section(".noinit")));
std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> NoInit_BackUpTime __attribute__((section(".noinit")));

void nrfx_gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  if (pin == Pinetime::PinMap::Cst816sIrq) {
    systemTask.OnTouchEvent();
    return;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (pin == Pinetime::PinMap::PowerPresent and action == NRF_GPIOTE_POLARITY_TOGGLE) {
    xTimerStartFromISR(debounceChargeTimer, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  } else if (pin == Pinetime::PinMap::Button) {
    xTimerStartFromISR(debounceTimer, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void DebounceTimerChargeCallback(TimerHandle_t xTimer) {
  xTimerStop(xTimer, 0);
  systemTask.PushMessage(Pinetime::System::Messages::OnChargingEvent);
}

void DebounceTimerCallback(TimerHandle_t /*unused*/) {
  systemTask.PushMessage(Pinetime::System::Messages::HandleButtonEvent);
}

void SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler(void) {
  if (((NRF_SPIM0->INTENSET & (1 << 6)) != 0) && NRF_SPIM0->EVENTS_END == 1) {
    NRF_SPIM0->EVENTS_END = 0;
    spi.OnEndEvent();
  }

  if (((NRF_SPIM0->INTENSET & (1 << 19)) != 0) && NRF_SPIM0->EVENTS_STARTED == 1) {
    NRF_SPIM0->EVENTS_STARTED = 0;
    spi.OnStartedEvent();
  }

  if (((NRF_SPIM0->INTENSET & (1 << 1)) != 0) && NRF_SPIM0->EVENTS_STOPPED == 1) {
    NRF_SPIM0->EVENTS_STOPPED = 0;
  }
}

static void (*radio_isr_addr)();
static void (*rng_isr_addr)();
static void (*rtc0_isr_addr)();

/* Some interrupt handlers required for NimBLE radio driver */
extern "C" {
void RADIO_IRQHandler(void) {
  ((void (*)()) radio_isr_addr)();
}

void RNG_IRQHandler(void) {
  ((void (*)()) rng_isr_addr)();
}

void RTC0_IRQHandler(void) {
  ((void (*)()) rtc0_isr_addr)();
}

void WDT_IRQHandler(void) {
  nrf_wdt_event_clear(NRF_WDT_EVENT_TIMEOUT);
}

void npl_freertos_hw_set_isr(int irqn, void (*addr)()) {
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
    default:
      break;
  }
}

uint32_t npl_freertos_hw_enter_critical(void) {
  uint32_t ctx = __get_PRIMASK();
  __disable_irq();
  return (ctx & 0x01);
}

void npl_freertos_hw_exit_critical(uint32_t ctx) {
  if (ctx == 0) {
    __enable_irq();
  }
}

static struct ble_npl_eventq g_eventq_dflt;

struct ble_npl_eventq* nimble_port_get_dflt_eventq(void) {
  return &g_eventq_dflt;
}

void nimble_port_run(void) {
  struct ble_npl_event* event;
  while (true) {
    event = ble_npl_eventq_get(&g_eventq_dflt, BLE_NPL_TIME_FOREVER);
    ble_npl_event_run(event);
  }
}

void BleHost(void* /*unused*/) {
  nimble_port_run();
}

void nimble_port_init(void) {
  void os_msys_init(void);
  void ble_store_ram_init(void);
  ble_npl_eventq_init(&g_eventq_dflt);
  os_msys_init();
  ble_hs_init();
  ble_store_ram_init();

  int res = hal_timer_init(5, nullptr);
  ASSERT(res == 0);
  res = os_cputime_init(32768);
  ASSERT(res == 0);
  ble_ll_init();
  ble_hci_ram_init();
  nimble_port_freertos_init(BleHost);
}

void nimble_port_ll_task_func(void* args) {
  extern void ble_ll_task(void*);
  ble_ll_task(args);
}
}

void calibrate_lf_clock_rc(nrf_drv_clock_evt_type_t /*event*/) {
  // 16 * 0.25s = 4s calibration cycle
  // Not recursive, call is deferred via internal calibration timer
  nrf_drv_clock_calibration_start(16, calibrate_lf_clock_rc);
}

int main() {
  logger.Init();

  nrf_drv_clock_init();
  nrf_drv_clock_lfclk_request(nullptr);

  // When loading the firmware via the Wasp-OS reloader-factory, which uses the used internal LF RC oscillator,
  // the LF clock has to be explicitly restarted because InfiniTime uses the external crystal oscillator if available.
  // If the clock is not restarted, the Bluetooth timers fail to initialize.
  nrfx_clock_lfclk_start();
  while (!nrf_clock_lf_is_running()) {
  }

// The RC source for the LF clock has to be calibrated
#if (CLOCK_CONFIG_LF_SRC == NRF_CLOCK_LFCLK_RC)
  nrf_drv_clock_calibration_start(0, calibrate_lf_clock_rc);
#endif

  // Unblock i2c?
  nrf_gpio_cfg(Pinetime::PinMap::TwiScl,
               NRF_GPIO_PIN_DIR_OUTPUT,
               NRF_GPIO_PIN_INPUT_DISCONNECT,
               NRF_GPIO_PIN_NOPULL,
               NRF_GPIO_PIN_S0D1,
               NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_pin_set(Pinetime::PinMap::TwiScl);
  for (uint8_t i = 0; i < 16; i++) {
    nrf_gpio_pin_toggle(Pinetime::PinMap::TwiScl);
    nrf_delay_us(5);
  }
  nrf_gpio_cfg_default(Pinetime::PinMap::TwiScl);

  debounceTimer = xTimerCreate("debounceTimer", 10, pdFALSE, nullptr, DebounceTimerCallback);
  debounceChargeTimer = xTimerCreate("debounceTimerCharge", 200, pdFALSE, nullptr, DebounceTimerChargeCallback);

  // retrieve version stored by bootloader
  Pinetime::BootloaderVersion::SetVersion(NRF_TIMER2->CC[0]);

  if (NoInit_MagicWord == NoInit_MagicValue) {
    dateTimeController.SetCurrentTime(NoInit_BackUpTime);
  } else {
    // Clear Memory to known state
    memset(&__start_noinit_data, 0, (uintptr_t) &__stop_noinit_data - (uintptr_t) &__start_noinit_data);
    NoInit_MagicWord = NoInit_MagicValue;
  }

  systemTask.Start();

  nimble_port_init();

  vTaskStartScheduler();

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }
}
