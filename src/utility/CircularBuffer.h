#pragma once

#include <array>
#include <cstddef>

namespace Pinetime {
  namespace Utility {
    template <class T, size_t S>
    struct CircularBuffer {
      constexpr size_t Size() const {
        return S;
      }

      size_t Idx() const {
        return idx;
      }

      T& operator[](size_t n) {
        return data[(idx + n) % S];
      }

      const T& operator[](size_t n) const {
        return data[(idx + n) % S];
      }

      void operator++() {
        idx++;
        idx %= S;
      }

      void operator++(int) {
        operator++();
      }

      void operator--() {
        if (idx > 0) {
          idx--;
        } else {
          idx = S - 1;
        }
      }

      void operator--(int) {
        operator--();
      }

      std::array<T, S> data;
      size_t idx = 0;
    };
  }
}
