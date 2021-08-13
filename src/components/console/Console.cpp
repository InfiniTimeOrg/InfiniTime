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
                Pinetime::Drivers::TwiMaster& twiMaster,
                Pinetime::Controllers::MotionController& motionController):
                systemTask {systemTask},
                nimbleController{nimbleController},
                fs{fs},
                lvgl{lvgl},
                motorController{motorController},
                touchPanel{touchPanel},
                spiNorFlash{spiNorFlash},
                twiMaster{twiMaster},
                motionController{motionController}
{
}

void Console::Init()
{
    auto rxCallback = [this](char *str, int length) {
        this->Received(str, length);
    };

    nimbleController.bleNus().RegisterRxCallback(rxCallback);
}

void Console::Print(const std::string str)
{
    nimbleController.bleNus().Print(str);
}

static bool cmdCmp(char *buffer, const std::string search)
{
    return strncmp(buffer, search.c_str(), search.length()) == 0;
}

static lv_color_t parseHexColorString(const char *str)
{
    char tmp[3];

    // Skip index 0 with '#'
    tmp[0] = str[1];
    tmp[1] = str[2];
    tmp[2] = '\0';
    int red = strtol(tmp, NULL, 16);

    tmp[0] = str[3];
    tmp[1] = str[4];
    tmp[2] = '\0';
    int green = strtol(tmp, NULL, 16);

    tmp[0] = str[5];
    tmp[1] = str[6];
    tmp[2] = '\0';
    int blue = strtol(tmp, NULL, 16);

    return lv_color_make(red, green, blue);
}

// Example showing how you can use console to change LVGL properties temporarily for development purpose
// This might be improved for position and width of elements
static void setObjectsColor(const char *hexColor)
{
    lv_obj_t* actStrc = lv_scr_act();

    lv_obj_t * child;
    child = lv_obj_get_child(actStrc, NULL);
    while(child) {
        lv_obj_set_style_local_text_color(child, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, parseHexColorString(hexColor));
        child = lv_obj_get_child(actStrc, child);
    }
}

void Console::AccelerometerDebug()
{
    char accBuf[32];
    snprintf(accBuf, sizeof(accBuf), "%d, %d, %d\n", motionController.X(), motionController.Y(), motionController.Z());
    Print(accBuf);
}

void Console::CommandLvgl(const char *args[], uint16_t argc)
{
    char lvbuf[128];
    snprintf(lvbuf, sizeof(lvbuf), "argc: %d, cmd: %s, 1:%s, 2:%s, 3:%s", argc, args[0], args[1], args[2], args[3]);
    Print(lvbuf);

    // TODO: list of objects, changing position, size & color would be great
    
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    snprintf(lvbuf, sizeof(lvbuf), "used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n", (int)(mon.total_size - mon.free_size),
    mon.used_pct,
    mon.frag_pct,
    (int)mon.free_biggest_size);
    Print(lvbuf);

    // List active screen objects
    lv_obj_t* actStrc = lv_scr_act();
    uint16_t childCount = lv_obj_count_children(actStrc);
    snprintf(lvbuf, sizeof(lvbuf), "children: %d\n", childCount);
    Print(lvbuf);

    lv_obj_t * child;
    uint16_t i = 0;
    child = lv_obj_get_child(actStrc, NULL);
    while(child) {
        snprintf(lvbuf, sizeof(lvbuf), "#%d, x: %d, y: %d, w: %d, h: %d\n", i++, lv_obj_get_x(child), lv_obj_get_y(child), lv_obj_get_width(child), lv_obj_get_height(child));
        Print(lvbuf);
        vTaskDelay(50); // Add small delay for each item, so the print buffer has time to be send over BLE
        
        child = lv_obj_get_child(actStrc, child);
    }
}

void Console::Process()
{
    static uint32_t accCount = 0;

    // Simple stupid comparison, later would be nice to add commands lookup table with argument parsing
    if(hasCommandFlag)
    {
        hasCommandFlag = false;

        static constexpr int maxArgumentsCount = 20;
        static constexpr int maxBufferLength = 128;

        char arg_buffer[maxBufferLength];
        const char *args[maxArgumentsCount];
        
        // Copy string, becase we replace ' ' with '\0' for proper string termination
        strncpy(arg_buffer, rxBuffer, sizeof(arg_buffer));

        // First argument is always command name itself
        uint16_t argc = 1;
        args[0] = arg_buffer;

        uint16_t param_len = strlen(rxBuffer);

        for (uint8_t i = 0; i < param_len; i++)
        {
            if (rxBuffer[i] == ' ' && param_len > (i + 1))
            {
                arg_buffer[i] = '\0';
                args[argc++] = &arg_buffer[i+1];
            }

            if (argc == maxArgumentsCount)
            {
                // Max argument count reached
                break;
            }
        }

        // This AT > OK needs to be there, because https://terminal.hardwario.com/ waits for the answer
        // When we use or create better webpage terminal, this can go out
        if(cmdCmp(rxBuffer, "AT"))
        {
            Print((char*)"OK\r\n");
        }
        else if(cmdCmp(rxBuffer, "COLOR"))
        {
            if(argc == 2)
            {
                setObjectsColor(args[1]);
            }
            else
            {
                Print("Expects 1 parameter");
            }
        }
        else if(cmdCmp(rxBuffer, "LVGL"))
        {
            CommandLvgl(args, argc);
        }
        else if(cmdCmp(rxBuffer, "VIBRATE"))
        {
            motorController.SetDuration(100);
        }
        else if(cmdCmp(rxBuffer, "FS"))
        {
            // TODO: add directory listings etc.
        }
        else if(cmdCmp(rxBuffer, "WKUP"))
        {
            systemTask.PushMessage(Pinetime::System::Messages::GoToRunning);
        }
        else if(cmdCmp(rxBuffer, "SLEEP"))
        {
            systemTask.PushMessage(Pinetime::System::Messages::GoToSleep);
        }
        else if(cmdCmp(rxBuffer, "SPINOR"))
        {
            // TODO: print RAW data from FLASH
        }
        else if(cmdCmp(rxBuffer, "ACC"))
        {
            // Print 50 accelerometer measurements
            accCount = 50;
        }
    }

    // Debug print accelerometer values
    if(accCount)
    {
        accCount--;
        AccelerometerDebug();
    }
}

void Console::Received(char* str, int length)
{
    for(int i = 0; i < length; i++)
    {
        // Wrap if input is too long without CR/LN
        if(rxPos == bufferSize - 1)
        {
            rxPos = 0;
        }

        rxBuffer[rxPos++] = str[i];
        rxBuffer[rxPos] = '\0'; // terminate for debug print 

        if(str[i] == 13 || str[i] == 10)
        {
            rxPos = 0;
            hasCommandFlag = true;
            break;
        }
    }
}