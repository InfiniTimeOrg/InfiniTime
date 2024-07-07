#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"
#include <cassert>

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
        static constexpr int max_locals = 16;
        static constexpr int stack_size = 32;

        enum DataType : uint8_t { Integer, String, LvglObject };

        struct Value {
          DataType type;

          union {
            uint32_t i;
            lv_obj_t* obj;

            struct {
              char* s;
              uint16_t cap;
            };
          } data;

          Value() : type(Integer), data {0} {
          }

          Value(DataType t) : type(t), data {0} {
          }

          Value(uint32_t i) : type(Integer) {
            data.i = i;
          }

          Value(lv_obj_t* obj) : type(LvglObject) {
            data.obj = obj;
          }

          Value(char* s, uint16_t cap) : type(String) {
            data.s = s;
            data.cap = cap;
          }

          ~Value() {
            switch (type) {
              case String:
                delete[] data.s;
                break;

              case LvglObject:
                lv_obj_del(data.obj);
                break;

              default:
                break;
            }
          }
        } __packed;

        enum OpcodeShort : uint8_t {
          StoreLocal,
          LoadLocal,
          Branch,
          Push0,
          PushU8,
          PushU16,
          PushU24,
          PushU32,
          PushEmptyString,
          Duplicate,
          LoadString,

          StartPeriodicRefresh,
          StopPeriodicRefresh,

          SetLabelText,
          SetObjectAlign,
          CreateLabel,
          SetStyleLocalInt,
          SetStyleLocalColor,
          SetStyleLocalOpa,
          SetStyleLocalFont,
          WaitRefresh,

          Add,
          Subtract,
          Multiply,
          Divide,

          GrowString,
          Concat
        };

        enum OpcodeLong : uint16_t {};

        uint8_t read_byte(size_t pos);
        uint16_t read_u16(size_t pos);
        uint32_t read_u24(size_t pos);
        uint32_t read_u32(size_t pos);

        uint8_t* code;
        size_t code_len;
        size_t ptr = 0;

        Value locals[max_locals];

        Value stack[stack_size];
        uint8_t stack_pointer = 0;

        lv_task_t* taskRefresh = nullptr;

        void run();
        void asm_assert(bool condition);

        Value pop() {
          asm_assert(stack_pointer > 0);
          return stack[--stack_pointer];
        }

        Value pop(DataType type) {
          asm_assert(stack_pointer > 0);
          Value v = stack[--stack_pointer];
          asm_assert(v.type == type);
          return v;
        }

        uint32_t pop_uint32() {
          return pop(Integer).data.i;
        }

        void push(Value v) {
          asm_assert(stack_pointer < stack_size);
          stack[stack_pointer] = v;
          stack_pointer++;
        }
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