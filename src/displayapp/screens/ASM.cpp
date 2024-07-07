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
  lv_obj_clean(lv_scr_act());
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
    if (ptr >= code_len) {
      break;
    }

    OpcodeShort opcode = static_cast<OpcodeShort>(code[ptr]);
    if (opcode & (1 << 7)) {
      // Long opcode
      OpcodeLong opcode = static_cast<OpcodeLong>(code[ptr] << 8 | code[ptr + 1]);

      NRF_LOG_INFO("Long opcode: %d", opcode);

      ptr += 2;

      switch (opcode) {
        default:
          NRF_LOG_ERROR("Unknown opcode: 0x%04X", opcode);
          break;
      }
    } else {
      ptr++;

      NRF_LOG_INFO("Short opcode: %d", opcode);

      switch (opcode) {
        case WaitRefresh:
          return;

        case Push0:
          push(Value((uint32_t) 0));
          break;

        case PushU8:
          push(Value(read_byte(ptr)));
          ptr++;
          break;

        case PushU16:
          push(Value(read_u16(ptr)));
          ptr += 2;
          break;

        case PushU24:
          push(Value(read_u24(ptr)));
          ptr += 3;
          break;

        case PushU32:
          push(Value(read_u32(ptr)));
          ptr += 4;
          break;

        case Duplicate:
          // TODO: Check stack_pointer
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

          push(Value(text, length + 1));
          break;
        }

        case StoreLocal:
          locals[read_byte(ptr++)] = pop();
          break;

        case LoadLocal:
          push(locals[read_byte(ptr++)]);
          break;

        case Branch:
          assert(stack_pointer >= 1);
          ptr = pop_uint32();
          break;

        case SetLabelText: {
          Value str = pop();
          assert(str.type == String);
          Value obj = pop(LvglObject);

          lv_label_set_text(obj.data.obj, str.data.s);
          break;
        }

        case CreateLabel:
          push(Value(lv_label_create(lv_scr_act(), NULL)));
          break;

        case SetObjectAlign: {
          assert(stack_pointer >= 3);
          int16_t y = pop_uint32();
          int16_t x = pop_uint32();
          uint8_t align = pop_uint32();
          Value obj = pop(LvglObject);
          lv_obj_align(obj.data.obj, lv_scr_act(), align, x, y);
          break;
        }

        case SetStyleLocalInt:
        case SetStyleLocalFont:
        case SetStyleLocalColor: {
          assert(stack_pointer >= 3);
          uint32_t value = pop_uint32();
          uint32_t prop = pop_uint32();
          uint32_t part = pop_uint32();
          Value obj = pop(LvglObject);

          switch (opcode) {
            case SetStyleLocalInt:
              _lv_obj_set_style_local_int(obj.data.obj, part, prop, value);
              break;

            case SetStyleLocalColor:
              _lv_obj_set_style_local_color(obj.data.obj, part, prop, lv_color_hex(value));
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
                _lv_obj_set_style_local_ptr(obj.data.obj, part, prop, font);

              break;
            }

            default:
              break;
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
