#include "components/ble/ImmediateAlertService.h"
#include <cstring>
#include "components/ble/NotificationManager.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t ImmediateAlertService::immediateAlertServiceUuid;
constexpr ble_uuid16_t ImmediateAlertService::alertLevelUuid;

namespace {
  int AlertLevelCallback(uint16_t /*conn_handle*/, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    auto* immediateAlertService = static_cast<ImmediateAlertService*>(arg);
    return immediateAlertService->OnAlertLevelChanged(attr_handle, ctxt);
  }

  const char* ToString(ImmediateAlertService::Levels level) {
    switch (level) {
      case ImmediateAlertService::Levels::NoAlert:
        return "Alert : None";
      case ImmediateAlertService::Levels::HighAlert:
        return "Alert : High";
      case ImmediateAlertService::Levels::MildAlert:
        return "Alert : Mild";
      default:
        return "";
    }
  }
}

ImmediateAlertService::ImmediateAlertService(Pinetime::System::SystemTask& systemTask,
                                             Pinetime::Controllers::NotificationManager& notificationManager)
  : systemTask {systemTask},
    notificationManager {notificationManager},
    characteristicDefinition {{.uuid = &alertLevelUuid.u,
                               .access_cb = AlertLevelCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
                               .val_handle = &alertLevelHandle},
                              {0}},
    serviceDefinition {
      {/* Device Information Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &immediateAlertServiceUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    } {
}

void ImmediateAlertService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int ImmediateAlertService::OnAlertLevelChanged(uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (attributeHandle == alertLevelHandle) {
    if (context->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
      auto alertLevel = static_cast<Levels>(context->om->om_data[0]);
      auto* alertString = ToString(alertLevel);

      NotificationManager::Notification notif;
      std::memcpy(notif.message.data(), alertString, strlen(alertString));
      notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
      notificationManager.Push(std::move(notif));

      systemTask.PushMessage(Pinetime::System::Messages::OnNewNotification);
    }
  }

  return 0;
}
