#include "displayapp/screens/AppController.h"

using namespace Pinetime::Applications;

AppController::AppController() {
  symbols[current_apps] = Twos::Symbol;
  constructors[current_apps] = Twos::Get;
  current_apps++;
};
