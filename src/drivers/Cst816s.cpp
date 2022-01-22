#include "drivers/Cst816s.h"
#include <FreeRTOS.h>
#include <legacy/nrf_drv_gpiote.h>
#include <nrfx_log.h>
#include <task.h>
#include "drivers/PinMap.h"

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

bool Cst816S::Init() {
  nrf_gpio_cfg_output(PinMap::Cst816sReset);
  nrf_gpio_pin_clear(PinMap::Cst816sReset);
  vTaskDelay(5);
  nrf_gpio_pin_set(PinMap::Cst816sReset);
  vTaskDelay(50);

  // Wake the touchpanel up
  uint8_t dummy;
  twiMaster.Read(twiAddress, 0x15, &dummy, 1);
  vTaskDelay(5);
  twiMaster.Read(twiAddress, 0xa7, &dummy, 1);
  vTaskDelay(5);

  // TODO This function check that the device IDs from the controller are equal to the ones
  // we expect. However, it seems to return false positive (probably in case of communication issue).
  // Also, it seems that some users have pinetimes that works correctly but that report different device IDs
  // Until we know more about this, we'll just read the IDs but not take any action in case they are not 'valid'
  CheckDeviceIds();

  /*
  [2] EnConLR - Continuous operation can slide around
  [1] EnConUD - Slide up and down to enable continuous operation
  [0] EnDClick - Enable Double-click action
  */
  static constexpr uint8_t motionMask = 0b00000101;
  twiMaster.Write(twiAddress, 0xEC, &motionMask, 1);

  /*
  [7] EnTest - Interrupt pin to test, enable automatic periodic issued after a low pulse.
  [6] EnTouch - When a touch is detected, a periodic pulsed Low.
  [5] EnChange - Upon detecting a touch state changes, pulsed Low.
  [4] EnMotion - When the detected gesture is pulsed Low.
  [0] OnceWLP - Press gesture only issue a pulse signal is low.
  */
  static constexpr uint8_t irqCtl = 0b01110000;
  twiMaster.Write(twiAddress, 0xFA, &irqCtl, 1);

  return true;
}

Cst816S::TouchInfos Cst816S::GetTouchInfo() {
  Cst816S::TouchInfos info;
  uint8_t touchData[7];

  auto ret = twiMaster.Read(twiAddress, 0, touchData, sizeof(touchData));
  if (ret != TwiMaster::ErrorCodes::NoError) {
    info.isValid = false;
    return info;
  }

  // This can only be 0 or 1
  uint8_t nbTouchPoints = touchData[touchPointNumIndex] & 0x0f;
  uint8_t xHigh = touchData[touchXHighIndex] & 0x0f;
  uint8_t xLow = touchData[touchXLowIndex];
  uint16_t x = (xHigh << 8) | xLow;
  uint8_t yHigh = touchData[touchYHighIndex] & 0x0f;
  uint8_t yLow = touchData[touchYLowIndex];
  uint16_t y = (yHigh << 8) | yLow;
  Gestures gesture = static_cast<Gestures>(touchData[gestureIndex]);

  // Validity check
  if (x >= maxX || y >= maxY ||
      (gesture != Gestures::None && gesture != Gestures::SlideDown && gesture != Gestures::SlideUp && gesture != Gestures::SlideLeft &&
       gesture != Gestures::SlideRight && gesture != Gestures::SingleTap && gesture != Gestures::DoubleTap &&
       gesture != Gestures::LongPress)) {
    info.isValid = false;
    return info;
  }

  info.x = x;
  info.y = y;
  info.touching = (nbTouchPoints > 0);
  info.gesture = gesture;
  info.isValid = true;
  return info;
}

void Cst816S::Sleep() {
  nrf_gpio_pin_clear(PinMap::Cst816sReset);
  vTaskDelay(5);
  nrf_gpio_pin_set(PinMap::Cst816sReset);
  vTaskDelay(50);
  static constexpr uint8_t sleepValue = 0x03;
  twiMaster.Write(twiAddress, 0xA5, &sleepValue, 1);
  NRF_LOG_INFO("[TOUCHPANEL] Sleep");
}

void Cst816S::Wakeup() {
  Init();
  NRF_LOG_INFO("[TOUCHPANEL] Wakeup");
}

bool Cst816S::CheckDeviceIds() {
  // There's mixed information about which register contains which information
  if (twiMaster.Read(twiAddress, 0xA7, &chipId, 1) == TwiMaster::ErrorCodes::TransactionFailed) {
    chipId = 0xFF;
    return false;
  }
  if (twiMaster.Read(twiAddress, 0xA8, &vendorId, 1) == TwiMaster::ErrorCodes::TransactionFailed) {
    vendorId = 0xFF;
    return false;
  }
  if (twiMaster.Read(twiAddress, 0xA9, &fwVersion, 1) == TwiMaster::ErrorCodes::TransactionFailed) {
    fwVersion = 0xFF;
    return false;
  }

  return chipId == 0xb4 && vendorId == 0 && fwVersion == 1;
}
