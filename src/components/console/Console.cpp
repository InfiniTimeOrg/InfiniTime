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
                 Pinetime::Controllers::MotionController& motionController)
  : systemTask {systemTask},
    nimbleController {nimbleController},
    fs {fs},
    lvgl {lvgl},
    motorController {motorController},
    touchPanel {touchPanel},
    spiNorFlash {spiNorFlash},
    twiMaster {twiMaster},
    motionController {motionController} {
}

void Console::Init() {
  auto rxCallback = [this](char* str, int length) {
    this->Received(str, length);
  };

  nimbleController.bleNus().RegisterRxCallback(rxCallback);
}

void Console::Print(const std::string str) {
  nimbleController.bleNus().Print(str);
}

static bool cmdCmp(char* buffer, const std::string search) {
  return strncmp(buffer, search.c_str(), search.length()) == 0;
}

void Console::Process() {
  static constexpr int maxArgumentsCount = 4;
  static constexpr int maxBufferLength = 64;

  char arg_buffer[maxBufferLength];
  const char* args[maxArgumentsCount];

  // Copy string, becase we replace ' ' with '\0' for proper string termination
  strncpy(arg_buffer, rxBuffer, sizeof(arg_buffer));

  // First argument is always command name itself
  uint16_t argc = 1;
  args[0] = arg_buffer;

  uint16_t param_len = strlen(rxBuffer);

  for (uint8_t i = 0; i < param_len; i++) {
    if (rxBuffer[i] == ' ' && param_len > (i + 1)) {
      arg_buffer[i] = '\0';
      args[argc++] = &arg_buffer[i + 1];
    }

    if (argc == maxArgumentsCount) {
      // Max argument count reached
      break;
    }
  }

  (void) args;
  (void) argc;

  // Simple stupid command comparison, later would be nice to add commands lookup table with argument parsing
  if (cmdCmp(rxBuffer, "VIBRATE")) {
    motorController.RunForDuration(100);
  } else if (cmdCmp(rxBuffer, "WKUP")) {
    systemTask.PushMessage(Pinetime::System::Messages::GoToRunning);
  } else if (cmdCmp(rxBuffer, "SLEEP")) {
    systemTask.PushMessage(Pinetime::System::Messages::GoToSleep);
  }
}

void Console::Received(char* str, int length) {
  for (int i = 0; i < length; i++) {
    // Wrap if input is too long without CR/LN
    if (rxPos == bufferSize - 1) {
      rxPos = 0;
    }

    rxBuffer[rxPos++] = str[i];
    rxBuffer[rxPos] = '\0'; // terminate for debug print

    if (str[i] == '\n' || str[i] == '\r') {
      rxPos = 0;
      systemTask.PushMessage(System::Messages::ConsoleProcess);
      break;
    }
  }
}