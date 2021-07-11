#include "systemtask/SystemTask.h"
#include <FreeRTOS.h>
#include <task.h>

namespace Pinetime {
  namespace Controllers {
    class ButtonHandler {
      public:
        TaskHandle_t Start();
        static void Process(void* instance);
        void Work();
        void Register(Pinetime::System::SystemTask* systemTask);
      private:
        TaskHandle_t taskHandle;
        Pinetime::System::SystemTask* systemTask = nullptr;
    };
  }
}
