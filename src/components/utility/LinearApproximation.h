#pragma once

#include <array>

namespace Pinetime {
  namespace Utility {

    // based on: https://github.com/SHristov92/LinearApproximation/blob/main/Linear.h
    template <typename Key, typename Value, std::size_t Size>
    class LinearApproximation {
      using Point = struct {
        Key key;
        Value value;
      };

    public:
      LinearApproximation(const std::array<Point, Size> &&points)
        : points{points} {}

      Value GetValue(Key key) const {
        if (key <= points[0].key) {
          return points[0].value;
        }

        for (size_t i = 1; i < Size; i++) {
          if (key < points[i].key) {
            return points[i-1].value + (key - points[i-1].key) * (points[i].value - points[i-1].value) / (points[i].key - points[i-1].key);
          }
        }

        return points[Size - 1].value;
      }

    private:
      std::array<Point, Size> points;
    };
  }
}
