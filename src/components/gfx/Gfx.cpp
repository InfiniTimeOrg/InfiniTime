#include "components/gfx/Gfx.h"
#include "drivers/St7789.h"
using namespace Pinetime::Components;

Gfx::Gfx(Pinetime::Drivers::St7789& lcd) : lcd {lcd} {
}

void Gfx::Init() {
}

void Gfx::ClearScreen() {
  SetBackgroundColor(0x0000);

  state.remainingIterations = 240 + 1;
  state.currentIteration = 0;
  state.busy = true;
  state.action = Action::FillRectangle;
  state.taskToNotify = xTaskGetCurrentTaskHandle();

  lcd.DrawBuffer(0, 0, width, height, reinterpret_cast<const uint8_t*>(buffer), width * 2);
  WaitTransferFinished();
}

void Gfx::FillRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
  SetBackgroundColor(color);

  state.remainingIterations = h;
  state.currentIteration = 0;
  state.busy = true;
  state.action = Action::FillRectangle;
  state.color = color;
  state.taskToNotify = xTaskGetCurrentTaskHandle();

  lcd.DrawBuffer(x, y, w, h, reinterpret_cast<const uint8_t*>(buffer), width * 2);

  WaitTransferFinished();
}

void Gfx::FillRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t* b) {
  state.remainingIterations = h;
  state.currentIteration = 0;
  state.busy = true;
  state.action = Action::FillRectangle;
  state.color = 0x00;
  state.taskToNotify = xTaskGetCurrentTaskHandle();

  lcd.DrawBuffer(x, y, w, h, reinterpret_cast<const uint8_t*>(b), width * 2);

  WaitTransferFinished();
}

void Gfx::DrawString(uint8_t x, uint8_t y, uint16_t color, const char* text, const FONT_INFO* p_font, bool wrap) {
  if (y > (height - p_font->height)) {
    // Not enough space to write even single char.
    return;
  }

  uint8_t current_x = x;
  uint8_t current_y = y;

  for (size_t i = 0; text[i] != '\0'; i++) {
    if (text[i] == '\n') {
      current_x = x;
      current_y += p_font->height + p_font->height / 10;
    } else {
      DrawChar(p_font, (uint8_t) text[i], &current_x, current_y, color);
    }

    uint8_t char_idx = text[i] - p_font->startChar;
    uint16_t char_width = text[i] == ' ' ? (p_font->height / 2) : p_font->charInfo[char_idx].widthBits;

    if (current_x > (width - char_width)) {
      if (wrap) {
        current_x = x;
        current_y += p_font->height + p_font->height / 10;
      } else {
        break;
      }

      if (y > (height - p_font->height)) {
        break;
      }
    }
  }
}

void Gfx::DrawChar(const FONT_INFO* font, uint8_t c, uint8_t* x, uint8_t y, uint16_t color) {
  uint8_t char_idx = c - font->startChar;
  uint16_t bytes_in_line = CEIL_DIV(font->charInfo[char_idx].widthBits, 8);
  uint16_t bg = 0x0000;

  if (c == ' ') {
    *x += font->height / 2;
    return;
  }

  // Build first line
  for (uint16_t j = 0; j < bytes_in_line; j++) {
    for (uint8_t k = 0; k < 8; k++) {
      if ((1 << (7 - k)) & font->data[font->charInfo[char_idx].offset + j]) {
        buffer[(j * 8) + k] = color;
      } else {
        buffer[(j * 8) + k] = bg;
      }
    }
  }

  state.remainingIterations = font->height + 0;
  state.currentIteration = 0;
  state.busy = true;
  state.action = Action::DrawChar;
  state.font = const_cast<FONT_INFO*>(font);
  state.character = c;
  state.color = color;
  state.taskToNotify = xTaskGetCurrentTaskHandle();

  lcd.DrawBuffer(*x, y, bytes_in_line * 8, font->height, reinterpret_cast<const uint8_t*>(&buffer), bytes_in_line * 8 * 2);
  WaitTransferFinished();

  *x += font->charInfo[char_idx].widthBits + font->spacePixels;
}

void Gfx::pixel_draw(uint8_t x, uint8_t y, uint16_t color) {
  lcd.DrawPixel(x, y, color);
}

void Gfx::Sleep() {
  lcd.Sleep();
}

void Gfx::Wakeup() {
  lcd.Wakeup();
}

void Gfx::SetBackgroundColor(uint16_t color) {
  for (int i = 0; i < width; i++) {
    buffer[i] = color;
  }
}

bool Gfx::GetNextBuffer(uint8_t** data, size_t& size) {
  if (!state.busy)
    return false;
  state.remainingIterations = state.remainingIterations - 1;
  if (state.remainingIterations == 0) {
    state.busy = false;
    NotifyEndOfTransfer(state.taskToNotify);
    return false;
  }

  if (state.action == Action::FillRectangle) {
    *data = reinterpret_cast<uint8_t*>(buffer);
    size = width * 2;
  } else if (state.action == Action::DrawChar) {
    uint16_t bg = 0x0000;
    uint8_t char_idx = state.character - state.font->startChar;
    uint16_t bytes_in_line = CEIL_DIV(state.font->charInfo[char_idx].widthBits, 8);

    for (uint16_t j = 0; j < bytes_in_line; j++) {
      for (uint8_t k = 0; k < 8; k++) {
        if ((1 << (7 - k)) & state.font->data[state.font->charInfo[char_idx].offset + ((state.currentIteration + 1) * bytes_in_line) + j]) {
          buffer[(j * 8) + k] = state.color;
        } else {
          buffer[(j * 8) + k] = bg;
        }
      }
    }

    *data = reinterpret_cast<uint8_t*>(buffer);
    size = bytes_in_line * 8 * 2;
  }

  state.currentIteration = state.currentIteration + 1;

  return true;
}

void Gfx::NotifyEndOfTransfer(TaskHandle_t task) {
  if (task != nullptr) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(task, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

void Gfx::WaitTransferFinished() const {
  ulTaskNotifyTake(pdTRUE, 500);
}

void Gfx::SetScrollArea(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines) {
  lcd.VerticalScrollDefinition(topFixedLines, scrollLines, bottomFixedLines);
}

void Gfx::SetScrollStartLine(uint16_t line) {
  lcd.VerticalScrollStartAddress(line);
}
