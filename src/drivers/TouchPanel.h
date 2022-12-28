#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>

namespace Pinetime {
  namespace Drivers {
    template <typename touchpanelImpl>
    concept IsTouchPanel = requires(touchpanelImpl touchpanel) {
                             { touchpanel.Init() };
                             { touchpanel.GetTouchInfo() };
                             { touchpanel.Sleep() };
                             { touchpanel.Wakeup() };
                             { touchpanel.GetChipId() };
                             { touchpanel.GetVendorId() };
                             { touchpanel.GetFwVersion() };
                           };

    namespace TouchPanels {
      enum class Gestures : uint8_t {
        None = 0x00,
        SlideDown = 0x01,
        SlideUp = 0x02,
        SlideLeft = 0x03,
        SlideRight = 0x04,
        SingleTap = 0x05,
        DoubleTap = 0x0B,
        LongPress = 0x0C
      };
      struct TouchInfos {
        uint16_t x = 0;
        uint16_t y = 0;
        Gestures gesture = Gestures::None;
        bool touching = false;
        bool isValid = false;
      };
    }

    namespace Interface {
      template <class T>
        requires IsTouchPanel<T>
      class Touchpanel {
      public:
        explicit Touchpanel(T& impl) : impl {impl} {}
        Touchpanel(const Touchpanel&) = delete;
        Touchpanel& operator=(const Touchpanel&) = delete;
        Touchpanel(Touchpanel&&) = delete;
        Touchpanel& operator=(Touchpanel&&) = delete;

        bool Init() {
          return impl.Init();
        }

        TouchPanels::TouchInfos GetTouchInfo() {
          return impl.GetTouchInfo();
        }

        void Sleep() {
          impl.Sleep();
        }

        void Wakeup() {
          impl.Wakeup();
        }

        uint8_t GetChipId() const {
          return impl.GetChipId();
        }

        uint8_t GetVendorId() const {
          return impl.GetVendorId();
        }

        uint8_t GetFwVersion() const {
          return impl.GetFwVersion();
        }

      private:
        T& impl;
      };
    }
  }
}
