#include "displayapp/screens/WatchFaceDigital.h"

#include "watchface_digital.h"
#include "WatchFaceDigital.h"

using namespace Pinetime::Applications::Screens;

WatchFaceDigital::WatchFaceDigital(AppControllers& controllers) : Pawn(controllers, std::make_unique<Pawn::HeatshrinkFile>(std::make_unique<Pawn::ConstFile>(watchface_digital, watchface_digital_len))) {
}
