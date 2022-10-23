#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>

namespace Pinetime {
  namespace Drivers {
    template <typename T>
    concept IsFlashMemory = requires(T memory, uint32_t address, uint8_t* buffer, const uint8_t* constBuffer, size_t size) {
                              { memory.ReadIdentification() };
                              { memory.ReadStatusRegister() } -> std::same_as<uint8_t>;
                              { memory.ReadConfigurationRegister() } -> std::same_as<uint8_t>;
                              { memory.WriteInProgress() } -> std::same_as<bool>;
                              { memory.WriteEnabled() } -> std::same_as<bool>;
                              { memory.Read(address, buffer, size) };
                              { memory.Write(address, constBuffer, size) };
                              { memory.WriteEnable() };
                              { memory.SectorErase(address) };
                              { memory.ReadSecurityRegister() } -> std::same_as<uint8_t>;
                              { memory.ProgramFailed() } -> std::same_as<bool>;
                              { memory.EraseFailed() } -> std::same_as<bool>;
                              { memory.Init() };
                              { memory.Uninit() };
                              { memory.Sleep() };
                              { memory.Wakeup() };
                            };

    namespace Interface {
      template <class T>
        requires IsFlashMemory<T>
      class SpiNorFlash {
      public:
        explicit SpiNorFlash(T& spi) : impl {spi} {
        }
        SpiNorFlash(const SpiNorFlash&) = delete;
        SpiNorFlash& operator=(const SpiNorFlash&) = delete;
        SpiNorFlash(SpiNorFlash&&) = delete;
        SpiNorFlash& operator=(SpiNorFlash&&) = delete;

        struct __attribute__((packed)) Identification {
          uint8_t manufacturer = 0;
          uint8_t type = 0;
          uint8_t density = 0;
        };

        Identification ReadIdentificaion() {
          return impl.ReadIdentificaion();
        }

        uint8_t ReadStatusRegister() {
          return impl.ReadStatusRegister();
        }

        bool WriteInProgress() {
          return impl.WriteInProgress();
        }

        bool WriteEnabled() {
          return impl.WriteEnabled();
        }

        uint8_t ReadConfigurationRegister() {
          return impl.ReadConfigurationRegister();
        }

        void Read(uint32_t address, uint8_t* buffer, size_t size) {
          impl.Read(address, buffer, size);
        }

        void Write(uint32_t address, const uint8_t* buffer, size_t size) {
          impl.Write(address, buffer, size);
        }

        void WriteEnable() {
          return impl.WriteEnable();
        }

        void SectorErase(uint32_t sectorAddress) {
          impl.SectorErase(sectorAddress);
        }

        uint8_t ReadSecurityRegister() {
          return impl.ReadSecurityRegister();
        }

        bool ProgramFailed() {
          return impl.ProgramFailed();
        }

        bool EraseFailed() {
          return impl.EraseFailed();
        }

        void Init() {
          impl.Init();
        }

        void Uninit() {
          impl.Uninit();
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