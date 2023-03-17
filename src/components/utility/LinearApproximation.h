#pragma once

#include <cstddef>
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
      LinearApproximation(const std::array<Point, Size>&& sorted_points) : points {sorted_points} {
      }

      Value GetValue(Key key) const {
        if (key <= points[0].key) {
          return points[0].value;
        }

        for (std::size_t i = 1; i < Size; i++) {
          const auto& p = points[i];
          const auto& p_prev = points[i - 1];

          if (key < p.key) {
            return p_prev.value + (key - p_prev.key) * (p.value - p_prev.value) / (p.key - p_prev.key);
          }
        }

        return points[Size - 1].value;
      }

    private:
      std::array<Point, Size> points;
    };
  }
}
