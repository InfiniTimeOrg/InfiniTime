#include "displayapp/screens/AppController.h"
#include "displayapp/screens/Twos.h"


void AppController::AppController() {
  symbols[current_apps] = Twos::Symbol;
  constructors[current_apps] = Twos::Get;
  current_apps++;
};
