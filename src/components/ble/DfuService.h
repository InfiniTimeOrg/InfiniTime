#pragma once

#include <cstdint>
#include <array>

#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Drivers {
    class SpiNorFlash;
  }

  namespace Controllers {
    class Ble;

    class DfuService {
    public:
      DfuService(Pinetime::System::SystemTask& systemTask,
                 Pinetime::Controllers::Ble& bleController,
                 Pinetime::Drivers::SpiNorFlash& spiNorFlash);
      void Init();
      int OnServiceData(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      void OnTimeout();
      void Reset();

      class NotificationManager {
      public:
        NotificationManager();
        bool AsyncSend(uint16_t connection, uint16_t charactHandle, uint8_t* data, size_t size);
        void Send(uint16_t connection, uint16_t characteristicHandle, const uint8_t* data, const size_t s);

      private:
        TimerHandle_t timer;
        uint16_t connectionHandle = 0;
        uint16_t characteristicHandle = 0;
        size_t size = 0;
        uint8_t buffer[10];

      public:
        void OnNotificationTimer();
        void Reset();
      };

      class DfuImage {
      public:
        DfuImage(Pinetime::Drivers::SpiNorFlash& spiNorFlash) : spiNorFlash {spiNorFlash} {
        }

        void Init(size_t chunkSize, size_t totalSize, uint16_t expectedCrc);
        void Erase();
        void Append(uint8_t* data, size_t size);
        bool Validate();
        bool IsComplete();

      private:
        Pinetime::Drivers::SpiNorFlash& spiNorFlash;
        static constexpr size_t bufferSize = 200;
        bool ready = false;
        size_t chunkSize = 0;
        size_t totalSize = 0;
        size_t maxSize = 475136;
        size_t bufferWriteIndex = 0;
        size_t totalWriteIndex = 0;
        static constexpr size_t writeOffset = 0x40000;
        uint8_t tempBuffer[bufferSize];
        uint16_t expectedCrc = 0;

        void WriteMagicNumber();
        uint16_t ComputeCrc(uint8_t const* p_data, uint32_t size, uint16_t const* p_crc);
      };

    private:
      Pinetime::System::SystemTask& systemTask;
      Pinetime::Controllers::Ble& bleController;
      DfuImage dfuImage;
      NotificationManager notificationManager;

      static constexpr uint16_t dfuServiceId {0x1530};
      static constexpr uint16_t packetCharacteristicId {0x1532};
      static constexpr uint16_t controlPointCharacteristicId {0x1531};
      static constexpr uint16_t revisionCharacteristicId {0x1534};

      uint16_t revision {0x0008};

      static constexpr ble_uuid128_t serviceUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x30, 0x15, 0x00, 0x00}};

      static constexpr ble_uuid128_t packetCharacteristicUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x32, 0x15, 0x00, 0x00}};

      static constexpr ble_uuid128_t controlPointCharacteristicUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x31, 0x15, 0x00, 0x00}};

      static constexpr ble_uuid128_t revisionCharacteristicUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x34, 0x15, 0x00, 0x00}};

      struct ble_gatt_chr_def characteristicDefinition[4];
      struct ble_gatt_svc_def serviceDefinition[2];
      uint16_t packetCharacteristicHandle;
      uint16_t controlPointCharacteristicHandle;
      uint16_t revisionCharacteristicHandle;

      enum class States : uint8_t { Idle, Init, Start, Data, Validate, Validated };
      States state = States::Idle;

      enum class ImageTypes : uint8_t {
        NoImage = 0x00,
        SoftDevice = 0x01,
        Bootloader = 0x02,
        SoftDeviceAndBootloader = 0x03,
        Application = 0x04
      };

      enum class Opcodes : uint8_t {
        StartDFU = 0x01,
        InitDFUParameters = 0x02,
        ReceiveFirmwareImage = 0x03,
        ValidateFirmware = 0x04,
        ActivateImageAndReset = 0x05,
        PacketReceiptNotificationRequest = 0x08,
        Response = 0x10,
        PacketReceiptNotification = 0x11
      };

      enum class ErrorCodes {
        NoError = 0x01,
        InvalidState = 0x02,
        NotSupported = 0x03,
        DataSizeExceedsLimits = 0x04,
        CrcError = 0x05,
        OperationFailed = 0x06
      };

      uint8_t nbPacketsToNotify = 0;
      uint32_t nbPacketReceived = 0;
      uint32_t bytesReceived = 0;

      uint32_t softdeviceSize = 0;
      uint32_t bootloaderSize = 0;
      uint32_t applicationSize = 0;
      uint16_t expectedCrc = 0;

      int SendDfuRevision(os_mbuf* om) const;
      int WritePacketHandler(uint16_t connectionHandle, os_mbuf* om);
      int ControlPointHandler(uint16_t connectionHandle, os_mbuf* om);

      TimerHandle_t timeoutTimer;
    };
  }
}