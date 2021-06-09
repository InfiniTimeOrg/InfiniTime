// nrf
#include <hal/nrf_rtc.h>
#include <hal/nrf_wdt.h>
#include <legacy/nrf_drv_clock.h>
#include <libraries/gpiote/app_gpiote.h>
#include <libraries/timer/app_timer.h>
#include <softdevice/common/nrf_sdh.h>

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

#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motor/MotorController.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "drivers/Spi.h"
#include "drivers/SpiMaster.h"
#include "drivers/SpiNorFlash.h"
#include "drivers/St7789.h"
#include "drivers/TwiMaster.h"
#include "drivers/Cst816s.h"
#include "systemtask/SystemTask.h"

#if NRF_LOG_ENABLED
  #include "logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
  #include "logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

#include <memory>

static constexpr uint8_t pinSpiSck = 2;
static constexpr uint8_t pinSpiMosi = 3;
static constexpr uint8_t pinSpiMiso = 4;
static constexpr uint8_t pinSpiFlashCsn = 5;
static constexpr uint8_t pinLcdCsn = 25;
static constexpr uint8_t pinLcdDataCommand = 18;
static constexpr uint8_t pinTwiScl = 7;
static constexpr uint8_t pinTwiSda = 6;
static constexpr uint8_t touchPanelTwiAddress = 0x15;
static constexpr uint8_t motionSensorTwiAddress = 0x18;
static constexpr uint8_t heartRateSensorTwiAddress = 0x44;

Pinetime::Drivers::SpiMaster spi {Pinetime::Drivers::SpiMaster::SpiModule::SPI0,
                                  {Pinetime::Drivers::SpiMaster::BitOrder::Msb_Lsb,
                                   Pinetime::Drivers::SpiMaster::Modes::Mode3,
                                   Pinetime::Drivers::SpiMaster::Frequencies::Freq8Mhz,
                                   pinSpiSck,
                                   pinSpiMosi,
                                   pinSpiMiso}};

Pinetime::Drivers::Spi lcdSpi {spi, pinLcdCsn};
Pinetime::Drivers::St7789 lcd {lcdSpi, pinLcdDataCommand};

Pinetime::Drivers::Spi flashSpi {spi, pinSpiFlashCsn};
Pinetime::Drivers::SpiNorFlash spiNorFlash {flashSpi};

// The TWI device should work @ up to 400Khz but there is a HW bug which prevent it from
// respecting correct timings. According to erratas heet, this magic value makes it run
// at ~390Khz with correct timings.
static constexpr uint32_t MaxTwiFrequencyWithoutHardwareBug {0x06200000};
Pinetime::Drivers::TwiMaster twiMaster {Pinetime::Drivers::TwiMaster::Modules::TWIM1,
                                        Pinetime::Drivers::TwiMaster::Parameters {MaxTwiFrequencyWithoutHardwareBug, pinTwiSda, pinTwiScl}};
Pinetime::Drivers::Cst816S touchPanel {twiMaster, touchPanelTwiAddress};
#ifdef PINETIME_IS_RECOVERY
static constexpr bool isFactory = true;
  #include "displayapp/DummyLittleVgl.h"
  #include "displayapp/DisplayAppRecovery.h"
Pinetime::Components::LittleVgl lvgl {lcd, touchPanel};
#else
static constexpr bool isFactory = false;
  #include "displayapp/LittleVgl.h"
  #include "displayapp/DisplayApp.h"
Pinetime::Components::LittleVgl lvgl {lcd, touchPanel};
#endif

Pinetime::Drivers::Bma421 motionSensor {twiMaster, motionSensorTwiAddress};
Pinetime::Drivers::Hrs3300 heartRateSensor {twiMaster, heartRateSensorTwiAddress};

TimerHandle_t debounceTimer;
TimerHandle_t debounceChargeTimer;
Pinetime::Controllers::Battery batteryController;
Pinetime::Controllers::Ble bleController;
void ble_manager_set_ble_connection_callback(void (*connection)());
void ble_manager_set_ble_disconnection_callback(void (*disconnection)());
static constexpr uint8_t pinTouchIrq = 28;
static constexpr uint8_t pinPowerPresentIrq = 19;
std::unique_ptr<Pinetime::System::SystemTask> systemTask;

Pinetime::Controllers::Settings settingsController {spiNorFlash};

Pinetime::Controllers::MotorController motorController {settingsController};

void nrfx_gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  if (pin == pinTouchIrq) {
    systemTask->OnTouchEvent();
    return;
  }

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (pin == pinPowerPresentIrq and action == NRF_GPIOTE_POLARITY_TOGGLE) {
    xTimerStartFromISR(debounceChargeTimer, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return;
  }

  xTimerStartFromISR(debounceTimer, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

extern "C" {
void vApplicationIdleHook(void) {
  if (!isFactory)
    lv_tick_inc(1);
}
}

void DebounceTimerChargeCallback(TimerHandle_t xTimer) {
  xTimerStop(xTimer, 0);
  systemTask->PushMessage(Pinetime::System::SystemTask::Messages::OnChargingEvent);
}

void DebounceTimerCallback(TimerHandle_t xTimer) {
  xTimerStop(xTimer, 0);
  systemTask->OnButtonPushed();
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

static void (*radio_isr_addr)(void);
static void (*rng_isr_addr)(void);
static void (*rtc0_isr_addr)(void);

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

uint32_t npl_freertos_hw_enter_critical(void) {
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

struct ble_npl_eventq* nimble_port_get_dflt_eventq(void) {
  return &g_eventq_dflt;
}

void nimble_port_run(void) {
  struct ble_npl_event* ev;

  while (1) {
    ev = ble_npl_eventq_get(&g_eventq_dflt, BLE_NPL_TIME_FOREVER);
    ble_npl_event_run(ev);
  }
}

void BleHost(void*) {
  nimble_port_run();
}

void nimble_port_init(void) {
  void os_msys_init(void);
  void ble_store_ram_init(void);
  ble_npl_eventq_init(&g_eventq_dflt);
  os_msys_init();
  ble_hs_init();
  ble_store_ram_init();

  int res;
  res = hal_timer_init(5, NULL);
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

int main(void) {
  logger.Init();

  nrf_drv_clock_init();

  debounceTimer = xTimerCreate("debounceTimer", 200, pdFALSE, (void*) 0, DebounceTimerCallback);
  debounceChargeTimer = xTimerCreate("debounceTimerCharge", 200, pdFALSE, (void*) 0, DebounceTimerChargeCallback);

  systemTask = std::make_unique<Pinetime::System::SystemTask>(spi,
                                                              lcd,
                                                              spiNorFlash,
                                                              twiMaster,
                                                              touchPanel,
                                                              lvgl,
                                                              batteryController,
                                                              bleController,
                                                              motorController,
                                                              heartRateSensor,
                                                              motionSensor,
                                                              settingsController);
  systemTask->Start();
  nimble_port_init();

  vTaskStartScheduler();

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }
}
