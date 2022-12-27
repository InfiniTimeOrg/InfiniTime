#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>

namespace Pinetime {
  namespace Drivers {
    template <typename TwiImpl>
      concept IsTwi = requires(TwiImpl twi, uint8_t deviceAddress, uint8_t registerAddress, uint8_t* data, const uint8_t* constData, size_t size) {
                      { twi.Init() };
                      { twi.Write(deviceAddress, registerAddress, constData, size) };
                      { twi.Read(deviceAddress, registerAddress, data, size) };
                      { twi.Sleep() };
                      { twi.Wakeup() };
                    };

    namespace Interface {
      template <class T>
        requires IsTwi<T>
      class TwiMaster {
      public:
        explicit TwiMaster(T& impl) : impl {impl} {}
        TwiMaster(const TwiMaster&) = delete;
        TwiMaster& operator=(const TwiMaster&) = delete;
        TwiMaster(TwiMaster&&) = delete;
        TwiMaster& operator=(TwiMaster&&) = delete;

        enum class ErrorCodes { NoError, TransactionFailed };

        void Init() {
          impl.Init();
        }

        ErrorCodes Read(uint8_t deviceAddress, uint8_t registerAddress, uint8_t* buffer, size_t size) {
          return static_cast<ErrorCodes>(impl.Read(deviceAddress, registerAddress, buffer, size));
        }

        ErrorCodes Write(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t* data, size_t size) {
          return static_cast<ErrorCodes>(impl.Write(deviceAddress, registerAddress, data, size));
        }

        void Sleep() {
          impl.Sleep();
        }

        void Wakeup() {
          impl.WakeUp();
        }

      private:
        T& impl;
      };
    }
  }
}
