#include "systemtask/SystemTask.h"
#include <FreeRTOS.h>
#include <task.h>

namespace Pinetime {
  namespace Controllers {
    class ButtonHandler {
      public:
        TaskHandle_t Start();
        static void Process(void* instance);
        void Register(Pinetime::System::SystemTask* systemTask);
        void WakeUp();
        void Work();
      private:
        Pinetime::System::SystemTask* systemTask = nullptr;
        TaskHandle_t taskHandle;
    };
  }
}
