#include "components/ble/DeviceInformationService.h"

#include <lv_i18n/lv_i18n.h>
#include <nrf_log.h>

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t DeviceInformationService::manufacturerNameUuid;
constexpr ble_uuid16_t DeviceInformationService::modelNumberUuid;
constexpr ble_uuid16_t DeviceInformationService::serialNumberUuid;
constexpr ble_uuid16_t DeviceInformationService::fwRevisionUuid;
constexpr ble_uuid16_t DeviceInformationService::deviceInfoUuid;
constexpr ble_uuid16_t DeviceInformationService::hwRevisionUuid;
constexpr ble_uuid16_t DeviceInformationService::swRevisionUuid;
constexpr ble_uuid16_t DeviceInformationService::languageUuid;

int DeviceInformationCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto deviceInformationService = static_cast<DeviceInformationService*>(arg);
  return deviceInformationService->OnDeviceInfoRequested(conn_handle, attr_handle, ctxt);
}

void DeviceInformationService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int DeviceInformationService::OnDeviceInfoRequested(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt) {
  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    if (ble_uuid_u16(ctxt->chr->uuid) == languageId) {
      char language[3];
      int res = os_mbuf_copydata(ctxt->om, 0, 2, language);

      language[2] = 0;

      if (res < 0) {
        NRF_LOG_ERROR("Error reading BLE Data writing to Device Information Language (too little data)")
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
      }

      NRF_LOG_INFO("Received language: %s", language);

      int result = lv_i18n_set_locale(language);

      if (result != 0) {
        return BLE_ATT_ERR_INSUFFICIENT_ENC;
      }
      return 0;
    }
    return BLE_ATT_ERR_WRITE_NOT_PERMITTED;
  }

  const char* str;

  switch (ble_uuid_u16(ctxt->chr->uuid)) {
    case manufacturerNameId:
      str = manufacturerName;
      break;
    case modelNumberId:
      str = modelNumber;
      break;
    case serialNumberId:
      str = serialNumber;
      break;
    case fwRevisionId:
      str = fwRevision;
      break;
    case hwRevisionId:
      str = hwRevision;
      break;
    case swRevisionId:
      str = swRevision;
      break;
    case languageId:
      str = lv_i18n_get_current_locale();
      break;
    default:
      return BLE_ATT_ERR_UNLIKELY;
  }

  int res = os_mbuf_append(ctxt->om, str, strlen(str));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

DeviceInformationService::DeviceInformationService()
  : characteristicDefinition {{
                                .uuid = &manufacturerNameUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &modelNumberUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &serialNumberUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &fwRevisionUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &hwRevisionUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &swRevisionUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                              },
                              {
                                .uuid = &languageUuid.u,
                                .access_cb = DeviceInformationCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                              },
                              {0}},
    serviceDefinition {
      {/* Device Information Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &deviceInfoUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    } {
}
