#pragma once

#include <cstdint>
#include <string>
#include <lvgl/lvgl.h>
#include "Screen.h"
#include "components/ble/QrService.h"
#include "libs/QR-Code-generator/c/qrcodegen.h"

#define MAXLISTITEMS 4

namespace Pinetime {
  namespace Components {
    class LittleVgl;
  }
  namespace Applications {
    namespace Screens {

      class Qr : public Screen {
      public:
        Qr(DisplayApp* app, Pinetime::Components::LittleVgl& lvgl, Pinetime::Controllers::QrService& qrService);

        ~Qr() override;

        void Refresh() override;

        bool OnButtonPushed() override;

        bool OnTouchEvent(TouchEvents event) override;

        bool OnTouchEvent(uint16_t x, uint16_t y) override;

        void drawQrList();
        void drawQr(std::string qrText);

        void resetScreen();

        void OnButtonEvent(lv_obj_t* object, lv_event_t event);

        bool showingQrCode = false;

      private:
        lv_task_t* taskRefresh;
        std::array<Pinetime::Controllers::QrService::QrInfo, 4> qrList;
        lv_obj_t* itemApps[MAXLISTITEMS];

        Pinetime::Components::LittleVgl& lvgl;
        Pinetime::Controllers::QrService& qrService;

        uint8_t qrSize;
        uint8_t qrModuleSize;
        uint8_t offset;
        lv_area_t area;

        static constexpr uint16_t border = 1;
        static constexpr uint16_t maxPixelsPerQrModuleSide = LV_HOR_RES_MAX / (21 + 2);

        static constexpr uint8_t colorBufferSize = maxPixelsPerQrModuleSide * maxPixelsPerQrModuleSide;
        lv_color_t colorBuffer[colorBufferSize];

        static constexpr uint8_t qrVersionMin = 1;
        static constexpr uint8_t qrVersionMax = 25;
        uint8_t qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(qrVersionMax)];
        uint8_t tempBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(qrVersionMax)];
      };
    }
  }
}
