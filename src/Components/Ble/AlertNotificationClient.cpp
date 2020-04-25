#include <SystemTask/SystemTask.h>
#include "NotificationManager.h"

#include "AlertNotificationClient.h"


using namespace Pinetime::Controllers;
constexpr ble_uuid16_t AlertNotificationClient::ansServiceUuid;

constexpr ble_uuid16_t AlertNotificationClient::supportedNewAlertCategoryUuid;
constexpr ble_uuid16_t AlertNotificationClient::supportedUnreadAlertCategoryUuid ;
constexpr ble_uuid16_t AlertNotificationClient::newAlertUuid;
constexpr ble_uuid16_t AlertNotificationClient::unreadAlertStatusUuid;
constexpr ble_uuid16_t AlertNotificationClient::controlPointUuid;

int Pinetime::Controllers::AlertNotificationDiscoveryEventCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                                             const struct ble_gatt_svc *service, void *arg) {
  auto client = static_cast<AlertNotificationClient*>(arg);
  return client->OnDiscoveryEvent(conn_handle, error, service);
}

int Pinetime::Controllers::AlertNotificationCharacteristicsDiscoveryEventCallback(uint16_t conn_handle,
                    const struct ble_gatt_error *error,
                    const struct ble_gatt_chr *chr, void *arg) {
  auto client = static_cast<AlertNotificationClient*>(arg);
  return client->OnCharacteristicsDiscoveryEvent(conn_handle, error, chr);
}

int Pinetime::Controllers::NewAlertSubcribeCallback(uint16_t conn_handle,
                     const struct ble_gatt_error *error,
                     struct ble_gatt_attr *attr,
                     void *arg) {
  auto client = static_cast<AlertNotificationClient*>(arg);
  return client->OnNewAlertSubcribe(conn_handle, error, attr);
}

int Pinetime::Controllers::AlertNotificationDescriptorDiscoveryEventCallback(uint16_t conn_handle,
                                                                             const struct ble_gatt_error *error,
                                                                             uint16_t chr_val_handle,
                                                                             const struct ble_gatt_dsc *dsc,
                                                                             void *arg) {
  NRF_LOG_INFO("ANS VCS");
  auto client = static_cast<AlertNotificationClient*>(arg);
  return client->OnDescriptorDiscoveryEventCallback(conn_handle, error, chr_val_handle, dsc);
}

AlertNotificationClient::AlertNotificationClient(Pinetime::System::SystemTask& systemTask,
        Pinetime::Controllers::NotificationManager& notificationManager) :
        systemTask{systemTask}, notificationManager{notificationManager}{

}

void AlertNotificationClient::StartDiscovery(uint16_t connectionHandle) {
  ble_gattc_disc_svc_by_uuid(connectionHandle, ((ble_uuid_t*)&ansServiceUuid), AlertNotificationDiscoveryEventCallback, this);
}

bool AlertNotificationClient::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error, const ble_gatt_svc *service) {
  if(service == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("ANS Discovery complete");
    ble_gattc_disc_all_dscs(connectionHandle, newAlertHandle, ansEndHandle, AlertNotificationDescriptorDiscoveryEventCallback, this);
    return true;
  }

  if(service != nullptr && ble_uuid_cmp(((ble_uuid_t*)&ansServiceUuid), &service->uuid.u) == 0) {
    NRF_LOG_INFO("ANS discovered : 0x%x", service->start_handle);
    ble_gattc_disc_all_chrs(connectionHandle, service->start_handle, service->end_handle, AlertNotificationCharacteristicsDiscoveryEventCallback, this);
    ansEndHandle = service->end_handle;
  }
  return false;
}

void AlertNotificationClient::Init() {

}

int AlertNotificationClient::OnCharacteristicsDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error,
                                                                                    const ble_gatt_chr *characteristic) {
  if(error->status != 0 && error->status != BLE_HS_EDONE) {
    NRF_LOG_INFO("ANS Characteristic discovery ERROR");
    return 0;
  }

  if(characteristic == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("ANS Characteristic discovery complete");
  } else {
    if(characteristic != nullptr && ble_uuid_cmp(((ble_uuid_t*)&supportedNewAlertCategoryUuid), &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANS Characteristic discovered : supportedNewAlertCategoryUuid");
      supportedNewAlertCategoryHandle = characteristic->val_handle;
    } else if(characteristic != nullptr && ble_uuid_cmp(((ble_uuid_t*)&supportedUnreadAlertCategoryUuid), &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANS Characteristic discovered : supportedUnreadAlertCategoryUuid");
      supportedUnreadAlertCategoryHandle = characteristic->val_handle;
    } else if(characteristic != nullptr && ble_uuid_cmp(((ble_uuid_t*)&newAlertUuid), &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANS Characteristic discovered : newAlertUuid");
      newAlertHandle = characteristic->val_handle;
      newAlertDefHandle = characteristic->def_handle;
    } else if(characteristic != nullptr && ble_uuid_cmp(((ble_uuid_t*)&unreadAlertStatusUuid), &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANS Characteristic discovered : unreadAlertStatusUuid");
      unreadAlertStatusHandle = characteristic->val_handle;
    } else if(characteristic != nullptr && ble_uuid_cmp(((ble_uuid_t*)&controlPointUuid), &characteristic->uuid.u) == 0) {
      NRF_LOG_INFO("ANS Characteristic discovered : controlPointUuid");
      controlPointHandle = characteristic->val_handle;
    }else
      NRF_LOG_INFO("ANS Characteristic discovered : 0x%x", characteristic->val_handle);
    }
  return 0;
}

int AlertNotificationClient::OnNewAlertSubcribe(uint16_t connectionHandle, const ble_gatt_error *error,
                                                ble_gatt_attr *attribute) {
  if(error->status == 0) {
    NRF_LOG_INFO("ANS New alert subscribe OK");
  } else {
    NRF_LOG_INFO("ANS New alert subscribe ERROR");
  }

  return 0;
}

int AlertNotificationClient::OnDescriptorDiscoveryEventCallback(uint16_t connectionHandle, const ble_gatt_error *error,
                                                                uint16_t characteristicValueHandle,
                                                                const ble_gatt_dsc *descriptor) {
  if(error->status == 0) {
    if(characteristicValueHandle == newAlertHandle && ble_uuid_cmp(((ble_uuid_t*)&newAlertUuid), &descriptor->uuid.u)) {
      if(newAlertDescriptorHandle == 0) {
        NRF_LOG_INFO("ANS Descriptor discovered : %d", descriptor->handle);
        newAlertDescriptorHandle = descriptor->handle;
        uint8_t value[2];
        value[0] = 1;
        value[1] = 0;
        ble_gattc_write_flat(connectionHandle, newAlertDescriptorHandle, value, sizeof(value), NewAlertSubcribeCallback, this);
      }
    }
  }
  return 0;
}

void AlertNotificationClient::OnNotification(ble_gap_event *event) {
  if(event->notify_rx.attr_handle == newAlertHandle) {
    size_t notifSize = OS_MBUF_PKTLEN(event->notify_rx.om);
    uint8_t data[notifSize + 1];
    data[notifSize] = '\0';
    os_mbuf_copydata(event->notify_rx.om, 0, notifSize, data);
    char *s = (char *) &data[2];
    NRF_LOG_INFO("DATA : %s", s);

    notificationManager.Push(Pinetime::Controllers::NotificationManager::Categories::SimpleAlert, s, notifSize + 1);
    systemTask.PushMessage(Pinetime::System::SystemTask::Messages::OnNewNotification);
  }
}
