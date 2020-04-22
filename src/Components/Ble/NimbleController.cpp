#include <Components/DateTime/DateTimeController.h>

#include "NimbleController.h"
#include <services/gatt/ble_svc_gatt.h>
#include <services/gap/ble_svc_gap.h>
#include <host/util/util.h>
#include <host/ble_hs_id.h>
#include <host/ble_hs.h>
#include <host/ble_gap.h>
#include <hal/nrf_rtc.h>

using namespace Pinetime::Controllers;

NimbleController::NimbleController(DateTime& datetimeController) : dateTimeController{datetimeController} {
  ctsUuid.u.type = BLE_UUID_TYPE_16;
  ctsUuid.value = BleGatServiceCts;

  ctsCurrentTimeUuid.u.type = BLE_UUID_TYPE_16;
  ctsCurrentTimeUuid.value = bleGattCharacteristicCurrentTime;
}

int GAPEventCallback(struct ble_gap_event *event, void *arg) {
  auto nimbleController = static_cast<NimbleController*>(arg);
  return nimbleController->OnGAPEvent(event);
}

int DiscoveryEventCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                     const struct ble_gatt_svc *service, void *arg) {
  auto nimbleController = static_cast<NimbleController*>(arg);
  return nimbleController->OnDiscoveryEvent(conn_handle, error, service);
}

int CharacteristicDiscoveredCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                     const struct ble_gatt_chr *chr, void *arg) {
  auto nimbleController = static_cast<NimbleController*>(arg);
  return nimbleController->OnCharacteristicDiscoveryEvent(conn_handle, error, chr);
}

static int CurrentTimeReadCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                        struct ble_gatt_attr *attr, void *arg) {
  auto nimbleController = static_cast<NimbleController*>(arg);
  return nimbleController->OnCurrentTimeReadResult(conn_handle, error, attr);
}

void NimbleController::Init() {
  while (!ble_hs_synced()) {}

  ble_svc_gap_init();
  ble_svc_gatt_init();

  int res;
  res = ble_hs_util_ensure_addr(0);
  res = ble_hs_id_infer_auto(0, &addrType);
  res = ble_svc_gap_device_name_set(deviceName);
}

void NimbleController::StartAdvertising() {
  ble_svc_gap_device_name_set("Pinetime-JF");

  /* set adv parameters */
  struct ble_gap_adv_params adv_params;
  struct ble_hs_adv_fields fields;
  /* advertising payload is split into advertising data and advertising
     response, because all data cannot fit into single packet; name of device
     is sent as response to scan request */
  struct ble_hs_adv_fields rsp_fields;

  /* fill all fields and parameters with zeros */
  memset(&adv_params, 0, sizeof(adv_params));
  memset(&fields, 0, sizeof(fields));
  memset(&rsp_fields, 0, sizeof(rsp_fields));

  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

  fields.flags = BLE_HS_ADV_F_DISC_GEN |
                 BLE_HS_ADV_F_BREDR_UNSUP;
//  fields.uuids128 = BLE_UUID128(BLE_UUID128_DECLARE(
//          0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
//          0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff));
  fields.num_uuids128 = 0;
  fields.uuids128_is_complete = 0;;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  rsp_fields.name = (uint8_t *)"Pinetime-JF";
  rsp_fields.name_len = strlen("Pinetime-JF");
  rsp_fields.name_is_complete = 1;

  int res;
  res = ble_gap_adv_set_fields(&fields);
  assert(res == 0);

  res = ble_gap_adv_rsp_set_fields(&rsp_fields);

  res = ble_gap_adv_start(addrType, NULL, 10000,
                          &adv_params, GAPEventCallback, this);


}

int NimbleController::OnGAPEvent(ble_gap_event *event) {
  switch (event->type) {
    case BLE_GAP_EVENT_ADV_COMPLETE:
      NRF_LOG_INFO("Advertising event : BLE_GAP_EVENT_ADV_COMPLETE");
      NRF_LOG_INFO("advertise complete; reason=%d", event->adv_complete.reason);
      StartAdvertising();
      break;
    case BLE_GAP_EVENT_CONNECT: {
      NRF_LOG_INFO("Advertising event : BLE_GAP_EVENT_CONNECT");

      /* A new connection was established or a connection attempt failed. */
      NRF_LOG_INFO("connection %s; status=%d ", event->connect.status == 0 ? "established" : "failed",
                   event->connect.status);

      if (event->connect.status != 0) {
        /* Connection failed; resume advertising. */
        StartAdvertising();
      } else {
        connectionHandle = event->connect.conn_handle;

        ble_gattc_disc_svc_by_uuid(connectionHandle, ((ble_uuid_t*)&ctsUuid), DiscoveryEventCallback, this);
      }
    }
      break;
    case BLE_GAP_EVENT_DISCONNECT:
      NRF_LOG_INFO("Advertising event : BLE_GAP_EVENT_DISCONNECT");
      NRF_LOG_INFO("disconnect; reason=%d ", event->disconnect.reason);

      /* Connection terminated; resume advertising. */
      StartAdvertising();
      break;
    case BLE_GAP_EVENT_CONN_UPDATE:
      NRF_LOG_INFO("Advertising event : BLE_GAP_EVENT_CONN_UPDATE");
      /* The central has updated the connection parameters. */
      NRF_LOG_INFO("connection updated; status=%d ", event->conn_update.status);
      break;
    case BLE_GAP_EVENT_ENC_CHANGE:
      /* Encryption has been enabled or disabled for this connection. */
      NRF_LOG_INFO("encryption change event; status=%d ", event->enc_change.status);
      return 0;
    case BLE_GAP_EVENT_SUBSCRIBE:
      NRF_LOG_INFO("subscribe event; conn_handle=%d attr_handle=%d "
                        "reason=%d prevn=%d curn=%d previ=%d curi=???\n",
                  event->subscribe.conn_handle,
                  event->subscribe.attr_handle,
                  event->subscribe.reason,
                  event->subscribe.prev_notify,
                  event->subscribe.cur_notify,
                  event->subscribe.prev_indicate);
      return 0;
    case BLE_GAP_EVENT_MTU:
      NRF_LOG_INFO("mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                  event->mtu.conn_handle,
                  event->mtu.channel_id,
                  event->mtu.value);
      return 0;

    case BLE_GAP_EVENT_REPEAT_PAIRING: {
      /* We already have a bond with the peer, but it is attempting to
       * establish a new secure link.  This app sacrifices security for
       * convenience: just throw away the old bond and accept the new link.
       */

      /* Delete the old bond. */
      struct ble_gap_conn_desc desc;
      ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
      ble_store_util_delete_peer(&desc.peer_id_addr);

      /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
       * continue with the pairing operation.
       */
    }
      return BLE_GAP_REPEAT_PAIRING_RETRY;
    default:
      NRF_LOG_INFO("Advertising event : %d", event->type);
      break;
  }
  return 0;
}

int NimbleController::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error, const ble_gatt_svc *service) {
  if(service == nullptr && error->status == BLE_HS_EDONE)
    NRF_LOG_INFO("Discovery complete");

  if(service != nullptr && ble_uuid_cmp(((ble_uuid_t*)&ctsUuid), &service->uuid.u) == 0) {
    NRF_LOG_INFO("CTS discovered : 0x%x", service->start_handle);
    ble_gattc_disc_chrs_by_uuid(connectionHandle, service->start_handle, service->end_handle, ((ble_uuid_t*)&ctsCurrentTimeUuid), CharacteristicDiscoveredCallback, this);
  }

  return 0;
}

int NimbleController::OnCharacteristicDiscoveryEvent(uint16_t conn_handle, const ble_gatt_error *error,
                                                     const ble_gatt_chr *characteristic) {
  if(characteristic == nullptr && error->status == BLE_HS_EDONE)
    NRF_LOG_INFO("Characteristic discovery complete");

  if(characteristic != nullptr && ble_uuid_cmp(((ble_uuid_t*)&ctsCurrentTimeUuid), &characteristic->uuid.u) == 0) {
    NRF_LOG_INFO("CTS Characteristic discovered : 0x%x", characteristic->val_handle);

    ble_gattc_read(conn_handle, characteristic->val_handle, CurrentTimeReadCallback, this);
  }
  return 0;
}

int NimbleController::OnCurrentTimeReadResult(uint16_t conn_handle, const ble_gatt_error *error, const ble_gatt_attr *attribute) {
  if(error->status == 0) {
    // TODO check that attribute->handle equals the handle discovered in OnCharacteristicDiscoveryEvent
    CtsData result;
    os_mbuf_copydata(attribute->om, 0, sizeof(CtsData), &result);
    NRF_LOG_INFO("Received data: %d-%d-%d %d:%d:%d", result.year,
                 result.month, result.dayofmonth,
                 result.hour, result.minute, result.second);
    dateTimeController.SetTime(result.year, result.month, result.dayofmonth,
                             0, result.hour, result.minute, result.second, nrf_rtc_counter_get(portNRF_RTC_REG));
  } else {
    NRF_LOG_INFO("Error retrieving current time: %d", error->status);
  }
  return 0;
}





