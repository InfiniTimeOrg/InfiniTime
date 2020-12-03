#include "DeviceInformationService.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t DeviceInformationService::manufacturerNameUuid;
constexpr ble_uuid16_t DeviceInformationService::modelNumberUuid;
constexpr ble_uuid16_t DeviceInformationService::serialNumberUuid;
constexpr ble_uuid16_t DeviceInformationService::fwRevisionUuid;
constexpr ble_uuid16_t DeviceInformationService::deviceInfoUuid;
constexpr ble_uuid16_t DeviceInformationService::hwRevisionUuid;
constexpr ble_uuid16_t DeviceInformationService::swRevisionUuid;


int DeviceInformationCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
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


int DeviceInformationService::OnDeviceInfoRequested(uint16_t conn_handle, uint16_t attr_handle,
                                                    struct ble_gatt_access_ctxt *ctxt) {
  const char *str;

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
    default:
      return BLE_ATT_ERR_UNLIKELY;
  }

  int res = os_mbuf_append(ctxt->om, str, strlen(str));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

DeviceInformationService::DeviceInformationService() :
        characteristicDefinition{
                {
                        .uuid = (ble_uuid_t *) &manufacturerNameUuid,
                        .access_cb = DeviceInformationCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_READ,
                },
                {
                        .uuid = (ble_uuid_t *) &modelNumberUuid,
                        .access_cb = DeviceInformationCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_READ,
                },
                {
                        .uuid = (ble_uuid_t *) &serialNumberUuid,
                        .access_cb = DeviceInformationCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_READ,
                },
                {
                        .uuid = (ble_uuid_t *) &fwRevisionUuid,
                        .access_cb = DeviceInformationCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_READ,
                },
                {
                        .uuid = (ble_uuid_t *) &hwRevisionUuid,
                        .access_cb = DeviceInformationCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_READ,
                },
                {
                        .uuid = (ble_uuid_t *) &swRevisionUuid,
                        .access_cb = DeviceInformationCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_READ,
                },
                {
                  0
                }
        },
        serviceDefinition{
                {
                        /* Device Information Service */
                        .type = BLE_GATT_SVC_TYPE_PRIMARY,
                        .uuid = (ble_uuid_t *) &deviceInfoUuid,
                        .characteristics = characteristicDefinition
                },
                {
                        0
                },
        }
         {

}

