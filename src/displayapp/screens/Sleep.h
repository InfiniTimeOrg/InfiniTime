#pragma once

#include <array>
#include <cstdint>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/sleep/SleepTracker.h"
#include "systemtask/SystemTask.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Sleep : public Screen {
      public:
        explicit Sleep(Controllers::SleepTracker& tracker);
        ~Sleep() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        static constexpr lv_coord_t graphX = 18;
        static constexpr lv_coord_t graphWidth = 218;
        static constexpr lv_coord_t graphTop = 126;
        static constexpr lv_coord_t rowHeight = 18;

        uint8_t PageCount() const;
        bool NightForPage(uint8_t page, Controllers::SleepTracker::Night& night) const;
        void ShowPage();
        void DrawGraph(const Controllers::SleepTracker::Night& night);

        Controllers::SleepTracker& tracker;
        Controllers::SleepTracker::Night live {};
        bool haveLive = false;
        uint8_t historyOffset = 0; // History entries hidden because the live night replaces them
        uint8_t page = 0;

        std::array<lv_point_t, Controllers::SleepTracker::MaxEpochs * 2> graphPoints {};

        lv_obj_t* labelPage;
        lv_obj_t* labelDate;
        lv_obj_t* labelLive;
        lv_obj_t* labelAsleep;
        lv_obj_t* labelAsleepCaption;
        lv_obj_t* labelTimes;
        lv_obj_t* graph;
        lv_obj_t* graphAxis;
        lv_obj_t* labelStats;
        lv_obj_t* labelEmpty;
      };
    }

    template <>
    struct AppTraits<Apps::Sleep> {
      static constexpr Apps app = Apps::Sleep;
      static constexpr const char* icon = Screens::Symbols::moon;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Sleep(controllers.systemTask->sleep());
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
