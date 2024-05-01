#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include <lvgl/src/lv_hal/lv_hal_disp.h>
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class WeatherColorTester : public Screen {
      public:
        explicit WeatherColorTester(Pinetime::Components::LittleVgl& lvgl);
        WeatherColorTester() = delete;
        ~WeatherColorTester() override;
        void Refresh() override;
      private:
        Pinetime::Components::LittleVgl& lvgl;
        lv_task_t* taskRefresh;
        static constexpr int x = 0;
        static constexpr int y = 240;
        static constexpr int bufferSize = (480);
        lv_color_t buffer[bufferSize];

        lv_color_t currentColor;
        //static lv_disp_draw_buf_t disp_buf;
        };
    }
    
    
    template <>
    struct AppTraits<Apps::WeatherColorTester> {
      static constexpr Apps app = Apps::WeatherColorTester;
      static constexpr const char* icon = Screens::Symbols::bolt;
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WeatherColorTester(controllers.lvgl);
      }
    };
  }
}
