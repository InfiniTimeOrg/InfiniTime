#include "QrService.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

int QRCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
  auto qrService = static_cast<QrService *>(arg);
  return qrService->OnCommand(conn_handle, attr_handle, ctxt);
}

QrService::QrService(Pinetime::System::SystemTask &system) : m_system(system) {
  qrsTextCharUuid.value[11] = qrsTextCharId[0];
  qrsTextCharUuid.value[12] = qrsTextCharId[1];

  characteristicDefinition[0] = {.uuid = (ble_uuid_t *) (&qrsTextCharUuid),
      .access_cb = QRCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ
  };
  characteristicDefinition[1] = {0};

  serviceDefinition[0] = {
      .type = BLE_GATT_SVC_TYPE_PRIMARY,
      .uuid = (ble_uuid_t *) &qrsUuid,
      .characteristics = characteristicDefinition
  };
  serviceDefinition[1] = {0};
  
  qrList[0].name = "Github";
  qrList[0].text = "https://github.com/JF002/InfiniTime";
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

int QrService::OnCommand(uint16_t conn_handle, uint16_t attr_handle,
                                                   struct ble_gatt_access_ctxt *ctxt) {
  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    size_t notifSize = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t data[notifSize + 1];
    data[notifSize] = '\0';
    os_mbuf_copydata(ctxt->om, 0, notifSize, data);
    char *s = (char *) &data[0];
    NRF_LOG_INFO("DATA : %s", s);
    
    const char d[2] = "|";
    char* qrId = strtok(s, d);
    char* qrName = strtok(NULL, d);
    char* qrText = strtok(NULL, d);
    
    if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t *) &qrsTextCharUuid) == 0) {
      if (std::stoi(qrId) < MAXLISTITEMS && qrId > 0 && qrName != NULL && qrText != NULL) {
        qrList[std::stoi(qrId)].name = qrName;
        qrList[std::stoi(qrId)].text = qrText;
      }
    }
  }
  return 0;
}

std::array<QrService::QrInfo, 4> QrService::getQrList() {
  return qrList;
}
