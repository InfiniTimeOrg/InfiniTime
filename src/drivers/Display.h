#pragma once
#include <concepts>
#include <cstdint>
#include <cstddef>

namespace Pinetime {
  namespace Drivers {
    template <typename DisplayImpl>
    concept IsDisplay = requires(DisplayImpl display, uint16_t line, uint16_t coord, uint32_t color, uint16_t dimension, const uint8_t* data, size_t size) {
                          { display.Init() };
                          { display.Uninit() };
                          { display.DrawPixel(coord, coord, color) };
                          { display.VerticalScrollDefinition(line, line, line) };
                          { display.VerticalScrollStartAddress(line) };
                          { display.DrawBuffer(coord, coord, dimension, dimension, data, size) };
                          { display.Sleep() };
                          { display.Wakeup() };
                        };

    namespace Interface {
      template <class T>
        requires IsDisplay<T>
      class Display {
      public:
        explicit Display(T& impl) : impl {impl} {}
        Display(const Display&) = delete;
        Display& operator=(const Display&) = delete;
        Display(Display&&) = delete;
        Display& operator=(Display&&) = delete;

        void Init() {
          impl.Init();
        }

        void Uninit() {
          impl.Uninit();
        }

        void DrawPixel(uint16_t x, uint16_t y, uint32_t color) {
          impl.DrawPixel(x, y, color);
        }

        void VerticalScrollDefinition(uint16_t topFixedLines, uint16_t scrollLines, uint16_t bottomFixedLines) {
          impl.VerticalScrollDefinition(topFixedLines, scrollLines, bottomFixedLines);
        }

        void VerticalScrollStartAddress(uint16_t line) {
          impl.VerticalScrollStartAddress(line);
        }

        void DrawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* data, size_t size) {
          impl.DrawBuffer(x, y, width, height, data, size);
        }

        void Sleep() {
          impl.Sleep();
        }

        void Wakeup() {
          impl.Wakeup();
        }

      private:
        T& impl;
      };
    }
  }
}
