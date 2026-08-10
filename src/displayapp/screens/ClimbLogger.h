#pragma once

#include <array>
#include <cstddef>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/widgets/Counter.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Controllers {
    class MotorController;
    class DateTime;
  }

  namespace Applications {
    namespace Screens {
      // Defined in ClimbLogger.cpp, alongside the hard-coded catalog
      // fixture. Step 3 replaces the fixture with a real read of
      // catalog.csv from LittleFS; this shape is expected to survive
      // that move (it matches CLAUDE.md's catalog record data model).
      struct CatalogEntry;

      // Roadmap Step 2: filter (area -> colour -> grade [-> climb if the
      // filter doesn't land on exactly one]) -> result -> attempts, against
      // the hard-coded catalog fixture. A single Screen instance rebuilds
      // its widgets per step rather than pushing separate Screen objects,
      // matching the pattern FirmwareUpdate.h uses for its own multi-step
      // flow with an internal `enum class States`.
      class ClimbLogger : public Screen {
      public:
        ClimbLogger(Controllers::MotorController& motorController, Controllers::DateTime& dateTimeController);
        ~ClimbLogger() override;

        void OnButtonMatrixEvent(lv_obj_t* obj, lv_event_t event);
        void OnLogButtonEvent(lv_obj_t* obj, lv_event_t event);

        // Max distinct options offered at any one filter step. Public so
        // the free helper functions in ClimbLogger.cpp (which build those
        // option lists ahead of a ClimbLogger method call) can size their
        // buffers against it.
        static constexpr size_t kMaxOptions = 8;

      private:
        enum class Step { Area, Colour, Grade, Climb, Result, Attempts };
        enum class Result { Flash, Send, Fell, Project };

        Controllers::MotorController& motorController;
        Controllers::DateTime& dateTimeController;

        Step step = Step::Area;
        const char* selectedArea = nullptr;
        const char* selectedColour = nullptr;
        const char* selectedGrade = nullptr;
        const CatalogEntry* selectedClimb = nullptr;
        Result selectedResult = Result::Send;

        // Backing storage for the lv_btnmatrix map of the current step
        // ("\n"-separated rows, "" terminator) — sized for kMaxOptions
        // one-per-row buttons plus separators and the terminator.
        std::array<const char*, kMaxOptions * 2 + 1> optionsMap {};

        lv_obj_t* titleLabel = nullptr;
        lv_obj_t* buttonMatrix = nullptr;
        lv_obj_t* logButton = nullptr;
        Widgets::Counter attemptsCounter = Widgets::Counter(1, 20, jetbrains_mono_42);

        void ShowStep();
        void ShowOptionsStep(const char* title, const char* const* options, size_t count);
        void ShowResultStep();
        void ShowAttemptsStep();

        size_t CollectAreas(std::array<const char*, kMaxOptions>& out) const;
        size_t CollectColours(std::array<const char*, kMaxOptions>& out) const;
        size_t CollectGrades(std::array<const char*, kMaxOptions>& out) const;
        size_t CollectMatchingClimbs(std::array<const CatalogEntry*, kMaxOptions>& out) const;

        void OnOptionSelected(const char* text);
        void LogAndReset();

        static const char* ToResultString(Result result);
      };
    }

    template <>
    struct AppTraits<Apps::ClimbLogger> {
      static constexpr Apps app = Apps::ClimbLogger;
      static constexpr const char* icon = Screens::Symbols::mountain;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::ClimbLogger(controllers.motorController, controllers.dateTimeController);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
