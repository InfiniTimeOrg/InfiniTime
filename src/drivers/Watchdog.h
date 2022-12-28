#pragma once
#include <concepts>
#include <cstdint>

namespace Pinetime {
  namespace Drivers {
    namespace Watchdogs {
      enum class ResetReasons { ResetPin, Watchdog, SoftReset, CpuLockup, SystemOff, LpComp, DebugInterface, NFC, HardReset };
    }

    template <typename WatchdogImpl>
    concept IsWatchdog = requires(WatchdogImpl watchdog) {
                           { watchdog.Start() };
                           { watchdog.Kick() };
                           { watchdog.ResetReason() } -> std::same_as<Watchdogs::ResetReasons>;
                           };

    namespace Interface {
      template <class T>
        requires IsWatchdog<T>
      class Watchdog {
      public:
        explicit Watchdog(T& impl) : impl {impl} {}
        Watchdog(const Watchdog&) = delete;
        Watchdog& operator=(const Watchdog&) = delete;
        Watchdog(Watchdog&&) = delete;
        Watchdog& operator=(Watchdog&&) = delete;

        void Setup(uint8_t timeoutSeconds) {
          impl.Setup(timeoutSeconds);
        }

        void Start() {
          impl.Start();
        }

        void Kick() {
          impl.Kick();
        }

        Watchdogs::ResetReasons ResetReason() const {
          return impl.ResetReason();
        }

        static const char* ResetReasonToString(Watchdogs::ResetReasons reason) {
          switch (reason) {
            case Watchdogs::ResetReasons::ResetPin:
              return "Reset pin";
            case Watchdogs::ResetReasons::Watchdog:
              return "Watchdog";
            case Watchdogs::ResetReasons::DebugInterface:
              return "Debug interface";
            case Watchdogs::ResetReasons::LpComp:
              return "LPCOMP";
            case Watchdogs::ResetReasons::SystemOff:
              return "System OFF";
            case Watchdogs::ResetReasons::CpuLockup:
              return "CPU Lock-up";
            case Watchdogs::ResetReasons::SoftReset:
              return "Soft reset";
            case Watchdogs::ResetReasons::NFC:
              return "NFC";
            case Watchdogs::ResetReasons::HardReset:
              return "Hard reset";
            default:
              return "Unknown";
          }
        }

      private:
        T& impl;
      };
    }
  }
}
