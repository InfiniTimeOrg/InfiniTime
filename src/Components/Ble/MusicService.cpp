#include <SystemTask/SystemTask.h>
#include "MusicService.h"

int MSCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
  auto musicService = static_cast<Pinetime::Controllers::MusicService*>(arg);
  return musicService->OnCommand(conn_handle, attr_handle, ctxt);
}

Pinetime::Controllers::MusicService::MusicService(Pinetime::System::SystemTask &system) : m_system(system)
{
    msUuid.value[11] = msId[0];
    msUuid.value[12] = msId[1];
    msEventCharUuid.value[11] = msEventCharId[0];
    msEventCharUuid.value[12] = msEventCharId[1];
    msStatusCharUuid.value[11] = msStatusCharId[0];
    msStatusCharUuid.value[12] = msStatusCharId[1];
    msTrackCharUuid.value[11] = msTrackCharId[0];
    msTrackCharUuid.value[12] = msTrackCharId[1];
    msArtistCharUuid.value[11] = msArtistCharId[0];
    msArtistCharUuid.value[12] = msArtistCharId[1];
    msAlbumCharUuid.value[11] = msAlbumCharId[0];
    msAlbumCharUuid.value[12] = msAlbumCharId[1];

    characteristicDefinition[0] = { .uuid = (ble_uuid_t*)(&msEventCharUuid),
                                    .access_cb = MSCallback,
                                    .arg = this,
                                    .flags =  BLE_GATT_CHR_F_NOTIFY,
                                    .val_handle = &m_eventHandle
    };
    characteristicDefinition[1] = { .uuid = (ble_uuid_t*)(&msStatusCharUuid),
                                    .access_cb = MSCallback,
                                    .arg = this,
                                    .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
    };
    characteristicDefinition[2] = { .uuid = (ble_uuid_t*)(&msTrackCharUuid),
                                    .access_cb = MSCallback,
                                    .arg = this,
                                    .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
    };
    characteristicDefinition[3] = { .uuid = (ble_uuid_t*)(&msArtistCharUuid),
                                    .access_cb = MSCallback,
                                    .arg = this,
                                    .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
    };
    characteristicDefinition[4] = { .uuid = (ble_uuid_t*)(&msAlbumCharUuid),
                                    .access_cb = MSCallback,
                                    .arg = this,
                                    .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
    };
    characteristicDefinition[5] = {0};

    serviceDefinition[0] = {
                        .type = BLE_GATT_SVC_TYPE_PRIMARY,
                        .uuid = (ble_uuid_t *) &msUuid,
                        .characteristics = characteristicDefinition
    };
    serviceDefinition[1] = {0};

    m_artist = "Waiting for";
    m_album = "";
    m_track = "track information...";
}

void Pinetime::Controllers::MusicService::Init()
{
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
        NRF_LOG_INFO("DATA : %s", s);
        if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *)&msArtistCharUuid) == 0) {
            m_artist = s;
        } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *)&msTrackCharUuid) == 0) {
            m_track = s;
        } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *)&msAlbumCharUuid) == 0) {
            m_album = s;
        } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *)&msStatusCharUuid) == 0) {
            m_status = s[0];
        }
  }
  return 0;
}

std::string Pinetime::Controllers::MusicService::album()
{
    return m_album;
}

std::string Pinetime::Controllers::MusicService::artist()
{
    return m_artist;
}

std::string Pinetime::Controllers::MusicService::track()
{
    return m_track;
}

unsigned char Pinetime::Controllers::MusicService::status()
{
    return m_status;
}

void Pinetime::Controllers::MusicService::event(char event)
{
    auto *om = ble_hs_mbuf_from_flat(&event, 1);
    int ret;

    uint16_t connectionHandle = m_system.nimble().connHandle();

    if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
        return;
    }

    ret = ble_gattc_notify_custom(connectionHandle, m_eventHandle, om);
}

