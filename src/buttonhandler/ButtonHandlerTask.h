#include "systemtask/SystemTask.h"
#include <FreeRTOS.h>
#include <task.h>

namespace Pinetime {
  namespace Controllers {
    class ButtonHandler {
      public:
        void Register(Pinetime::System::SystemTask* systemTask);
        void Start();
        void WakeUp();
      private:
        static void Process(void* instance);
        void Work();
        Pinetime::System::SystemTask* systemTask = nullptr;
        TaskHandle_t taskHandle;
    };
  }
}
