#include <array>
#include <cstddef>

template <typename T, size_t N>
class StaticStack {
public:
  T Pop();
  void Push(T element);
  void Reset();
  T Top();

private:
  std::array<T, N> elementArray;
  // Number of elements in stack, points to the next empty slot
  size_t stackPointer = 0;
};

// Returns random data when popping from empty array.
template <typename T, size_t N>
T StaticStack<T, N>::Pop() {
  if (stackPointer > 0) {
    stackPointer--;
  }
  return elementArray[stackPointer];
}

template <typename T, size_t N>
void StaticStack<T, N>::Push(T element) {
  if (stackPointer < elementArray.size()) {
    elementArray[stackPointer] = element;
    stackPointer++;
  }
}

template <typename T, size_t N>
void StaticStack<T, N>::Reset() {
  stackPointer = 0;
}

template <typename T, size_t N>
T StaticStack<T, N>::Top() {
  return elementArray[stackPointer - 1];
}
