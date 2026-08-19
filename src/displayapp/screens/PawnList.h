#pragma once

#include <vector>
#include <string>
#include "displayapp/apps/Apps.h"
#include "Screen.h"
#include "displayapp/Controllers.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class PawnList : public Screen {
      public:
        struct AppListing {
          PawnList* pawnList;
          std::string name;
          int index;
        };

        explicit PawnList(Pinetime::Controllers::FS& filesystem, Applications::DisplayApp* displayApp);
        ~PawnList() override;

        void ButtonClickedHandler(int n);
        bool OnTouchEvent(TouchEvents event) override;

      private:
        std::vector<AppListing> pawnApps {};
        unsigned int currentScreen = 0;
        Applications::DisplayApp* displayApp;

        void ShowScreen();
      };
    }

    template <>
    struct AppTraits<Apps::PawnList> {
      static constexpr Apps app = Apps::PawnList;
      static constexpr const char* icon = "L";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::PawnList(controllers.filesystem, controllers.displayApp);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}