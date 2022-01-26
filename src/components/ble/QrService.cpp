#include "QrService.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

namespace {
  // 0004yyxx-78fc-48fe-8e23-433b3a1942d0
  constexpr ble_uuid128_t CharUuid(uint8_t x, uint8_t y) {
    return ble_uuid128_t {.u = {.type = BLE_UUID_TYPE_128},
                          .value = {0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, x, y, 0x04, 0x00}};
  }

  // 00040000-78fc-48fe-8e23-433b3a1942d0
  constexpr ble_uuid128_t BaseUuid() {
    return CharUuid(0x00, 0x00);
  }

  constexpr ble_uuid128_t qrServiceUuid {BaseUuid()};
  constexpr ble_uuid128_t qrCharUuid {CharUuid(0x01, 0x00)};

  int QrCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    auto qrService = static_cast<QrService*>(arg);
    return qrService->OnCommand(conn_handle, attr_handle, ctxt);
  }
}

QrService::QrService(Pinetime::System::SystemTask& system)
  : system {system},
    characteristicDefinition {
      {.uuid = &qrCharUuid.u, .access_cb = QrCallback, .arg = this, .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ}, {0}},
    serviceDefinition {
      {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &qrServiceUuid.u, .characteristics = characteristicDefinition},
      {0},
    } {
  qrList[0].name = "Github";
  qrList[0].text = "https://github.com/InfiniTimeOrg/InfiniTime";
  qrList[1].name = "Twitter";
  qrList[1].text = "https://twitter.com/codingfield";
  qrList[2].name = "Mastodon";
  qrList[2].text = "https://mastodon.codingfield.com/@JF";
  qrList[3].name = "Donate!";
  qrList[3].text = "https://liberapay.com/JF002";
}

void QrService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int QrService::OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt) {
  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    size_t notifSize = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t data[notifSize + 1];
    data[notifSize] = '\0';
    os_mbuf_copydata(ctxt->om, 0, notifSize, data);
    char* s = (char*) &data[0];
    NRF_LOG_INFO("DATA : %s", s);

    const char* d = "|";
    const char* qrId = strtok(s, d);
    const char* qrName = strtok(NULL, d);
    const char* qrText = strtok(NULL, d);

    if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t*) &qrCharUuid) == 0) {
      if (qrId != NULL) {
        if (atoi(qrId) == 0 || qrName == NULL || qrText == NULL) {
          qrList[0].name = "Wrong format";
          qrList[0].text = qrId;
        } else if (atoi(qrId) > 0 && atoi(qrId) < MAXLISTITEMS + 1) {
          qrList[atoi(qrId) - 1].name = qrName;
          qrList[atoi(qrId) - 1].text = qrText;
        }
      }
    }
  }
  return 0;
}

std::array<QrService::QrInfo, 4> QrService::getQrList() {
  return qrList;
}
