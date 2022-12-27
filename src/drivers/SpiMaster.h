#pragma once
#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace Pinetime {
  namespace Drivers {
    template <typename SpiImpl>
    concept IsSpiMaster =
      requires(SpiImpl spi, uint8_t pin, const uint8_t* constData, uint8_t* data, const uint8_t* constCommand, uint8_t* command, size_t size) {
        { spi.Init() } -> std::same_as<bool>;
        { spi.Write(pin, constData, size) } -> std::same_as<bool>;
        { spi.Read(pin, command, size, data, size) } -> std::same_as<bool>;
        { spi.WriteCmdAndBuffer(pin, constCommand, size, constData, size) } -> std::same_as<bool>;
        { spi.OnStartedEvent() };
        { spi.OnEndEvent() };
        { spi.Sleep() };
        { spi.Wakeup() };
      };

    namespace Interface {
      template <class T>
        requires IsSpiMaster<T>
      class SpiMaster {
      public:
        SpiMaster(T& spiMaster) : impl {spiMaster} {
        }
        SpiMaster(const SpiMaster&) = delete;
        SpiMaster& operator=(const SpiMaster&) = delete;
        SpiMaster(SpiMaster&&) = delete;
        SpiMaster& operator=(SpiMaster&&) = delete;

        bool Init() {
          return impl.Init();
        }

        bool Write(uint8_t pinCsn, const uint8_t* data, size_t size) {
          return impl.Write(pinCsn, data, size);
        }

        bool Read(uint8_t pinCsn, uint8_t* cmd, size_t cmdSize, uint8_t* data, size_t dataSize) {
          return impl.Read(pinCsn, cmd, cmdSize, data, dataSize);
        }

        bool WriteCmdAndBuffer(uint8_t pinCsn, const uint8_t* cmd, size_t cmdSize, const uint8_t* data, size_t dataSize) {
          return impl.WriteCmdAndBuffer(pinCsn, cmd, cmdSize, data, dataSize);
        }

        void OnStartedEvent() {
          impl.OnStartedEvent();
        }

        void OnEndEvent() {
          impl.OnEndEvent();
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
