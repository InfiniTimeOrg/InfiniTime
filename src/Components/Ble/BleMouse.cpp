
#include "BleMouse.h"
#include "HidService.h"

using namespace Pinetime::Controllers;

BleMouse::BleMouse(HidService& hidService) : hidService{hidService} {

}

void BleMouse::Move(uint8_t x, uint8_t y) {
  hidService.SendMoveReport(x, y);
}
