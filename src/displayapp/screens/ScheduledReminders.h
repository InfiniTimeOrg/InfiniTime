#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ScheduledReminders : public Screen {
      public:
        ScheduledReminders();
        ~ScheduledReminders() override;
      };
    }

    template <>
    struct AppTraits<Apps::ScheduledReminders> {
      static constexpr Apps app = Apps::ScheduledReminders;
      static constexpr const char* icon = Screens::Symbols::clock;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::ScheduledReminders();
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}