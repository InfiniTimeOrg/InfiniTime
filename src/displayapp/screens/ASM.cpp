#include "ASM.h"

#include <libraries/log/nrf_log.h>
#include <assert.h>

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

      if (opcode >= SelectSlot0 && opcode <= SelectSlotMax) {
        current_slot = opcode - SelectSlot0;
        continue;
      }

      switch (opcode) {
        case WaitRefresh:
          return;

        case Push0:
          stack[stack_pointer++] = 0;
          break;

        case PushU8:
          stack[stack_pointer++] = read_byte(ptr);
          ptr++;
          break;

        case PushU16:
          stack[stack_pointer++] = read_u16(ptr);
          ptr += 2;
          break;

        case PushU24:
          stack[stack_pointer++] = read_u24(ptr);
          ptr += 3;
          break;

        case PushU32:
          stack[stack_pointer++] = read_u32(ptr);
          ptr += 4;
          break;

        case Branch:
          assert(stack_pointer >= 1);
          ptr = stack[--stack_pointer];
          break;

        case SetLabelText: { // TODO: Remove double allocation here
          assert(stack_pointer >= 1);
          size_t ptr = stack[--stack_pointer];

          int length = read_byte(ptr);
          char* text = new char[length + 1];
          text[length] = '\0';

          for (int i = 0; i < length; i++) {
            text[i] = read_byte(ptr + 1 + i);
          }

          lv_label_set_text(slots[current_slot], text);

          delete[] text;
          break;
        }

        case CreateLabel:
          slots[current_slot] = lv_label_create(lv_scr_act(), NULL); //
          break;

        case SetObjectAlign: {
          assert(stack_pointer >= 3); // TODO: Compactize this
          int16_t y = stack[--stack_pointer];
          int16_t x = stack[--stack_pointer];
          uint8_t align = stack[--stack_pointer];
          lv_obj_align(slots[current_slot], lv_scr_act(), align, x, y);
          break;
        }

        case SetStyleLocalInt:
        case SetStyleLocalFont:
        case SetStyleLocalColor: {
          assert(stack_pointer >= 3);
          uint32_t value = stack[--stack_pointer];
          uint32_t prop = stack[--stack_pointer];
          uint32_t part = stack[--stack_pointer];

          switch (opcode) {
            case SetStyleLocalInt:
              _lv_obj_set_style_local_int(slots[current_slot], part, prop, value);
              break;

            case SetStyleLocalColor:
              _lv_obj_set_style_local_color(slots[current_slot], part, prop, lv_color_hex(value));
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
                _lv_obj_set_style_local_ptr(slots[current_slot], part, prop, font);

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