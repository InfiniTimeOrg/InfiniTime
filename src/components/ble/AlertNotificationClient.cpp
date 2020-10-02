#include <systemtask/SystemTask.h>
#include "NotificationManager.h"

#include "AlertNotificationClient.h"


using namespace Pinetime::Controllers;
constexpr ble_uuid16_t AlertNotificationClient::ansServiceUuid;

constexpr ble_uuid16_t AlertNotificationClient::supportedNewAlertCategoryUuid;
constexpr ble_uuid16_t AlertNotificationClient::supportedUnreadAlertCategoryUuid ;
constexpr ble_uuid16_t AlertNotificationClient::newAlertUuid;
constexpr ble_uuid16_t AlertNotificationClient::unreadAlertStatusUuid;
constexpr ble_uuid16_t AlertNotificationClient::controlPointUuid;

int Pinetime::Controllers::NewAlertSubcribeCallback(uint16_t conn_handle,
                     const struct ble_gatt_error *error,
                     struct ble_gatt_attr *attr,
                     void *arg) {
  auto client = static_cast<AlertNotificationClient*>(arg);
  return client->OnNewAlertSubcribe(conn_handle, error, attr);
}

AlertNotificationClient::AlertNotificationClient(Pinetime::System::SystemTask& systemTask,
        Pinetime::Controllers::NotificationManager& notificationManager) :
        systemTask{systemTask}, notificationManager{notificationManager}{

}

bool AlertNotificationClient::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error, const ble_gatt_svc *service) {
  if(service == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("ANS Discovery complete");
    return true;
  }

  if(service != nullptr && ble_uuid_cmp(((ble_uuid_t*)&ansServiceUuid), &service->uuid.u) == 0) {
    NRF_LOG_INFO("ANS discovered : 0x%x", service->start_handle);
      ansStartHandle = service->start_handle;
      ansEndHandle = service->end_handle;
      isDiscovered = true;
  }
  return false;
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
    // TODO implement this with more memory safety (and constexpr)
    static const size_t maxBufferSize{21};
    static const size_t maxMessageSize{18};
    size_t bufferSize = min(OS_MBUF_PKTLEN(event->notify_rx.om), maxBufferSize);

    uint8_t data[bufferSize];
    os_mbuf_copydata(event->notify_rx.om, 0, bufferSize, data);

    char *s = (char *) &data[3];
    auto messageSize = min(maxMessageSize, (bufferSize-3));

    for (uint i = 0; i < messageSize-1; i++) {
      if (s[i] == 0x00) {
        s[i] = 0x0A;
      }
    }
    s[messageSize-1] = '\0';

    notificationManager.Push(Pinetime::Controllers::NotificationManager::Categories::SimpleAlert, s, messageSize);
    systemTask.PushMessage(Pinetime::System::SystemTask::Messages::OnNewNotification);
  }
}

bool AlertNotificationClient::IsDiscovered() const {
  return isDiscovered;
}

uint16_t AlertNotificationClient::StartHandle() const {
  return ansStartHandle;
}

uint16_t AlertNotificationClient::EndHandle() const {
  return ansEndHandle;
}

uint16_t AlertNotificationClient::NewAlerthandle() const {
  return newAlertHandle;
}
