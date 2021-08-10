
#include "Console.h"
#include "systemtask/SystemTask.h"
#include "components/ble/NimbleController.h"
#include "components/ble/BleNus.h"


using namespace Pinetime::Components;

Console::Console(Pinetime::System::SystemTask& systemTask,
                Pinetime::Controllers::NimbleController& nimbleController,
                Pinetime::Controllers::FS& fs,
                Pinetime::Components::LittleVgl& lvgl,
                Pinetime::Controllers::MotorController& motorController,
                Pinetime::Drivers::Cst816S& touchPanel,
                Pinetime::Drivers::SpiNorFlash& spiNorFlash,
                Pinetime::Drivers::TwiMaster& twiMaster):
                systemTask {systemTask},
                nimbleController{nimbleController},
                fs{fs},
                lvgl{lvgl},
                motorController{motorController},
                touchPanel{touchPanel},
                spiNorFlash{spiNorFlash},
                twiMaster{twiMaster}
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
    bool hasCommand = false;
    //char b[128];

    for(int i = 0; i < length; i++)
    {
        rxBuffer[rxPos++] = str[i];
        rxBuffer[rxPos] = '\0'; // terminate for debug out

        if(str[i] == 13 || str[i] == 10)
        {
            rxPos = 0;
            hasCommand = true;
            break;
        }
    }

    //sprintf(b, "rx: %s, len: %d, buffer: %s\r\n", str, length, rxBuffer);
    //nimbleController.bleNus().Print(b);


    // Simple stupid comparison, later would be nice to add commands lookup table with argument parsing
    if(hasCommand)
    {
         //sprintf(b, "cmd: %s\r\n", rxBuffer);
         //nimbleController.bleNus().Print(b);

        // This AT > OK needs to be there, because https://terminal.hardwario.com/ waits for the answer
        // When we use or create better webpage terminal, this can go out
        if(strncmp(rxBuffer, "AT", 2) == 0)
        {
            nimbleController.bleNus().Print((char*)"OK\r\n");
        }
        else if(strncmp(rxBuffer, "LVGL", 4) == 0)
        {
            // TODO: list of objects, changing position, size & color would be great
            
            char lvbuf[128];
            lv_mem_monitor_t mon;
            lv_mem_monitor(&mon);
            snprintf(lvbuf, sizeof(lvbuf), "used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n", (int)(mon.total_size - mon.free_size),
            mon.used_pct,
            mon.frag_pct,
            (int)mon.free_biggest_size);
            nimbleController.bleNus().Print(lvbuf);
        }
        else if(strncmp(rxBuffer, "VIBRATE", 7) == 0)
        {
            motorController.SetDuration(100);
        }
        else if(strncmp(rxBuffer, "FS", 2) == 0)
        {
            // TODO: add directory listings etc.
            /*
            lfs_file_t settingsFile;

            if(fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_RDONLY) != LFS_ERR_OK)
            {
                return;
            }*/

        }
        else if(strncmp(rxBuffer, "WKUP", 4) == 0)
        {
            systemTask.PushMessage(Pinetime::System::Messages::GoToRunning);
        }
        else if(strncmp(rxBuffer, "SLEEP", 5) == 0)
        {
            systemTask.PushMessage(Pinetime::System::Messages::GoToSleep);
        }
        else if(strncmp(rxBuffer, "SPINOR", 6) == 0)
        {
            // Not working yet
            /*
            uint8_t flashBuffer[64];
            char lineBuffer[64];
            spiNorFlash.Read(0x0, flashBuffer, sizeof(flashBuffer));

            lineBuffer[0] = '\0';

            uint8_t *ptr = flashBuffer;

            for(uint32_t i = 0; i < sizeof(flashBuffer) / 8; i++)
            {
                snprintf(lineBuffer, sizeof(lineBuffer), "%02X %02X %02X %02X %02X %02X %02X %02X\r\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]);
                ptr += 8;

                nimbleController.bleNus().Print(lineBuffer);
                vTaskDelay(50); // not sure if this has to be, needs to be inspected how BLE communicates and if buffers are ok
            }
            */
        }
    }
}