#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include <BlinkApp/BlinkApp.h>
#include <libraries/bsp/bsp.h>
#include <legacy/nrf_drv_clock.h>
#include <libraries/timer/app_timer.h>
#include <libraries/gpiote/app_gpiote.h>
#include <DisplayApp/DisplayApp.h>
#include <softdevice/common/nrf_sdh.h>
#include <softdevice/common/nrf_sdh_ble.h>
#include <ble/nrf_ble_qwr/nrf_ble_qwr.h>
#include <ble/nrf_ble_gatt/nrf_ble_gatt.h>
#include "ble_advdata.h"
#include "nrf_sdh_soc.h"
#include <ble/ble_advertising/ble_advertising.h>
#include <ble/ble_services/ble_hrs/ble_hrs.h>
#include <ble/ble_services/ble_bas/ble_bas.h>
#include <ble/ble_services/ble_dis/ble_dis.h>
#include <ble/common/ble_conn_params.h>
#include <ble/peer_manager/peer_manager.h>
#include <ble/peer_manager/peer_manager_handler.h>
#include <softdevice/common/nrf_sdh_freertos.h>

#include <ble/ble_db_discovery/ble_db_discovery.h>
#include <ble/ble_services/ble_cts_c/ble_cts_c.h>

#if NRF_LOG_ENABLED
#include "Logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
#include "Logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

Pinetime::Applications::BlinkApp blinkApp;
Pinetime::Applications::DisplayApp displayApp;
TaskHandle_t systemThread;

static void on_cts_c_evt(ble_cts_c_t * p_cts, ble_cts_c_evt_t * p_evt);
static void current_time_print(ble_cts_c_evt_t * p_evt);

extern "C" {
  void vApplicationIdleHook() {
    logger.Resume();
  }

  void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName ) {
    bsp_board_led_on(3);
  }
}

static void bsp_event_handler(bsp_event_t event)
{
  switch (event)
  {
    case BSP_EVENT_KEY_0:
      NRF_LOG_INFO("Button pressed");
      break;
    default:
      break;
  }
}



#define APP_BLE_CONN_CFG_TAG                1                                       /**< A tag identifying the SoftDevice BLE configuration. */
#define APP_BLE_OBSERVER_PRIO               3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
static uint16_t m_conn_handle         = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */
NRF_BLE_QWR_DEF(m_qwr);                                             /**< Context for the Queued Write module.*/
#define DEVICE_NAME                         "PineTime"                            /**< Name of device. Will be included in the advertising data. */

#define MIN_CONN_INTERVAL                   MSEC_TO_UNITS(400, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.4 seconds). */
#define MAX_CONN_INTERVAL                   MSEC_TO_UNITS(650, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.65 second). */
#define SLAVE_LATENCY                       0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                    MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory time-out (4 seconds). */
NRF_BLE_GATT_DEF(m_gatt);                                           /**< GATT module instance. */
#define APP_ADV_INTERVAL                    300                                     /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_DURATION                    18000                                       /**< The advertising duration (180 seconds) in units of 10 milliseconds. */
BLE_ADVERTISING_DEF(m_advertising);                                 /**< Advertising module instance. */

static ble_uuid_t m_adv_uuids[] =                                   /**< Universally unique service identifiers. */
        {
                {BLE_UUID_HEART_RATE_SERVICE, BLE_UUID_TYPE_BLE},
                {BLE_UUID_BATTERY_SERVICE, BLE_UUID_TYPE_BLE},
                {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE},
                {BLE_UUID_CURRENT_TIME_SERVICE, BLE_UUID_TYPE_BLE}
        };

BLE_HRS_DEF(m_hrs);                                                 /**< Heart rate service instance. */
BLE_BAS_DEF(m_bas);                                                 /**< Battery service instance. */
BLE_CTS_C_DEF(m_cts_c);                                                             /**< Current Time service instance. */
static pm_peer_id_t m_peer_id;
BLE_DB_DISCOVERY_DEF(m_ble_db_discovery);

#define MANUFACTURER_NAME                   "Codingfield"
#define FIRST_CONN_PARAMS_UPDATE_DELAY      5000                                    /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY       30000                                   /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT        3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                      1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                      0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                      0                                       /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS                  0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                       0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE              7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE              16                                      /**< Maximum encryption key size. */

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
  uint32_t err_code;

  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
      NRF_LOG_INFO("Connected");
      err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
      APP_ERROR_CHECK(err_code);
      m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
      err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
      APP_ERROR_CHECK(err_code);
      break;

    case BLE_GAP_EVT_DISCONNECTED:
      NRF_LOG_INFO("Disconnected");
      m_conn_handle = BLE_CONN_HANDLE_INVALID;
      if (p_ble_evt->evt.gap_evt.conn_handle == m_cts_c.conn_handle)
      {
        m_cts_c.conn_handle = BLE_CONN_HANDLE_INVALID;
      }
      break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
      NRF_LOG_DEBUG("PHY update request.");
      ble_gap_phys_t const phys =
              {

                      .tx_phys = BLE_GAP_PHY_AUTO,
                      .rx_phys = BLE_GAP_PHY_AUTO,
              };
      err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
      APP_ERROR_CHECK(err_code);
    } break;

    case BLE_GATTC_EVT_TIMEOUT:
      // Disconnect on GATT Client timeout event.
      NRF_LOG_DEBUG("GATT Client Timeout.");
      err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                       BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      APP_ERROR_CHECK(err_code);
      break;

    case BLE_GATTS_EVT_TIMEOUT:
      // Disconnect on GATT Server timeout event.
      NRF_LOG_DEBUG("GATT Server Timeout.");
      err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                       BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      APP_ERROR_CHECK(err_code);
      break;

    default:
      // No implementation needed.
      break;
  }
}

static void gap_params_init(void)
{
  ret_code_t              err_code;
  ble_gap_conn_params_t   gap_conn_params;
  ble_gap_conn_sec_mode_t sec_mode;

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

  err_code = sd_ble_gap_device_name_set(&sec_mode,
                                        (const uint8_t *)DEVICE_NAME,
                                        strlen(DEVICE_NAME));
  APP_ERROR_CHECK(err_code);

  err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT);
  APP_ERROR_CHECK(err_code);

  memset(&gap_conn_params, 0, sizeof(gap_conn_params));

  gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency     = SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
  APP_ERROR_CHECK(err_code);
}

static void gatt_init(void)
{
  ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
  APP_ERROR_CHECK(err_code);
}

static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
  uint32_t err_code;

  switch (ble_adv_evt)
  {
    case BLE_ADV_EVT_FAST:
      NRF_LOG_INFO("Fast advertising.");
      err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
      APP_ERROR_CHECK(err_code);
      break;

    case BLE_ADV_EVT_IDLE:
//      sleep_mode_enter();
      break;

    default:
      break;
  }
}

static void advertising_init(void)
{
  ret_code_t             err_code;
  ble_advertising_init_t init;

  memset(&init, 0, sizeof(init));

  init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
  init.advdata.include_appearance      = true;
  init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
  init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

  init.config.ble_adv_whitelist_enabled = true;
  init.config.ble_adv_fast_enabled  = true;
  init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
  init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

  init.evt_handler = on_adv_evt;

  err_code = ble_advertising_init(&m_advertising, &init);
  APP_ERROR_CHECK(err_code);

  ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

static void nrf_qwr_error_handler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);
}

void ble_srv_error_handler(uint32_t nrf_error) {
  NRF_LOG_INFO("ble_srv_error_handler");
}


void ble_cts_c_evt_handler(ble_cts_c_t * p_cts, ble_cts_c_evt_t * p_evt) {
  NRF_LOG_INFO("ble_cts_c_evt_handler");
}

void ble_db_discovery_evt_handler(ble_db_discovery_evt_t * p_evt) {
  NRF_LOG_INFO("ble_db_discovery_evt_handler");

}

static void current_time_error_handler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);
}


static void services_init(void)
{
  ret_code_t         err_code;
  ble_hrs_init_t     hrs_init;
  ble_bas_init_t     bas_init;
  ble_dis_init_t     dis_init;
  ble_cts_c_init_t   cts_init;
  nrf_ble_qwr_init_t qwr_init = {0};
  uint8_t            body_sensor_location;

  // Initialize Queued Write Module.
  qwr_init.error_handler = nrf_qwr_error_handler;

  err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
  APP_ERROR_CHECK(err_code);

  // Initialize Heart Rate Service.
  body_sensor_location = BLE_HRS_BODY_SENSOR_LOCATION_FINGER;

  memset(&hrs_init, 0, sizeof(hrs_init));

  hrs_init.evt_handler                 = NULL;
  hrs_init.is_sensor_contact_supported = true;
  hrs_init.p_body_sensor_location      = &body_sensor_location;

  // Here the sec level for the Heart Rate Service can be changed/increased.
  hrs_init.hrm_cccd_wr_sec = SEC_OPEN;
  hrs_init.bsl_rd_sec      = SEC_OPEN;

   // Initialize Battery Service.
  memset(&bas_init, 0, sizeof(bas_init));

  // Here the sec level for the Battery Service can be changed/increased.
  bas_init.bl_rd_sec        = SEC_OPEN;
  bas_init.bl_cccd_wr_sec   = SEC_OPEN;
  bas_init.bl_report_rd_sec = SEC_OPEN;

  bas_init.evt_handler          = NULL;
  bas_init.support_notification = true;
  bas_init.p_report_ref         = NULL;
  bas_init.initial_batt_level   = 100;

  err_code = ble_bas_init(&m_bas, &bas_init);
  APP_ERROR_CHECK(err_code);

  // Initialize Device Information Service.
  memset(&dis_init, 0, sizeof(dis_init));

  ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);

  dis_init.dis_char_rd_sec = SEC_OPEN;

  err_code = ble_dis_init(&dis_init);
  APP_ERROR_CHECK(err_code);

  // Initialize CTS.
  cts_init.evt_handler   = on_cts_c_evt;
  cts_init.error_handler = current_time_error_handler;
  err_code               = ble_cts_c_init(&m_cts_c, &cts_init);
  APP_ERROR_CHECK(err_code);
}

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
  ret_code_t err_code;

  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
  {
    err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
  }
}

static void conn_params_error_handler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);
}


static void conn_params_init(void)
{
  ret_code_t             err_code;
  ble_conn_params_init_t cp_init;

  memset(&cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params                  = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle    = m_hrs.hrm_handles.cccd_handle;
  cp_init.disconnect_on_fail             = false;
  cp_init.evt_handler                    = on_conn_params_evt;
  cp_init.error_handler                  = conn_params_error_handler;

  err_code = ble_conn_params_init(&cp_init);
  APP_ERROR_CHECK(err_code);
}


static void ble_stack_init(void)
{
  ret_code_t err_code;

  err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
  APP_ERROR_CHECK(err_code);

  // Enable BLE stack.
  err_code = nrf_sdh_ble_enable(&ram_start);
  APP_ERROR_CHECK(err_code);

  // Register a handler for BLE events.
  NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

static void delete_bonds(void)
{
  ret_code_t err_code;

  NRF_LOG_INFO("Erase bonds!");

  err_code = pm_peers_delete();
  APP_ERROR_CHECK(err_code);
}


static void advertising_start(void * p_erase_bonds)
{
  bool erase_bonds = *(bool*)p_erase_bonds;

  if (erase_bonds)
  {

    delete_bonds();
    // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
  }
  else
  {
//    memset(m_whitelist_peers, PM_PEER_ID_INVALID, sizeof(m_whitelist_peers));
//    m_whitelist_peer_cnt = (sizeof(m_whitelist_peers) / sizeof(pm_peer_id_t));
//
//    peer_list_get(m_whitelist_peers, &m_whitelist_peer_cnt);
//
//    ret = pm_whitelist_set(m_whitelist_peers, m_whitelist_peer_cnt);
//    APP_ERROR_CHECK(ret);
//
//    // Setup the device identies list.
//    // Some SoftDevices do not support this feature.
//    ret = pm_device_identities_list_set(m_whitelist_peers, m_whitelist_peer_cnt);
//    if (ret != NRF_ERROR_NOT_SUPPORTED)
//    {
//      APP_ERROR_CHECK(ret);
//    }
    ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
  }
}


static void pm_evt_handler(pm_evt_t const * p_evt)
{
  bool delete_bonds = false;
  ret_code_t err_code;
  pm_handler_on_pm_evt(p_evt);
  pm_handler_flash_clean(p_evt);

  auto idToStr = [](int id){
    switch(id) {
      case PM_EVT_BONDED_PEER_CONNECTED : return "PM_EVT_BONDED_PEER_CONNECTED";
      case PM_EVT_CONN_SEC_START : return "PM_EVT_CONN_SEC_START";
      case PM_EVT_CONN_SEC_SUCCEEDED : return "PM_EVT_CONN_SEC_SUCCEEDED";
      case PM_EVT_CONN_SEC_FAILED : return "PM_EVT_CONN_SEC_FAILED";
      case PM_EVT_CONN_SEC_CONFIG_REQ : return "PM_EVT_CONN_SEC_CONFIG_REQ";
      case PM_EVT_CONN_SEC_PARAMS_REQ : return "PM_EVT_CONN_SEC_PARAMS_REQ";
      case PM_EVT_STORAGE_FULL : return "PM_EVT_STORAGE_FULL";
      case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED : return "PM_EVT_PEER_DATA_UPDATE_SUCCEEDED";
      case PM_EVT_ERROR_UNEXPECTED : return "PM_EVT_ERROR_UNEXPECTED";
      case PM_EVT_PEER_DATA_UPDATE_FAILED : return "PM_EVT_PEER_DATA_UPDATE_FAILED";
      case PM_EVT_PEER_DELETE_SUCCEEDED : return "PM_EVT_PEER_DELETE_SUCCEEDED";
      case PM_EVT_PEER_DELETE_FAILED : return "PM_EVT_PEER_DELETE_FAILED";
      case PM_EVT_PEERS_DELETE_SUCCEEDED : return "PM_EVT_PEERS_DELETE_SUCCEEDED";
      case PM_EVT_PEERS_DELETE_FAILED : return "PM_EVT_PEERS_DELETE_FAILED";
      case PM_EVT_LOCAL_DB_CACHE_APPLIED : return "PM_EVT_LOCAL_DB_CACHE_APPLIED";
      case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED : return "PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED";
      case PM_EVT_SERVICE_CHANGED_IND_SENT : return "PM_EVT_SERVICE_CHANGED_IND_SENT";
      case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED : return "PM_EVT_SERVICE_CHANGED_IND_CONFIRMED";
      case PM_EVT_SLAVE_SECURITY_REQ : return "PM_EVT_SLAVE_SECURITY_REQ";
      case PM_EVT_FLASH_GARBAGE_COLLECTED : return "PM_EVT_FLASH_GARBAGE_COLLECTED";
      case PM_EVT_FLASH_GARBAGE_COLLECTION_FAILED : return "PM_EVT_FLASH_GARBAGE_COLLECTION_FAILED";


    }
    return "Unknown";
  };

  char toto[64] {0};
  NRF_LOG_INFO("pm_evt_handler %s", idToStr(p_evt->evt_id));

  switch (p_evt->evt_id)
  {
    case PM_EVT_CONN_SEC_SUCCEEDED:
    {
      m_peer_id = p_evt->peer_id;

      // Discover peer's services.
      err_code  = ble_db_discovery_start(&m_ble_db_discovery, p_evt->conn_handle);
      APP_ERROR_CHECK(err_code);
    } break;

    case PM_EVT_PEERS_DELETE_SUCCEEDED:
      advertising_start(&delete_bonds);
      break;

    default:
      break;
  }
}

static void peer_manager_init(void)
{
  ble_gap_sec_params_t sec_param;
  ret_code_t           err_code;

  err_code = pm_init();
  APP_ERROR_CHECK(err_code);

  memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

  // Security parameters to be used for all security procedures.
  sec_param.bond           = SEC_PARAM_BOND;
  sec_param.mitm           = SEC_PARAM_MITM;
  sec_param.lesc           = SEC_PARAM_LESC;
  sec_param.keypress       = SEC_PARAM_KEYPRESS;
  sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
  sec_param.oob            = SEC_PARAM_OOB;
  sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
  sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
  sec_param.kdist_own.enc  = 1;
  sec_param.kdist_own.id   = 1;
  sec_param.kdist_peer.enc = 1;
  sec_param.kdist_peer.id  = 1;

  err_code = pm_sec_params_set(&sec_param);
  APP_ERROR_CHECK(err_code);

  err_code = pm_register(pm_evt_handler);
  APP_ERROR_CHECK(err_code);
}

void SystemTask(void *) {
  APP_GPIOTE_INIT(2);
  app_timer_init();
  bool erase_bonds=false;
  nrf_sdh_freertos_init(advertising_start, &erase_bonds);
//  blinkApp.Start();
  displayApp.Start();

  while (1) {
    vTaskSuspend(nullptr);
  }
}

static void on_cts_c_evt(ble_cts_c_t * p_cts, ble_cts_c_evt_t * p_evt)
{
  ret_code_t err_code;

  NRF_LOG_INFO("CTS %d" , p_evt->evt_type);

  switch (p_evt->evt_type)
  {
    case BLE_CTS_C_EVT_DISCOVERY_COMPLETE:
      NRF_LOG_INFO("Current Time Service discovered on server.");
      err_code = ble_cts_c_handles_assign(&m_cts_c,
                                          p_evt->conn_handle,
                                          &p_evt->params.char_handles);

      ble_cts_c_current_time_read(&m_cts_c);
      APP_ERROR_CHECK(err_code);
      break;

    case BLE_CTS_C_EVT_DISCOVERY_FAILED:
      NRF_LOG_INFO("Current Time Service not found on server. ");
      // CTS not found in this case we just disconnect. There is no reason to stay
      // in the connection for this simple app since it all wants is to interact with CT
      if (p_evt->conn_handle != BLE_CONN_HANDLE_INVALID)
      {
        err_code = sd_ble_gap_disconnect(p_evt->conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
      }
      break;

    case BLE_CTS_C_EVT_DISCONN_COMPLETE:
      NRF_LOG_INFO("Disconnect Complete.");
      break;

    case BLE_CTS_C_EVT_CURRENT_TIME:
      NRF_LOG_INFO("Current Time received.");
      current_time_print(p_evt);
      break;

    case BLE_CTS_C_EVT_INVALID_TIME:
      NRF_LOG_INFO("Invalid Time received.");
      break;

    default:
      break;
  }
}

static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
  ble_cts_c_on_db_disc_evt(&m_cts_c, p_evt);
}


static void db_discovery_init(void)
{
  ret_code_t err_code = ble_db_discovery_init(db_disc_handler);
  APP_ERROR_CHECK(err_code);
}

static char const * day_of_week[] =
        {
                "Unknown",
                "Monday",
                "Tuesday",
                "Wednesday",
                "Thursday",
                "Friday",
                "Saturday",
                "Sunday"
        };

static char const * month_of_year[] =
        {
                "Unknown",
                "January",
                "February",
                "March",
                "April",
                "May",
                "June",
                "July",
                "August",
                "September",
                "October",
                "November",
                "December"
        };


static void current_time_print(ble_cts_c_evt_t * p_evt)
{
  NRF_LOG_INFO("\r\nCurrent Time:");
  NRF_LOG_INFO("\r\nDate:");

  NRF_LOG_INFO("\tDay of week   %s", (uint32_t)day_of_week[p_evt->
          params.
          current_time.
          exact_time_256.
          day_date_time.
          day_of_week]);

  if (p_evt->params.current_time.exact_time_256.day_date_time.date_time.day == 0)
  {
    NRF_LOG_INFO("\tDay of month  Unknown");
  }
  else
  {
    NRF_LOG_INFO("\tDay of month  %i",
                 p_evt->params.current_time.exact_time_256.day_date_time.date_time.day);
  }

  NRF_LOG_INFO("\tMonth of year %s",
               (uint32_t)month_of_year[p_evt->params.current_time.exact_time_256.day_date_time.date_time.month]);
  if (p_evt->params.current_time.exact_time_256.day_date_time.date_time.year == 0)
  {
    NRF_LOG_INFO("\tYear          Unknown");
  }
  else
  {
    NRF_LOG_INFO("\tYear          %i",
                 p_evt->params.current_time.exact_time_256.day_date_time.date_time.year);
  }
  NRF_LOG_INFO("\r\nTime:");
  NRF_LOG_INFO("\tHours     %i",
               p_evt->params.current_time.exact_time_256.day_date_time.date_time.hours);
  NRF_LOG_INFO("\tMinutes   %i",
               p_evt->params.current_time.exact_time_256.day_date_time.date_time.minutes);
  NRF_LOG_INFO("\tSeconds   %i",
               p_evt->params.current_time.exact_time_256.day_date_time.date_time.seconds);
  NRF_LOG_INFO("\tFractions %i/256 of a second",
               p_evt->params.current_time.exact_time_256.fractions256);

  NRF_LOG_INFO("\r\nAdjust reason:\r");
  NRF_LOG_INFO("\tDaylight savings %x",
               p_evt->params.current_time.adjust_reason.change_of_daylight_savings_time);
  NRF_LOG_INFO("\tTime zone        %x",
               p_evt->params.current_time.adjust_reason.change_of_time_zone);
  NRF_LOG_INFO("\tExternal update  %x",
               p_evt->params.current_time.adjust_reason.external_reference_time_update);
  NRF_LOG_INFO("\tManual update    %x",
               p_evt->params.current_time.adjust_reason.manual_time_update);

  displayApp.SetTime(p_evt->params.current_time.exact_time_256.day_date_time.date_time.minutes,
                     p_evt->params.current_time.exact_time_256.day_date_time.date_time.hours);
}


int main(void) {
  logger.Init();
  nrf_drv_clock_init();


  if (pdPASS != xTaskCreate(SystemTask, "MAIN", 256, nullptr, 0, &systemThread))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);

  ble_stack_init();
  gap_params_init();
  gatt_init();
  db_discovery_init();

  advertising_init();


  peer_manager_init();
  services_init();
  conn_params_init();



  vTaskStartScheduler();

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }
}



