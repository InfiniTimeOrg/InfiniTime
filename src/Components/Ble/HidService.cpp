#include "HidService.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t HidService::hidServiceUuid;
constexpr ble_uuid16_t HidService::protocolModeUuid;
constexpr ble_uuid16_t HidService::reportUuid;
constexpr ble_uuid16_t HidService::reportMapUuid;
constexpr ble_uuid16_t HidService::bootKeyboardInputReportUuid;
constexpr ble_uuid16_t HidService::bootKeyboardOutputReportUuid;
constexpr ble_uuid16_t HidService::bootMouseInputReportUuid;
constexpr ble_uuid16_t HidService::informationUuid;
constexpr ble_uuid16_t HidService::controlPointUuid;
constexpr ble_uuid16_t HidService::descriptorUuid;
constexpr ble_uuid16_t HidService::descriptorMapUuid;

namespace {
  char* OperationToString(uint8_t op) {
    char* operation;
    switch(op) {
      case BLE_GATT_ACCESS_OP_READ_CHR: operation = "READ CHR"; break;
      case BLE_GATT_ACCESS_OP_WRITE_CHR: operation = "WRITE CHR"; break;
      case BLE_GATT_ACCESS_OP_READ_DSC: operation = "READ DSC"; break;
      case BLE_GATT_ACCESS_OP_WRITE_DSC: operation = "WRITE DSC"; break;
      default: operation = "?"; break;
    }
    return operation;
  }

  int HidServiceCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    auto* hidService = static_cast<HidService*>(arg);
    return hidService->OnHidServiceRequested(conn_handle, attr_handle, ctxt);
  }

  int HidServiceReportDescriptorCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    auto* hidService = static_cast<HidService*>(arg);
    return hidService->OnReportDescriptorRequested(conn_handle, attr_handle, ctxt);
  }

  int HidServiceReportMouseDescriptorCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    auto* hidService = static_cast<HidService*>(arg);
    return hidService->OnReportDescriptorMouseRequested(conn_handle, attr_handle, ctxt);
  }


  int HidServiceReportMapDescriptorCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    auto* hidService = static_cast<HidService*>(arg);
    return hidService->OnReportMapDescriptorRequested(conn_handle, attr_handle, ctxt);
  }


  static const uint8_t                     report_map_data[] =
          {0x05, 0x01,  // Usage Pg (Generic Desktop)
           0x09, 0x06,  // Usage (Keyboard)
           0xA1, 0x01,  // Collection: (Application)
           0x85, 0x01,  // Report Id (1)
                  //
           0x05, 0x07,  //   Usage Pg (Key Codes)
           0x19, 0xE0,  //   Usage Min (224)
           0x29, 0xE7,  //   Usage Max (231)
           0x15, 0x00,  //   Log Min (0)
           0x25, 0x01,  //   Log Max (1)
                  //
                  //   Modifier byte
           0x75, 0x01,  //   Report Size (1)
           0x95, 0x08,  //   Report Count (8)
           0x81, 0x02,  //   Input: (Data, Variable, Absolute)
                  //
                  //   Reserved byte
           0x95, 0x01,  //   Report Count (1)
           0x75, 0x08,  //   Report Size (8)
           0x81, 0x01,  //   Input: (Constant)
                  //
                  //   LED report
           0x95, 0x05,  //   Report Count (5)
           0x75, 0x01,  //   Report Size (1)
           0x05, 0x08,  //   Usage Pg (LEDs)
           0x19, 0x01,  //   Usage Min (1)
           0x29, 0x05,  //   Usage Max (5)
           0x91, 0x02,  //   Output: (Data, Variable, Absolute)
                  //
                  //   LED report padding
           0x95, 0x01,  //   Report Count (1)
           0x75, 0x03,  //   Report Size (3)
           0x91, 0x01,  //   Output: (Constant)
                  //
                  //   Key arrays (6 bytes)
           0x95, 0x06,  //   Report Count (6)
           0x75, 0x08,  //   Report Size (8)
           0x15, 0x00,  //   Log Min (0)
           0x25, 0x65,  //   Log Max (101)
           0x05, 0x07,  //   Usage Pg (Key Codes)
           0x19, 0x00,  //   Usage Min (0)
           0x29, 0x65,  //   Usage Max (101)
           0x81, 0x00,  //   Input: (Data, Array)
                  //
           0xC0,        // End Collection



           0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)     0
           0x09, 0x02,                         // USAGE (Mouse)                    2
           0xa1, 0x01,                         // COLLECTION (Application)         4
           0x85, 0x02,                         //   REPORT_ID (Mouse)              6
           0x09, 0x01,                         //   USAGE (Pointer)                8
           0xa1, 0x00,                         //   COLLECTION (Physical)          10
           0x05, 0x09,                         //     USAGE_PAGE (Button)          12
           0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)     14
           0x29, 0x02,                         //     USAGE_MAXIMUM (Button 2)     16
           0x15, 0x00,                         //     LOGICAL_MINIMUM (0)          18
           0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          20
           0x75, 0x01,                         //     REPORT_SIZE (1)              22
           0x95, 0x02,                         //     REPORT_COUNT (2)             24
           0x81, 0x02,                         //     INPUT (Data,Var,Abs)         26
           0x95, 0x06,                         //     REPORT_COUNT (6)             28
           0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         30
           0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 32
           0x09, 0x30,                         //     USAGE (X)                    34
           0x09, 0x31,                         //     USAGE (Y)                    36
           0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)       38
           0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)        40
           0x75, 0x08,                         //     REPORT_SIZE (8)              42
           0x95, 0x02,                         //     REPORT_COUNT (2)             44
           0x81, 0x06,                         //     INPUT (Data,Var,Rel)         46
           0xc0,                               //   END_COLLECTION                 48
           0xc0                                // END_COLLECTION                   49/50
          };
}

HidService::HidService() :
        reportInDescriptorDefinition {
                {
                        .uuid = (const ble_uuid_t *) &descriptorUuid,
                        .att_flags = 5,
                        .access_cb = HidServiceCallback,
                },
                {0}
        },
        reportOutDescriptorDefinition {
                {
                        .uuid = (const ble_uuid_t *) &descriptorUuid,
                        .att_flags = 5,
                        .access_cb = HidServiceCallback,
                },
                {0}
        },
        reportKeyboardDescriptorDefinition{{
                                                   .uuid = (const ble_uuid_t *) &descriptorUuid,
                                                   .att_flags = 5,
                                                   .access_cb = HidServiceReportDescriptorCallback,
                                           },                         {0}
        },
        reportMouseDescriptorDefinition{{
                                                   .uuid = (const ble_uuid_t *) &descriptorUuid,
                                                   .att_flags = 5,
                                                   .access_cb = HidServiceReportMouseDescriptorCallback,
                                           },                         {0}
        },
        reportMapDescriptorDefinitions{
                {
                        .uuid = (const ble_uuid_t *) &descriptorMapUuid,
                        .att_flags = 5,
                        .access_cb = HidServiceReportMapDescriptorCallback,
                },{0}
        },
        characteristicDefinition{
                {
                        .uuid = (ble_uuid_t *) &protocolModeUuid,
                        .access_cb = HidServiceCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE_NO_RSP,
                        .val_handle = &protocolModeHandle

                },
                {
                        .uuid = (ble_uuid_t *) &reportUuid,
                        .access_cb = HidServiceCallback,
                        .arg = this,
                        .descriptors = reportKeyboardDescriptorDefinition,
                        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_WRITE,
                        .val_handle = &reportKeyboardHandle
                },
                {
                        .uuid = (ble_uuid_t *) &reportUuid,
                        .access_cb = HidServiceCallback,
                        .arg = this,
                        .descriptors = reportMouseDescriptorDefinition,
                        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_WRITE,
                        .val_handle = &reportMouseHandle
                },
                {
                        .uuid = (ble_uuid_t *) &reportMapUuid,
                        .access_cb = HidServiceCallback,
                        .arg = this,
                        .descriptors = reportMapDescriptorDefinitions,
                        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
                        .val_handle = &reportMapHandle
                },
                /*{
                        .uuid = (ble_uuid_t *) &bootKeyboardInputReportUuid,
                        .access_cb = HidServiceCallback,
                        .arg = this,
                        .descriptors = reportInDescriptorDefinition,
                        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                },
                {
                        .uuid = (ble_uuid_t *) &bootKeyboardOutputReportUuid,
                        .access_cb = HidServiceCallback,
                        .arg = this,
                        .descriptors = reportOutDescriptorDefinition,
                        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
                },
                {
                        .uuid = (ble_uuid_t *) &bootMouseInputReportUuid,
                        .access_cb = HidServiceCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                },*/
                {
                        .uuid = (ble_uuid_t *) &informationUuid,
                        .access_cb = HidServiceCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_READ,
                        .val_handle = &informationHandle
                },
                {
                        .uuid = (ble_uuid_t *) &controlPointUuid,
                        .access_cb = HidServiceCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
                        .val_handle = &controlPointHandle
                },
                {
                        0
                }
        },
        serviceDefinition{
                {
                        /* Device Information Service */
                        .type = BLE_GATT_SVC_TYPE_PRIMARY,
                        .uuid = (ble_uuid_t *) &hidServiceUuid,
                        .characteristics = characteristicDefinition
                },
                {
                        0
                },
        }{

}

void HidService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
  asm("nop");
}

int
HidService::OnHidServiceRequested(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt *context) {
  char* operation = ::OperationToString(context->op);

  int res = 0;
  char* attribute;
  if(attributeHandle == protocolModeHandle) {
    attribute = "protocolModeHandle";
    static uint8_t protcoleModeValue = 1;
    res = os_mbuf_append(context->om, &protcoleModeValue, 1);
  } else if(attributeHandle == reportKeyboardHandle) {
    attribute = "reportHandle";
  } else if(attributeHandle == reportMapHandle) {
    attribute = "reportMapHandle";
    res = os_mbuf_append(context->om, &report_map_data, sizeof(report_map_data));
  }
  else if(attributeHandle == informationHandle) {
    attribute = "informationHandle";
    static uint32_t infoValue = 0x01110002;
    res = os_mbuf_append(context->om, &infoValue, 4);

  }
  else if(attributeHandle == controlPointHandle) {
    attribute = "controlPointHandle";
  }
  else {
    attribute = "???";
  }

  NRF_LOG_INFO("HID : Attribute = %d = %s, operation = %s",attributeHandle,  attribute, operation);
  return res;
}

int HidService::OnReportDescriptorRequested(uint16_t connectionHandle, uint16_t attributeHandle,
                                            ble_gatt_access_ctxt *context) {
  NRF_LOG_INFO("HID : Attribute = %d = Callback report descriptor, operation = %s",attributeHandle, ::OperationToString(context->op));
  int res = 0;
  static uint16_t reportValue = 0x0101;

  res = os_mbuf_append(context->om, &reportValue, 2);
  return res;
}

int HidService::OnReportDescriptorMouseRequested(uint16_t connectionHandle, uint16_t attributeHandle,
                                            ble_gatt_access_ctxt *context) {
  NRF_LOG_INFO("HID : Attribute = %d = Callback report descriptor Mouse, operation = %s",attributeHandle, ::OperationToString(context->op));
  int res = 0;
  static uint16_t reportValue = 0x0102;

  res = os_mbuf_append(context->om, &reportValue, 2);
  return res;
}

int HidService::OnReportMapDescriptorRequested(uint16_t connectionHandle, uint16_t attributeHandle,
                                               ble_gatt_access_ctxt *context) {
  NRF_LOG_INFO("HID : Attribute = %d = Callback report map descriptor, operation = %s",attributeHandle, ::OperationToString(context->op));
  int res = 0;
  static uint16_t externalReportValue = 0x0036;

  res = os_mbuf_append(context->om, &externalReportValue, 2);
  return res;
}

uint8_t helloWorld[] {
  0x0b,
  0x08,
  0x0f,
  0x0f,
  0x12,
  0x2c,
  0x1d,
  0x12,
  0x15,
  0x0f,
  0x07,
  0x25
};
uint8_t testIndex = 0;
bool push = true;
void HidService::Test() {
# if 0
  uint8_t modif = (testIndex == 0) ? (1<<1) : 0;
  if(push && testIndex < 12) {
    uint8_t buf[9]{modif, 0x0, 0x00, helloWorld[testIndex], 0x00, 0x00, 0x00, 0x00, 0x00};
    auto *om = ble_hs_mbuf_from_flat(&buf, 9);

    uint16_t connectionHandle = 1;

    if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
      return;
    }
    ble_gattc_notify_custom(1, reportKeyboardHandle, om);
    testIndex++;
    push = false;
    NRF_LOG_INFO("PUSH %d %d", helloWorld[testIndex], modif);
  } else {
    static uint8_t buf[9]{0, 0x0, 0x00, 0, 0x00, 0x00, 0x00, 0x00, 0x00};
    auto *om = ble_hs_mbuf_from_flat(&buf, 9);

    uint16_t connectionHandle = 1;

    if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
      return;
    }
    ble_gattc_notify_custom(1, reportKeyboardHandle, om);
    NRF_LOG_INFO("Release");
    push = true;
  }

#endif

#if 0
  if(testIndex%2 == 0) {
    uint8_t buf[3]{0, 2, 0};
    auto *om = ble_hs_mbuf_from_flat(&buf, 3);

    uint16_t connectionHandle = 1;

    if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
      return;
    }
    ble_gattc_notify_custom(1, reportMouseHandle, om);
    testIndex++;
    push = false;
    NRF_LOG_INFO("UNPUSH");
  } else {
    uint8_t buf[3]{0, 0, 2};
    auto *om = ble_hs_mbuf_from_flat(&buf, 3);

    uint16_t connectionHandle = 1;

    if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
      return;
    }
    ble_gattc_notify_custom(1, reportMouseHandle, om);
    testIndex++;
    push = false;
    NRF_LOG_INFO("PUSH");
  }
#endif
}

void HidService::SendMoveReport(uint8_t x, uint8_t y) {
  uint8_t buf[3]{0, x, y};
  auto *om = ble_hs_mbuf_from_flat(&buf, 3);

  uint16_t connectionHandle = 1;

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }
  ble_gattc_notify_custom(1, reportMouseHandle, om);
  NRF_LOG_INFO("move");
}


