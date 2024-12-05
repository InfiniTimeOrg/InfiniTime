#include "components/ble/ImmediateAlertClient.h"
#include <cstring>
#include <nrf_log.h>
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t ImmediateAlertClient::immediateAlertClientUuid;
constexpr ble_uuid16_t ImmediateAlertClient::alertLevelCharacteristicUuid;

ImmediateAlertClient::ImmediateAlertClient(Pinetime::System::SystemTask& systemTask) : systemTask {systemTask} {
}

void ImmediateAlertClient::Init() {
}

bool ImmediateAlertClient::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error* error, const ble_gatt_svc* service) {
  if (service == nullptr && error->status == BLE_HS_EDONE) {
    if (iasHandles.has_value()) {
      NRF_LOG_INFO("[IAS] service found, starting characteristics discovery");

      ble_gattc_disc_all_chrs(connectionHandle,
                              iasHandles->startHandle,
                              iasHandles->endHandle,
                              OnImmediateAlertCharacteristicDiscoveredCallback,
                              this);
    } else {
      NRF_LOG_INFO("[IAS] service not found");
      onServiceDiscovered(connectionHandle);
    }
    return true;
  }

  if (service != nullptr && ble_uuid_cmp(&immediateAlertClientUuid.u, &service->uuid.u) == 0) {
    NRF_LOG_INFO("[IAS] discovered : 0x%x - 0x%x", service->start_handle, service->end_handle);
    iasHandles.emplace(HandleRange {
      .startHandle = service->start_handle,
      .endHandle = service->end_handle,
    });
  }
  return false;
}

int ImmediateAlertClient::OnCharacteristicDiscoveryEvent(uint16_t conn_handle,
                                                         const ble_gatt_error* error,
                                                         const ble_gatt_chr* characteristic) {

  if (error->status != 0 && error->status != BLE_HS_EDONE) {
    NRF_LOG_INFO("[IAS] Characteristic discovery ERROR");
    onServiceDiscovered(conn_handle);
    return 0;
  }

  if (characteristic == nullptr && error->status == BLE_HS_EDONE) {
    if (!alertLevelHandle.has_value()) {
      NRF_LOG_INFO("[IAS] Alert level characteristic not found.");
      onServiceDiscovered(conn_handle);
    }

    return 0;
  }

  if (characteristic != nullptr && ble_uuid_cmp(&alertLevelCharacteristicUuid.u, &characteristic->uuid.u) == 0) {
    NRF_LOG_INFO("[IAS] Characteristic discovered : 0x%x", characteristic->val_handle);
    alertLevelHandle = characteristic->val_handle;
  }
  return 0;
}

void ImmediateAlertClient::Discover(uint16_t connectionHandle, std::function<void(uint16_t)> onServiceDiscovered) {
  NRF_LOG_INFO("[IAS] Starting discovery");
  this->onServiceDiscovered = onServiceDiscovered;
  ble_gattc_disc_svc_by_uuid(connectionHandle, &immediateAlertClientUuid.u, OnDiscoveryEventCallback, this);
}

bool ImmediateAlertClient::SendImmediateAlert(ImmediateAlertClient::Levels level) {

  auto* om = ble_hs_mbuf_from_flat(&level, 1);

  uint16_t connectionHandle = systemTask.nimble().connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return false;
  }
  if (!alertLevelHandle.has_value()) {
    return false;
  }

  ble_gattc_write_no_rsp(connectionHandle, *alertLevelHandle, om);
  return true;
}

int ImmediateAlertClient::OnDiscoveryEventCallback(uint16_t conn_handle,
                                                   const struct ble_gatt_error* error,
                                                   const struct ble_gatt_svc* service,
                                                   void* arg) {
  auto client = static_cast<ImmediateAlertClient*>(arg);
  return client->OnDiscoveryEvent(conn_handle, error, service);
}

int ImmediateAlertClient::OnImmediateAlertCharacteristicDiscoveredCallback(uint16_t conn_handle,
                                                                           const struct ble_gatt_error* error,
                                                                           const struct ble_gatt_chr* chr,
                                                                           void* arg) {
  auto client = static_cast<ImmediateAlertClient*>(arg);
  return client->OnCharacteristicDiscoveryEvent(conn_handle, error, chr);
}
