#include "ASM.h"

#include <libraries/log/nrf_log.h>
#include <assert.h>
#include <cstdio>

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

constexpr uint32_t handler_return_pc_mark = 1 << 31;

struct CallbackInfo {
  ASM* instance;
  uint32_t callback_pc;
};

static void event_handler(lv_obj_t* obj, lv_event_t event) {
  CallbackInfo* cbInfo = static_cast<CallbackInfo*>(lv_obj_get_user_data(obj));
  cbInfo->instance->OnObjectEvent(obj, event);
}

ASM::ASM(Controllers::DateTime& dateTimeController,
         const Controllers::Battery& batteryController,
         const Controllers::Ble& bleController,
         Controllers::FS& fsController)
  : dateTimeController(dateTimeController), statusIcons(batteryController, bleController), fs(fsController) {

  int result = fsController.FileOpen(&file, "program.bin", LFS_O_RDONLY);
  asm_assert(result >= 0);

  result = fsController.FileSeek(&file, 0, LFS_SEEK_END);
  asm_assert(result >= 0);
  program_size = result;
  fsController.FileSeek(&file, 0, LFS_SEEK_SET);

  populate_cache(0);

  Refresh();
}

ASM::~ASM() {
  if (taskRefresh != nullptr) {
    lv_task_del(taskRefresh);
  }
  if (showingStatusIcons) {
    lv_obj_del(statusIcons.GetObject());
  }

  fs.FileClose(&file);

  // We don't need to clean the screen since all objects are deleted when their shared_ptr is dropped
  // lv_obj_clean(lv_scr_act());
}

void ASM::populate_cache(size_t pos) {
  int result = fs.FileSeek(&file, pos);
  asm_assert(result >= 0);

  result = fs.FileRead(&file, cache, cache_size);
  asm_assert(result >= 0);

  cache_start = pos;
}

void ASM::run() {
  while (pc < program_size) {
    OpcodeShort opcode = static_cast<OpcodeShort>(read_byte(pc));
    if (static_cast<uint8_t>(opcode) & (1 << 7)) {
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
        case OpcodeShort::WaitRefresh:
          return;

        case OpcodeShort::Push0:
          push(std::make_shared<ValueInteger>(0));
          break;

        case OpcodeShort::PushU8:
          push(std::make_shared<ValueInteger>(read_byte(pc)));
          pc++;
          break;

        case OpcodeShort::PushU16:
          push(std::make_shared<ValueInteger>(read_u16(pc)));
          pc += 2;
          break;

        case OpcodeShort::PushU24:
          push(std::make_shared<ValueInteger>(read_u24(pc)));
          pc += 3;
          break;

        case OpcodeShort::PushU32:
          push(std::make_shared<ValueInteger>(read_u32(pc)));
          pc += 4;
          break;

        case OpcodeShort::PushEmptyString:
          push(std::make_shared<ValueString>(new char[1] {0}, 1));
          break;

        case OpcodeShort::Duplicate:
          push(stack[stack_pointer - 1]);
          break;

        case OpcodeShort::Pop:
          pop();
          break;

        case OpcodeShort::LoadString: {
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

        case OpcodeShort::StoreLocal:
          locals[read_byte(pc++)] = pop();
          break;

        case OpcodeShort::LoadLocal:
          push(locals[read_byte(pc++)]);
          break;

        case OpcodeShort::Branch: {
          uint32_t value = pop_uint32();

          if ((value & handler_return_pc_mark) != 0) {
            pc = value & ~handler_return_pc_mark;
            return;
          }

          pc = value;
          break;
        }

        case OpcodeShort::Call: {
          uint32_t next = pc;
          pc = pop_uint32();
          push(std::make_shared<ValueInteger>(next));
          break;
        }

        case OpcodeShort::StartPeriodicRefresh:
          if (taskRefresh == nullptr) {
            taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
          }
          break;

        case OpcodeShort::StopPeriodicRefresh:
          if (taskRefresh != nullptr) {
            lv_task_del(taskRefresh);
            taskRefresh = nullptr;
          }
          break;

        case OpcodeShort::SetLabelText: {
          auto str = pop<ValueString>(String);
          auto obj = pop<ValueLvglObject>(LvglObject);

          lv_label_set_text(obj->obj, str->str);
          break;
        }

        case OpcodeShort::CreateLabel:
          push(std::make_shared<ValueLvglObject>(lv_label_create(lv_scr_act(), NULL)));
          break;

        case OpcodeShort::CreateButton:
          push(std::make_shared<ValueLvglObject>(lv_btn_create(lv_scr_act(), NULL)));
          break;

        case OpcodeShort::SetObjectAlign: {
          int16_t y = pop_uint32();
          int16_t x = pop_uint32();
          uint8_t align = pop_uint32();
          auto obj = pop<ValueLvglObject>(LvglObject);
          lv_obj_align(obj->obj, lv_scr_act(), align, x, y);
          break;
        }

        case OpcodeShort::SetObjectSize: {
          int16_t h = pop_uint32();
          int16_t w = pop_uint32();
          auto obj = pop<ValueLvglObject>(LvglObject);
          lv_obj_set_size(obj->obj, w, h);
          break;
        }

        case OpcodeShort::SetObjectParent: {
          auto parent = pop<ValueLvglObject>(LvglObject);
          auto child = pop<ValueLvglObject>(LvglObject);
          lv_obj_set_parent(child->obj, parent->obj);
          break;
        }

        case OpcodeShort::SetStyleLocalInt:
        case OpcodeShort::SetStyleLocalFont:
        case OpcodeShort::SetStyleLocalColor: {
          uint32_t value = pop_uint32();
          uint32_t prop = pop_uint32();
          uint32_t part = pop_uint32();
          auto obj = pop<ValueLvglObject>(LvglObject);

          switch (opcode) {
            case OpcodeShort::SetStyleLocalInt:
              _lv_obj_set_style_local_int(obj->obj, part, prop, value);
              break;

            case OpcodeShort::SetStyleLocalColor:
              _lv_obj_set_style_local_color(obj->obj, part, prop, lv_color_hex(value));
              break;

            case OpcodeShort::SetStyleLocalFont: {
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

        case OpcodeShort::SetEventHandler: {
          uint32_t cb_pc = pop_uint32();
          auto obj = pop<ValueLvglObject>(LvglObject);
          CallbackInfo* cb = new CallbackInfo {this, cb_pc};

          lv_obj_set_user_data(obj->obj, cb);
          lv_obj_set_event_cb(obj->obj, event_handler);
          break;
        }

        case OpcodeShort::Add:
          push(std::make_shared<ValueInteger>(pop_uint32() + pop_uint32()));
          break;

        case OpcodeShort::Subtract:
          push(std::make_shared<ValueInteger>(pop_uint32() - pop_uint32()));
          break;

        case OpcodeShort::Multiply:
          push(std::make_shared<ValueInteger>(pop_uint32() * pop_uint32()));
          break;

        case OpcodeShort::Divide: {
          uint32_t b = pop_uint32();
          uint32_t a = pop_uint32();
          push(std::make_shared<ValueInteger>(a / b));
          break;
        }

        case OpcodeShort::GrowString: {
          auto len = pop_uint32();
          auto str = pop<ValueString>(String);

          size_t new_cap = len + str->capacity;

          char* new_str = new char[new_cap];
          memcpy(new_str, str->str, str->capacity);

          push(std::make_shared<ValueString>(new_str, new_cap));
          break;
        }

        case OpcodeShort::ClearString: {
          auto str = pop<ValueString>(String);
          if (str->capacity > 0)
            str->str[0] = '\0';

          push(str);
          break;
        }

        case OpcodeShort::Concat: {
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

            size_t aLen = strlen(aString->str);
            size_t need_cap = aLen + 12 + 1;

            if (aString->capacity - aLen >= need_cap) {
              snprintf(aString->str + aLen, aString->capacity - aLen, "%lu", bInt->i);

              push(a);
            } else {
              char* s = new char[need_cap];
              memcpy(s, aString->str, aLen);
              snprintf(s + aLen, need_cap - aLen, "%lu", bInt->i);

              push(std::make_shared<ValueString>(s, need_cap));
            }
          } else {
            asm_assert(false);
          }
          break;
        }

        case OpcodeShort::PushCurrentDateTime: {
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

        case OpcodeShort::PushCurrentTicks:
          push(std::make_shared<ValueInteger>((xTaskGetTickCount() * configTICK_RATE_HZ) / 1000));
          break;

        case OpcodeShort::FormatDateTime: {
          auto fmt = pop<ValueString>(String);
          auto time = pop<ValueDateTime>(DateTime);

          constexpr int max_len = 16;
          char* str = new char[max_len]; // TODO: Allow user to reuse string in stack

          strftime(str, max_len, fmt->str, &time->tm);

          push(std::make_shared<ValueString>(str, max_len));
          break;
        }

        case OpcodeShort::RealignObject:
          lv_obj_realign(pop<ValueLvglObject>(LvglObject)->obj);
          break;

        case OpcodeShort::ShowStatusIcons:
          if (!showingStatusIcons) {
            showingStatusIcons = true;
            statusIcons.Create();
          }
          break;

        case OpcodeShort::Equals: {
          auto b = pop();
          auto a = pop();
          push(std::make_shared<ValueInteger>(a.get()->equals(b.get()) ? 1 : 0));
          break;
        }

        case OpcodeShort::Negate:
          push(std::make_shared<ValueInteger>(pop().get()->isTruthy() ? 0 : 1));
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

void ASM::_asm_assert(bool condition, const char* msg) {
  if (!condition) {
    // TODO: Handle better

    if (msg)
      NRF_LOG_ERROR("Assertion failed: %s", msg);

    for (;;) {
    }
  }
}

void ASM::OnObjectEvent(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED)
    return;

  CallbackInfo* cb = static_cast<CallbackInfo*>(lv_obj_get_user_data(obj));

  if (cb) {
    push(std::make_shared<ValueInteger>(pc | handler_return_pc_mark));
    pc = cb->callback_pc;

    run();
  }
}
