
#include "Console.h"
#include "systemtask/SystemTask.h"
#include "components/ble/NimbleController.h"
#include "components/ble/BleNus.h"


using namespace Pinetime::Components;

Console::Console(Pinetime::System::SystemTask& systemTask, Pinetime::Controllers::NimbleController& nimbleController): systemTask {systemTask}, nimbleController{nimbleController}
{

}

void Console::Init()
{
    auto rxCallback = [this](char *str, int length) {
        this->Received(str, length);
    };

    nimbleController.bleNus().ConsoleRegister(rxCallback);
}

void Console::Print(char *str)
{
    //nimbleController.Print(str);
    nimbleController.bleNus().Print(str);
}

void Console::Received(char* str, int length)
{

    char b[32];

    for(int i = 0; i < length, i++)
    {
        rxBuffer[rxPos++] = str[i];

        if(str[i] == 13 || str[i] == 10)
        {
            break;
        }
    }

    sprintf(b, "rx: %s, len: %d", str, length);

    nimbleController.bleNus().Print(b);
}