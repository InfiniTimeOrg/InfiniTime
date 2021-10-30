#include "components/ble/NimbleController.h"
#include <cstring>

#include <hal/nrf_rtc.h>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_hs.h>
#include <host/ble_hs_id.h>
#include <host/util/util.h>
#include <controller/ble_ll.h>
#undef max
#undef min
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/datetime/DateTimeController.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

NimbleController::NimbleController(Pinetime::System::SystemTask& systemTask,
                                   Pinetime::Controllers::Ble& bleController,
                                   DateTime& dateTimeController,
                                   Pinetime::Controllers::NotificationManager& notificationManager,
                                   Controllers::Battery& batteryController,
                                   Pinetime::Drivers::SpiNorFlash& spiNorFlash,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController)
  : systemTask {systemTask},
    bleController {bleController},
    dateTimeController {dateTimeController},
    notificationManager {notificationManager},
    spiNorFlash {spiNorFlash},
    dfuService {systemTask, bleController, spiNorFlash},
    currentTimeClient {dateTimeController},
    anService {systemTask, notificationManager},
    alertNotificationClient {systemTask, notificationManager},
    currentTimeService {dateTimeController},
    musicService {systemTask},
    navService {systemTask},
    batteryInformationService {batteryController},
    immediateAlertService {systemTask, notificationManager},
    heartRateService {systemTask, heartRateController},
    motionService{systemTask, motionController},
    serviceDiscovery({&currentTimeClient, &alertNotificationClient}) {
}

void nimble_on_reset(int reason) {
  NRF_LOG_INFO("Nimble lost sync, resetting state; reason=%d", reason);
}

void nimble_on_sync(void) {
  int rc;

  NRF_LOG_INFO("Nimble is synced");

  rc = ble_hs_util_ensure_addr(0);
  ASSERT(rc == 0);

  nptr->StartAdvertising();
}

int GAPEventCallback(struct ble_gap_event* event, void* arg) {
  auto nimbleController = static_cast<NimbleController*>(arg);
  return nimbleController->OnGAPEvent(event);
}

void NimbleController::Init() {
  while (!ble_hs_synced()) {
  }

  nptr = this;
  ble_hs_cfg.reset_cb = nimble_on_reset;
  ble_hs_cfg.sync_cb = nimble_on_sync;
  ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

  ble_svc_gap_init();
  ble_svc_gatt_init();

  deviceInformationService.Init();
  currentTimeClient.Init();
  currentTimeService.Init();
  musicService.Init();
  navService.Init();
  anService.Init();
  dfuService.Init();
  batteryInformationService.Init();
  immediateAlertService.Init();
  heartRateService.Init();
  motionService.Init();

  int rc;
  rc = ble_hs_util_ensure_addr(0);
  ASSERT(rc == 0);
  rc = ble_hs_id_infer_auto(0, &addrType);
  ASSERT(rc == 0);
  rc = ble_svc_gap_device_name_set(deviceName);
  ASSERT(rc == 0);
  rc = ble_svc_gap_device_appearance_set(0xC2);
  ASSERT(rc == 0);
  Pinetime::Controllers::Ble::BleAddress address;
  rc = ble_hs_id_copy_addr(addrType, address.data(), nullptr);
  ASSERT(rc == 0);

  bleController.Address(std::move(address));
  switch (addrType) {
    case BLE_OWN_ADDR_PUBLIC:
      bleController.AddressType(Ble::AddressTypes::Public);
      break;
    case BLE_OWN_ADDR_RANDOM:
      bleController.AddressType(Ble::AddressTypes::Random);
      break;
    case BLE_OWN_ADDR_RPA_PUBLIC_DEFAULT:
      bleController.AddressType(Ble::AddressTypes::RPA_Public);
      break;
    case BLE_OWN_ADDR_RPA_RANDOM_DEFAULT:
      bleController.AddressType(Ble::AddressTypes::RPA_Random);
      break;
  }

  rc = ble_gatts_start();
  ASSERT(rc == 0);

  if (!ble_gap_adv_active() && !bleController.IsConnected())
    StartAdvertising();
}

void NimbleController::StartAdvertising() {
  struct ble_gap_adv_params adv_params;
  struct ble_hs_adv_fields fields;
  struct ble_hs_adv_fields rsp_fields;

  memset(&adv_params, 0, sizeof(adv_params));
  memset(&fields, 0, sizeof(fields));
  memset(&rsp_fields, 0, sizeof(rsp_fields));

  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  /* fast advertise for 30 sec */
  if (fastAdvCount < 15) {
    adv_params.itvl_min = 32;
    adv_params.itvl_max = 47;
    fastAdvCount++;
  } else {
    adv_params.itvl_min = 1636;
    adv_params.itvl_max = 1651;
  }

  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
  fields.uuids128 = &dfuServiceUuid;
  fields.num_uuids128 = 1;
  fields.uuids128_is_complete = 1;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  rsp_fields.name = reinterpret_cast<const uint8_t*>(deviceName);
  rsp_fields.name_len = strlen(deviceName);
  rsp_fields.name_is_complete = 1;

  int rc;
  rc = ble_gap_adv_set_fields(&fields);
  ASSERT(rc == 0);

  rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
  ASSERT(rc == 0);

  rc = ble_gap_adv_start(addrType, NULL, 2000, &adv_params, GAPEventCallback, this);
  ASSERT(rc == 0);
}

int NimbleController::OnGAPEvent(ble_gap_event* event) {
  switch (event->type) {
    case BLE_GAP_EVENT_ADV_COMPLETE:
      NRF_LOG_INFO("Advertising event : BLE_GAP_EVENT_ADV_COMPLETE");
      NRF_LOG_INFO("reason=%d; status=%0X", event->adv_complete.reason, event->connect.status);
      StartAdvertising();
      break;

    case BLE_GAP_EVENT_CONNECT:
      /* A new connection was established or a connection attempt failed. */
      NRF_LOG_INFO("Connect event : BLE_GAP_EVENT_CONNECT");
      NRF_LOG_INFO("connection %s; status=%0X ", event->connect.status == 0 ? "established" : "failed", event->connect.status);

      if (event->connect.status != 0) {
        /* Connection failed; resume advertising. */
        currentTimeClient.Reset();
        alertNotificationClient.Reset();
        connectionHandle = BLE_HS_CONN_HANDLE_NONE;
        bleController.Disconnect();
        fastAdvCount = 0;
        StartAdvertising();
      } else {
        connectionHandle = event->connect.conn_handle;
        bleController.Connect();
        systemTask.PushMessage(Pinetime::System::Messages::BleConnected);
        // Service discovery is deferred via systemtask
      }
      break;

    case BLE_GAP_EVENT_DISCONNECT:
      /* Connection terminated; resume advertising. */
      NRF_LOG_INFO("Disconnect event : BLE_GAP_EVENT_DISCONNECT");
      NRF_LOG_INFO("disconnect reason=%d", event->disconnect.reason);
      currentTimeClient.Reset();
      alertNotificationClient.Reset();
      connectionHandle = BLE_HS_CONN_HANDLE_NONE;
      bleController.Disconnect();
      fastAdvCount = 0;
      StartAdvertising();
      break;

    case BLE_GAP_EVENT_CONN_UPDATE:
      /* The central has updated the connection parameters. */
      NRF_LOG_INFO("Update event : BLE_GAP_EVENT_CONN_UPDATE");
      NRF_LOG_INFO("update status=%0X ", event->conn_update.status);
      break;

    case BLE_GAP_EVENT_CONN_UPDATE_REQ:
      /* The central has requested updated connection parameters */
      NRF_LOG_INFO("Update event : BLE_GAP_EVENT_CONN_UPDATE_REQ");
      NRF_LOG_INFO("update request : itvl_min=%d itvl_max=%d latency=%d supervision=%d",
                   event->conn_update_req.peer_params->itvl_min,
                   event->conn_update_req.peer_params->itvl_max,
                   event->conn_update_req.peer_params->latency,
                   event->conn_update_req.peer_params->supervision_timeout);
      break;

    case BLE_GAP_EVENT_ENC_CHANGE:
      /* Encryption has been enabled or disabled for this connection. */
      NRF_LOG_INFO("Security event : BLE_GAP_EVENT_ENC_CHANGE");
      NRF_LOG_INFO("encryption change event; status=%0X ", event->enc_change.status);
      break;

    case BLE_GAP_EVENT_PASSKEY_ACTION:
      /* Authentication has been requested for this connection.
       * Standards insist that the rand() PRNG be deterministic.
       * Use the nimble TRNG since rand() is predictable.
       */
      NRF_LOG_INFO("Security event : BLE_GAP_EVENT_PASSKEY_ACTION");
      if (event->passkey.params.action == BLE_SM_IOACT_DISP) {
        struct ble_sm_io pkey = {0};
        pkey.action = event->passkey.params.action;
        pkey.passkey = ble_ll_rand() % 1000000;
        bleController.SetPairingKey(pkey.passkey);
        systemTask.PushMessage(Pinetime::System::Messages::OnPairing);
        ble_sm_inject_io(event->passkey.conn_handle, &pkey);
      }
      break;

    case BLE_GAP_EVENT_SUBSCRIBE:
      NRF_LOG_INFO("Subscribe event; conn_handle=%d attr_handle=%d "
                   "reason=%d prevn=%d curn=%d previ=%d curi=???\n",
                   event->subscribe.conn_handle,
                   event->subscribe.attr_handle,
                   event->subscribe.reason,
                   event->subscribe.prev_notify,
                   event->subscribe.cur_notify,
                   event->subscribe.prev_indicate);

      if(event->subscribe.reason == BLE_GAP_SUBSCRIBE_REASON_TERM) {
        heartRateService.UnsubscribeNotification(event->subscribe.conn_handle, event->subscribe.attr_handle);
        motionService.UnsubscribeNotification(event->subscribe.conn_handle, event->subscribe.attr_handle);
      }
      else if(event->subscribe.prev_notify == 0 && event->subscribe.cur_notify == 1) {
        heartRateService.SubscribeNotification(event->subscribe.conn_handle, event->subscribe.attr_handle);
        motionService.SubscribeNotification(event->subscribe.conn_handle, event->subscribe.attr_handle);
      }
      else if(event->subscribe.prev_notify == 1 && event->subscribe.cur_notify == 0) {
        heartRateService.UnsubscribeNotification(event->subscribe.conn_handle, event->subscribe.attr_handle);
        motionService.UnsubscribeNotification(event->subscribe.conn_handle, event->subscribe.attr_handle);
      }
      break;

    case BLE_GAP_EVENT_MTU:
      NRF_LOG_INFO("MTU Update event; conn_handle=%d cid=%d mtu=%d", event->mtu.conn_handle, event->mtu.channel_id, event->mtu.value);
      break;

    case BLE_GAP_EVENT_REPEAT_PAIRING: {
      NRF_LOG_INFO("Pairing event : BLE_GAP_EVENT_REPEAT_PAIRING");
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
      /* Attribute data is contained in event->notify_rx.attr_data. */
      NRF_LOG_INFO("Notify event : BLE_GAP_EVENT_NOTIFY_RX");
      size_t notifSize = OS_MBUF_PKTLEN(event->notify_rx.om);

      NRF_LOG_INFO("received %s; conn_handle=%d attr_handle=%d "
                   "attr_len=%d",
                   event->notify_rx.indication ? "indication" : "notification",
                   event->notify_rx.conn_handle,
                   event->notify_rx.attr_handle,
                   notifSize);

      alertNotificationClient.OnNotification(event);
    } break;

    case BLE_GAP_EVENT_NOTIFY_TX:
      NRF_LOG_INFO("Notify event : BLE_GAP_EVENT_NOTIFY_TX");
      break;

    case BLE_GAP_EVENT_IDENTITY_RESOLVED:
      NRF_LOG_INFO("Identity event : BLE_GAP_EVENT_IDENTITY_RESOLVED");
      break;

    default:
      NRF_LOG_INFO("UNHANDLED GAP event : %d", event->type);
      break;
  }
  return 0;
}

void NimbleController::StartDiscovery() {
  if (connectionHandle != BLE_HS_CONN_HANDLE_NONE) {
    serviceDiscovery.StartDiscovery(connectionHandle);
  }
}

uint16_t NimbleController::connHandle() {
  return connectionHandle;
}

void NimbleController::NotifyBatteryLevel(uint8_t level) {
  if (connectionHandle != BLE_HS_CONN_HANDLE_NONE) {
    batteryInformationService.NotifyBatteryLevel(connectionHandle, level);
  }
}
