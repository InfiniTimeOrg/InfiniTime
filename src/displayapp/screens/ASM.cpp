#include "ASM.h"

#include <libraries/log/nrf_log.h>
#include <assert.h>
#include <cstdio>

#include "asm_data.h"

using namespace Pinetime::Applications::Screens;

constexpr lv_font_t* fonts[] = {
  &fontawesome_weathericons,
  &jetbrains_mono_42,
  &jetbrains_mono_76,
  &jetbrains_mono_bold_20,
  &jetbrains_mono_extrabold_compressed,
  &lv_font_sys_48,
  &open_sans_light,
};
constexpr int num_fonts = sizeof(fonts) / sizeof(fonts[0]);

ASM::ASM(Controllers::DateTime& dateTimeController, const Controllers::Battery& batteryController, const Controllers::Ble& bleController)
  : dateTimeController(dateTimeController), statusIcons(batteryController, bleController) {
  this->code = out_bin;
  this->code_len = out_bin_len;

  Refresh();
}

ASM::~ASM() {
  if (taskRefresh != nullptr) {
    lv_task_del(taskRefresh);
  }

  // We don't need to clean the screen since all objects are deleted when their shared_ptr is dropped
  // lv_obj_clean(lv_scr_act());
}

void ASM::run() {
  for (;;) {
    if (pc >= code_len) {
      break;
    }

    OpcodeShort opcode = static_cast<OpcodeShort>(read_byte(pc));
    if (opcode & (1 << 7)) {
      // Long opcode
      OpcodeLong opcode = static_cast<OpcodeLong>(read_u16(pc));

      NRF_LOG_INFO("Long opcode: %d", opcode);

      pc += 2;

      switch (opcode) {
        default:
          NRF_LOG_ERROR("Unknown opcode: 0x%04X", opcode);
          break;
      }
    } else {
      pc++;

      NRF_LOG_INFO("Short opcode: %d", opcode);

      switch (opcode) {
        case WaitRefresh:
          return;

        case Push0:
          push(std::make_shared<ValueInteger>(0));
          break;

        case PushU8:
          push(std::make_shared<ValueInteger>(read_byte(pc)));
          pc++;
          break;

        case PushU16:
          push(std::make_shared<ValueInteger>(read_u16(pc)));
          pc += 2;
          break;

        case PushU24:
          push(std::make_shared<ValueInteger>(read_u24(pc)));
          pc += 3;
          break;

        case PushU32:
          push(std::make_shared<ValueInteger>(read_u32(pc)));
          pc += 4;
          break;

        case PushEmptyString:
          push(std::make_shared<ValueString>(new char[1] {0}, 1));
          break;

        case Duplicate:
          push(stack[stack_pointer - 1]);
          break;

        case LoadString: {
          uint32_t ptr = pop_uint32();

          int length = read_byte(ptr);
          char* text = new char[length + 1];
          text[length] = '\0';

          for (int i = 0; i < length; i++) {
            text[i] = read_byte(ptr + 1 + i);
          }

          push(std::make_shared<ValueString>(text, length + 1));
          break;
        }

        case StoreLocal:
          locals[read_byte(pc++)] = pop();
          break;

        case LoadLocal:
          push(locals[read_byte(pc++)]);
          break;

        case Branch:
          pc = pop_uint32();
          break;

        case Call: {
          uint32_t next = pc;
          pc = pop_uint32();
          push(std::make_shared<ValueInteger>(next));
          break;
        }

        case StartPeriodicRefresh:
          if (taskRefresh == nullptr) {
            taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
          }
          break;

        case StopPeriodicRefresh:
          if (taskRefresh != nullptr) {
            lv_task_del(taskRefresh);
            taskRefresh = nullptr;
          }
          break;

        case SetLabelText: {
          auto str = pop<ValueString>(String);
          auto obj = pop<ValueLvglObject>(LvglObject);

          lv_label_set_text(obj->obj, str->str);
          break;
        }

        case CreateLabel:
          push(std::make_shared<ValueLvglObject>(lv_label_create(lv_scr_act(), NULL)));
          break;

        case SetObjectAlign: {
          int16_t y = pop_uint32();
          int16_t x = pop_uint32();
          uint8_t align = pop_uint32();
          auto obj = pop<ValueLvglObject>(LvglObject);
          lv_obj_align(obj->obj, lv_scr_act(), align, x, y);
          break;
        }

        case SetStyleLocalInt:
        case SetStyleLocalFont:
        case SetStyleLocalColor: {
          uint32_t value = pop_uint32();
          uint32_t prop = pop_uint32();
          uint32_t part = pop_uint32();
          auto obj = pop<ValueLvglObject>(LvglObject);

          switch (opcode) {
            case SetStyleLocalInt:
              _lv_obj_set_style_local_int(obj->obj, part, prop, value);
              break;

            case SetStyleLocalColor:
              _lv_obj_set_style_local_color(obj->obj, part, prop, lv_color_hex(value));
              break;

            case SetStyleLocalFont: {
              if (value < num_fonts) {
                _lv_obj_set_style_local_ptr(obj->obj, part, prop, fonts[value]);
              }
              break;
            }

            default:
              break;
          }
          break;
        }

        case Add: {
          push(std::make_shared<ValueInteger>(pop_uint32() + pop_uint32()));
          break;
        }

        case Subtract:
          push(std::make_shared<ValueInteger>(pop_uint32() - pop_uint32()));
          break;

        case Multiply:
          push(std::make_shared<ValueInteger>(pop_uint32() * pop_uint32()));
          break;

        case Divide:
          push(std::make_shared<ValueInteger>(pop_uint32() / pop_uint32()));
          break;

        case GrowString: {
          auto len = pop_uint32();
          auto str = pop<ValueString>(String);

          size_t new_cap = len + str->capacity;
          asm_assert(new_cap >= str->capacity);

          char* new_str = new char[new_cap];
          memcpy(new_str, str->str, str->capacity);

          push(std::make_shared<ValueString>(new_str, new_cap));
          break;
        }

        case ClearString: {
          auto str = pop<ValueString>(String);
          if (str->capacity > 0)
            str->str[0] = '\0';

          push(std::make_shared<ValueString>(str->str, str->capacity));
          break;
        }

        case Concat: {
          auto b = pop();
          auto a = pop();

          if (a->type() == String && b->type() == String) {
            auto aString = static_cast<ValueString*>(a.get());
            auto bString = static_cast<ValueString*>(b.get());

            int len_a = strlen(aString->str);
            int len_b = strlen(bString->str);

            size_t new_len = len_a + len_b + 1;

            if (aString->capacity >= new_len) {
              strcat(aString->str, bString->str);

              push(a);
            } else {
              char* s = new char[new_len + 1];
              strcpy(s, aString->str);
              strcat(s, bString->str);

              push(std::make_shared<ValueString>(s, new_len + 1));
            }
          } else if (a->type() == String && b->type() == Integer) {
            auto aString = static_cast<ValueString*>(a.get());
            auto bInt = static_cast<ValueInteger*>(b.get());

            if (bInt) {
              size_t cap = strlen(aString->str) + 12 + 1;
              char* s = new char[cap];
              snprintf(s, cap, "%s%lu", aString->str, bInt->i);

              push(std::make_shared<ValueString>(s, cap));
            }
          } else {
            asm_assert(false);
          }
          break;
        }

        case PushCurrentTime: {
          auto time = dateTimeController.CurrentDateTime();
          std::tm tm {
            .tm_sec = dateTimeController.Seconds(),
            .tm_min = dateTimeController.Minutes(),
            .tm_hour = dateTimeController.Hours(),
            .tm_mday = dateTimeController.Day(),
            .tm_mon = static_cast<int>(dateTimeController.Month()) - 1,
            .tm_year = dateTimeController.Year() - 1900,
            .tm_wday = static_cast<int>(dateTimeController.DayOfWeek()),
            .tm_yday = dateTimeController.DayOfYear() - 1,
          };

          push(std::make_shared<ValueDateTime>(time, tm));
          break;
        }

        case FormatDateTime: {
          auto fmt = pop<ValueString>(String);
          auto time = pop<ValueDateTime>(DateTime);

          constexpr int max_len = 16;
          char* str = new char[max_len]; // TODO: Allow user to reuse string in stack

          strftime(str, max_len, fmt->str, &time->tm);

          push(std::make_shared<ValueString>(str, max_len));
          break;
        }

        case RealignObject:
          lv_obj_realign(pop<ValueLvglObject>(LvglObject)->obj);
          break;

        case ShowStatusIcons:
          if (!showingStatusIcons) {
            showingStatusIcons = true;
            statusIcons.Create();
          }
          break;

        default:
          NRF_LOG_ERROR("Unknown opcode: 0x%02X", opcode);
          break;
      }
    }
  }
}

void ASM::Refresh() {
  run();

  if (showingStatusIcons) {
    statusIcons.Update();
  }
}

void ASM::asm_assert(bool condition) {
  if (!condition) {
    // TODO: Handle better
    for (;;) {
    }
  }
}
