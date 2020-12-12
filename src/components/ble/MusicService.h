/*  Copyright (C) 2020 JF, Adam Pigg, Avamander

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <cstdint>
#include <string>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#undef max
#undef min

//c7e50000-78fc-48fe-8e23-43b37a1942d0
#define MUSIC_SERVICE_UUID_BASE {0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, 0x00, 0x00, 0xe5, 0xc7}

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    
    class MusicService {
    public:
      explicit MusicService(Pinetime::System::SystemTask &system);
      
      void Init();
      
      int OnCommand(uint16_t conn_handle, uint16_t attr_handle,
                    struct ble_gatt_access_ctxt *ctxt);
      
      void event(char event);
      
      std::string getArtist();
      
      std::string getTrack();
      
      std::string getAlbum();
      
      int getProgress();
      
      int getTrackLength();
      
      float getPlaybackSpeed();
      
      bool isPlaying();
      
      static const char EVENT_MUSIC_OPEN = 0xe0;
      static const char EVENT_MUSIC_PLAY = 0x00;
      static const char EVENT_MUSIC_PAUSE = 0x01;
      static const char EVENT_MUSIC_NEXT = 0x03;
      static const char EVENT_MUSIC_PREV = 0x04;
      static const char EVENT_MUSIC_VOLUP = 0x05;
      static const char EVENT_MUSIC_VOLDOWN = 0x06;
      
      enum MusicStatus {
        NotPlaying = 0x00,
        Playing = 0x01
      };
    private:
      static constexpr uint8_t msId[2] = {0x00, 0x01};
      static constexpr uint8_t msEventCharId[2] = {0x00, 0x02};
      static constexpr uint8_t msStatusCharId[2] = {0x00, 0x03};
      static constexpr uint8_t msArtistCharId[2] = {0x00, 0x04};
      static constexpr uint8_t msTrackCharId[2] = {0x00, 0x05};
      static constexpr uint8_t msAlbumCharId[2] = {0x00, 0x06};
      static constexpr uint8_t msPositionCharId[2] = {0x00, 0x07};
      static constexpr uint8_t msTotalLengthCharId[2] = {0x00, 0x08};
      static constexpr uint8_t msTrackNumberCharId[2] = {0x00, 0x09};
      static constexpr uint8_t msTrackTotalCharId[2] = {0x00, 0x0a};
      static constexpr uint8_t msPlaybackSpeedCharId[2] = {0x00, 0x0b};
      static constexpr uint8_t msRepeatCharId[2] = {0x00, 0x0c};
      static constexpr uint8_t msShuffleCharId[2] = {0x00, 0x0d};
      
      ble_uuid128_t msUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      
      ble_uuid128_t msEventCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msStatusCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msArtistCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msTrackCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msAlbumCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msPositionCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msTotalLengthCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msTrackNumberCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msTrackTotalCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msPlaybackSpeedCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msRepeatCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      ble_uuid128_t msShuffleCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = MUSIC_SERVICE_UUID_BASE
      };
      
      struct ble_gatt_chr_def characteristicDefinition[14];
      struct ble_gatt_svc_def serviceDefinition[2];
      
      uint16_t eventHandle;
      
      std::string artistName;
      std::string albumName;
      std::string trackName;
      
      bool playing;
      
      int trackProgress;
      int trackLength;
      int trackNumber;
      int tracksTotal;
      
      float playbackSpeed;
      
      bool repeat;
      bool shuffle;
      
      Pinetime::System::SystemTask &m_system;
    };
  }
}

