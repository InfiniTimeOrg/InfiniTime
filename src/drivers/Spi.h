#pragma once
#include <cstdint>
#include <cstddef>
#include <utility>
#include <concepts>

namespace Pinetime {
  namespace Drivers {
    template <typename T>
    concept IsSpi = requires(T s, const uint8_t* constData, uint8_t* data, const uint8_t* constCommand, uint8_t* command, size_t size) {
                      { s.Write(constData, size) } -> std::same_as<bool>;
                      { s.Read(command, size, data, size) } -> std::same_as<bool>;
                    };

    namespace Interface {
      template <class T>
        requires IsSpi<T>
      class Spi {
      public:
        Spi(T& spi) : impl {spi} {
        }
        Spi(const Spi&) = delete;
        Spi& operator=(const Spi&) = delete;
        Spi(Spi&&) = delete;
        Spi& operator=(Spi&&) = delete;

        bool Init() {
          return impl.Init();
        }

        bool Write(const uint8_t* data, size_t size) {
          return impl.Write(data, size);
        }

        bool Read(uint8_t* cmd, size_t cmdSize, uint8_t* data, size_t dataSize) {
          return impl.Read(cmd, cmdSize, data, dataSize);
        }

        bool WriteCmdAndBuffer(const uint8_t* cmd, size_t cmdSize, const uint8_t* data, size_t dataSize) {
          return impl.WriteCmdAndBuffer(cmd, cmdSize, data, dataSize);
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