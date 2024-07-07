#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

#include <cassert>
#include <memory>

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

        // TODO: Use fancy C++ type stuff
        struct Value {
          virtual DataType type() = 0;
        };

        struct ValueInteger : public Value {
          uint32_t i;

          ValueInteger(uint32_t i) : i(i) {
          }

          DataType type() override {
            return Integer;
          }
        };

        struct ValueString : public Value {
          char* str;
          uint16_t capacity;

          ValueString(char* str, uint16_t cap) : str(str), capacity(cap) {
          }

          ~ValueString() {
            delete[] str;
          }

          DataType type() override {
            return String;
          }
        };

        struct ValueLvglObject : public Value {
          lv_obj_t* obj;

          ValueLvglObject(lv_obj_t* obj) : obj(obj) {
          }

          ~ValueLvglObject() {
            lv_obj_del(obj);
          }

          DataType type() override {
            return LvglObject;
          }
        };

        enum OpcodeShort : uint8_t {
          StoreLocal,
          LoadLocal,
          Branch,
          Call,
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
        size_t pc = 0;

        std::shared_ptr<Value> locals[max_locals] = {};

        std::shared_ptr<Value> stack[stack_size] = {};
        uint8_t stack_pointer = 0;

        lv_task_t* taskRefresh = nullptr;

        void run();
        void asm_assert(bool condition);

        std::shared_ptr<Value> pop() {
          asm_assert(stack_pointer > 0);

          stack_pointer--;

          auto v = stack[stack_pointer];
          stack[stack_pointer] = nullptr;

          return v;
        }

        template <typename T>
        std::shared_ptr<T> pop(DataType type)
          requires(std::is_base_of_v<Value, T>)
        {
          auto v = pop();
          asm_assert(v->type() == type);

          return std::static_pointer_cast<T>(v);
        }

        uint32_t pop_uint32() {
          return pop<ValueInteger>(Integer)->i;
        }

        void push(std::shared_ptr<Value> v) {
          asm_assert(stack_pointer < stack_size);

          stack[stack_pointer++] = v;
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