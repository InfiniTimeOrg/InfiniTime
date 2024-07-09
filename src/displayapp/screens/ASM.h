#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "displayapp/widgets/StatusIcons.h"
#include "components/datetime/DateTimeController.h"
#include "Symbols.h"

#include <cassert>
#include <memory>
#include <chrono>

#if DEBUG
  #define STRINGIZE_DETAIL(x)   #x
  #define STRINGIZE(x)          STRINGIZE_DETAIL(x)
  #define asm_assert(condition) _asm_assert(condition, __FILE__ ":" STRINGIZE(__LINE__) " " #condition)
#else
  #define asm_assert(condition) _asm_assert(condition, NULL)
#endif

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ASM : public Screen {
      public:
        ASM(Controllers::DateTime&, const Controllers::Battery&, const Controllers::Ble&, Controllers::FS&);
        ~ASM();

        void Refresh() override;

        void OnObjectEvent(lv_obj_t* obj, lv_event_t event);

      private:
        static constexpr int num_slots = 16;
        static constexpr int max_locals = 16;
        static constexpr int stack_size = 32;
        static constexpr int cache_size = 16;

        enum DataType : uint8_t { Integer, String, LvglObject, DateTime };

        // TODO: Use fancy C++ type stuff
        struct Value {
          virtual DataType type() = 0;
          virtual bool equals(Value* other) = 0;
          virtual bool isTruthy() = 0;
        };

        struct ValueInteger : public Value {
          uint32_t i;

          ValueInteger(uint32_t i) : i(i) {
          }

          DataType type() override {
            return Integer;
          }

          bool equals(Value* other) override {
            return other->type() == Integer && i == static_cast<ValueInteger*>(other)->i;
          }

          bool isTruthy() override {
            return i != 0;
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

          bool equals(Value* other) override {
            return other->type() == String && strcmp(str, static_cast<ValueString*>(other)->str) == 0;
          }

          bool isTruthy() override {
            return capacity > 0 && str[0] != '\0';
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

          bool equals(Value* other) override {
            return other->type() == LvglObject && obj == static_cast<ValueLvglObject*>(other)->obj;
          }

          bool isTruthy() override {
            return obj != nullptr;
          }
        };

        struct ValueDateTime : public Value {
          std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> time;
          std::tm tm;

          ValueDateTime(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> time, std::tm tm)
            : time(time), tm(tm) {
          }

          DataType type() override {
            return DateTime;
          }

          bool equals(Value* other) override {
            return other->type() == DateTime && time == static_cast<ValueDateTime*>(other)->time;
          }

          bool isTruthy() override {
            return true;
          }
        };

        enum class OpcodeShort : uint8_t {
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
          PushCurrentDateTime,
          PushCurrentTicks,
          Duplicate,
          Pop,
          LoadString,

          StartPeriodicRefresh,
          StopPeriodicRefresh,
          ShowStatusIcons,

          CreateLabel,
          CreateButton,
          SetLabelText,
          SetObjectAlign,
          SetObjectSize,
          SetObjectParent,
          SetStyleLocalInt,
          SetStyleLocalColor,
          SetStyleLocalOpa,
          SetStyleLocalFont,
          SetEventHandler,
          RealignObject,
          WaitRefresh,

          Add,
          Subtract,
          Multiply,
          Divide,
          Equals,
          Negate,

          GrowString,
          ClearString,
          Concat,
          FormatDateTime
        };

        enum class OpcodeLong : uint16_t {};

        void populate_cache(size_t pos);

        uint8_t read_byte(size_t pos) {
          if (pos < cache_start || pos >= cache_start + cache_size) {
            populate_cache(pos);
          }

          return cache[pos - cache_start];
        }

        uint16_t read_u16(size_t pos) {
          return static_cast<uint16_t>(read_byte(pos + 1) << 8 | read_byte(pos));
        }

        uint32_t read_u24(size_t pos) {
          return static_cast<uint32_t>(read_byte(pos + 2) << 16 | read_byte(pos + 1) << 8 | read_byte(pos));
        }

        uint32_t read_u32(size_t pos) {
          return static_cast<uint32_t>(read_byte(pos + 3) << 24 | read_byte(pos + 2) << 16 | read_byte(pos + 1) << 8 | read_byte(pos));
        }

        lfs_file_t file;
        uint8_t cache[cache_size];
        size_t cache_start;
        size_t program_size;
        size_t pc = 0;

        std::shared_ptr<Value> locals[max_locals] = {};

        std::shared_ptr<Value> stack[stack_size] = {};
        uint8_t stack_pointer = 0;

        lv_task_t* taskRefresh = nullptr;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::FS& fs;
        std::unique_ptr<Widgets::StatusIcons> statusIcons;

        void run();
        void _asm_assert(bool condition, const char* msg);

        std::shared_ptr<Value> pop() {
          asm_assert(stack_pointer > 0);

          stack_pointer--;

          auto v = stack[stack_pointer];
          stack[stack_pointer] = nullptr;

          return v;
        }

        template <typename T>
        std::shared_ptr<T> pop(DataType wantType)
          requires(std::is_base_of_v<Value, T>)
        {
          auto v = pop();
          asm_assert(v->type() == wantType);

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

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::ASM(controllers.dateTimeController,
                                controllers.batteryController,
                                controllers.bleController,
                                controllers.filesystem);
      };
    };
  };
}