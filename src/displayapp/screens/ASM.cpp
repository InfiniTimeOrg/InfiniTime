#include "ASM.h"

#include <libraries/log/nrf_log.h>
#include <assert.h>
#include <cstdio>

#include "asm_data.h"

using namespace Pinetime::Applications::Screens;

#define u16(n) (n) >> 8, (n) & 0xFF

ASM::ASM() {
  this->code = out_bin;
  this->code_len = out_bin_len;

  run();
}

ASM::~ASM() {
  if (taskRefresh != nullptr) {
    lv_task_del(taskRefresh);
  }

  // We don't need to clean the screen since all objects are deleted when their shared_ptr is dropped
  // lv_obj_clean(lv_scr_act());
}

uint8_t ASM::read_byte(size_t pos) {
  return code[pos];
}

uint16_t ASM::read_u16(size_t pos) {
  return static_cast<uint16_t>(code[pos + 1] << 8 | code[pos]);
}

uint32_t ASM::read_u24(size_t pos) {
  return static_cast<uint32_t>(code[pos + 2] << 16 | code[pos + 1] << 8 | code[pos]);
}

uint32_t ASM::read_u32(size_t pos) {
  return static_cast<uint32_t>(code[pos + 3] << 24 | code[pos + 2] << 16 | code[pos + 1] << 8 | code[pos]);
}

void ASM::run() {
  for (;;) {
    if (pc >= code_len) {
      break;
    }

    OpcodeShort opcode = static_cast<OpcodeShort>(code[pc]);
    if (opcode & (1 << 7)) {
      // Long opcode
      OpcodeLong opcode = static_cast<OpcodeLong>(code[pc] << 8 | code[pc + 1]);

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
          push(std::make_shared<Value>((uint32_t) 0));
          break;

        case PushU8:
          push(std::make_shared<Value>(read_byte(pc)));
          pc++;
          break;

        case PushU16:
          push(std::make_shared<Value>(read_u16(pc)));
          pc += 2;
          break;

        case PushU24:
          push(std::make_shared<Value>(read_u24(pc)));
          pc += 3;
          break;

        case PushU32:
          push(std::make_shared<Value>(read_u32(pc)));
          pc += 4;
          break;

        case PushEmptyString:
          push(std::make_shared<Value>(new char[1] {0}, 1));
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

          push(std::make_shared<Value>(text, length + 1));
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
          push(std::make_shared<Value>(next));
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
          auto str = pop(String);
          auto obj = pop(LvglObject);

          lv_label_set_text(obj->data.lvobj, str->data.s);
          break;
        }

        case CreateLabel:
          push(std::make_shared<Value>(lv_label_create(lv_scr_act(), NULL)));
          break;

        case SetObjectAlign: {
          int16_t y = pop_uint32();
          int16_t x = pop_uint32();
          uint8_t align = pop_uint32();
          auto obj = pop(LvglObject);
          lv_obj_align(obj->data.lvobj, lv_scr_act(), align, x, y);
          break;
        }

        case SetStyleLocalInt:
        case SetStyleLocalFont:
        case SetStyleLocalColor: {
          uint32_t value = pop_uint32();
          uint32_t prop = pop_uint32();
          uint32_t part = pop_uint32();
          auto obj = pop(LvglObject);

          switch (opcode) {
            case SetStyleLocalInt:
              _lv_obj_set_style_local_int(obj->data.lvobj, part, prop, value);
              break;

            case SetStyleLocalColor:
              _lv_obj_set_style_local_color(obj->data.lvobj, part, prop, lv_color_hex(value));
              break;

            case SetStyleLocalFont: {
              lv_font_t* font = NULL;

              switch (value) {
                case 0:
                  font = &jetbrains_mono_extrabold_compressed;
                  break;

                default:
                  NRF_LOG_ERROR("Unknown font: %d", value);
                  break;
              }

              if (font)
                _lv_obj_set_style_local_ptr(obj->data.lvobj, part, prop, font);

              break;
            }

            default:
              break;
          }
          break;
        }

        case Add: {
          push(std::make_shared<Value>(pop_uint32() + pop_uint32()));
          break;
        }

        case Subtract:
          push(std::make_shared<Value>(pop_uint32() - pop_uint32()));
          break;

        case Multiply:
          push(std::make_shared<Value>(pop_uint32() * pop_uint32()));
          break;

        case Divide:
          push(std::make_shared<Value>(pop_uint32() / pop_uint32()));
          break;

        case GrowString: {
          auto len = pop(Integer);
          auto str = pop(String);

          size_t new_cap = len->data.i + str->data.cap;
          asm_assert(new_cap >= str->data.cap);

          char* new_str = new char[new_cap];
          memcpy(new_str, str->data.s, str->data.cap);

          push(std::make_shared<Value>(new_str, new_cap));
          break;
        }

        case Concat: {
          auto b = pop();
          auto a = pop();

          if (a->type == String && b->type == String) {
            int len_a = strlen(a->data.s);
            int len_b = strlen(b->data.s);

            size_t new_len = len_a + len_b + 1;

            if (a->data.cap >= new_len) {
              strcat(a->data.s, b->data.s);

              push(a);
            } else {
              char* s = new char[new_len + 1];
              strcpy(s, a->data.s);
              strcat(s, b->data.s);

              push(std::make_shared<Value>(s, new_len + 1));
            }
          } else if (a->type == String && b->type == Integer) {
            size_t cap = strlen(a->data.s) + 12 + 1;
            char* s = new char[cap];
            snprintf(s, cap, "%s%lu", a->data.s, b->data.i);

            push(std::make_shared<Value>(s, cap));
          } else {
            asm_assert(false);
          }
          break;
        }

        default:
          NRF_LOG_ERROR("Unknown opcode: 0x%02X", opcode);
          break;
      }
    }
  }
}

void ASM::Refresh() {
  run();
}

void ASM::asm_assert(bool condition) {
  if (!condition) {
    // TODO: Handle better
    for (;;) {
    }
  }
}
