
#include <Components/DateTime/DateTimeController.h>

#include <SystemTask/SystemTask.h>
#include <Components/Ble/NotificationManager.h>
#include <hal/nrf_rtc.h>

#include "NimbleController.h"
#include "MusicService.h"
#include <services/gatt/ble_svc_gatt.h>
#include <services/gap/ble_svc_gap.h>
#include <host/util/util.h>
#include <host/ble_hs_id.h>
#include <host/ble_hs.h>
#include <host/ble_gap.h>



using namespace Pinetime::Controllers;

// TODO I'm not satisfied by how this code looks like (AlertNotificationClient and CurrentTimeClient must
// expose too much data, too many callbacks -> NimbleController -> CTS/ANS client.
// Let's try to improve this code (and keep it working!)

NimbleController::NimbleController(Pinetime::System::SystemTask& systemTask,
                                   Pinetime::Controllers::Ble& bleController,
        DateTime& dateTimeController,
        Pinetime::Controllers::NotificationManager& notificationManager,
        Pinetime::Drivers::SpiNorFlash& spiNorFlash) :
        systemTask{systemTask},
        bleController{bleController},
        dateTimeController{dateTimeController},
        notificationManager{notificationManager},
        spiNorFlash{spiNorFlash},
        dfuService{systemTask, bleController, spiNorFlash},
        currentTimeClient{dateTimeController},
        anService{systemTask, notificationManager},
        alertNotificationClient{systemTask, notificationManager},
        currentTimeService{dateTimeController},
        musicService{systemTask} {

}

int GAPEventCallback(struct ble_gap_event *event, void *arg) {
  auto nimbleController = static_cast<NimbleController*>(arg);
  return nimbleController->OnGAPEvent(event);
}

int CurrentTimeCharacteristicDiscoveredCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                                const struct ble_gatt_chr *chr, void *arg) {
  auto client = static_cast<NimbleController*>(arg);
  return client->OnCTSCharacteristicDiscoveryEvent(conn_handle, error, chr);
}

int AlertNotificationCharacteristicDiscoveredCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                                const struct ble_gatt_chr *chr, void *arg) {
  auto client = static_cast<NimbleController*>(arg);
  return client->OnANSCharacteristicDiscoveryEvent(conn_handle, error, chr);
}

int CurrentTimeReadCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                   struct ble_gatt_attr *attr, void *arg) {
  auto client = static_cast<NimbleController*>(arg);
  return client->OnCurrentTimeReadResult(conn_handle, error, attr);
}

int AlertNotificationDescriptorDiscoveryEventCallback(uint16_t conn_handle,
                                                                             const struct ble_gatt_error *error,
                                                                             uint16_t chr_val_handle,
                                                                             const struct ble_gatt_dsc *dsc,
                                                                             void *arg) {
  auto client = static_cast<NimbleController*>(arg);
  return client->OnANSDescriptorDiscoveryEventCallback(conn_handle, error, chr_val_handle, dsc);
}

void NimbleController::Init() {
  while (!ble_hs_synced()) {}

  ble_svc_gap_init();
  ble_svc_gatt_init();

  deviceInformationService.Init();
  currentTimeClient.Init();
  currentTimeService.Init();
  musicService.Init();

  anService.Init();

  dfuService.Init();
  int res;
  res = ble_hs_util_ensure_addr(0);
  ASSERT(res == 0);
  res = ble_hs_id_infer_auto(0, &addrType);
  ASSERT(res == 0);
  res = ble_svc_gap_device_name_set(deviceName);
  ASSERT(res == 0);
  Pinetime::Controllers::Ble::BleAddress address;
  res = ble_hs_id_copy_addr(addrType, address.data(), nullptr);
  ASSERT(res == 0);
  bleController.AddressType((addrType == 0) ? Ble::AddressTypes::Public : Ble::AddressTypes::Random);
  bleController.Address(std::move(address));

  res = ble_gatts_start();
  ASSERT(res == 0);
}

void NimbleController::StartAdvertising() {
  if(ble_gap_adv_active()) return;

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
  fields.uuids128 = &dfuServiceUuid;
  fields.num_uuids128 = 1;
  fields.uuids128_is_complete = 1;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  rsp_fields.name = (uint8_t *)"Pinetime-JF";
  rsp_fields.name_len = strlen("Pinetime-JF");
  rsp_fields.name_is_complete = 1;

  int res;
  res = ble_gap_adv_set_fields(&fields);
//  ASSERT(res == 0); // TODO this one sometimes fails with error 22 (notsync)

  res = ble_gap_adv_rsp_set_fields(&rsp_fields);
//  ASSERT(res == 0);

  res = ble_gap_adv_start(addrType, NULL, 180000,
                          &adv_params, GAPEventCallback, this);
//  ASSERT(res == 0);// TODO I've disabled these ASSERT as they sometime asserts and reset the mcu.
  // For now, the advertising is restarted as soon as it ends. There may be a race condition
  // that prevent the advertising from restarting reliably.
  // I remove the assert to prevent this uncesseray crash, but in the long term, the management of
  // the advertising should be improve (better error handling, and advertise for 3 minutes after
  // the application has been woken up, for example.
}

int OnAllSvrDisco(uint16_t conn_handle,
                                 const struct ble_gatt_error *error,
                                 const struct ble_gatt_svc *service,
                                 void *arg) {
  auto nimbleController = static_cast<NimbleController*>(arg);
  return nimbleController->OnDiscoveryEvent(conn_handle, error, service);
  return 0;
}

int NimbleController::OnGAPEvent(ble_gap_event *event) {
  switch (event->type) {
    case BLE_GAP_EVENT_ADV_COMPLETE:
      NRF_LOG_INFO("Advertising event : BLE_GAP_EVENT_ADV_COMPLETE");
      NRF_LOG_INFO("advertise complete; reason=%dn status=%d", event->adv_complete.reason, event->connect.status);
      break;
    case BLE_GAP_EVENT_CONNECT: {
      NRF_LOG_INFO("Advertising event : BLE_GAP_EVENT_CONNECT");

      /* A new connection was established or a connection attempt failed. */
      NRF_LOG_INFO("connection %s; status=%d ", event->connect.status == 0 ? "established" : "failed",
                   event->connect.status);

      if (event->connect.status != 0) {
        /* Connection failed; resume advertising. */
        StartAdvertising();
        bleController.Disconnect();
      } else {
        bleController.Connect();
        systemTask.PushMessage(Pinetime::System::SystemTask::Messages::BleConnected);
        connectionHandle = event->connect.conn_handle;
        // Service discovery is deffered via systemtask
      }
    }
      break;
    case BLE_GAP_EVENT_DISCONNECT:
      NRF_LOG_INFO("Advertising event : BLE_GAP_EVENT_DISCONNECT");
      NRF_LOG_INFO("disconnect; reason=%d", event->disconnect.reason);

      /* Connection terminated; resume advertising. */
      connectionHandle = BLE_HS_CONN_HANDLE_NONE;
      bleController.Disconnect();
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

    case BLE_GAP_EVENT_NOTIFY_RX: {
      /* Peer sent us a notification or indication. */
      size_t notifSize = OS_MBUF_PKTLEN(event->notify_rx.om);

      NRF_LOG_INFO("received %s; conn_handle=%d attr_handle=%d "
                   "attr_len=%d",
                   event->notify_rx.indication ?
                   "indication" :
                   "notification",
                   event->notify_rx.conn_handle,
                   event->notify_rx.attr_handle,
                   notifSize);

      alertNotificationClient.OnNotification(event);
      return 0;
    }
      /* Attribute data is contained in event->notify_rx.attr_data. */

    default:
//      NRF_LOG_INFO("Advertising event : %d", event->type);
      break;
  }
  return 0;
}

int NimbleController::OnDiscoveryEvent(uint16_t i, const ble_gatt_error *error, const ble_gatt_svc *service) {
  if(service == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("Service Discovery complete");
    if(currentTimeClient.IsDiscovered()) {
      ble_gattc_disc_all_chrs(connectionHandle, currentTimeClient.StartHandle(), currentTimeClient.EndHandle(),
                              CurrentTimeCharacteristicDiscoveredCallback, this);

    } else if(alertNotificationClient.IsDiscovered()) {
      ble_gattc_disc_all_chrs(connectionHandle, alertNotificationClient.StartHandle(), alertNotificationClient.EndHandle(),
                              AlertNotificationCharacteristicDiscoveredCallback, this);
    }
  }

  alertNotificationClient.OnDiscoveryEvent(i, error, service);
  currentTimeClient.OnDiscoveryEvent(i, error, service);
  return 0;
}

int NimbleController::OnCTSCharacteristicDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error,
                                                        const ble_gatt_chr *characteristic) {
  if(characteristic == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("CTS characteristic Discovery complete");
    ble_gattc_read(connectionHandle, currentTimeClient.CurrentTimeHandle(), CurrentTimeReadCallback, this);
    return 0;
  }
  return currentTimeClient.OnCharacteristicDiscoveryEvent(connectionHandle, error, characteristic);
}

int NimbleController::OnANSCharacteristicDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error,
                                                        const ble_gatt_chr *characteristic) {
  if(characteristic == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("ANS characteristic Discovery complete");
    ble_gattc_disc_all_dscs(connectionHandle,
            alertNotificationClient.NewAlerthandle(), alertNotificationClient.EndHandle(),
            AlertNotificationDescriptorDiscoveryEventCallback, this);
    return 0;
  }
  return alertNotificationClient.OnCharacteristicsDiscoveryEvent(connectionHandle, error, characteristic);
}

int NimbleController::OnCurrentTimeReadResult(uint16_t connectionHandle, const ble_gatt_error *error, ble_gatt_attr *attribute) {
  currentTimeClient.OnCurrentTimeReadResult(connectionHandle, error, attribute);

  if (alertNotificationClient.IsDiscovered()) {
    ble_gattc_disc_all_chrs(connectionHandle, alertNotificationClient.StartHandle(),
                            alertNotificationClient.EndHandle(),
                            AlertNotificationCharacteristicDiscoveredCallback, this);
  }
  return 0;
}

int NimbleController::OnANSDescriptorDiscoveryEventCallback(uint16_t connectionHandle, const ble_gatt_error *error,
                                                            uint16_t characteristicValueHandle,
                                                            const ble_gatt_dsc *descriptor) {
  return alertNotificationClient.OnDescriptorDiscoveryEventCallback(connectionHandle, error, characteristicValueHandle, descriptor);
}

void NimbleController::StartDiscovery() {
  ble_gattc_disc_all_svcs(connectionHandle, OnAllSvrDisco, this);
}


uint16_t NimbleController::connHandle() {
    return connectionHandle;
}

