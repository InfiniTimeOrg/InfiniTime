#pragma once

#include <array>
#include <cstddef>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/screens/Symbols.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Controllers {
    class MotorController;
    class DateTime;
  }

  namespace Applications {
    namespace Screens {
      // Gym -> Style -> Type -> Grade -> Attempt, then logged immediately
      // (no separate confirm step). A single Screen instance rebuilds its
      // button matrix per step rather than pushing separate Screen
      // objects, matching the pattern FirmwareUpdate.h uses for its own
      // multi-step flow with an internal `enum class States`.
      //
      // Every step remembers the previous log's choice: the matching
      // option is pre-checked when a step's screen opens, so repeating the
      // same climb type is a single confirming tap per step. Remembered
      // values are written to a small /climbs/last_selection.csv after
      // each successful log and read back in the constructor, so they
      // survive this Screen object being destroyed and recreated --
      // whether from exiting the app, screen timeout, or anything else --
      // rather than relying on in-memory state alone. See
      // LoadRememberedSelections / SaveRememberedSelections.
      class ClimbLogger : public Screen {
      public:
        ClimbLogger(Controllers::MotorController& motorController,
                    Controllers::DateTime& dateTimeController,
                    Controllers::FS& filesystem);
        ~ClimbLogger() override;

        void OnButtonMatrixEvent(lv_obj_t* obj, lv_event_t event);

        // Max distinct options offered at any one step. Public so the free
        // helper functions in ClimbLogger.cpp (which build those option
        // lists ahead of a ClimbLogger method call) can size their buffers
        // against it.
        static constexpr size_t kMaxOptions = 8;

      private:
        enum class Step { Gym, Style, Type, Grade, Attempt };

        Controllers::MotorController& motorController;
        Controllers::DateTime& dateTimeController;
        Controllers::FS& filesystem;

        Step step = Step::Gym;

        // Current step's picks, and — since never cleared on log — also
        // next time's remembered defaults.
        const char* selectedGym = nullptr;
        const char* selectedStyle = nullptr;
        const char* selectedType = nullptr;
        const char* selectedGrade = nullptr;
        const char* selectedAttempt = nullptr;

        // Backing storage for the lv_btnmatrix map of the current step
        // ("\n"-separated rows, "" terminator) — sized for kMaxOptions
        // buttons plus one separator per row plus the terminator.
        std::array<const char*, kMaxOptions * 2 + 1> optionsMap {};

        lv_obj_t* titleLabel = nullptr;
        lv_obj_t* buttonMatrix = nullptr;

        void ShowStep();
        void ShowOptionsStep(const char* title, const char* const* options, size_t count, const char* rememberedValue);

        void OnOptionSelected(const char* text);
        void LogAndReset();
        void WriteLogEntry();
        void LoadRememberedSelections();
        void SaveRememberedSelections();

        bool IsBoulderStyle() const;
      };
    }

    template <>
    struct AppTraits<Apps::ClimbLogger> {
      static constexpr Apps app = Apps::ClimbLogger;
      static constexpr const char* icon = Screens::Symbols::mountain;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::ClimbLogger(controllers.motorController, controllers.dateTimeController, controllers.filesystem);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
