#include "drivers/Cst816s.h"
#include "drivers/PinMap.h"
#include <FreeRTOS.h>
#include <legacy/nrf_drv_gpiote.h>
#include <nrfx_log.h>
#include <task.h>

using namespace Pinetime::Drivers;

/*
 * References :
 * This implementation is based on this article :
 * https://medium.com/@ly.lee/building-a-rust-driver-for-pinetimes-touch-controller-cbc1a5d5d3e9 Touch panel datasheet (weird chinese
 * translation) : https://wiki.pine64.org/images/5/51/CST816S%E6%95%B0%E6%8D%AE%E6%89%8B%E5%86%8CV1.1.en.pdf
 *
 * TODO: We need a complete datasheet and protocol reference!
 * For register desciptions, see Cst816s_registers.h. Information was collected from various chinese datasheets and documents.
 * */

Cst816S::Cst816S(TwiMaster& twiMaster, uint8_t twiAddress) : twiMaster {twiMaster}, twiAddress {twiAddress} {
}

bool Cst816S::Init() {
  // Reset the touch driver
  nrf_gpio_cfg_output(PinMap::Cst816sReset);
  nrf_gpio_pin_clear(PinMap::Cst816sReset);
  vTaskDelay(10);
  nrf_gpio_pin_set(PinMap::Cst816sReset);
  vTaskDelay(50);

  // Chip ID is suspected to be dependent on embedded firmware and factory configuration,
  // and not (just) on hardware type and revision.
  if (twiMaster.Read(twiAddress, CHIP_ID, &chipId, 1) == TwiMaster::ErrorCodes::TransactionFailed) {
    chipId = 0xFF;
  }
  // Vendor / project ID and firmware version can vary between devices.
  if (twiMaster.Read(twiAddress, PROJ_ID, &vendorId, 1) == TwiMaster::ErrorCodes::TransactionFailed) {
    vendorId = 0xFF;
  }
  if (twiMaster.Read(twiAddress, FW_VERSION, &fwVersion, 1) == TwiMaster::ErrorCodes::TransactionFailed) {
    fwVersion = 0xFF;
  }

  // These configuration settings will be ignored by chips which were
  // fused / pre-configured in the factory (GESTURE and REPORT settings).
  // This mainly applies to CST716, however there may be CST816S with static configurations as well.
  // The other, freely configureable ones (DYNAMIC), are configured in reporting mode here.

  // Configure motion behaviour
  static constexpr uint8_t motionMask = MOTION_MASK_EN_DCLICK | MOTION_MASK_EN_CON_UD | MOTION_MASK_EN_CON_LR;
  twiMaster.Write(twiAddress, MOTION_MASK, &motionMask, 1);

  // Configure interrupt generating events
  static constexpr uint8_t irqCtl = IRQ_CTL_EN_MOTION | IRQ_CTL_EN_CHANGE | IRQ_CTL_EN_TOUCH;
  twiMaster.Write(twiAddress, IRQ_CTL, &irqCtl, 1);

  return true;
}

Cst816S::TouchInfos Cst816S::GetTouchInfo() {
  // Some chips fail to wake up even though the reset pin has been toggled.
  // They only provide an I2C communication window after a touch interrupt,
  // so the first touch interrupt is used to force initialisation.
  if (firstEvent) {
    Init();
    firstEvent = false;
    // The data registers should now be reset, so this touch event will be detected as invalid.
  }

  // Read gesture metadata and first touch point coordinate block
  Cst816S::TouchInfos info;
  uint8_t touchData[P1_Y_POS_L + 1];
  auto ret = twiMaster.Read(twiAddress, 0x00, touchData, sizeof(touchData));
  if (ret != TwiMaster::ErrorCodes::NoError)
    return info;

  // Assemble 12 bit point coordinates from lower 8 bits and higher 4 bits
  info.x = ((touchData[P1_X_POS_H] & POS_H_POS_MASK) << 8) | touchData[P1_X_POS_L];
  info.y = ((touchData[P1_Y_POS_H] & POS_H_POS_MASK) << 8) | touchData[P1_Y_POS_L];
  // Evaluate number of touch points
  info.touching = (touchData[TD_STATUS] & TD_STATUS_MASK) > 0;
  // Decode gesture ID
  info.gesture = static_cast<Gestures>(touchData[GESTURE_ID]);

  // Validity check, verify value ranges
  info.isValid = (info.x < maxX && info.y < maxY &&
                  (info.gesture == Gestures::None || info.gesture == Gestures::SlideDown || info.gesture == Gestures::SlideUp ||
                   info.gesture == Gestures::SlideLeft || info.gesture == Gestures::SlideRight || info.gesture == Gestures::SingleTap ||
                   info.gesture == Gestures::DoubleTap || info.gesture == Gestures::LongPress));

  return info;
}

void Cst816S::Sleep() {
  // This only controls the CST716, the CST816S will ignore this register.
  // The CST816S power state is managed using auto-sleep.

  static constexpr uint8_t sleepValue = PWR_MODE_DEEP_SLEEP;
  twiMaster.Write(twiAddress, PWR_MODE_CST716, &sleepValue, 1);

  NRF_LOG_INFO("[TOUCHPANEL] Sleep");
}

void Cst816S::Wakeup() {
  Init();
  NRF_LOG_INFO("[TOUCHPANEL] Wakeup");
}
