
#include <hal/nrf_rtc.h>
#include "NotificationManager.h"
#include <SystemTask/SystemTask.h>

#include "AlertNotificationService.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t AlertNotificationService::ansUuid;
constexpr ble_uuid16_t AlertNotificationService::ansCharUuid;


int AlertNotificationCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
  auto anService = static_cast<AlertNotificationService*>(arg);
  return anService->OnAlert(conn_handle, attr_handle, ctxt);
}

void AlertNotificationService::Init() {
  ble_gatts_count_cfg(serviceDefinition);
  ble_gatts_add_svcs(serviceDefinition);
}

AlertNotificationService::AlertNotificationService ( Pinetime::System::SystemTask& systemTask, Pinetime::Controllers::NotificationManager& notificationManager ) : m_systemTask{systemTask}, m_notificationManager{notificationManager},
    characteristicDefinition{
                {
                        .uuid = (ble_uuid_t *) &ansCharUuid,
                        .access_cb = AlertNotificationCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_WRITE
                },
                {
                  0
                }
        },
        serviceDefinition{
                {
                        /* Device Information Service */
                        .type = BLE_GATT_SVC_TYPE_PRIMARY,
                        .uuid = (ble_uuid_t *) &ansUuid,
                        .characteristics = characteristicDefinition
                },
                {
                        0
                },
        }
{
}

int AlertNotificationService::OnAlert(uint16_t conn_handle, uint16_t attr_handle,
                                                    struct ble_gatt_access_ctxt *ctxt) {

  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        size_t notifSize = OS_MBUF_PKTLEN(ctxt->om);
        uint8_t data[notifSize + 1];
        data[notifSize] = '\0';
        os_mbuf_copydata(ctxt->om, 0, notifSize, data);
        char *s = (char *) &data[3];
        NRF_LOG_INFO("DATA : %s", s);

        for(int i = 0; i <= notifSize; i++)
        {
            if(s[i] == 0x00)
            {
                s[i] = 0x0A;
            }
        }

        m_notificationManager.Push(Pinetime::Controllers::NotificationManager::Categories::SimpleAlert, s, notifSize + 1);
        m_systemTask.PushMessage(Pinetime::System::SystemTask::Messages::OnNewNotification);
  }
  return 0;
}
