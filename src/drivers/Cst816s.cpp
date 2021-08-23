#include "Cst816s.h"
#include <FreeRTOS.h>
#include <legacy/nrf_drv_gpiote.h>
#include <nrfx_log.h>
#include <task.h>

using namespace Pinetime::Drivers;

/* References :
 * This implementation is based on this article :
 * https://medium.com/@ly.lee/building-a-rust-driver-for-pinetimes-touch-controller-cbc1a5d5d3e9 Touch panel datasheet (weird chinese
 * translation) : https://wiki.pine64.org/images/5/51/CST816S%E6%95%B0%E6%8D%AE%E6%89%8B%E5%86%8CV1.1.en.pdf
 *
 * TODO : we need a complete datasheet and protocol reference!
 * */

Cst816S::Cst816S(TwiMaster& twiMaster, uint8_t twiAddress) : twiMaster {twiMaster}, twiAddress {twiAddress} {
}

void Cst816S::Init() {
  nrf_gpio_cfg_output(pinReset);
  nrf_gpio_pin_clear(pinReset);
  vTaskDelay(5);
  nrf_gpio_pin_set(pinReset);
  vTaskDelay(50);

  // Wake the touchpanel up
  uint8_t dummy;
  twiMaster.Read(twiAddress, 0x15, &dummy, 1);
  vTaskDelay(5);
  twiMaster.Read(twiAddress, 0xa7, &dummy, 1);
  vTaskDelay(5);

  /*
  [2] EnConLR - Continuous operation can slide around
  [1] EnConUD - Slide up and down to enable continuous operation
  [0] EnDClick - Enable Double-click action
  */
  static constexpr uint8_t motionMask = 0b00000101;
  twiMaster.Write(twiAddress, 0xEC, &motionMask, 1);
}

Cst816S::TouchInfos Cst816S::GetTouchInfo() {
  Cst816S::TouchInfos info;

  auto ret = twiMaster.Read(twiAddress, 0, touchData, sizeof(touchData));
  if (ret != TwiMaster::ErrorCodes::NoError)
    return {};

  auto nbTouchPoints = touchData[2] & 0x0f;

  uint8_t i = 0;

  uint8_t pointId = (touchData[touchIdIndex + (touchStep * i)]) >> 4;
  if (nbTouchPoints == 0 && pointId == lastTouchId)
    return info;

  info.isTouch = true;

  auto xHigh = touchData[touchXHighIndex + (touchStep * i)] & 0x0f;
  auto xLow = touchData[touchXLowIndex + (touchStep * i)];
  uint16_t x = (xHigh << 8) | xLow;

  auto yHigh = touchData[touchYHighIndex + (touchStep * i)] & 0x0f;
  auto yLow = touchData[touchYLowIndex + (touchStep * i)];
  uint16_t y = (yHigh << 8) | yLow;

  auto action = touchData[touchEventIndex + (touchStep * i)] >> 6; /* 0 = Down, 1 = Up, 2 = contact*/

  info.x = x;
  info.y = y;
  info.action = action;
  info.gesture = static_cast<Gestures>(touchData[gestureIndex]);

  return info;
}

void Cst816S::Sleep() {
  nrf_gpio_pin_clear(pinReset);
  vTaskDelay(5);
  nrf_gpio_pin_set(pinReset);
  vTaskDelay(50);
  static constexpr uint8_t sleepValue = 0x03;
  twiMaster.Write(twiAddress, 0xA5, &sleepValue, 1);
  NRF_LOG_INFO("[TOUCHPANEL] Sleep");
}

void Cst816S::Wakeup() {
  Init();
  NRF_LOG_INFO("[TOUCHPANEL] Wakeup");
}