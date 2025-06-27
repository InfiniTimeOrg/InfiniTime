#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class WatchFaceQRCode : public Screen {
      public:
        WatchFaceQRCode(Components::LittleVgl& lvgl,
                        Controllers::DateTime& dateTimeController,
                        const Controllers::Battery& batteryController,
                        const Controllers::Ble& bleController,
                        Controllers::Settings& settingsController,
                        Controllers::MotorController& motor);
        ~WatchFaceQRCode() override;

        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;
        bool OnButtonPushed() override;

        void Refresh() override;

      private:
        void BuzzBinary(uint8_t number) const;
        void RefreshQRCode() const;
        std::string MakeQRTimeText() const;

        Utility::DirtyValue<int> batteryPercentRemaining{};
        Utility::DirtyValue<bool> powerPresent{};
        Utility::DirtyValue<bool> bleState{};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime{};

        // Add more strings to this to be able to cycle through them with long taps.
        // Strings must be <=78 bytes long and encoded in ISO-8859-1.
        // ISO-8859-1 is ASCII compatible so if you don't use unusual characters you don't need to worry about that.
        // Has a limit of 255 items.
        std::vector<std::string> altTexts = {
          "https://www.youtube.com/watch?v=dQw4w9WgXcQ",
          "++++[>++++<-]>+[>+++++>++++>++<<<-]>-.>++++.---.>--.<++.------.<-------.>++++.",
          "While you were reading this message, I snuck a blueberry into your pocket."
        };
        // altTextIndex 0 means main watch screen
        unsigned int altTextIndex;
        bool altTextIndexUpdated;
        uint32_t altTextLastChangedTime;
        constexpr static uint32_t altTextTimeout = pdMS_TO_TICKS(60000);

        /// Quiet zone is the empty space around the QR code. This is expressed in pixels of the QR code itself.
        /// Should not be set to 0, that will render the code unscannable.
        constexpr static uint16_t quietZoneSize = 2;
        std::unique_ptr<lv_color_t[]> blackBuffer;
        std::unique_ptr<lv_color_t[]> whiteBuffer;

        Components::LittleVgl& lvgl;
        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::Settings& settingsController;
        Controllers::MotorController& motor;

        lv_task_t* taskRefresh;
      };

      /// Class for interacting with bits and nybbles in a byte array
      class BitByteArray {
      public:
        BitByteArray(uint16_t sizeBytes);

        uint16_t length() const;

        uint8_t& operator[](uint16_t index);
        uint8_t getByte(uint16_t index) const;
        void setByte(uint16_t index, uint8_t value);
        bool getBit(uint16_t index) const;
        void setBit(uint16_t index, bool value);
        uint8_t getNybble(uint16_t index) const;
        void setNybble(uint16_t index, uint8_t value);

      private:
        std::unique_ptr<uint8_t[]> byteArray;
        uint16_t size;
      };

      /// Small wrapper class to BitByteArray to interpret its bits as a 2D canvas.
      /// Can be used as a normal BitByteArray. Coordinates start with (0,0) at top left.
      class BitByteArray2D : BitByteArray {
      public:
        BitByteArray2D(uint16_t width, uint16_t height);

        bool getBit2D(uint16_t indexX, uint16_t indexY) const;
        void setBit2D(uint16_t indexX, uint16_t indexY, bool value);

        void FillBits(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool value);

      private:
        uint16_t width;
        uint16_t height;
      };

      /// QR code generator class. Only need to call QRCodeGenerator::GenerateCode("text").
      /// Can only generate 4L codes (33x33 modules, 78 byte contents).
      class QRCodeGenerator {
      public:
        // Not instantiable. More of a holder for data and functions related to GenerateCode().
        QRCodeGenerator() = delete;
        ~QRCodeGenerator() = delete;

        static std::shared_ptr<BitByteArray2D> GenerateCode(const std::string& text);

      private:
        static std::shared_ptr<BitByteArray> GenerateCodeData(const std::string& text);
        static void AppendCodeEC(std::shared_ptr<BitByteArray> baseData);
        static std::shared_ptr<BitByteArray2D> GenerateCodeImage(std::shared_ptr<BitByteArray> codeData);

        // intToAlpha[0] is junk, do not use
        constexpr static uint8_t intToAlpha[256] = {0, 0, 1, 25, 2, 50, 26, 198, 3, 223, 51, 238, 27, 104, 199, 75, 4, 100, 224, 14, 52,
                                                    141, 239, 129, 28, 193, 105, 248, 200, 8, 76, 113, 5, 138, 101, 47, 225, 36, 15, 33, 53,
                                                    147, 142, 218, 240, 18, 130, 69, 29, 181, 194, 125, 106, 39, 249, 185, 201, 154, 9, 120,
                                                    77, 228, 114, 166, 6, 191, 139, 98, 102, 221, 48, 253, 226, 152, 37, 179, 16, 145, 34,
                                                    136, 54, 208, 148, 206, 143, 150, 219, 189, 241, 210, 19, 92, 131, 56, 70, 64, 30, 66,
                                                    182, 163, 195, 72, 126, 110, 107, 58, 40, 84, 250, 133, 186, 61, 202, 94, 155, 159, 10,
                                                    21, 121, 43, 78, 212, 229, 172, 115, 243, 167, 87, 7, 112, 192, 247, 140, 128, 99, 13,
                                                    103, 74, 222, 237, 49, 197, 254, 24, 227, 165, 153, 119, 38, 184, 180, 124, 17, 68, 146,
                                                    217, 35, 32, 137, 46, 55, 63, 209, 91, 149, 188, 207, 205, 144, 135, 151, 178, 220, 252,
                                                    190, 97, 242, 86, 211, 171, 20, 42, 93, 158, 132, 60, 57, 83, 71, 109, 65, 162, 31, 45,
                                                    67, 216, 183, 123, 164, 118, 196, 23, 73, 236, 127, 12, 111, 246, 108, 161, 59, 82, 41,
                                                    157, 85, 170, 251, 96, 134, 177, 187, 204, 62, 90, 203, 89, 95, 176, 156, 169, 160, 81,
                                                    11, 245, 22, 235, 122, 117, 44, 215, 79, 174, 213, 233, 230, 231, 173, 232, 116, 214,
                                                    244, 234, 168, 80, 88, 175};
        constexpr static uint8_t alphaToInt[256] = {1, 2, 4, 8, 16, 32, 64, 128, 29, 58, 116, 232, 205, 135, 19, 38, 76, 152, 45, 90, 180,
                                                    117, 234, 201, 143, 3, 6, 12, 24, 48, 96, 192, 157, 39, 78, 156, 37, 74, 148, 53, 106,
                                                    212, 181, 119, 238, 193, 159, 35, 70, 140, 5, 10, 20, 40, 80, 160, 93, 186, 105, 210,
                                                    185, 111, 222, 161, 95, 190, 97, 194, 153, 47, 94, 188, 101, 202, 137, 15, 30, 60, 120,
                                                    240, 253, 231, 211, 187, 107, 214, 177, 127, 254, 225, 223, 163, 91, 182, 113, 226, 217,
                                                    175, 67, 134, 17, 34, 68, 136, 13, 26, 52, 104, 208, 189, 103, 206, 129, 31, 62, 124,
                                                    248, 237, 199, 147, 59, 118, 236, 197, 151, 51, 102, 204, 133, 23, 46, 92, 184, 109,
                                                    218, 169, 79, 158, 33, 66, 132, 21, 42, 84, 168, 77, 154, 41, 82, 164, 85, 170, 73, 146,
                                                    57, 114, 228, 213, 183, 115, 230, 209, 191, 99, 198, 145, 63, 126, 252, 229, 215, 179,
                                                    123, 246, 241, 255, 227, 219, 171, 75, 150, 49, 98, 196, 149, 55, 110, 220, 165, 87,
                                                    174, 65, 130, 25, 50, 100, 200, 141, 7, 14, 28, 56, 112, 224, 221, 167, 83, 166, 81,
                                                    162, 89, 178, 121, 242, 249, 239, 195, 155, 43, 86, 172, 69, 138, 9, 18, 36, 72, 144,
                                                    61, 122, 244, 245, 247, 243, 251, 235, 203, 139, 11, 22, 44, 88, 176, 125, 250, 233,
                                                    207, 131, 27, 54, 108, 216, 173, 71, 142, 1};
        constexpr static uint8_t generatorPolynomialAlpha[21] = {0, 17, 60, 79, 50, 61, 163, 26, 187, 202, 180, 221, 225, 83, 239, 156, 164,
                                                                 212, 212, 188, 190};
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::QRCode> {
      static constexpr WatchFace watchFace = WatchFace::QRCode;
      static constexpr const char* name = "QRCode";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceQRCode(controllers.lvgl,
                                            controllers.dateTimeController,
                                            controllers.batteryController,
                                            controllers.bleController,
                                            controllers.settingsController,
                                            controllers.motorController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}