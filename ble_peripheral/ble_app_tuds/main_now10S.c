
#define USE_NRF_EXAMPLE 0

#define USE_APPSH 1

#define USE_DIS 1

//#define BLE_DFU_APP_SUPPORT  // ifdef NOTE: BLE_DFU_APP_SUPPORT is defined in Project Options C Compiler Defies Section
#define USE_DM  1     //must be set to 1 if BLE_DFU_APP_SUPPORT is defined

#define USE_DM_E  0


#define USE_TUDS 1


#include "myapp.h" 
#include "ma_adc.h"


#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_assert.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"

#if USE_TUDS
#include "app_tuds.h"
#endif

#if USE_DIS
#include "ble_dis.h"            //ks add service dis
#endif



#ifdef BLE_DFU_APP_SUPPORT
#include "ble_dfu.h"            //ks add service ble_dfu
#include "dfu_app_handler.h"    //ks add dfu_app_handler.c
#endif // BLE_DFU_APP_SUPPORT

#include "ble_conn_params.h"
#include "boards.h"
#include "sensorsim.h"          //ks add sensorsim lib
#include "softdevice_handler.h" //ks add device_manager ble_module (peripheral)

#if USE_APPSH
#include "app_scheduler.h"
#include "softdevice_handler_appsh.h"
#include "app_timer_appsh.h"   //ks add lib app_scheduler, app_timer_with_app_scheduler, sd_handler_with_app_scheduler
#else
#include "app_timer.h"
#endif

#if USE_DM
#include "device_manager.h"  //ks also included in  #include "dfu_app_handler.h"
#include "pstorage.h"
#endif

/*
#include "app_trace.h"
#include "bsp.h"
*/
#include "nrf_delay.h"
/*
#include "bsp_btn_ble.h"

#include "sensorsim.h"
#include "app_button.h"
*/


//#############################################################################
// DEFINES - start
#define APP_TIMER_PRESCALER              0                                          /**< Value of the RTC1 PRESCALER register. */
// DEFINES - end
//#############################################################################


//#############################################################################
// DEFINES - start
//-- ble_stack_init --//
#define IS_SRVC_CHANGED_CHARACT_PRESENT  1                                          /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define CENTRAL_LINK_COUNT               0     //NEW Rev11                          /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT            1     //NEW Rev11                          /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/


//-- dis_init --//
#define MANUFACTURER_NAME                "T and D"                                  /**< Manufacturer. Will be passed to Device Information Service. */


#define APP_ADV_INTERVAL_1000MS         1600 // mg_6_advX50ms_inTicks                /**< The advertising interval (in units of 0.625 ms. This value (1600) corresponds to 1000 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      0    //LPP = never?180                       /**< The advertising time-out in units of seconds. */

/*
#define APP_ADV_FAST_INTERVAL               0x0028  // 0x28 == 40 * 0.625 = 25ms    // Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). 
#define APP_ADV_SLOW_INTERVAL               0x0C80                                  // Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds). 
#define APP_ADV_FAST_TIMEOUT                30                                      // The duration of the fast advertising period (in seconds). 
#define APP_ADV_SLOW_TIMEOUT                180                                     // The duration of the slow advertising period (in seconds). 
*/


//#include "ma_timers.h" // for APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE
//#define APP_TIMER_PRESCALER                 0                                       /**< Value of the RTC1 PRESCALER register. */
//#define APP_TIMER_MAX_TIMERS                (4 + BSP_APP_TIMERS_NUMBER) <defunct>   /**< Maximum number of simultaneously created timers. */
//#define APP_TIMER_OP_QUEUE_SIZE             10                                      /**< Size of timer operation queues. */










//-- gap_params_init --//
#define DEVICE_NAME                      "T&D_UDS_K11-S130"                         /**< Name of device. Will be included in the advertising data. */

//-- gap_params_init --//
//#define DEVICE_NAME                         "Nordic_Keyboard"                     /**< Name of device. Will be included in the advertising data. */
/*lint -emacro(524, MIN_CONN_INTERVAL) // Loss of precision */
//#define MIN_CONN_INTERVAL                   MSEC_TO_UNITS(7.5, UNIT_1_25_MS)      /**< Minimum connection interval (7.5 ms) */
//#define MAX_CONN_INTERVAL                   MSEC_TO_UNITS(30, UNIT_1_25_MS)       /**< Maximum connection interval (30 ms). */
//#define SLAVE_LATENCY                       6                                     /**< Slave latency. */
//#define CONN_SUP_TIMEOUT                    MSEC_TO_UNITS(430, UNIT_10_MS)        /**< Connection supervisory timeout (430 ms). */

//#define MIN_CONN_INTERVAL            MSEC_TO_UNITS(20, UNIT_1_25_MS)         /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
//#define MAX_CONN_INTERVAL            MSEC_TO_UNITS(75, UNIT_1_25_MS)         /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define MIN_CONN_INTERVAL            MSEC_TO_UNITS( 7.5, UNIT_1_25_MS)         /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL            MSEC_TO_UNITS(30  , UNIT_1_25_MS)         /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT             MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */


//https://devzone.nordicsemi.com/question/63233/nrf51822-disconnects-connection-after-1-min/
//-- conn_params_init --//
#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)     /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)    /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                              /**< Number of attempts before giving up the connection parameter negotiation. */

//-- device_manager_init --//
#define SEC_PARAM_BOND                   1                                              /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                              /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                           /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                              /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                              /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                             /**< Maximum encryption key size. */

#define DEAD_BEEF                           0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#if USE_APPSH
#define SCHED_MAX_EVENT_DATA_SIZE_A      (MAX(APP_TIMER_SCHED_EVT_SIZE, sizeof(uniEvent_t)))
//#define SCHED_MAX_EVENT_DATA_SIZE        MAX(APP_TIMER_SCHED_EVT_SIZE   , BLE_STACK_HANDLER_SCHED_EVT_SIZE)  //APPSH  /**< Maximum size of scheduler events. */
#define SCHED_MAX_EVENT_DATA_SIZE        MAX(SCHED_MAX_EVENT_DATA_SIZE_A, BLE_STACK_HANDLER_SCHED_EVT_SIZE)  //APPSH  /**< Maximum size of scheduler events. */

#define SCHED_QUEUE_SIZE                 100 //10                                      //APPSH    
#endif

#ifdef BLE_DFU_APP_SUPPORT
#define DFU_REV_MAJOR                    0x00                                       /** DFU Major revision number to be exposed. */
#define DFU_REV_MINOR                    0x01                                       /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                     ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)     /** DFU Revision number to be exposed. Combined of major and minor versions. */
#define APP_SERVICE_HANDLE_START         0x000C                                     /**< Handle of first application specific service when when service changed characteristic is present. */
#define BLE_HANDLE_MAX                   0xFFFF                                     /**< Max handle value in BLE. */

STATIC_ASSERT(IS_SRVC_CHANGED_CHARACT_PRESENT);                                     /** When having DFU Service support in application the Service Changed Characteristic should always be present. */
#endif // BLE_DFU_APP_SUPPORT


//-- dis Device Information Service --//
#define PNP_ID_VENDOR_ID_SOURCE          0x02                                           /**< Vendor ID Source. */
#define PNP_ID_VENDOR_ID                 0x1915                                         /**< Vendor ID. */
#define PNP_ID_PRODUCT_ID                0xEEEE                                         /**< Product ID. */
#define PNP_ID_PRODUCT_VERSION           0x0001                                         /**< Product Version. */


// DEFINES - end
//#############################################################################


//#############################################################################
// BSS - start
/*static*/ uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */





#if USE_TUDS
ble_tuds_t  m_ble_tuds;                                  // Structure to identify the BLKUP Service.
uint32_t timers_init_tuds_part(void);
void services_init_tuds_part(void);
void application_timers_start_tuds_part(void);
#endif



//static ble_gap_sec_params_t           m_sec_params;                               /**< Security requirements for this application. */



#if USE_DM
static dm_application_instance_t        m_app_handle;                                  /**< Application identifier allocated by device manager. */
static dm_handle_t                      m_bonded_peer_handle;                          /**< Device reference handle to the current bonded central. */
#endif


#ifdef BLE_DFU_APP_SUPPORT
static ble_dfu_t                        m_dfus;                                    /**< Structure used to identify the DFU service. */
#endif // BLE_DFU_APP_SUPPORT
// BSS - end 1
//#############################################################################

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2            /**< Reply when unsupported features are requested. */

typedef enum
{
    BLE_NO_ADV,               /**< No advertising running. */
    BLE_DIRECTED_ADV,         /**< Direct advertising to the latest central. */
    BLE_FAST_ADV_WHITELIST,   /**< Advertising with whitelist. */
    BLE_FAST_ADV,             /**< Fast advertising running. */
    BLE_SLOW_ADV,             /**< Slow advertising running. */
    BLE_SLEEP,                /**< Go to system-off. */
} ble_advertising_mode_t;

// BSS - end 2
//#############################################################################



static void dumb_Delay(int i)
{
    volatile int k,j;
    
    for( k=0 ;k< i;k++)
        for( j=0 ;j<1000;j++);
    
}

#if 1 //DEBUG
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{

    dbgPrint("\r\n");
    dbgPrint("\r\napp_error_handler");
    
    dbgPrintf("\r\nid   = %d 0x%x", id, id);
    dbgPrintf("\r\npc   = %d", pc);
    dbgPrintf("\r\ninfo = %d 0x%x", info, info);
    
    dbgPrint("\r\napp_error_handler");
    dumb_Delay(100);
    
    while(1);
}

void my_app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{

    dbgPrint("\r\n");
    dbgPrint("\r\napp_error_handler");
    
    dbgPrintf("\r\nerror code = %d 0x%x", error_code, error_code);
    dbgPrintf("\r\nLine       = %d", line_num);
    dbgPrintf("\r\nFile       = %s", p_file_name);
    
    dbgPrint("\r\napp_error_handler");
    dumb_Delay(100);
    
    while(1);
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
/*static*/ void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name) //!!! static
{
    my_app_error_handler(DEAD_BEEF, line_num, p_file_name);
    //app_error_handler(DEAD_BEEF, line_num, p_file_name);
}
#endif //  Debug




/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_pre_init(void)
{
#if USE_APPSH
    // Initialize timer module, making it use the scheduler.
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);
#else
    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
#endif
}


uint32_t P7_timer_init(void);
uint32_t P8_timer_init(void);


static void timers_init()
{
    GP_timer_init();         // Init the General Purpose 1-tick per second timer
#if USE_ADCON_TIMER
    battLoad_timer_init();      // Init the on-shot timer for Battery Load timer (for reading the ADC)
#endif
    ma_uart_timer_init();    // Init the timer for Uart RxTimeout/Shutdown timing
    ma_holdoff_timer_init(); // Init the ... what is THIS timer ?!?
    
#if USE_TUDS
    timers_init_tuds_part();
#endif

    P7_timer_init();         // Init the Parameter 7 timer (micro-processor Wake Timing
    P8_timer_init();         // Init the Parameter 8 timer (micro-processor Wake Timing
}


/**@brief   Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of 
 *          the device. It also sets the permissions and appearance.
 */
// https://developer.bluetooth.org/gatt/services/Pages/ServiceViewer.aspx?u=org.bluetooth.service.generic_access.xml
// Service Name: "Generic Access", Assigned Number: 0x1800
/*
    Service Characteristics

      Name: "Device Name" , 0x2A00
        Type: org.bluetooth.characteristic.gap.device_name                                 Requirement: Mandatory
        ->@sd_ble_gap_device_name_set

      Name: "Appearance"  , 0x2A01
        Type: org.bluetooth.characteristic.gap.appearance                                  Requirement: Mandatory
        ->  sd_ble_gap_appearance_set

      Name: "Peripheral Privacy Flag" , 0x2A02
        Type: org.bluetooth.characteristic.gap.peripheral_privacy_flag                     Requirement: Optional

      Name: "Reconnection Address" , 0x2A03
        Type: org.bluetooth.characteristic.gap.reconnection_address                        Requirement: Conditional
        
      Name: "Peripheral Preferred Connection Parameters" , 0x2A04
        Type: org.bluetooth.characteristic.gap.peripheral_preferred_connection_parameters  Requirement: Optional
        ->  sd_ble_gap_ppcp_set
*/


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
void gap_device_name_only_set(char * nextName)
{
    uint32_t                err_code;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    
    // Characteristic "Device Name", Assigned number: 0x2A00
    // https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.device_name.xml
    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) nextName,  strlen(nextName));
    APP_ERROR_CHECK(err_code);
}

static void gap_params_init(char *device_name) //DEVICE_NAME
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    
    gap_device_name_only_set(device_name);

    //ble_gap_conn_sec_mode_t sec_mode;
    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);    
    //// Characteristic "Device Name", Assigned number: 0x2A00
    //// https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.device_name.xml
    //err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) device_name, strlen(device_name));
    //APP_ERROR_CHECK(err_code);
        
    // Characteristic "Appearance", Assigned number: 0x2A01
    // https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
    uint16_t appearance; //karel
    appearance = 0x0000; //karel
    err_code = sd_ble_gap_appearance_set(appearance); //karel
    APP_ERROR_CHECK(err_code);
  
    // Characteristic "Peripheral Preferred Connection Parameters" Assigned number: 0x2A04
    // https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.peripheral_preferred_connection_parameters.xml
    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


#ifdef BLE_DFU_APP_SUPPORT
/**@brief Function for stopping advertising.
 */
static void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for loading application-specific context after establishing a secure connection.
 *
 * @details This function will load the application context and check if the ATT table is marked as
 *          changed. If the ATT table is marked as changed, a Service Changed Indication
 *          is sent to the peer if the Service Changed CCCD is set to indicate.
 *
 * @param[in] p_handle The Device Manager handle that identifies the connection for which the context
 *                     should be loaded.
 */
static void app_context_load(dm_handle_t const * p_handle)
{
    uint32_t                 err_code;
    static uint32_t          context_data;
    dm_application_context_t context;

    context.len    = sizeof(context_data);
    context.p_data = (uint8_t *)&context_data;

    err_code = dm_application_context_get(p_handle, &context);
    if (err_code == NRF_SUCCESS)
    {
        // Send Service Changed Indication if ATT table has changed.
        if ((context_data & (DFU_APP_ATT_TABLE_CHANGED << DFU_APP_ATT_TABLE_POS)) != 0)
        {
            err_code = sd_ble_gatts_service_changed(m_conn_handle, APP_SERVICE_HANDLE_START, BLE_HANDLE_MAX);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != BLE_ERROR_INVALID_CONN_HANDLE) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != BLE_ERROR_NO_TX_PACKETS) && // was BLE_ERROR_NO_TX_BUFFERS ?error?
                (err_code != NRF_ERROR_BUSY) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
            {
                APP_ERROR_HANDLER(err_code);
            }
        }

        err_code = dm_application_context_delete(p_handle);
        APP_ERROR_CHECK(err_code);
    }
    else if (err_code == DM_NO_APP_CONTEXT)
    {
        // No context available. Ignore.
    }
    else
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/** @snippet [DFU BLE Reset prepare] */
/**@brief Function for preparing for system reset.
 *
 * @details This function implements @ref dfu_app_reset_prepare_t. It will be called by
 *          @ref dfu_app_handler.c before entering the bootloader/DFU.
 *          This allows the current running application to shut down gracefully.
 */
static void reset_prepare(void) //NANNY
{
    uint32_t err_code;

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {

        autoTimeout_Start( AUTOTIMEOUT_NONE );
        // Disconnect from peer.
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        err_code = bsp_indication_set(BSP_INDICATE_IDLE);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        // If not connected, the device will be advertising. Hence stop the advertising.
        advertising_stop();
    }

    err_code = ble_conn_params_stop();
    APP_ERROR_CHECK(err_code);

    nrf_delay_ms(500);
}
/** @snippet [DFU BLE Reset prepare] */
#endif // BLE_DFU_APP_SUPPORT



#if USE_DIS
/**@brief Function for initializing Device Information Service.
 */
static void dis_init(void)
{
    uint32_t         err_code;
    ble_dis_init_t   dis_init_obj;

    ble_dis_pnp_id_t pnp_id;

    pnp_id.vendor_id_source = PNP_ID_VENDOR_ID_SOURCE;
    pnp_id.vendor_id        = PNP_ID_VENDOR_ID;
    pnp_id.product_id       = PNP_ID_PRODUCT_ID;
    pnp_id.product_version  = PNP_ID_PRODUCT_VERSION;

    // Initialize Device Information Service.
    memset(&dis_init_obj, 0, sizeof(dis_init_obj));
/* ALT
    ble_srv_ascii_to_utf8(&dis_init_obj.manufact_name_str, (char *)MANUFACTURER_NAME);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init_obj.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init_obj.dis_attr_md.write_perm);
 ALT */
/* ALT2 */
    ble_srv_ascii_to_utf8(&dis_init_obj.manufact_name_str, MANUFACTURER_NAME);
    dis_init_obj.p_pnp_id = &pnp_id;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&dis_init_obj.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init_obj.dis_attr_md.write_perm);
/* ALT2 */

    err_code = ble_dis_init(&dis_init_obj);
    APP_ERROR_CHECK(err_code);
    
}
#endif


// TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS
// TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS TUDS
#if USE_TUDS
//extern void tuds_service_init(void);
void services_init_tuds_part(void);
#endif




//.............................................................................
ble_dfu_t *lp_dfu;
ble_dfu_evt_t * lp_evt;
ble_dfu_evt_t saved_dfu_evt;

void NANNY_normalBoot_schedule_9E_ON_cmd(void)
{
    prime_NANNY_ON();
    uniEvent_t LEvt;
    LEvt.evtType = evt_core_NANNY_trigger;
    core_thread_QueueSend(&LEvt);
}
void NANNY_dfu_schedule_9E_OFF_cmd(void)
{
    prime_NANNY_OFF();
    uniEvent_t LEvt;
    LEvt.evtType = evt_core_NANNY_trigger;
    core_thread_QueueSend(&LEvt);
}
void NANNY_call_after_9E_OFF_cmd(void)
{
    // dfu_app_handler.c
    // Here we continue with the standard handler code
    // dfu_app_on_dfu_evt() will execute, AND WILL execute the 
    //    case BLE_DFU_START:  code
    dfu_app_on_dfu_evt(lp_dfu, &saved_dfu_evt);
}

void _local_dfu_app_on_dfu_evt(ble_dfu_t * p_dfu, ble_dfu_evt_t * p_evt)
{
    lp_dfu = p_dfu;
    saved_dfu_evt = *p_evt;
    
    switch (p_evt->ble_dfu_evt_type)
    {
        case BLE_DFU_START:
            NANNY_dfu_schedule_9E_OFF_cmd();
            return;
            //break;
    }

    // dfu_app_handler.c
    // Here we fall through, because it was not a BLE_DFU_START event
    // dfu_app_on_dfu_evt() will execute, but will not execute the 
    //    case BLE_DFU_START:  code
    dfu_app_on_dfu_evt(p_dfu, p_evt);
}

static void  dfu_init(void)
{
#ifdef BLE_DFU_APP_SUPPORT
    uint32_t       err_code;
    /** @snippet [DFU BLE Service initialization] */
    ble_dfu_init_t   dfus_init;

    // Initialize the Device Firmware Update Service.
    memset(&dfus_init, 0, sizeof(dfus_init));

    dfus_init.evt_handler   = _local_dfu_app_on_dfu_evt; //NANNY
    //dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.error_handler = NULL;
    dfus_init.revision      = DFU_REVISION;

    dbgPrintf("\r\nble_dfu_init()");
    err_code = ble_dfu_init(&m_dfus, &dfus_init);
    dbgPrintf("\r\nble_dfu_init err_code= 0x%08x", err_code);
    APP_ERROR_CHECK(err_code);

    dfu_app_reset_prepare_set(reset_prepare); //NANNY
    //dfu_app_reset_prepare_set(NANNY_reset_prepare);
    
    dfu_app_dm_appl_instance_set(m_app_handle);
    /** @snippet [DFU BLE Service initialization] */
#endif // BLE_DFU_APP_SUPPORT
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
#if USE_DIS
    dis_init();
#endif

    dfu_init();

#if USE_TUDS
    services_init_tuds_part();
#endif

}




/**@brief       Function for handling an event from the Connection Parameters Module.
 *
 * @details     This function will be called for all events in the Connection Parameters Module
 *              which are passed to the application.
 *
 * @note        All this function does is to disconnect. This could have been done by simply setting
 *              the disconnect_on_fail config parameter, but instead we use the event handler
 *              mechanism to demonstrate its use.
 *
 * @param[in]   p_evt   Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
    
    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        autoTimeout_Start( AUTOTIMEOUT_NONE );
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief       Function for handling errors from the Connection Parameters module.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}



/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;
    
    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
//REF    cp_init.start_on_notify_cccd_handle    = m_hrs.hrm_handles.cccd_handle;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;  //karel -> force negotiation see ble_conn_params.c (232)
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_DIRECTED:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_DIRECTED);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_SLOW:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_FAST_WHITELIST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_SLOW_WHITELIST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;
#if USE_DM
#if USE_DM_E
        case BLE_ADV_EVT_WHITELIST_REQUEST:
        {
            ble_gap_whitelist_t whitelist;
            ble_gap_addr_t    * p_whitelist_addr[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            ble_gap_irk_t     * p_whitelist_irk[BLE_GAP_WHITELIST_IRK_MAX_COUNT];

            whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
            whitelist.irk_count  = BLE_GAP_WHITELIST_IRK_MAX_COUNT;
            whitelist.pp_addrs   = p_whitelist_addr;
            whitelist.pp_irks    = p_whitelist_irk;

            err_code = dm_whitelist_create(&m_app_handle, &whitelist);
            APP_ERROR_CHECK(err_code);

            err_code = ble_advertising_whitelist_reply(&whitelist);
            APP_ERROR_CHECK(err_code);
            break;
        }
        case BLE_ADV_EVT_PEER_ADDR_REQUEST:
        {
            ble_gap_addr_t peer_address;

            // Only Give peer address if we have a handle to the bonded peer.
            if(m_bonded_peer_handle.appl_id != DM_INVALID_ID)
            {
                            
                err_code = dm_peer_addr_get(&m_bonded_peer_handle, &peer_address);
                APP_ERROR_CHECK(err_code);

                err_code = ble_advertising_peer_addr_reply(&peer_address);
                APP_ERROR_CHECK(err_code);
                
            }
            break;
        }
#endif //#if USE_DME
#endif //#if USE_DM

        default:
            break;
    }
}

#if USE_NRF_EXAMPLE
typedef struct
{
    uint32_t value;
}my_char_t;

//static dm_handle_t m_bonded_dev_handle; //m_bonded_peer_handle
static my_char_t   m_my_char;
#endif


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                              err_code;
    ble_gatts_rw_authorize_reply_params_t auth_reply;
#if USE_NRF_EXAMPLE
    //api_result_t               retval;
    ret_code_t                 retval;
    dm_application_context_t   app_context;    
#endif

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);

            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_EVT_TX_COMPLETE:
            // Send next key event
            //reference HID device    (void) ble_buffered_dequeue(true);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            // Dequeue all keys without transmission.
            //reference HID device    (void) ble_buffered_dequeue(false);

            m_conn_handle = BLE_CONN_HANDLE_INVALID;

            // disabling alert 3. signal - used for capslock ON
            err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
            APP_ERROR_CHECK(err_code);
        

#if USE_NRF_EXAMPLE
            app_context.len    = sizeof(my_char_t);
            app_context.p_data = ((uint8_t *)&m_my_char); 

            // Want to store the last updated value of the characteristic persistently.
            //retval = dm_application_context_set(&m_bonded_dev_handle, &app_context); //m_bonded_peer_handle
            retval = dm_application_context_set(&m_bonded_peer_handle, &app_context); //m_bonded_peer_handle
            if (retval == NRF_SUCCESS)
            {
                dbgPrintf("\r\ndm_application_context_set OK");
                // Wait for DM_EVT_APPL_CONTEXT_STORED event.
                // Note do not reuse or change m_my_char.
            }
            else
            {
                // Failed to store application context.
                dbgPrintf("\r\ndm_application_context_set failed rtn = %d, 0x%08x", retval, retval);
            }
#endif

            break;

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(m_conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            if(p_ble_evt->evt.gatts_evt.params.authorize_request.type
               != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                     == BLE_GATTS_OP_PREP_WRITE_REQ)
                    || (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                     == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
                    || (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                     == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (p_ble_evt->evt.gatts_evt.params.authorize_request.type
                        == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                    auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(m_conn_handle,&auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        case BLE_GATTC_EVT_TIMEOUT:
        case BLE_GATTS_EVT_TIMEOUT:
            autoTimeout_Start( AUTOTIMEOUT_NONE );

            // Disconnect on GATT Server and Client timeout events.
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            autoTimeout_Start(AUTOTIMEOUT_NONE);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief   Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
/////////////////////////////////////////////////////////////////

bool m_bleIsConnected = false;

/////////////////////////////////////////////////////////////////
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    uniEvent_t LEvt;

    autoTimeout_Start( AUTOTIMEOUT_STDTIME ); // If there is activity then restart timeout 

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            m_bleIsConnected = true;
            LEvt.evtType = evt_ConState_Connected;
            core_thread_QueueSend(&LEvt); 
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            /* NANNY*/
            // don't set here m_bleIsConnected = false;
            LEvt.evtType = evt_ConState_Disconnected;
            core_thread_QueueSend(&LEvt); 
            /*NANNY*/
            break;
	}

#if USE_DM
    dm_ble_evt_handler(p_ble_evt);  //OK  Bonding -> BLE_GAP_EVT_SEC_PARAMS_REQUEST, BLE_GAP_EVT_CONN_SEC_UPDATE
#endif
    
#if USE_TUDS
    ble_tuds_on_ble_evt(&m_ble_tuds, p_ble_evt); //karel
#endif
    
    ble_conn_params_on_ble_evt(p_ble_evt);   //asasasasOK
    bsp_btn_ble_on_ble_evt(p_ble_evt); // BSP call HOW DO WE USE IT ???!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifdef BLE_DFU_APP_SUPPORT
    /** @snippet [Propagating BLE Stack events to DFU Service] */
    ble_dfu_on_ble_evt(&m_dfus, p_ble_evt);
    /** @snippet [Propagating BLE Stack events to DFU Service] */
#endif // BLE_DFU_APP_SUPPORT
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);    
}


/**@brief   Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */

/* REF from nrf_soc.h
//@brief SoC Events. 
enum NRF_SOC_EVTS
{
  NRF_EVT_HFCLKSTARTED,                         //0 // Event indicating that the HFCLK has started. 
  NRF_EVT_POWER_FAILURE_WARNING,                //1 // Event indicating that a power failure warning has occurred. 
  NRF_EVT_FLASH_OPERATION_SUCCESS,              //2 // Event indicating that the ongoing flash operation has completed successfully. 
  NRF_EVT_FLASH_OPERATION_ERROR,                //3 // Event indicating that the ongoing flash operation has timed out with an error. 
  NRF_EVT_RADIO_BLOCKED,                        //4 // Event indicating that a radio timeslot was blocked. 
  NRF_EVT_RADIO_CANCELED,                       //5 // Event indicating that a radio timeslot was canceled by SoftDevice. 
  NRF_EVT_RADIO_SIGNAL_CALLBACK_INVALID_RETURN, //6 // Event indicating that a radio signal callback handler return was invalid. 
  NRF_EVT_RADIO_SESSION_IDLE,                   //7 // Event indicating that a radio session is idle. 
  NRF_EVT_RADIO_SESSION_CLOSED,                 //8 // Event indicating that a radio session is closed. 
  NRF_EVT_NUMBER_OF_EVTS                        //9 // ....
};
REF */

static void power_failure_sys_event_handler(uint32_t sys_evt);
static void sys_evt_dispatch(uint32_t sys_evt)
{

    power_failure_sys_event_handler(sys_evt);
    
#if USE_DM
    dbgPrintf("\r\n------------------------ sys_evt_dispatch sys_evt = %d, 0x%08x", sys_evt, sys_evt);
    pstorage_sys_event_handler(sys_evt);
#endif
    ble_advertising_on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;
    
#if USE_APPSH
    // Initialize the SoftDevice handler module.
#if USE_SDK_V11
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
    SOFTDEVICE_HANDLER_APPSH_INIT(&clock_lf_cfg, true); //APPSH
#else
    SOFTDEVICE_HANDLER_APPSH_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, true); //APPSH
#endif

#else
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);
#endif

    
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT, PERIPHERAL_LINK_COUNT, &ble_enable_params);
    APP_ERROR_CHECK(err_code);

#ifdef BLE_DFU_APP_SUPPORT
    ble_enable_params.gatts_enable_params.service_changed = 1;
#endif // BLE_DFU_APP_SUPPORT
    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    ble_enable_params.common_enable_params.vs_uuid_count   = 3; //karel S130 V2

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

#if USE_DM    
    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
#endif
}



#if USE_APPSH
/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void) //APPSH
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE); //APPSH
}

#include "priority_scheduler.h"
static void priority_scheduler_init(void)
{
    PRIORITY_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}
#endif


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
/* REFERENCE
static void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    static uint8_t size = 0;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:

            autoTimeout_Start( AUTOTIMEOUT_NONE );

            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }

            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        //case BSP_EVENT_KEY_0: ref from HID device
        //    break;

        default:
            break;
    }
}
REFERENCE */



#if USE_DM
/**@brief Function for handling the Device Manager events.
 *
 * @param[in]   p_evt   Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const    * p_handle,
                                           dm_event_t const     * p_event,
                                           ret_code_t           event_result)
{
    APP_ERROR_CHECK(event_result);
#if USE_NRF_EXAMPLE
    //api_result_t               retval;
    ret_code_t                 retval;
    dm_application_context_t   app_context;    
#endif
    switch(p_event->event_id)
    {
#if !USE_NRF_EXAMPLE
        case DM_EVT_DEVICE_CONTEXT_LOADED: // Fall through.
#endif
        case DM_EVT_SECURITY_SETUP_COMPLETE:
            m_bonded_peer_handle = (*p_handle);
            break;
        
#if USE_NRF_EXAMPLE
        case DM_EVT_DEVICE_CONTEXT_LOADED:
        {       
            // This event is generated as soon as a bonded device reconnects and its bond and 
            // service information have been loaded. This may be most appropriate to load application
            // context.             
            app_context.len    = sizeof(my_char_t);
            app_context.p_data = ((uint8_t *)&m_my_char);

            retval = dm_application_context_get(p_handle,&app_context);
            if (retval == NRF_SUCCESS)
            {
                // DM_EVT_APPL_CONTEXT_LOADED event generated to indicate load complete.
                // Note do not reuse or change m_my_char.
                dbgPrintf("\r\nDDD dm_application_context_get OK");
            }
            else
            {
                dbgPrintf("\r\nDDD dm_application_context_get failed rtn = %d, 0x%08x", retval, retval);
            }
            break;
        }
        case DM_EVT_APPL_CONTEXT_LOADED:        
        {
            // Set char value using sd_ble_gatts_value_set
            // This event is generated as soon as a bonded device reconnects and its bond and 
            // service information have been loaded. This may be most appropriate to load application
            // context.             
            app_context.len    = sizeof(my_char_t);
            app_context.p_data = ((uint8_t *)&m_my_char);

            retval = dm_application_context_get(p_handle,&app_context);
            if (retval == NRF_SUCCESS)
            {
                // DM_EVT_APPL_CONTEXT_LOADED event generated to indicate load complete.
                // Note do not reuse or change m_my_char.
                dbgPrintf("\r\ndm_application_context_get OK");
            }
            else
            {
                dbgPrintf("\r\ndm_application_context_get failed rtn = %d, 0x%08x", retval, retval);
            }
            break;
        }
#endif
        
        
    }
    
#ifdef BLE_DFU_APP_SUPPORT                         // REF main_hrs
    if (p_event->event_id == DM_EVT_LINK_SECURED)  // REF main_hrs
    {                                              // REF main_hrs
        app_context_load(p_handle);                // REF main_hrs
    }                                              // REF main_hrs
#endif // BLE_DFU_APP_SUPPORT


    return NRF_SUCCESS;
}

/* unused
static void sec_params_init(void)
{
    m_sec_params.bond         = SEC_PARAM_BOND;
    m_sec_params.mitm         = SEC_PARAM_MITM;
    m_sec_params.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    m_sec_params.oob          = SEC_PARAM_OOB;  
    m_sec_params.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    m_sec_params.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
}
*/

/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */

#if USE_DM
#define BOND_DELETE_ALL_BUTTON_ID           1                                       /**< Button used for deleting all bonded centrals during startup. */
#endif

static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t  register_param;

    // Initialize peer device handle.
    err_code = dm_handle_initialize(&m_bonded_peer_handle); // REF hid_kb
    APP_ERROR_CHECK(err_code);
    
    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

/* ALT
    // Clear all bonded centrals if the Bonds Delete button is pushed.
    err_code = bsp_button_is_pressed(BOND_DELETE_ALL_BUTTON_ID,&(init_param.clear_persistent_data));
    APP_ERROR_CHECK(err_code);
ALT */

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);
    
    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}
#endif //#if USE_DM


/**@brief Function for handling advertising errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
/* UNCOMMENT IF USED
static void ble_advertising_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}
 UNCOMMENT IF USED */


#define USE_TEST_OF_MANU_SPECIFIC_DATA_ADV     1//1
#define USE_TEST_OF_DEVICE_NAME_IN_DATA_SCAN   1//1

/**@brief Function for initializing the Advertising functionality.
 */
void advertising_init_mg_new(uint32_t param_APP_ADV_INTERVAL) //mg_6_advX50ms_inTicks)
{
    uint32_t       err_code;
    ble_advdata_t  advdata;
    ble_advdata_t  scanrsp;

 

    memset(&advdata, 0, sizeof(advdata));

    // ----- flags -----
    advdata.flags             =  BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    
    // ----- Set MANUFACTURER SPECIFIC DATA in Advertising Packet -----
    ble_advdata_manuf_data_t  adv_manuf_data;
    uint8_array_t             adv_manuf_data_array;

    adv_manuf_data_array.p_data = &mg_ManufacturerSpecific_rsp26[2];
    adv_manuf_data_array.size = 24; // Fixed at 24. (24 + 2(for Company ID) = 26)
    adv_manuf_data.company_identifier = mg_ManufacturerSpecific_rsp26[1] * 256 + mg_ManufacturerSpecific_rsp26[0]; //0x00FE; //0x00FE, 0xFFFF are reserved
    adv_manuf_data.data = adv_manuf_data_array;
    
    // ----- manuf_specific_data -----
    advdata.p_manuf_specific_data = &adv_manuf_data;


    // ----- set The Shortened Name in the Scan Response -----
    memset(&scanrsp, 0, sizeof(scanrsp));

    scanrsp.include_appearance = false;
    scanrsp.name_type = BLE_ADVDATA_SHORT_NAME; //BLE_ADVDATA_FULL_NAME = 1; BLE_ADVDATA_NO_NAME = 0;
    //scanrsp.short_name_len = 26; // max 26
    scanrsp.short_name_len = strlen((char*)mg_ShortenedName_rsp26); 
    // Note: the BLE_ADVDATA_xxx_NAME (source: mg_ShortenedName_rsp26) is set elsewhere ..... sd_ble_gap_device_name_set( ... );
    

    err_code = ble_advdata_set(&advdata, &scanrsp);
    APP_ERROR_CHECK(err_code);
    
#if USE_DM
    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
  //options.ble_adv_fast_interval = APP_ADV_INTERVAL;       //mg_6_advX50ms_inTicks
    options.ble_adv_fast_interval = param_APP_ADV_INTERVAL; //mg_6_advX50ms_inTicks
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    /* REF
    ble_adv_modes_config_t options =
    {
        BLE_ADV_WHITELIST_ENABLED,
        BLE_ADV_DIRECTED_ENABLED,
        BLE_ADV_DIRECTED_SLOW_DISABLED, 0,0,
        BLE_ADV_FAST_ENABLED, APP_ADV_FAST_INTERVAL, APP_ADV_FAST_TIMEOUT,
        BLE_ADV_SLOW_ENABLED, APP_ADV_SLOW_INTERVAL, APP_ADV_SLOW_TIMEOUT
    };
    */

  //err_code = ble_advertising_init(&advdata, NULL,     &options, on_adv_evt, ble_advertising_error_handler);
    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
#endif
}





/* UNUSED
#include "asp.h"

void asp_event_handler( asp_event_t p)
{
}

static void user_asp_init(void)
{
    asp_event_t startup_event;

    uint32_t err_code = asp_init( 0, 0,  asp_event_handler); // see bsp_init
    APP_ERROR_CHECK(err_code);

    err_code = asp_ble_init(NULL, &startup_event); // see bsp_btn_ble_init
    APP_ERROR_CHECK(err_code);

    // *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}
UNUSED*/

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}



static void power_failure_sys_event_handler(uint32_t sys_evt)
{
    if( sys_evt == NRF_EVT_POWER_FAILURE_WARNING)
    {
        dbgPrint("\r\nNRF_EVT_POWER_FAILURE_WARNING");    
    }
}

#include "nrf_soc.h" //V11
void reset_testing_stuff()
{
#if 0 //V11TODO
    uint32_t err_code;

    dbgPrint("\r\nreset_testing_stuff");
    
    //-----
    uint32_t reset_reason = 0;
    err_code = sd_power_reset_reason_get( &reset_reason );
    dbgPrintf("\r\nreset_testing_stuff 0x%08x", reset_reason);
 
    //-----
    err_code = sd_power_pof_enable( true );
    err_code = sd_power_pof_enable( false );
    dbgPrintf("\r\nsd_power_pof_enable   err_code = 0x%08x", err_code);
    
    nrf_power_failure_threshold_t threshold;
    //threshold = NRF_POWER_THRESHOLD_V21;
    //threshold = NRF_POWER_THRESHOLD_V23;
    threshold = NRF_POWER_THRESHOLD_V25;
    //threshold = NRF_POWER_THRESHOLD_V27;
    err_code = sd_power_pof_threshold_set(threshold);
    dbgPrintf("\r\nsd_power_pof_threshold_set   err_code = 0x%08x", err_code);
 
    
    //-----
    
    // the RESETREAS register
    /* Bit 18 : Reset from wake-up from OFF mode detected by entering into debug interface mode. */
    //#define POWER_RESETREAS_DIF_Pos (18UL) /*!< Position of DIF field. */
    //#define POWER_RESETREAS_DIF_Msk (0x1UL << POWER_RESETREAS_DIF_Pos) /*!< Bit mask of DIF field. */
    //#define POWER_RESETREAS_DIF_NotDetected (0UL) /*!< Reset not detected. */
    //#define POWER_RESETREAS_DIF_Detected (1UL) /*!< Reset detected. */
    if( (reset_reason & POWER_RESETREAS_DIF_Msk) == POWER_RESETREAS_DIF_Detected )
    {
        dbgPrint("\r\nRESETREAS_DIF");
    }

    /* Bit 17 : Reset from wake-up from OFF mode detected by the use of ANADETECT signal from LPCOMP. */
    #define POWER_RESETREAS_LPCOMP_Pos (17UL) /*!< Position of LPCOMP field. */
    #define POWER_RESETREAS_LPCOMP_Msk (0x1UL << POWER_RESETREAS_LPCOMP_Pos) /*!< Bit mask of LPCOMP field. */
    #define POWER_RESETREAS_LPCOMP_NotDetected (0UL) /*!< Reset not detected. */
    #define POWER_RESETREAS_LPCOMP_Detected (1UL) /*!< Reset detected. */
    if( (reset_reason & POWER_RESETREAS_LPCOMP_Msk) == POWER_RESETREAS_LPCOMP_Detected )
    {
        dbgPrint("\r\nRESETREAS_LPCOMP");
    }
    
    /* Bit 16 : Reset from wake-up from OFF mode detected by the use of DETECT signal from GPIO. */
    #define POWER_RESETREAS_OFF_Pos (16UL) /*!< Position of OFF field. */
    #define POWER_RESETREAS_OFF_Msk (0x1UL << POWER_RESETREAS_OFF_Pos) /*!< Bit mask of OFF field. */
    #define POWER_RESETREAS_OFF_NotDetected (0UL) /*!< Reset not detected. */
    #define POWER_RESETREAS_OFF_Detected (1UL) /*!< Reset detected. */
    if( (reset_reason & POWER_RESETREAS_OFF_Msk) == POWER_RESETREAS_OFF_Detected )
    {
        dbgPrint("\r\nRESETREAS_OFF");
    }
    
    /* Bit 3 : Reset from CPU lock-up detected. */
    #define POWER_RESETREAS_LOCKUP_Pos (3UL) /*!< Position of LOCKUP field. */
    #define POWER_RESETREAS_LOCKUP_Msk (0x1UL << POWER_RESETREAS_LOCKUP_Pos) /*!< Bit mask of LOCKUP field. */
    #define POWER_RESETREAS_LOCKUP_NotDetected (0UL) /*!< Reset not detected. */
    #define POWER_RESETREAS_LOCKUP_Detected (1UL) /*!< Reset detected. */
    if( (reset_reason & POWER_RESETREAS_LOCKUP_Msk) == POWER_RESETREAS_LOCKUP_Detected )
    {
        dbgPrint("\r\nRESETREAS_LOCKUP");
    }
    
    /* Bit 2 : Reset from AIRCR.SYSRESETREQ detected. */
    #define POWER_RESETREAS_SREQ_Pos (2UL) /*!< Position of SREQ field. */
    #define POWER_RESETREAS_SREQ_Msk (0x1UL << POWER_RESETREAS_SREQ_Pos) /*!< Bit mask of SREQ field. */
    #define POWER_RESETREAS_SREQ_NotDetected (0UL) /*!< Reset not detected. */
    #define POWER_RESETREAS_SREQ_Detected (1UL) /*!< Reset detected. */
    if( (reset_reason & POWER_RESETREAS_SREQ_Msk) == POWER_RESETREAS_SREQ_Detected )
    {
        dbgPrint("\r\nRESETREAS_SREQ");
    }
    
    /* Bit 1 : Reset from watchdog detected. */
    #define POWER_RESETREAS_DOG_Pos (1UL) /*!< Position of DOG field. */
    #define POWER_RESETREAS_DOG_Msk (0x1UL << POWER_RESETREAS_DOG_Pos) /*!< Bit mask of DOG field. */
    #define POWER_RESETREAS_DOG_NotDetected (0UL) /*!< Reset not detected. */
    #define POWER_RESETREAS_DOG_Detected (1UL) /*!< Reset detected. */
    if( (reset_reason & POWER_RESETREAS_DOG_Msk) == POWER_RESETREAS_DOG_Detected )
    {
        dbgPrint("\r\nRESETREAS_DOG");
    }
    
    /* Bit 0 : Reset from pin-reset detected. */
    #define POWER_RESETREAS_RESETPIN_Pos (0UL) /*!< Position of RESETPIN field. */
    #define POWER_RESETREAS_RESETPIN_Msk (0x1UL << POWER_RESETREAS_RESETPIN_Pos) /*!< Bit mask of RESETPIN field. */
    #define POWER_RESETREAS_RESETPIN_NotDetected (0UL) /*!< Reset not detected. */
    #define POWER_RESETREAS_RESETPIN_Detected (1UL) /*!< Reset detected. */
    if( (reset_reason & POWER_RESETREAS_RESETPIN_Msk) == POWER_RESETREAS_RESETPIN_Detected )
    {
        dbgPrint("\r\nRESETREAS_RESETPIN");
    }

#endif //V11TODO
    
}


void pinsTUG_Init(void);
void pinsTUG_25_Assert(void);
void pinsTUG_25_Release(void);
void pinsTUG_25_Invert(void);

//debug uint32_t DUMB_counterA = 0;

int main_tuds(void)
{
    bool erase_bonds;
    //uint32_t err_code;

    pinWakeUp_Init();

    //not used - app_trace_init();
    
    dbgPrint_Init();     // karel - debug via spi-master SDO
#if USE_PRINTF_OVER_SDO
    nrf_delay_ms(500);
    dbgPrint("1");
    nrf_delay_ms(500);
    dbgPrint("2");
    nrf_delay_ms(500);
    dbgPrint("3");
    nrf_delay_ms(500);
    dbgPrint("4");
    nrf_delay_ms(500);
    dbgPrint("5");
        
    dbgPrint("\r\n");
//    dbgPrint("\r\nAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
//    dbgPrint("\r\nBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");
    dbgPrint("\r\nCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC");
    dbgPrint("\r\n");
#endif
        
	ma_adc_config();

    timers_pre_init();
    timers_init();

    ble_stack_init();

#if USE_APPSH
    scheduler_init();           // <> NOTE: KAREL Adjust Queue size etc as needed
    priority_scheduler_init();  // <> NOTE: KAREL Adjust Queue size etc as needed
#else
#endif
    
#if USE_DM
    erase_bonds = false;
    device_manager_init(erase_bonds);
#endif


    mg_ShortenedName_rsp26_set("");
    mg_ShortenedName_rsp26_set(DEVICE_NAME);
    gap_params_init( (char*)mg_ShortenedName_rsp26 );

    services_init();

    mg_ManufacturerSpecific_rsp26_setInitialValue();
    mg_6_advX50ms_inTicks = APP_ADV_INTERVAL_1000MS;
    advertising_init_mg_new(mg_6_advX50ms_inTicks);
    
    conn_params_init();

    uart_thread_init();    // ma_uart.c

    core_thread_init();    // ma_thread.c


    //SAS  sd_power_ramon_set(0x00030003); //karel
    //  NRF_POWER_MODE_CONSTLAT,  // Constant latency mode. See power management in the reference manual. 
	//  NRF_POWER_MODE_LOWPWR     // Low power mode. See power management in the reference manual. 
    //SAS  nrf_power_mode_t power_mode = NRF_POWER_MODE_LOWPWR;
	//SAS  err_code = sd_power_mode_set(power_mode);


    NADC_proc( NADC_action_RESET);
    bln_proc(BLN_PROC_INIT_TRIGGER);
    blp_proc(BLP_PROC_INIT_TRIGGER);
    
    GP_timer_start( APP_TIMER_TICKS(GP_TIMER_PERIOD_1000MS, APP_TIMER_PRESCALER) );

    //debug reset_testing_stuff();

    dbgPrint("\r\nMain setup Finished");
    
    extern bool BLN_boot;
    BLN_boot = true;

    //debug DUMB_counterA = 0;
    //debug pinsTUG_Init();

    for (;;)
    {
#if USE_APPSH
        priority_sched_execute();
        app_sched_execute();
#else
#endif
        //debug pinsTUG_25_Assert();
        power_manage();
        //debug pinsTUG_25_Release();
        //debug //pinsTUG_25_Invert();

        //debug DUMB_counterA++;
    }
}





