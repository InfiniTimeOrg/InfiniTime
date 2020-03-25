#include <libraries/util/sdk_errors.h>
#include <softdevice/common/nrf_sdh.h>
#include <libraries/util/app_error.h>
#include <softdevice/common/nrf_sdh_ble.h>
#include <libraries/log/nrf_log.h>
#include <ble/nrf_ble_qwr/nrf_ble_qwr.h>
#include <ble/ble_services/ble_cts_c/ble_cts_c.h>
#include <ble/nrf_ble_gatt/nrf_ble_gatt.h>
#include <ble/ble_advertising/ble_advertising.h>
#include <ble/peer_manager/peer_manager.h>
#include <ble/peer_manager/peer_manager_handler.h>
#include <ble/ble_services/ble_hrs/ble_hrs.h>
#include <ble/ble_services/ble_bas/ble_bas.h>
#include <ble/ble_services/ble_dis/ble_dis.h>
#include <ble/ble_services/ble_ans_c/ble_ans_c.h>
#include <ble/common/ble_conn_params.h>
#include <libraries/fds/fds.h>
#include "nrf_sdh_soc.h"

#include "BleManager.h"

void ble_manager_init_stack();

void ble_manager_init_gap_params();


void ble_manager_init_gatt();
void ble_manager_init_db_discovery();
void ble_manager_init_advertising();
void ble_manager_init_peer_manager();
void ble_manager_init_services();
void ble_manager_init_connection_params();

void ble_manager_event_handler(ble_evt_t const *p_ble_evt, void *p_context);
void ble_manager_discover_handler(ble_db_discovery_evt_t *p_evt);
void ble_manager_advertising_event_handler(ble_adv_evt_t ble_adv_evt);
void ble_manager_peer_manager_event_handler(pm_evt_t const *p_evt);
void ble_manager_delete_bonds();
void ble_manager_queue_write_error_handler(uint32_t nrf_error);
void ble_manager_cts_event_handler(ble_cts_c_t *p_cts, ble_cts_c_evt_t *p_evt);
void ble_manager_cts_error_handler(uint32_t nrf_error);
void ble_manager_cts_print_time(ble_cts_c_evt_t *p_evt);
void ble_manager_conn_params_event_handler(ble_conn_params_evt_t *p_evt);
void ble_manager_conn_params_error_handler(uint32_t nrf_error);

typedef enum
{
  ALERT_NOTIFICATION_DISABLED, /**< Alert Notifications has been disabled. */
  ALERT_NOTIFICATION_ENABLED,  /**< Alert Notifications has been enabled. */
  ALERT_NOTIFICATION_ON,       /**< Alert State is on. */
} ble_ans_c_alert_state_t;

void on_ans_c_evt(ble_ans_c_evt_t * p_evt);
void alert_notification_error_handler(uint32_t nrf_error);
void handle_alert_notification(ble_ans_c_evt_t * p_evt);
void supported_alert_notification_read(void);
void alert_notification_setup(void);
void control_point_setup(ble_ans_c_evt_t * p_evt);

uint16_t ble_manager_connection_handle = BLE_CONN_HANDLE_INVALID; // Handle of the current connection.
NRF_BLE_QWR_DEF(ble_manager_queue_write); // Context for the Queued Write module.
BLE_CTS_C_DEF(ble_manager_cts_client); // Current Time service instance.
NRF_BLE_GATT_DEF(ble_manager_gatt); // GATT module instance.
BLE_ADVERTISING_DEF(ble_manager_advertising); // Advertising module instance.
BLE_DB_DISCOVERY_DEF(ble_manager_db_discovery);
BLE_ANS_C_DEF(m_ans_c);

static uint8_t m_alert_message_buffer[MESSAGE_BUFFER_SIZE]; /**< Message buffer for optional notify messages. */
static ble_ans_c_alert_state_t m_new_alert_state    = ALERT_NOTIFICATION_DISABLED;  /**< State that holds the current state of New Alert Notifications, i.e. Enabled, Alert On, Disabled. */
static ble_ans_c_alert_state_t m_unread_alert_state = ALERT_NOTIFICATION_DISABLED;  /**< State that holds the current state of Unread Alert Notifications, i.e. Enabled, Alert On, Disabled. */

static ble_uuid_t ble_manager_advertising_uuids[] = /* Universally unique service identifiers.*/
        {
//                {BLE_UUID_HEART_RATE_SERVICE,         BLE_UUID_TYPE_BLE},
//                {BLE_UUID_BATTERY_SERVICE,            BLE_UUID_TYPE_BLE},
                {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE},
                {BLE_UUID_CURRENT_TIME_SERVICE,       BLE_UUID_TYPE_BLE}
        };

static char const *day_of_week[] =
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

static char const *month_of_year[] =
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

static char const * lit_catid[BLE_ANS_NB_OF_CATEGORY_ID] =
        {
                "Simple alert",
                "Email",
                "News",
                "Incoming call",
                "Missed call",
                "SMS/MMS",
                "Voice mail",
                "Schedule",
                "High prioritized alert",
                "Instant message"
        };


void ble_manager_init() {
  ble_manager_init_stack();
  ble_manager_init_gap_params();
  ble_manager_init_gatt();
  ble_manager_init_db_discovery();
  ble_manager_init_advertising();
  ble_manager_init_services();
  ble_manager_init_connection_params();
}

void ble_manager_init_stack() {
  ret_code_t err_code;

  err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  err_code = nrf_sdh_ble_default_cfg_set(BLE_MANAGER_CONN_CFG_TAG, &ram_start);
  APP_ERROR_CHECK(err_code);

  // Enable BLE stack.
  err_code = nrf_sdh_ble_enable(&ram_start);
  APP_ERROR_CHECK(err_code);

  // Register a handler for BLE events.
  NRF_SDH_BLE_OBSERVER(m_ble_observer, BLE_MANAGER__OBSERVER_PRIO, ble_manager_event_handler, NULL);
}

void (*OnNewTimeCallback)(current_time_char_t*);
void ble_manager_set_new_time_callback(void (*OnNewTime)(current_time_char_t*)) {
  OnNewTimeCallback = OnNewTime;
}

void (*OnBleConnectionCallback)();
void ble_manager_set_ble_connection_callback(void (*OnBleConnection)()) {
  OnBleConnectionCallback = OnBleConnection;
}

void (*OnBleDisconnectionCallback)();
void ble_manager_set_ble_disconnection_callback(void (*OnBleDisconnection)()) {
  OnBleDisconnectionCallback = OnBleDisconnection;
}

void (*OnNewNotificationCallback)(const char* message, uint8_t size);
void ble_manager_set_new_notification_callback(void (*OnNewNotification)(const char*, uint8_t size)) {
  OnNewNotificationCallback = OnNewNotification;
}


void ble_manager_event_handler(ble_evt_t const *p_ble_evt, void *p_context) {
  uint32_t err_code;

  switch (p_ble_evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
      NRF_LOG_INFO("Connected");
      ble_manager_connection_handle = p_ble_evt->evt.gap_evt.conn_handle;
      err_code = nrf_ble_qwr_conn_handle_assign(&ble_manager_queue_write, ble_manager_connection_handle);
      OnBleConnectionCallback();
      APP_ERROR_CHECK(err_code);
      break;

    case BLE_GAP_EVT_DISCONNECTED:
      NRF_LOG_INFO("Disconnected");
      ble_manager_connection_handle = BLE_CONN_HANDLE_INVALID;
      if (p_ble_evt->evt.gap_evt.conn_handle == ble_manager_cts_client.conn_handle) {
        ble_manager_cts_client.conn_handle = BLE_CONN_HANDLE_INVALID;
      }
      OnBleDisconnectionCallback();
      break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
      NRF_LOG_DEBUG("PHY update request.");
      ble_gap_phys_t const phys =
              {

                      .tx_phys = BLE_GAP_PHY_AUTO,
                      .rx_phys = BLE_GAP_PHY_AUTO,
              };
      err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
      APP_ERROR_CHECK(err_code);
    }
      break;

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

void ble_manager_init_gap_params() {
  ret_code_t err_code;
  ble_gap_conn_params_t gap_conn_params;
  ble_gap_conn_sec_mode_t sec_mode;

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

  err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) BLE_MANAGER_DEVICE_NAME,
                                        strlen(BLE_MANAGER_DEVICE_NAME));
  APP_ERROR_CHECK(err_code);

  err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT);
  APP_ERROR_CHECK(err_code);

  memset(&gap_conn_params, 0, sizeof(gap_conn_params));

  gap_conn_params.min_conn_interval = BLE_MANAGER_MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = BLE_MANAGER_MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency = BLE_MANAGER_SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout = BLE_MANAGER_CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
  APP_ERROR_CHECK(err_code);
}

void ble_manager_init_gatt() {
  ret_code_t err_code = nrf_ble_gatt_init(&ble_manager_gatt, NULL);
  APP_ERROR_CHECK(err_code);
}

void ble_manager_init_db_discovery() {
  ret_code_t err_code = ble_db_discovery_init(ble_manager_discover_handler);
  APP_ERROR_CHECK(err_code);
}

void ble_manager_discover_handler(ble_db_discovery_evt_t *p_evt) {
  ble_cts_c_on_db_disc_evt(&ble_manager_cts_client, p_evt);
  NRF_LOG_INFO("ble_ans_c_on_db_disc_evt");
  ble_ans_c_on_db_disc_evt(&m_ans_c, p_evt);
}

void ble_manager_init_advertising() {
  ret_code_t err_code;
  ble_advertising_init_t init;

  memset(&init, 0, sizeof(init));

  init.advdata.name_type = BLE_ADVDATA_FULL_NAME;
  init.advdata.include_appearance = true;
  init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  init.advdata.uuids_complete.uuid_cnt =
          sizeof(ble_manager_advertising_uuids) / sizeof(ble_manager_advertising_uuids[0]);
  init.advdata.uuids_complete.p_uuids = ble_manager_advertising_uuids;

  init.config.ble_adv_whitelist_enabled = true;
  init.config.ble_adv_fast_enabled = true;
  init.config.ble_adv_fast_interval = BLE_MANAGER_ADV_INTERVAL;
  init.config.ble_adv_fast_timeout = BLE_MANAGER_ADV_DURATION;

  init.evt_handler = ble_manager_advertising_event_handler;

  err_code = ble_advertising_init(&ble_manager_advertising, &init);
  APP_ERROR_CHECK(err_code);

  ble_advertising_conn_cfg_tag_set(&ble_manager_advertising, BLE_MANAGER_CONN_CFG_TAG);
}

void ble_manager_advertising_event_handler(ble_adv_evt_t ble_adv_evt) {
  uint32_t err_code;

  switch (ble_adv_evt) {
    case BLE_ADV_EVT_FAST:
      NRF_LOG_INFO("Fast advertising.");
      break;

    case BLE_ADV_EVT_IDLE:
      break;

    default:
      break;
  }
}

bool record_delete_next(void)
{
  fds_find_token_t  tok   = {0};
  fds_record_desc_t desc  = {0};

  if (fds_record_iterate(&desc, &tok) == FDS_SUCCESS)
  {
    ret_code_t rc = fds_record_delete(&desc);
    if (rc != FDS_SUCCESS)
    {
      return false;
    }

    return true;
  }
  else
  {
    /* No records left to delete. */
    return false;
  }
}

void ble_manager_init_peer_manager() {
  ble_gap_sec_params_t sec_param;
  ret_code_t err_code;

  err_code = pm_init();
  if(err_code != NRF_SUCCESS) {
    // Many errors can occur here, but the most probable is the "Storage full" error from sdk/components/libraries/fds/fds.c : FDS_ERR_NO_PAGES.
    // TODO : it erases the whole memory, it's not nice to do that...
    NRF_LOG_WARNING("Error while initializing BLE peer management, Erasing all record from memory");
    do {

    } while(record_delete_next());
    err_code = pm_init();
  }
  APP_ERROR_CHECK(err_code);

  memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

  // Security parameters to be used for all security procedures.
  sec_param.bond = BLE_MANAGER_SEC_PARAM_BOND;
  sec_param.mitm = BLE_MANAGER_SEC_PARAM_MITM;
  sec_param.lesc = BLE_MANAGER_SEC_PARAM_LESC;
  sec_param.keypress = BLE_MANAGER_SEC_PARAM_KEYPRESS;
  sec_param.io_caps = BLE_MANAGER_SEC_PARAM_IO_CAPABILITIES;
  sec_param.oob = BLE_MANAGER_SEC_PARAM_OOB;
  sec_param.min_key_size = BLE_MANAGER_SEC_PARAM_MIN_KEY_SIZE;
  sec_param.max_key_size = BLE_MANAGER_SEC_PARAM_MAX_KEY_SIZE;
  sec_param.kdist_own.enc = 1;
  sec_param.kdist_own.id = 1;
  sec_param.kdist_peer.enc = 1;
  sec_param.kdist_peer.id = 1;

  err_code = pm_sec_params_set(&sec_param);
  APP_ERROR_CHECK(err_code);

  err_code = pm_register(ble_manager_peer_manager_event_handler);
  APP_ERROR_CHECK(err_code);
}

void ble_manager_peer_manager_event_handler(pm_evt_t const *p_evt) {
  bool delete_bonds = false;
  ret_code_t err_code;
  pm_handler_on_pm_evt(p_evt);
  pm_handler_flash_clean(p_evt);

  switch (p_evt->evt_id) {
    case PM_EVT_CONN_SEC_SUCCEEDED: {
//      m_peer_id = p_evt->peer_id;

      // Discover peer's services.
      err_code = ble_db_discovery_start(&ble_manager_db_discovery, p_evt->conn_handle);
      APP_ERROR_CHECK(err_code);
    }
      break;

    case PM_EVT_PEERS_DELETE_SUCCEEDED:
      ble_manager_start_advertising(&delete_bonds);
      break;

    case PM_EVT_STORAGE_FULL:  {
      // Run garbage collection on the flash.
      err_code = fds_gc();
      if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
      {
        // Retry.
      }
      else
      {
        APP_ERROR_CHECK(err_code);
      }
    }break;

    default:
      break;
  }
}

void ble_manager_start_advertising(void *p_erase_bonds) {
  bool erase_bonds = *(bool *) p_erase_bonds;

  if (erase_bonds) {
    ble_manager_delete_bonds();
    // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
  } else {
    ret_code_t err_code = ble_advertising_start(&ble_manager_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
  }
}

void handle_alert_notification(ble_ans_c_evt_t * p_evt)
{
  ret_code_t err_code;

  if (p_evt->uuid.uuid == BLE_UUID_UNREAD_ALERT_CHAR)
  {
    if (m_unread_alert_state == ALERT_NOTIFICATION_ENABLED)
    {
//      err_code = bsp_indication_set(BSP_INDICATE_ALERT_1);
      APP_ERROR_CHECK(err_code);
      m_unread_alert_state = ALERT_NOTIFICATION_ON;
      NRF_LOG_INFO("Unread Alert state: On.");
      NRF_LOG_INFO("  Category:                 %s",
                   (uint32_t)lit_catid[p_evt->data.alert.alert_category]);
      NRF_LOG_INFO("  Number of unread alerts:  %d",
                   p_evt->data.alert.alert_category_count);
    }
  }
  else if (p_evt->uuid.uuid == BLE_UUID_NEW_ALERT_CHAR)
  {
//    if (m_new_alert_state == ALERT_NOTIFICATION_ENABLED)
    if(true)
    {
//      err_code = bsp_indication_set(BSP_INDICATE_ALERT_0);
//      APP_ERROR_CHECK(err_code);
      m_new_alert_state = ALERT_NOTIFICATION_ON;
      NRF_LOG_INFO("New Alert state: On.");
      NRF_LOG_INFO("  Category:                 %s",
                   (uint32_t)lit_catid[p_evt->data.alert.alert_category]);
      NRF_LOG_INFO("  Number of new alerts:     %d",
                   p_evt->data.alert.alert_category_count);
      NRF_LOG_INFO("  Text String Information:  (%d) %s",
                   p_evt->data.alert.alert_msg_length, (uint32_t)p_evt->data.alert.p_alert_msg_buf);

      OnNewNotificationCallback(p_evt->data.alert.p_alert_msg_buf, p_evt->data.alert.alert_msg_length);
    }
  }
  else
  {
    // Only Unread and New Alerts exists, thus do nothing.
  }
}

void supported_alert_notification_read(void)
{
  NRF_LOG_INFO("Read supported Alert Notification characteristics on the connected peer.");

  ret_code_t err_code;

  err_code = ble_ans_c_new_alert_read(&m_ans_c);
  APP_ERROR_CHECK(err_code);

  err_code = ble_ans_c_unread_alert_read(&m_ans_c);
  APP_ERROR_CHECK(err_code);

}

void alert_notification_setup(void)
{
  ret_code_t err_code;

  err_code = ble_ans_c_enable_notif_new_alert(&m_ans_c);
  APP_ERROR_CHECK(err_code);

  m_new_alert_state = ALERT_NOTIFICATION_ENABLED;
  NRF_LOG_INFO("New Alert State: Enabled.");

  err_code = ble_ans_c_enable_notif_unread_alert(&m_ans_c);
  APP_ERROR_CHECK(err_code);

  m_unread_alert_state = ALERT_NOTIFICATION_ENABLED;
  NRF_LOG_INFO("Unread Alert State: Enabled.");

  NRF_LOG_INFO("Notifications enabled.");
}

void control_point_setup(ble_ans_c_evt_t * p_evt)
{
  uint32_t                err_code;
  ble_ans_control_point_t setting;

  if (p_evt->uuid.uuid == BLE_UUID_SUPPORTED_UNREAD_ALERT_CATEGORY_CHAR)
  {
    setting.command  = ANS_ENABLE_UNREAD_CATEGORY_STATUS_NOTIFICATION;
    setting.category = (ble_ans_category_id_t)p_evt->data.alert.alert_category;
    NRF_LOG_INFO("Unread status notification enabled for received categories.");
  }
  else if (p_evt->uuid.uuid == BLE_UUID_SUPPORTED_NEW_ALERT_CATEGORY_CHAR)
  {
    setting.command  = ANS_ENABLE_NEW_INCOMING_ALERT_NOTIFICATION;
    setting.category = (ble_ans_category_id_t)p_evt->data.alert.alert_category;
    NRF_LOG_INFO("New incoming notification enabled for received categories.");
  }
  else
  {
    return;
  }

  err_code = ble_ans_c_control_point_write(&m_ans_c, &setting);
  APP_ERROR_CHECK(err_code);
}

void on_ans_c_evt(ble_ans_c_evt_t * p_evt)
{
  ret_code_t err_code;
  NRF_LOG_INFO("ANS %d", p_evt->evt_type);


  switch (p_evt->evt_type)
  {
    case BLE_ANS_C_EVT_DISCOVERY_FAILED:
      NRF_LOG_INFO("ANS discovery failed");
      break;
    case BLE_ANS_C_EVT_NOTIFICATION:
      handle_alert_notification(p_evt);
      NRF_LOG_INFO("Alert Notification received from server, UUID: %X.", p_evt->uuid.uuid);
      break; // BLE_ANS_C_EVT_NOTIFICATION

    case BLE_ANS_C_EVT_DISCOVERY_COMPLETE:
      NRF_LOG_INFO("Alert Notification Service discovered on the server.");
      err_code = ble_ans_c_handles_assign(&m_ans_c,
                                          p_evt->conn_handle,
                                          &p_evt->data.service);
      APP_ERROR_CHECK(err_code);
      supported_alert_notification_read();
      alert_notification_setup();
      break; // BLE_ANS_C_EVT_DISCOVERY_COMPLETE

    case BLE_ANS_C_EVT_READ_RESP:
      NRF_LOG_INFO("Alert Setup received from server, UUID: %X.", p_evt->uuid.uuid);
      control_point_setup(p_evt);
      break; // BLE_ANS_C_EVT_READ_RESP

    case BLE_ANS_C_EVT_DISCONN_COMPLETE:
      m_new_alert_state    = ALERT_NOTIFICATION_DISABLED;
      m_unread_alert_state = ALERT_NOTIFICATION_DISABLED;

//      err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
      APP_ERROR_CHECK(err_code);
      break; // BLE_ANS_C_EVT_DISCONN_COMPLETE

    default:
      // No implementation needed.
      break;
  }
}

void alert_notification_error_handler(uint32_t nrf_error)
{
  APP_ERROR_HANDLER(nrf_error);
}

void ble_manager_init_services() {
  ret_code_t err_code;
  ble_hrs_init_t hrs_init;
  ble_bas_init_t bas_init;
  ble_dis_init_t dis_init;
  ble_cts_c_init_t cts_init;
  ble_ans_c_init_t ans_init_obj;
  nrf_ble_qwr_init_t qwr_init = {0};
  uint8_t body_sensor_location;

  // Initialize Queued Write Module.
  qwr_init.error_handler = ble_manager_queue_write_error_handler;

  err_code = nrf_ble_qwr_init(&ble_manager_queue_write, &qwr_init);
  APP_ERROR_CHECK(err_code);

  // Initialize Heart Rate Service.
  body_sensor_location = BLE_HRS_BODY_SENSOR_LOCATION_FINGER;

  memset(&hrs_init, 0, sizeof(hrs_init));

  hrs_init.evt_handler = NULL;
  hrs_init.is_sensor_contact_supported = true;
  hrs_init.p_body_sensor_location = &body_sensor_location;

  // Here the sec level for the Heart Rate Service can be changed/increased.
  hrs_init.hrm_cccd_wr_sec = SEC_OPEN;
  hrs_init.bsl_rd_sec = SEC_OPEN;

  // Initialize Battery Service.
//  memset(&bas_init, 0, sizeof(bas_init));
//
//  // Here the sec level for the Battery Service can be changed/increased.
//  bas_init.bl_rd_sec        = SEC_OPEN;
//  bas_init.bl_cccd_wr_sec   = SEC_OPEN;
//  bas_init.bl_report_rd_sec = SEC_OPEN;
//
//  bas_init.evt_handler          = NULL;
//  bas_init.support_notification = true;
//  bas_init.p_report_ref         = NULL;
//  bas_init.initial_batt_level   = 100;
//
//  err_code = ble_bas_init(&m_bas, &bas_init);
//  APP_ERROR_CHECK(err_code);

  // Initialize Device Information Service.
  memset(&dis_init, 0, sizeof(dis_init));

  ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *) BLE_MANAGER_MANUFACTURER_NAME);

  dis_init.dis_char_rd_sec = SEC_OPEN;

  err_code = ble_dis_init(&dis_init);
  APP_ERROR_CHECK(err_code);

  // Initialize CTS.
  cts_init.evt_handler = ble_manager_cts_event_handler;
  cts_init.error_handler = ble_manager_cts_error_handler;
  err_code = ble_cts_c_init(&ble_manager_cts_client, &cts_init);
  APP_ERROR_CHECK(err_code);

  // Alert Notification service
  memset(&ans_init_obj, 0, sizeof(ans_init_obj));
  memset(m_alert_message_buffer, 0, MESSAGE_BUFFER_SIZE);

  ans_init_obj.evt_handler         = on_ans_c_evt;
  ans_init_obj.message_buffer_size = MESSAGE_BUFFER_SIZE;
  ans_init_obj.p_message_buffer    = m_alert_message_buffer;
  ans_init_obj.error_handler       = alert_notification_error_handler;

  NRF_LOG_INFO("ble_ans_c_init");
  err_code = ble_ans_c_init(&m_ans_c, &ans_init_obj);
  NRF_SDH_BLE_OBSERVER(ans_observer, BLE_ANS_C_BLE_OBSERVER_PRIO,*ble_ans_c_on_ble_evt, &m_ans_c);

  APP_ERROR_CHECK(err_code);
}

void ble_manager_queue_write_error_handler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

void ble_manager_cts_event_handler(ble_cts_c_t *p_cts, ble_cts_c_evt_t *p_evt) {
  ret_code_t err_code;

  NRF_LOG_INFO("CTS %d", p_evt->evt_type);

  switch (p_evt->evt_type) {
    case BLE_CTS_C_EVT_DISCOVERY_COMPLETE:
      NRF_LOG_INFO("Current Time Service discovered on server.");
      err_code = ble_cts_c_handles_assign(&ble_manager_cts_client,
                                          p_evt->conn_handle,
                                          &p_evt->params.char_handles);

      ble_cts_c_current_time_read(&ble_manager_cts_client);
      APP_ERROR_CHECK(err_code);
      break;

    case BLE_CTS_C_EVT_DISCOVERY_FAILED:
      NRF_LOG_INFO("Current Time Service not found on server. ");
      // CTS not found in this case we just disconnect. There is no reason to stay
      // in the connection for this simple app since it all wants is to interact with CT
//      if (p_evt->conn_handle != BLE_CONN_HANDLE_INVALID) {
//        err_code = sd_ble_gap_disconnect(p_evt->conn_handle,
//                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
//        APP_ERROR_CHECK(err_code);
//      }
      break;

    case BLE_CTS_C_EVT_DISCONN_COMPLETE:
      NRF_LOG_INFO("Disconnect Complete.");
      break;

    case BLE_CTS_C_EVT_CURRENT_TIME:
      NRF_LOG_INFO("Current Time received.");
      ble_manager_cts_print_time(p_evt);
      break;

    case BLE_CTS_C_EVT_INVALID_TIME:
      NRF_LOG_INFO("Invalid Time received.");
      break;

    default:
      break;
  }
}

void ble_manager_cts_error_handler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

void ble_manager_cts_print_time(ble_cts_c_evt_t *p_evt) {
  NRF_LOG_INFO("\r\nCurrent Time:");
  NRF_LOG_INFO("\r\nDate:");

  NRF_LOG_INFO("\tDay of week   %s", (uint32_t) day_of_week[p_evt->
          params.
          current_time.
          exact_time_256.
          day_date_time.
          day_of_week]);

  if (p_evt->params.current_time.exact_time_256.day_date_time.date_time.day == 0) {
    NRF_LOG_INFO("\tDay of month  Unknown");
  } else {
    NRF_LOG_INFO("\tDay of month  %i",
                 p_evt->params.current_time.exact_time_256.day_date_time.date_time.day);
  }

  NRF_LOG_INFO("\tMonth of year %s",
               (uint32_t) month_of_year[p_evt->params.current_time.exact_time_256.day_date_time.date_time.month]);
  if (p_evt->params.current_time.exact_time_256.day_date_time.date_time.year == 0) {
    NRF_LOG_INFO("\tYear          Unknown");
  } else {
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

  OnNewTimeCallback(&p_evt->params.current_time);
}

void ble_manager_init_connection_params() {
  ret_code_t err_code;
  ble_conn_params_init_t cp_init;

  memset(&cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail = false;
  cp_init.evt_handler = ble_manager_conn_params_event_handler;
  cp_init.error_handler = ble_manager_conn_params_error_handler;

  err_code = ble_conn_params_init(&cp_init);
  APP_ERROR_CHECK(err_code);
}

void ble_manager_conn_params_event_handler(ble_conn_params_evt_t *p_evt) {
  ret_code_t err_code;

  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
    err_code = sd_ble_gap_disconnect(ble_manager_connection_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    APP_ERROR_CHECK(err_code);
  }
}

void ble_manager_conn_params_error_handler(uint32_t nrf_error) {
  APP_ERROR_HANDLER(nrf_error);
}

void ble_manager_delete_bonds() {
  ret_code_t err_code;

  NRF_LOG_INFO("Erase bonds!");

  err_code = pm_peers_delete();
  APP_ERROR_CHECK(err_code);
}
