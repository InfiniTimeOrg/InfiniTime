#pragma once
#include <ble/ble_services/ble_cts_c/ble_cts_c.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BLE_MANAGER_CONN_CFG_TAG 1 /* A tag identifying the SoftDevice BLE configuration. */
#define BLE_MANAGER__OBSERVER_PRIO 3 /* Application's BLE observer priority. You shouldn't need to modify this value. */
#define BLE_MANAGER_DEVICE_NAME "PineTime" /* Name of device. Will be included in the advertising data.*/
#define BLE_MANAGER_MANUFACTURER_NAME "Codingfield"

#define BLE_MANAGER_MIN_CONN_INTERVAL MSEC_TO_UNITS(100, UNIT_1_25_MS) /* Minimum acceptable connection interval (0.4 seconds).*/
#define BLE_MANAGER_MAX_CONN_INTERVAL MSEC_TO_UNITS(650, UNIT_1_25_MS) /*Maximum acceptable connection interval (0.65 second).*/
#define BLE_MANAGER_SLAVE_LATENCY 0 /* Slave latency.*/
#define BLE_MANAGER_CONN_SUP_TIMEOUT MSEC_TO_UNITS(4000, UNIT_10_MS) /* Connection supervisory time-out (4 seconds).*/

#define BLE_MANAGER_ADV_INTERVAL 300 /* The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms).*/
#define BLE_MANAGER_ADV_DURATION  18000 /* The advertising duration (180 seconds) in units of 10 milliseconds.*/

#define BLE_MANAGER_SEC_PARAM_BOND 1 /* Perform bonding. */
#define BLE_MANAGER_SEC_PARAM_MITM 0 /* Man In The Middle protection not required. */
#define BLE_MANAGER_SEC_PARAM_LESC 0 /* LE Secure Connections not enabled. */
#define BLE_MANAGER_SEC_PARAM_KEYPRESS 0 /* Keypress notifications not enabled. */
#define BLE_MANAGER_SEC_PARAM_IO_CAPABILITIES BLE_GAP_IO_CAPS_NONE /* No I/O capabilities. */
#define BLE_MANAGER_SEC_PARAM_OOB 0 /* Out Of Band data not available. */
#define BLE_MANAGER_SEC_PARAM_MIN_KEY_SIZE 7 /* Minimum encryption key size. */
#define BLE_MANAGER_SEC_PARAM_MAX_KEY_SIZE  16 /* Maximum encryption key size. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY 5000 /* Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY 30000 /* Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT 3 /* Number of attempts before giving up the connection parameter negotiation. */

#define MESSAGE_BUFFER_SIZE             18 /**< Size of buffer holding optional messages in notifications. */
#define BLE_ANS_NB_OF_CATEGORY_ID       10 /**< Number of categories. */

void ble_manager_init();
void ble_manager_start_advertising(void *p_erase_bonds);
void ble_manager_init_peer_manager();

// TODO use signals from RTOS to notify new time
void ble_manager_set_new_time_callback(void (*OnNewTime)(current_time_char_t* currentTime));
void ble_manager_set_ble_disconnection_callback(void (*OnBleDisconnection)());
void ble_manager_set_ble_connection_callback(void (*OnBleConnection)());

void ble_manager_set_new_notification_callback(void (*OnNewNotification)(const char* message, uint8_t size));


#ifdef __cplusplus
}
#endif