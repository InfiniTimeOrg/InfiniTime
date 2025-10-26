#include "guardian/BeaconGuard.h"

// NimBLE (wbudowane w InfiniTime)
#include "host/ble_hs.h"
#include "nimble/ble.h"

namespace {
uint8_t adv_data[31];
bool started = false;

void start_adv()
{
  ble_gap_adv_params p{};
  p.conn_mode = BLE_GAP_CONN_MODE_NON;     // non-connectable
  p.disc_mode = BLE_GAP_DISC_MODE_NON;     // non-discoverable (czysty beacon)
  p.itvl_min = 160;                        // 100 ms
  p.itvl_max = 160;                        // 100 ms
  p.channel_map = 0x07;

  // zaczynamy nadawanie na stałe (0 = bez limitu)
  ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, nullptr, BLE_HS_FOREVER, &p, nullptr, nullptr);
}

void set_adv_payload(uint8_t hr, uint8_t batt)
{
  // AD struktury: [len][type][payload...]
  // 1) Flags (0x01): LE General + BR/EDR not supported
  uint8_t i = 0;
  adv_data[i++] = 2;          // len
  adv_data[i++] = 0x01;       // Flags
  adv_data[i++] = 0x06;       // LE General + BR/EDR not supported

  // 2) Manufacturer Specific Data (0xFF)
  // Company ID (tu 0xFFFF jako „test”), a potem nasz ładunek: 'G','R','D', hr, batt
  uint8_t payload[] = { 'G','R','D', hr, batt };
  adv_data[i++] = uint8_t(1 + 2 + sizeof(payload)); // len = type + companyID(2B) + payload
  adv_data[i++] = 0xFF;                              // type
  adv_data[i++] = 0xFF;                              // company ID LSB (demo)
  adv_data[i++] = 0xFF;                              // company ID MSB
  for (auto b : payload) adv_data[i++] = b;

  ble_gap_adv_set_data(adv_data, i);
}
} // namespace

namespace Guardian {
void BeaconGuard::Start()
{
  if (started) return;
  set_adv_payload(0, 0); // pierwsze „puste” dane
  start_adv();
  started = true;
}

void BeaconGuard::Update(uint8_t hr, uint8_t batt)
{
  if (!started) Start();
  set_adv_payload(hr, batt);
}
} // namespace Guardian
