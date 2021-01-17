#pragma once

#include <cstdint>
#include <string>
#include <lvgl/lvgl.h>
#include "Screen.h"
#include "components/ble/QrService.h"
#include "qrcodegen.h"

#define MAXLISTITEMS 4

namespace Pinetime {
  namespace Components {
    class LittleVgl;
  }
  namespace Applications {
    namespace Screens {

      class Qr : public Screen {
      public:
        Qr(DisplayApp* app, Pinetime::Components::LittleVgl& lvgl,
                            Pinetime::Controllers::QrService& qrService);

        ~Qr() override;

        bool Refresh() override;

        bool OnButtonPushed() override;

        bool OnTouchEvent(TouchEvents event) override;

        bool OnTouchEvent(uint16_t x, uint16_t y) override;

        void drawQrList();
        void drawQr(std::string qrText);
        
        void resetScreen();

        void OnButtonEvent(lv_obj_t * object, lv_event_t event);
        
        bool showingQrCode = false;
        
      private:

        std::array<Pinetime::Controllers::QrService::QrInfo, 4> qrList;
        lv_obj_t * itemApps[MAXLISTITEMS];
        
        Pinetime::Components::LittleVgl& lvgl;
        Pinetime::Controllers::QrService& qrService;

        bool running = true;

        uint8_t qrSize;
        uint8_t qrModuleSize;
        uint8_t bufferSize;
        uint8_t offset;
        lv_area_t area;

        static constexpr uint16_t border = 1;

        uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
        uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
      };
    }
  }
}
