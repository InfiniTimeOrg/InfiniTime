
#include "Console.h"
#include "systemtask/SystemTask.h"
#include "components/ble/NimbleController.h"
#include "components/ble/BleNus.h"


using namespace Pinetime::Components;

Console::Console(Pinetime::System::SystemTask& systemTask, Pinetime::Controllers::NimbleController& nimbleController): systemTask {systemTask}, nimbleController{nimbleController}
{

}


void Console::Print(char *str)
{
    nimbleController.Print(str);
}

void Console::Received(char *str)
{
    nimbleController.Print(str);
}