#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "utility/DirtyValue.h"
#include "displayapp/LittleVgl.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Maze {
      public:
        Maze();
        // I guess I don't need an explicit destructor since everything goes out of scope nicely?
        uint8_t get(int, int);
        uint8_t get(int);
        void set(int, int, uint8_t);
        void set(int, uint8_t);

        // 10x10 tiles on the maze
        static constexpr int WIDTH = 24;
        static constexpr int HEIGHT = 24;
        static constexpr int TILESIZE = 10;
        static constexpr int FLATSIZE = WIDTH*HEIGHT/2;
      private:
        uint8_t mazemap[FLATSIZE];
      };



      class WatchFaceMaze : public Screen {
      public:
        WatchFaceMaze(Pinetime::Components::LittleVgl&, Controllers::DateTime&);
        ~WatchFaceMaze() override;
		void Refresh() override;

      private:
        lv_task_t* taskRefresh;
        void GenerateMaze();
        void DrawMaze();

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        Controllers::DateTime& dateTimeController;
        Pinetime::Components::LittleVgl& lvgl;
        lv_color_t buf1[480];
        lv_color_t buf2[480];
        Maze maze;
      };
    }



    template <>
    struct WatchFaceTraits<WatchFace::Maze> {
      static constexpr WatchFace watchFace = WatchFace::Maze;
      static constexpr const char* name = "Maze";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceMaze(controllers.lvgl,
                                          controllers.dateTimeController/*,
                                              controllers.batteryController,
                                              controllers.bleController,
                                              controllers.settingsController*/);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}