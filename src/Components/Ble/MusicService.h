#pragma once

#include <cstdint>
#include <array>
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#include <string>

//c7e50000-78fc-48fe-8e23-43b37a1942d0
#define MUSIC_SERVICE_UUID_BASE {0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, 0x00, 0x00, 0xe5, 0xc7}

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {

    class MusicService {
      public:
        MusicService(Pinetime::System::SystemTask &system);
        void Init();
        int OnCommand(uint16_t conn_handle, uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt);

        std::string artist();
        std::string track();
        std::string album();
        unsigned char status();

        void event(char event);

        static const char EVENT_MUSIC_OPEN = 0xe0;
        static const char EVENT_MUSIC_PLAY = 0x00;
        static const char EVENT_MUSIC_PAUSE = 0x01;
        static const char EVENT_MUSIC_NEXT = 0x03;
        static const char EVENT_MUSIC_PREV = 0x04;
        static const char EVENT_MUSIC_VOLUP = 0x05;
        static const char EVENT_MUSIC_VOLDOWN = 0x06;
        static const char STATUS_MUSIC_PAUSED = 0x00;
        static const char STATUS_MUSIC_PLAYING = 0x01;

      private:
        static constexpr uint8_t msId[2] = {0x00, 0x01};
        static constexpr uint8_t msEventCharId[2] = {0x00, 0x02};
        static constexpr uint8_t msStatusCharId[2] = {0x00, 0x03};
        static constexpr uint8_t msArtistCharId[2] = {0x00, 0x04};
        static constexpr uint8_t msTrackCharId[2] = {0x00, 0x05};
        static constexpr uint8_t msAlbumCharId[2] = {0x00, 0x06};

        ble_uuid128_t msUuid {
                .u = { .type = BLE_UUID_TYPE_128 },
                .value = MUSIC_SERVICE_UUID_BASE
        };

        ble_uuid128_t msEventCharUuid {
                .u = { .type = BLE_UUID_TYPE_128 },
                .value = MUSIC_SERVICE_UUID_BASE
        };
        ble_uuid128_t msStatusCharUuid {
                .u = { .type = BLE_UUID_TYPE_128 },
                .value = MUSIC_SERVICE_UUID_BASE
        };
        ble_uuid128_t msArtistCharUuid {
                .u = { .type = BLE_UUID_TYPE_128 },
                .value = MUSIC_SERVICE_UUID_BASE
        };
        ble_uuid128_t msTrackCharUuid {
                .u = { .type = BLE_UUID_TYPE_128 },
                .value = MUSIC_SERVICE_UUID_BASE
        };
        ble_uuid128_t msAlbumCharUuid {
                .u = { .type = BLE_UUID_TYPE_128 },
                .value = MUSIC_SERVICE_UUID_BASE
        };

        struct ble_gatt_chr_def characteristicDefinition[6];
        struct ble_gatt_svc_def serviceDefinition[2];

        uint16_t m_eventHandle;

        std::string m_artist;
        std::string m_album;
        std::string m_track;

        unsigned char m_status;

        Pinetime::System::SystemTask& m_system;

    };
  }
}

