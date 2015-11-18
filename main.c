#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "app_error.h"
#include "app_gpiote.h"
#include "app_button.h"
#include "app_trace.h"
#include "app_scheduler.h"
#include "app_timer_appsh.h"
#include "app_util.h"
#include "ble.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_debug_assert_handler.h"
#include "ble_error_log.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "client_handling.h"
#include "device_manager.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf51_bitfields.h"
#include "pstorage.h"
#include "sdk_config.h"
#include "softdevice_handler_appsh.h"
#include "spi_master.h"
#include "twi_master.h"
#include "uart.h"


#ifdef ACTUAL_BOARD
static char* __attribute__((unused)) ident = "$Build: ACTUAL_BOARD " __DATE__ \
  ", " __TIME__ " $";
#else
static char* __attribute__((unused)) ident = "$Build: DEVKIT " __DATE__ \
  ", " __TIME__ " $";
#endif

/**@brief Variable length data encapsulation in terms of length and pointer to data */
typedef struct
{
    uint8_t     * p_data;                                                      /**< Pointer to data. */
    uint16_t      data_len;                                                    /**< Length of data. */
}data_t;

static dm_application_instance_t          m_dm_app_id;                         /**< Application identifier. */
static uint8_t                            m_peer_count = 0;                    /**< Number of peer's connected. */

static bool    m_memory_access_in_progress = false;

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                           /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define APPL_LOG 								app_trace_log

#define TARGET_DEV_NAME                     "nordic_adv"                           /**< Name of device. Will be included in the advertising data. */

#define APP_ADV_INTERVAL                64                                       /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                      /**< The advertising timeout (in units of seconds). */

#define SCAN_INTERVAL                    0x00A0                                  /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                      0x0050                                  /**< Determines scan window in units of 0.625 millisecond. */
#define MAX_PEER_COUNT                   3											                 /**< Maximum number of peer's application intends to manage. */


#define LED_RST                          12
#define MIN_CONNECTION_INTERVAL          MSEC_TO_UNITS(30, UNIT_1_25_MS)                /**< Determines minimum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL          MSEC_TO_UNITS(60, UNIT_1_25_MS)                /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY                    0                                              /**< Determines slave latency in counts of connection events. */           0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define APP_GPIOTE_MAX_USERS            2                                           /**< Maximum number of users of the GPIOTE handler. */

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)    /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define SEC_PARAM_TIMEOUT               30                                          /**< Timeout for Pairing Request or Security Request (in seconds). */
#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */



#define SCHED_MAX_EVENT_DATA_SIZE       sizeof(app_timer_event_t)                   /**< Maximum size of scheduler events. Note that scheduler BLE stack events do not contain any data, as the events are being pulled from the stack in the event handler. */

#define SCHED_QUEUE_SIZE                25                                          /**< Maximum number of events in the scheduler queue. */
#define SUPERVISION_TIMEOUT              MSEC_TO_UNITS(4000, UNIT_10_MS)

                          /**< Security requirements for this application. */

/**
 * @brief Scan parameters requested for scanning and connection.
 */
static const ble_gap_scan_params_t m_scan_param =
{
     0,                       // Active scanning not set.
     0,                       // Selective scanning not set.
     NULL,                    // White-list not set.
     (uint16_t)SCAN_INTERVAL, // Scan interval.
     (uint16_t)SCAN_WINDOW,   // Scan window.
     0                        // Never stop scanning unless explicitly asked to.
};

static app_gpiote_user_id_t           m_gpiote_user_id;

static const ble_gap_conn_params_t m_connection_param =
{
    (uint16_t)MIN_CONNECTION_INTERVAL,   // Minimum connection
    (uint16_t)MAX_CONNECTION_INTERVAL,   // Maximum connection
    0,                                   // Slave latency
    (uint16_t)SUPERVISION_TIMEOUT        // Supervision time-out
};

// Persistent storage system event handler
void pstorage_sys_event_handler (uint32_t p_evt);

/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
  // This call can be used for debug purposes during application development.
  // @note CAUTION: Activating this code will write the stack to flash on an error.
  //                This function should NOT be used in a final product.
  //                It is intended STRICTLY for development/debugging purposes.
  //                The flash write will happen EVEN if the radio is active, thus interrupting
  //                any communication.
  //                Use with care. Un-comment the line below to use.
  nrf_gpio_pin_clear(24);
  ble_debug_assert_handler(error_code, line_num, p_file_name);

  // On assert, the system can only recover with a reset.
  //NVIC_SystemReset();
}
static void scan_start(void)
{
    uint32_t err_code;
    uint32_t count;
    // Verify if there is any flash access pending, if yes delay starting scanning until
    // it's complete.
    err_code = pstorage_access_status_get(&count);
    APP_ERROR_CHECK(err_code);

    if (count != 0)
    {
        m_memory_access_in_progress = true;
        return;
    }
    err_code = sd_ble_gap_scan_start(&m_scan_param);
    APP_ERROR_CHECK(err_code);
}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Callback handling device manager events.
 *
 * @details This function is called to notify the application of device manager events.
 *
 * @param[in]   p_handle      Device Manager Handle. For link related events, this parameter
 *                            identifies the peer.
 * @param[in]   p_event       Pointer to the device manager event.
 * @param[in]   event_status  Status of the event.
 */
static ret_code_t device_manager_event_handler(const dm_handle_t    * p_handle,
                                                 const dm_event_t     * p_event,
                                                 const ret_code_t     event_result)
{
    uint32_t       err_code;

    switch(p_event->event_id)
    {
        case DM_EVT_CONNECTION:
            APPL_LOG("[APPL]:[0x%02X] >> DM_EVT_CONNECTION\r\n", p_handle->connection_id);
#ifdef ENABLE_DEBUG_LOG_SUPPORT
            ble_gap_addr_t * p_peer_addr;
            p_peer_addr = &p_event->event_param.p_gap_param->params.connected.peer_addr;
#endif // ENABLE_DEBUG_LOG_SUPPORT
            APPL_LOG("[APPL]:[%02X %02X %02X %02X %02X %02X]: Connection Established\r\n",
                            p_peer_addr->addr[0], p_peer_addr->addr[1], p_peer_addr->addr[2],
                            p_peer_addr->addr[3], p_peer_addr->addr[4], p_peer_addr->addr[5]);
            APPL_LOG("\r\n");

            APPL_LOG("[APPL]:[CI 0x%02X]: Requesting GATT client create\r\n",
                     p_handle->connection_id);
            err_code = client_handling_create(p_handle, p_event->event_param.p_gap_param->conn_handle);
            APP_ERROR_CHECK(err_code);

            m_peer_count++;
            if (m_peer_count < MAX_PEER_COUNT)
            {
                scan_start();
            }
            APPL_LOG("[APPL]:[0x%02X] << DM_EVT_CONNECTION\r\n", p_handle->connection_id);
            break;
        case DM_EVT_DISCONNECTION:
            APPL_LOG("[APPL]:[0x%02X] >> DM_EVT_DISCONNECTION\r\n", p_handle->connection_id);

            err_code = client_handling_destroy(p_handle);
            APP_ERROR_CHECK(err_code);

            if (m_peer_count == MAX_PEER_COUNT)
            {
                scan_start();
            }
            m_peer_count--;
            APPL_LOG("[APPL]:[0x%02X] << DM_EVT_DISCONNECTION\r\n", p_handle->connection_id);
            break;
        case DM_EVT_SECURITY_SETUP:
        {
            dm_handle_t handle = (*p_handle);
            APPL_LOG("[APPL]:[0x%02X] >> DM_EVT_SECURITY_SETUP\r\n", p_handle->connection_id);
            // Slave securtiy request received from peer, if from a non bonded device,
            // initiate security setup, else, wait for encryption to complete.
            err_code = dm_security_setup_req(&handle);
            APP_ERROR_CHECK(err_code);
            APPL_LOG("[APPL]:[0x%02X] << DM_EVT_SECURITY_SETUP\r\n", p_handle->connection_id);
            break;
        }
        case DM_EVT_SECURITY_SETUP_COMPLETE:
            APPL_LOG("[APPL]:[0x%02X] >> DM_EVT_SECURITY_SETUP_COMPLETE, result 0x%08X\r\n",
                      p_handle->connection_id, event_result);
            APPL_LOG("[APPL]:[0x%02X] << DM_EVT_SECURITY_SETUP_COMPLETE\r\n",
                      p_handle->connection_id);
            break;
        case DM_EVT_LINK_SECURED:
            APPL_LOG("[APPL]:[0x%02X] >> DM_LINK_SECURED_IND, result 0x%08X\r\n",
                      p_handle->connection_id, event_result);
            APPL_LOG("[APPL]:[0x%02X] << DM_LINK_SECURED_IND\r\n", p_handle->connection_id);
            break;
        case DM_EVT_DEVICE_CONTEXT_LOADED:
            APPL_LOG("[APPL]:[0x%02X] >> DM_EVT_LINK_SECURED\r\n", p_handle->connection_id);
            APP_ERROR_CHECK(event_result);
            APPL_LOG("[APPL]:[0x%02X] << DM_EVT_DEVICE_CONTEXT_LOADED\r\n", p_handle->connection_id);
            break;
        case DM_EVT_DEVICE_CONTEXT_STORED:
            APPL_LOG("[APPL]:[0x%02X] >> DM_EVT_DEVICE_CONTEXT_STORED\r\n", p_handle->connection_id);
            APP_ERROR_CHECK(event_result);
            APPL_LOG("[APPL]:[0x%02X] << DM_EVT_DEVICE_CONTEXT_STORED\r\n", p_handle->connection_id);
            break;
        case DM_EVT_DEVICE_CONTEXT_DELETED:
            APPL_LOG("[APPL]:[0x%02X] >> DM_EVT_DEVICE_CONTEXT_DELETED\r\n", p_handle->connection_id);
            APP_ERROR_CHECK(event_result);
            APPL_LOG("[APPL]:[0x%02X] << DM_EVT_DEVICE_CONTEXT_DELETED\r\n", p_handle->connection_id);
            break;
        default:
            break;
    }

    // Relay the event to client handling module.
    err_code = client_handling_dm_event_handler(p_handle, p_event, event_result);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}



/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */

/**
 * @brief Parses advertisement data, providing length and location of the field in case
 *        matching data is found.
 *
 * @param[in]  Type of data to be looked for in advertisement data.
 * @param[in]  Advertisement report length and pointer to report.
 * @param[out] If data type requested is found in the data report, type data length and
 *             pointer to data will be populated here.
 *
 * @retval NRF_SUCCESS if the data type is found in the report.
 * @retval NRF_ERROR_NOT_FOUND if the data type could not be found.
 */
static uint32_t adv_report_parse(uint8_t type, data_t * p_advdata, data_t * p_typedata)
{
    uint32_t index = 0;
    uint8_t * p_data;

    p_data = p_advdata->p_data;

    while (index < p_advdata->data_len)
    {
        uint8_t field_length = p_data[index];
        uint8_t field_type   = p_data[index+1];

        if (field_type == type)
        {
            p_typedata->p_data   = &p_data[index+2];
            p_typedata->data_len = field_length-1;
            return NRF_SUCCESS;
        }
        index += field_length+1;
    }
    return NRF_ERROR_NOT_FOUND;
}


static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t        err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_ADV_REPORT:
        {
            data_t adv_data;
            data_t type_data;

            // Initialize advertisement report for parsing.
            adv_data.p_data = p_ble_evt->evt.gap_evt.params.adv_report.data;
            adv_data.data_len = p_ble_evt->evt.gap_evt.params.adv_report.dlen;

            err_code = adv_report_parse(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
                                      &adv_data,
                                      &type_data);
            if (err_code != NRF_SUCCESS)
            {
                // Compare short local name in case complete name does not match.
                err_code = adv_report_parse(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME,
                                          &adv_data,
                                          &type_data);
            }

            // Verify if short or complete name matches target.
            if ((err_code == NRF_SUCCESS) &&
               (0 == memcmp(TARGET_DEV_NAME,type_data.p_data,type_data.data_len)))
            {
                err_code = sd_ble_gap_scan_stop();
                if (err_code != NRF_SUCCESS)
                {
                    APPL_LOG("[APPL]: Scan stop failed, reason %d\r\n", err_code);
                }

                err_code = sd_ble_gap_connect(&p_ble_evt->evt.gap_evt.params.adv_report.\
                                              peer_addr,
                                              &m_scan_param,
                                              &m_connection_param);

                if (err_code != NRF_SUCCESS)
                {
                    APPL_LOG("[APPL]: Connection Request Failed, reason %d\r\n", err_code);
                }
            }
            break;
        }
        case BLE_GAP_EVT_TIMEOUT:
            if(p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_SCAN)
            {
                APPL_LOG("[APPL]: Scan Timedout.\r\n");
            }
            else if (p_ble_evt->evt.gap_evt.params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
                APPL_LOG("[APPL]: Connection Request Timedout.\r\n");
            }
            break;
        default:
            break;
    }
}
/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */


static void on_sys_evt(uint32_t sys_evt)
{
    switch(sys_evt)
    {
        case NRF_EVT_FLASH_OPERATION_SUCCESS:
        case NRF_EVT_FLASH_OPERATION_ERROR:
            if (m_memory_access_in_progress)
            {
                m_memory_access_in_progress = false;
                scan_start();
            }
            break;
        default:
            // No implementation needed.
            break;
    }
}

static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    on_sys_evt(sys_evt);
}


static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    client_handling_ble_evt_handler(p_ble_evt);
    on_ble_evt(p_ble_evt);
}

static void ble_stack_init(void)
{
  uint32_t err_code;
  // Initialize the SoftDevice handler module.
#ifdef ACTUAL_BOARD
  SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_TEMP_4000MS_CALIBRATION, false);
#else
  SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, false);
#endif

  //enable pstorage
  APP_ERROR_CHECK(pstorage_init());

  // Enable BLE stack
  ble_enable_params_t ble_enable_params;
  memset(&ble_enable_params, 0, sizeof(ble_enable_params));

	ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
	ble_enable_params.gatts_enable_params.attr_tab_size = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
  err_code = sd_ble_enable(&ble_enable_params);
  APP_ERROR_CHECK(err_code);

  err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
  APP_ERROR_CHECK(err_code);

  // Register with the SoftDevice handler module for BLE events.
  err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
  APP_ERROR_CHECK(err_code);
}

static void device_manager_init(void)
{
    dm_application_param_t param;
    dm_init_param_t        init_param;

    uint32_t err_code;

    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    init_param.clear_persistent_data = false;

#ifdef BOND_DELETE_ALL_BUTTON_PIN
    // Clear all bonded devices if user requests to.
    init_param.clear_persistent_data =
        ((nrf_gpio_pin_read(BOND_DELETE_ALL_BUTTON_PIN) == 0)? true: false);
#endif

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&param.sec_param, 0, sizeof (ble_gap_sec_params_t));

    // Event handler to be registered with the module.
    param.evt_handler            = device_manager_event_handler;

    // Service or protocol context for device manager to load, store and apply on behalf of application.
    // Here set to client as application is a GATT client.
    param.service_type           = DM_PROTOCOL_CNTXT_GATT_CLI_ID;

    // Secuirty parameters to be used for security procedures.
    param.sec_param.bond         = SEC_PARAM_BOND;
    param.sec_param.mitm         = SEC_PARAM_MITM;
    param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    param.sec_param.oob          = SEC_PARAM_OOB;
    param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    param.sec_param.kdist_periph.enc = 1;
    param.sec_param.kdist_periph.id  = 1;

    err_code = dm_register(&m_dm_app_id,&param);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
  APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

/**@brief Function for initializing the GPIOTE handler module.
 */
static void gpiote_init(void)
{
  //uint32_t pins_transition_mask = 0;
  uint32_t err;
  APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);

  // err = app_gpiote_user_register(&m_gpiote_user_id,
  //                                0,
  //                                pins_transition_mask,
  //                                your_interrupt_pin_handler);
  err = app_gpiote_user_enable(m_gpiote_user_id);

  APP_ERROR_CHECK(err);

}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
  uint32_t err_code = sd_app_evt_wait();
  APP_ERROR_CHECK(err_code);
}

#define FLASH_INTERVAL             APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER) // in milliseconds
static app_timer_id_t                        m_flash_timer_id;

static void timers_start(void)
{
  uint32_t err_code;
  err_code = app_timer_start(m_flash_timer_id, FLASH_INTERVAL, NULL);
  APP_ERROR_CHECK(err_code);
}

static void timers_init(void)
{
  uint32_t err_code = 0;

  // Initialize timer module, making it use the scheduler
  APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, true);

  // err_code = app_timer_create(&m_flash_timer_id,
  //                             APP_TIMER_MODE_REPEATED,
  //                             your_timer_handler_here);
  APP_ERROR_CHECK(err_code);
}

static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
  uint32_t err_code;

  switch (pin_no)
    {
    case 1:
      //err_code = ble_lbs_on_button_change(&m_lbs, button_action);
      err_code = NRF_SUCCESS;
      if (err_code != NRF_SUCCESS &&
          err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
          err_code != NRF_ERROR_INVALID_STATE)
        {
          APP_ERROR_CHECK(err_code);
        }
      break;

    default:
      APP_ERROR_HANDLER(pin_no);
      break;
    }
}

static void pstorage_cb_handler(pstorage_handle_t  * handle,
                                uint8_t              op_code,
                                uint32_t             result,
                                uint8_t            * p_data,
                                uint32_t             data_len)
{
  UNUSED_PARAMETER(handle);
  UNUSED_PARAMETER(p_data);
  UNUSED_PARAMETER(data_len);
  switch(op_code)
    {
    case PSTORAGE_LOAD_OP_CODE:
           if (result == NRF_SUCCESS)
           {
               // Store operation successful.
           }
           else
           {
               // Store operation failed.
             APP_ERROR_CHECK(result);
           }
           // Source memory can now be reused or freed.
           break;
    case PSTORAGE_UPDATE_OP_CODE:
      if (result == NRF_SUCCESS)
        {
          // Update operation successful.
        }
      else
        {
          // Update operation failed.
          APP_ERROR_CHECK(result);
        }
      break;
    }
}

#define TOUCHPAD_ADDRESS    0x5a    /**< Touchpad TWI address in bits [6:0]. */

/**@brief Function for application main entry.
 */

bool snarf[128];
bool plif[128];
bool twi_master_clear_bus(void);


#define DELAY_MS               100        /*!< Timer Delay in milli-seconds */

int main(void)
{
  nrf_gpio_cfg_output(LED_RST);
  nrf_gpio_pin_clear(LED_RST);
  nrf_delay_us(10000);
  nrf_gpio_pin_set(LED_RST);
  nrf_delay_us(20000);
  twi_master_init();
  gpiote_init();
	timers_init();
  // Nordic Services Start
  ble_stack_init();
  scheduler_init();
	client_handling_init();
	device_manager_init();
	scan_start();
  // Board Services Start
	timers_start();

  for(;;)
  {

    app_sched_execute();
    nrf_delay_us(10000);
    power_manage();
  }
}

/**
 * @}
 */
