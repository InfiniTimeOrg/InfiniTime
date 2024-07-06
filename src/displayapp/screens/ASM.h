#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ASM : public Screen {
      public:
        ASM();
        ~ASM();

        void Refresh() override;

      private:
        static constexpr int num_slots = 16;
        static constexpr int stack_size = 32;

        enum OpcodeShort : uint8_t {
          Branch,
          Push0,
          PushU8,
          PushU16,
          PushU24,
          PushU32,
          SelectSlot0,
          SelectSlotMax = SelectSlot0 + ASM::num_slots - 1,
          SetLabelText,
          SetObjectAlign,
          CreateLabel,
          SetStyleLocalInt,
          SetStyleLocalColor,
          SetStyleLocalOpa,
          SetStyleLocalFont,
          WaitRefresh,
        };

        enum OpcodeLong : uint16_t {};

        uint8_t read_byte(size_t pos);
        uint16_t read_u16(size_t pos);
        uint32_t read_u24(size_t pos);
        uint32_t read_u32(size_t pos);

        uint8_t* code;
        size_t code_len;
        size_t ptr = 0;

        lv_obj_t* slots[num_slots] = {0};

        uint32_t stack[stack_size] = {0};
        uint8_t stack_pointer = 0;

        uint8_t current_slot = 0;

        void run();
      };
    }

    template <>
    struct AppTraits<Apps::ASM> {
      static constexpr Apps app = Apps::ASM;
      static constexpr const char* icon = Screens::Symbols::eye;

      static Screens::Screen* Create(AppControllers&) {
        return new Screens::ASM();
      };
    };
  };
}