#include "HomeService.h"

#include <libraries/log/nrf_log.h>
#include "components/ble/NimbleController.h"

namespace {
  // 0006yyxx-78fc-48fe-8e23-433b3a1942d0
  constexpr ble_uuid128_t CharUuid(uint8_t x, uint8_t y) {
    return ble_uuid128_t {.u = {.type = BLE_UUID_TYPE_128},
                          .value = {0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, x, y, 0x06, 0x00}};
  }

  // 00060000-78fc-48fe-8e23-433b3a1942d0
  constexpr ble_uuid128_t BaseUuid() {
    return CharUuid(0x00, 0x00);
  }

  constexpr ble_uuid128_t homeUuid {BaseUuid()};

  constexpr ble_uuid128_t homeOpenUuid {CharUuid(0x01, 0x00)};
  constexpr ble_uuid128_t homeLayoutUuid {CharUuid(0x02, 0x00)};
  constexpr ble_uuid128_t homePressUuid {CharUuid(0x03, 0x00)};

  int HomeCallback(uint16_t /*conn_handle*/, uint16_t /*attr_handle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    return static_cast<Pinetime::Controllers::HomeService*>(arg)->OnCommand(ctxt);
  }
} // namespace

Pinetime::Controllers::HomeService::HomeService(NimbleController& nimble) : nimble(nimble) {
  characteristicDefinition[0] = {.uuid = &homeLayoutUuid.u, .access_cb = HomeCallback, .arg = this, .flags = BLE_GATT_CHR_F_WRITE};
  characteristicDefinition[1] = {.uuid = &homeOpenUuid.u,
                                 .access_cb = HomeCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_NOTIFY,
                                 .val_handle = &eventOpenedHandle};
  characteristicDefinition[2] = {.uuid = &homePressUuid.u,
                                 .access_cb = HomeCallback,
                                 .arg = this,
                                 .flags = BLE_GATT_CHR_F_NOTIFY,
                                 .val_handle = &eventPressedHandle};
  characteristicDefinition[3] = {0};

  serviceDefinition[0] = {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &homeUuid.u, .characteristics = characteristicDefinition};
  serviceDefinition[1] = {0};
}

void Pinetime::Controllers::HomeService::Init() {
  uint8_t res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int Pinetime::Controllers::HomeService::OnCommand(ble_gatt_access_ctxt* ctxt) {
  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    size_t bufferSize = OS_MBUF_PKTLEN(ctxt->om);

    uint8_t data[bufferSize];
    os_mbuf_copydata(ctxt->om, 0, bufferSize, data);

    if (ble_uuid_cmp(ctxt->chr->uuid, &homeLayoutUuid.u) == 0) {
      auto screen = std::make_unique<Screen>();
      uint8_t* ptr = &data[0];

      numScreens = *ptr++;
      screen->index = *ptr++;

      screen->cols = *ptr >> 4;
      screen->rows = *ptr & 0x0F;
      ptr++;
      uint8_t num_comps = *(ptr++);

      for (size_t j = 0; j < num_comps; j++) {
        Component comp;
        comp.type = (ComponentType) (*(ptr++));

        comp.x = *ptr >> 4;
        comp.y = *ptr & 0x0F;
        ptr++;
        comp.w = *ptr >> 4;
        comp.h = *ptr & 0x0F;
        ptr++;

        uint8_t label_len = *(ptr++);
        auto label = std::make_unique<char[]>(label_len + 1);
        memcpy(label.get(), ptr, label_len);
        label.get()[label_len] = 0;
        ptr += label_len;

        comp.label = std::move(label);

        screen->components.emplace_back(std::move(comp));
      }

      currentScreen = std::move(screen);
      dataUpdateTime = xTaskGetTickCount();
    }
  }

  return 0;
}

bool Pinetime::Controllers::HomeService::NotifyOpened(int8_t screenIndex) {
  uint16_t connectionHandle = nimble.connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return false;
  }

  auto* om = ble_hs_mbuf_from_flat(&screenIndex, sizeof(screenIndex));
  ble_gattc_notify_custom(connectionHandle, eventOpenedHandle, om);

  return true;
}

bool Pinetime::Controllers::HomeService::OnOpened() {
  return NotifyOpened(0);
}

void Pinetime::Controllers::HomeService::OnViewScreen(uint8_t n) {
  NotifyOpened(n);
}

void Pinetime::Controllers::HomeService::OnClosed() {
  dataUpdateTime = 0;
  numScreens = 0;
  currentScreen.reset();

  NotifyOpened(-1);
}

void Pinetime::Controllers::HomeService::OnPressed(uint8_t screen, uint8_t componentId) {
  uint16_t connectionHandle = nimble.connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  uint8_t v[] = {screen, componentId};
  auto* om = ble_hs_mbuf_from_flat(v, sizeof(v));
  ble_gattc_notify_custom(connectionHandle, eventPressedHandle, om);
}
