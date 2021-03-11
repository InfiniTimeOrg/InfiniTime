#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include "Screen.h"
#include "components/brightness/BrightnessController.h"

namespace Pinetime {
  namespace Components {
    class LittleVgl;
  }
  namespace Applications {
    namespace Screens {
      class Torch : public Screen {
        public:
          Torch(Pinetime::Applications::DisplayApp *app, Controllers::BrightnessController& brightness, Pinetime::Components::LittleVgl& lvgl);
          ~Torch() override;
          bool Refresh() override;
          bool OnButtonPushed() override;

        private:
          Controllers::BrightnessController& brightness;
          Pinetime::Components::LittleVgl& lvgl;

          lv_obj_t* BgScreen;

          bool running = true;
      };
    }
  }
}
