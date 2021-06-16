/*  Copyright (C) 2020-2021 JF, Adam Pigg, Avamander

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

// 00000000-78fc-48fe-8e23-433b3a1942d0
#define MUSIC_SERVICE_UUID_BASE                                                                                                            \
  { 0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, 0x00, 0x00, 0x00, 0x00 }
#define MUSIC_SERVICE_CHAR_UUID(x, y)                                                                                                      \
  { 0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, x, y, 0x00, 0x00 }

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {

    class MusicService {
    public:
      explicit MusicService(Pinetime::System::SystemTask& system);

      void Init();

      int OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt);

      void event(char event);

      std::string getArtist() const;

      std::string getTrack() const;

      std::string getAlbum() const;

      int getProgress() const;

      int getTrackLength() const;

      float getPlaybackSpeed() const;

      bool isPlaying() const;

      static const char EVENT_MUSIC_OPEN = 0xe0;
      static const char EVENT_MUSIC_PLAY = 0x00;
      static const char EVENT_MUSIC_PAUSE = 0x01;
      static const char EVENT_MUSIC_NEXT = 0x03;
      static const char EVENT_MUSIC_PREV = 0x04;
      static const char EVENT_MUSIC_VOLUP = 0x05;
      static const char EVENT_MUSIC_VOLDOWN = 0x06;

      enum MusicStatus { NotPlaying = 0x00, Playing = 0x01 };

    private:
      ble_uuid128_t msUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_UUID_BASE};

      ble_uuid128_t msEventCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x01, 0x00)};
      ble_uuid128_t msStatusCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x02, 0x00)};
      ble_uuid128_t msArtistCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x03, 0x00)};
      ble_uuid128_t msTrackCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x04, 0x00)};
      ble_uuid128_t msAlbumCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x05, 0x00)};
      ble_uuid128_t msPositionCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x06, 0x00)};
      ble_uuid128_t msTotalLengthCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x07, 0x00)};
      ble_uuid128_t msTrackNumberCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x08, 0x00)};
      ble_uuid128_t msTrackTotalCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x09, 0x00)};
      ble_uuid128_t msPlaybackSpeedCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x0a, 0x00)};
      ble_uuid128_t msRepeatCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x0b, 0x00)};
      ble_uuid128_t msShuffleCharUuid {.u = {.type = BLE_UUID_TYPE_128}, .value = MUSIC_SERVICE_CHAR_UUID(0x0c, 0x00)};

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

      Pinetime::System::SystemTask& m_system;
    };
  }
}
