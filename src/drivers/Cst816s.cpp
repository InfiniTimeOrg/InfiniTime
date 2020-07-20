#include <FreeRTOS.h>
#include <task.h>
#include <nrfx_log.h>
#include <legacy/nrf_drv_gpiote.h>

#include "Cst816s.h"
using namespace Pinetime::Drivers;

/* References :
 * This implementation is based on this article : https://medium.com/@ly.lee/building-a-rust-driver-for-pinetimes-touch-controller-cbc1a5d5d3e9
 * Touch panel datasheet (weird chinese translation) : https://wiki.pine64.org/images/5/51/CST816S%E6%95%B0%E6%8D%AE%E6%89%8B%E5%86%8CV1.1.en.pdf
 *
 * TODO : we need a complete datasheet and protocol reference!
 * */

Cst816S::Cst816S(TwiMaster &twiMaster, uint8_t twiAddress) : twiMaster{twiMaster}, twiAddress{twiAddress} {

}

void Cst816S::Init() {
  nrf_gpio_cfg_output(pinReset);
  nrf_gpio_pin_set(pinReset);
  vTaskDelay(50);
  nrf_gpio_pin_clear(pinReset);
  vTaskDelay(5);
  nrf_gpio_pin_set(pinReset);
  vTaskDelay(50);

  // Wake the touchpanel up
  uint8_t dummy;
  twiMaster.Read(twiAddress, 0x15, &dummy, 1);
  vTaskDelay(5);
  twiMaster.Read(twiAddress, 0xa7, &dummy, 1);

}


Cst816S::TouchInfos Cst816S::GetTouchInfo() {
  Cst816S::TouchInfos info;

  twiMaster.Read(twiAddress, 0, touchData, 63);
  auto nbTouchPoints = touchData[2] & 0x0f;

//  uint8_t i = 0;
//  NRF_LOG_INFO("#########################")
  for(int i = 0; i < 1; i++) {
    uint8_t pointId = (touchData[touchIdIndex + (touchStep * i)]) >> 4;
    if(nbTouchPoints == 0 && pointId == lastTouchId) return info;

    // We fetch only the first touch point (the controller seems to handle only one anyway...)
    info.isTouch = true;


    auto xHigh = touchData[touchXHighIndex + (touchStep * i)] & 0x0f;
    auto xLow = touchData[touchXLowIndex + (touchStep * i)];
    uint16_t x = (xHigh << 8) | xLow;

    auto yHigh = touchData[touchYHighIndex + (touchStep * i)] & 0x0f;
    auto yLow = touchData[touchYLowIndex + (touchStep * i)];
    uint16_t y = (yHigh << 8) | yLow;

    auto action = touchData[touchEventIndex + (touchStep * i)] >> 6; /* 0 = Down, 1 = Up, 2 = contact*/
    auto finger = touchData[touchIdIndex + (touchStep * i)] >> 4;
    auto pressure = touchData[touchXYIndex + (touchStep * i)];
    auto area = touchData[touchMiscIndex + (touchStep * i)] >> 4;

    info.x = x;
    info.y = y;
    info.action = action;
    info.gesture = static_cast<Gestures>(touchData[gestureIndex]);

//    NRF_LOG_INFO("---------------")
//    NRF_LOG_INFO("ID : %d", pointId);
//    NRF_LOG_INFO("NB : %d", nbTouchPoints);
//    NRF_LOG_INFO("X/Y :%d / %d", info.x, info.y);
//    NRF_LOG_INFO("Action : %d", action);
//    NRF_LOG_INFO("Finger : %d", finger);
//    NRF_LOG_INFO("Pressure : %d", pressure);
//    NRF_LOG_INFO("area : %d", area);
//    NRF_LOG_INFO("Touch : %s", info.isTouch?"Yes" : "No");
//    switch(info.gesture) {// gesture
//      case Gestures::None: NRF_LOG_INFO("Gesture : None"); break;
//      case Gestures::SlideDown: NRF_LOG_INFO("Gesture : Slide Down"); break;
//      case Gestures::SlideUp: NRF_LOG_INFO("Gesture : Slide Up"); break;
//      case Gestures::SlideLeft: NRF_LOG_INFO("Gesture : Slide Left"); break;
//      case Gestures::SlideRight: NRF_LOG_INFO("Gesture : Slide Right"); break;
//      case Gestures::SingleTap: NRF_LOG_INFO("Gesture : Single click"); break;
//      case Gestures::DoubleTap: NRF_LOG_INFO("Gesture : Double click"); break;
//      case Gestures::LongPress: NRF_LOG_INFO("Gesture : Long press"); break;
//      default : NRF_LOG_INFO("Unknown"); break;
//    }

  }


  return info;
}

void Cst816S::Sleep() {
  // TODO re enable sleep mode
  //twiMaster.Sleep();
  nrf_gpio_cfg_default(6);
  nrf_gpio_cfg_default(7);
}

void Cst816S::Wakeup() {
  Init();
}