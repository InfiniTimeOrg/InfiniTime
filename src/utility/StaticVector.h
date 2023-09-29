#include <array>
#include <cstddef>

namespace Pinetime {
  namespace Utility {
    template <typename T, size_t N>
    class StaticVector {
    public:
      // Return true if successful
      bool TryPush(T element);
      void Erase(size_t index);
      T& operator[](size_t index);
      const T& operator[](size_t index) const;

      size_t Size() const {
        return nElements;
      }

    private:
      std::array<T, N> elementArray;
      size_t nElements = 0;
    };

    template <typename T, size_t N>
    bool StaticVector<T, N>::TryPush(T element) {
      if (nElements < elementArray.size()) {
        elementArray[nElements] = element;
        nElements++;
        return true;
      }
      return false;
    }

    template <typename T, size_t N>
    void StaticVector<T, N>::Erase(size_t index) {
      if (index < nElements) {
        for (size_t i = index; i < nElements - 1; i++) {
          elementArray[i] = elementArray[i + 1];
        }
        nElements--;
      }
    }

    template <typename T, size_t N>
    T& StaticVector<T, N>::operator[](size_t index) {
      return elementArray[index];
    }

    template <typename T, size_t N>
    const T& StaticVector<T, N>::operator[](size_t index) const {
      return elementArray[index];
    }

    // If the vector is full, erase the oldest entry so the push will succeed
    template <typename T, size_t N>
    void ForcePush(StaticVector<T, N>& staticVector, T element) {
      while (!staticVector.TryPush(element)) {
        staticVector.Erase(0);
      }
    }
  }
}
