#include <cstdio>
#include <libs/date/includes/date/date.h>
#include <Components/DateTime/DateTimeController.h>
#include <Version.h>
#include "Message.h"

using namespace Pinetime::Applications::Screens;

void Message::Refresh(bool fullRefresh) {
  if(fullRefresh) {
    gfx.FillRectangle(0,0,240,240,0xffff);
    gfx.DrawString(120, 10, 0x5555, "COUCOU", &smallFont, false);
  }
}
