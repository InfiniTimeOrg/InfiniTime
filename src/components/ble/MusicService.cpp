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
#include "MusicService.h"
#include "systemtask/SystemTask.h"

int MSCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
  auto musicService = static_cast<Pinetime::Controllers::MusicService *>(arg);
  return musicService->OnCommand(conn_handle, attr_handle, ctxt);
}

Pinetime::Controllers::MusicService::MusicService(Pinetime::System::SystemTask &system) : m_system(system) {
  msUuid.value[14] = msId[0];
  msUuid.value[15] = msId[1];

  msEventCharUuid.value[12] = msEventCharId[0];
  msEventCharUuid.value[13] = msEventCharId[1];
  msEventCharUuid.value[14] = msId[0];
  msEventCharUuid.value[15] = msId[1];

  msStatusCharUuid.value[12] = msStatusCharId[0];
  msStatusCharUuid.value[13] = msStatusCharId[1];
  msStatusCharUuid.value[14] = msId[0];
  msStatusCharUuid.value[15] = msId[1];

  msTrackCharUuid.value[12] = msTrackCharId[0];
  msTrackCharUuid.value[13] = msTrackCharId[1];
  msTrackCharUuid.value[14] = msId[0];
  msTrackCharUuid.value[15] = msId[1];

  msArtistCharUuid.value[12] = msArtistCharId[0];
  msArtistCharUuid.value[13] = msArtistCharId[1];
  msArtistCharUuid.value[14] = msId[0];
  msArtistCharUuid.value[15] = msId[1];

  msAlbumCharUuid.value[12] = msAlbumCharId[0];
  msAlbumCharUuid.value[13] = msAlbumCharId[1];
  msAlbumCharUuid.value[14] = msId[0];
  msAlbumCharUuid.value[15] = msId[1];

  msPositionCharUuid.value[12] = msPositionCharId[0];
  msPositionCharUuid.value[13] = msPositionCharId[1];
  msPositionCharUuid.value[14] = msId[0];
  msPositionCharUuid.value[15] = msId[1];

  msTotalLengthCharUuid.value[12] = msTotalLengthCharId[0];
  msTotalLengthCharUuid.value[13] = msTotalLengthCharId[1];
  msTotalLengthCharUuid.value[14] = msId[0];
  msTotalLengthCharUuid.value[15] = msId[1];

  msTrackNumberCharUuid.value[12] = msTrackNumberCharId[0];
  msTrackNumberCharUuid.value[13] = msTrackNumberCharId[1];
  msTrackNumberCharUuid.value[14] = msId[0];
  msTrackNumberCharUuid.value[15] = msId[1];

  msTrackTotalCharUuid.value[12] = msTrackTotalCharId[0];
  msTrackTotalCharUuid.value[13] = msTrackTotalCharId[1];
  msTrackTotalCharUuid.value[14] = msId[0];
  msTrackTotalCharUuid.value[15] = msId[1];

  msPlaybackSpeedCharUuid.value[12] = msPlaybackSpeedCharId[0];
  msPlaybackSpeedCharUuid.value[13] = msPlaybackSpeedCharId[1];
  msPlaybackSpeedCharUuid.value[14] = msId[0];
  msPlaybackSpeedCharUuid.value[15] = msId[1];

  msRepeatCharUuid.value[12] = msRepeatCharId[0];
  msRepeatCharUuid.value[13] = msRepeatCharId[1];
  msRepeatCharUuid.value[14] = msId[0];
  msRepeatCharUuid.value[15] = msId[1];

  msShuffleCharUuid.value[12] = msShuffleCharId[0];
  msShuffleCharUuid.value[13] = msShuffleCharId[1];
  msShuffleCharUuid.value[14] = msId[0];
  msShuffleCharUuid.value[15] = msId[1];
  
  characteristicDefinition[0] = {.uuid = (ble_uuid_t *) (&msEventCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags =  BLE_GATT_CHR_F_NOTIFY,
      .val_handle = &eventHandle
  };
  characteristicDefinition[1] = {.uuid = (ble_uuid_t *) (&msStatusCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[2] = {.uuid = (ble_uuid_t *) (&msTrackCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[3] = {.uuid = (ble_uuid_t *) (&msArtistCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[4] = {.uuid = (ble_uuid_t *) (&msAlbumCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[5] = {.uuid = (ble_uuid_t *) (&msPositionCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[6] = {.uuid = (ble_uuid_t *) (&msTotalLengthCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[7] = {.uuid = (ble_uuid_t *) (&msTotalLengthCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[8] = {.uuid = (ble_uuid_t *) (&msTrackNumberCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[9] = {.uuid = (ble_uuid_t *) (&msTrackTotalCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[10] = {.uuid = (ble_uuid_t *) (&msPlaybackSpeedCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[11] = {.uuid = (ble_uuid_t *) (&msRepeatCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[12] = {.uuid = (ble_uuid_t *) (&msShuffleCharUuid),
      .access_cb = MSCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[13] = {0};
  
  serviceDefinition[0] = {
      .type = BLE_GATT_SVC_TYPE_PRIMARY,
      .uuid = (ble_uuid_t *) &msUuid,
      .characteristics = characteristicDefinition
  };
  serviceDefinition[1] = {0};
  
  artistName = "Waiting for";
  albumName = "";
  trackName = "track information...";
  playing = false;
  repeat = false;
  shuffle = false;
  playbackSpeed = 1.0f;
  trackProgress = 0;
  trackLength = 0;
}

void Pinetime::Controllers::MusicService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);
  
  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int Pinetime::Controllers::MusicService::OnCommand(uint16_t conn_handle, uint16_t attr_handle,
                                                   struct ble_gatt_access_ctxt *ctxt) {
  
  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    size_t notifSize = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t data[notifSize + 1];
    data[notifSize] = '\0';
    os_mbuf_copydata(ctxt->om, 0, notifSize, data);
    char *s = (char *) &data[0];
    if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msArtistCharUuid) == 0) {
      artistName = s;
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msTrackCharUuid) == 0) {
      trackName = s;
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msAlbumCharUuid) == 0) {
      albumName = s;
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msStatusCharUuid) == 0) {
      playing = s[0];
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msRepeatCharUuid) == 0) {
      repeat = s[0];
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msShuffleCharUuid) == 0) {
      shuffle = s[0];
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msPositionCharUuid) == 0) {
      trackProgress = (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3];
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msTotalLengthCharUuid) == 0) {
      trackLength = (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3];
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msTrackNumberCharUuid) == 0) {
      trackNumber = (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3];
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msTrackTotalCharUuid) == 0) {
      tracksTotal = (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3];
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &msPlaybackSpeedCharUuid) == 0) {
      playbackSpeed = static_cast<float>(((s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3])) / 100.0f;
    }
  }
  return 0;
}

std::string Pinetime::Controllers::MusicService::getAlbum() {
  return albumName;
}

std::string Pinetime::Controllers::MusicService::getArtist() {
  return artistName;
}

std::string Pinetime::Controllers::MusicService::getTrack() {
  return trackName;
}

bool Pinetime::Controllers::MusicService::isPlaying() {
  return playing;
}

float Pinetime::Controllers::MusicService::getPlaybackSpeed() {
  return playbackSpeed;
}

void Pinetime::Controllers::MusicService::event(char event) {
  auto *om = ble_hs_mbuf_from_flat(&event, 1);
  
  uint16_t connectionHandle = m_system.nimble().connHandle();
  
  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }
  
  ble_gattc_notify_custom(connectionHandle, eventHandle, om);
}

int Pinetime::Controllers::MusicService::getProgress() {
  return trackProgress;
}

int Pinetime::Controllers::MusicService::getTrackLength() {
  return trackLength;
}

