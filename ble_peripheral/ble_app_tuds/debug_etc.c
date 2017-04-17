
#include "debug_etc.h"


//------------------------- NO Debug -------------------------
#if (USE_PRINTF_OVER_SDO==0)

// see debug_etc.h

//------------------------- YES Debug -------------------------
#else

char m_s1[256];
//int sdo_AppendText( const char * s );

// http://www.cplusplus.com/reference/cstdio/printf/
// http://www.cplusplus.com/reference/cstdio/vprintf/
int dbgPrintf( const char * format, ... )
{
    int r;
    va_list args;
    va_start (args, format);
    //r = vprintf (format, args);
    
    //vsprintf(m_s1, format, args);
    vsnprintf(m_s1, sizeof(m_s1), format, args);
    sdo_AppendText( m_s1);

    va_end (args);
    return(r);
}

//void sdo_Init(void);
//void dbgPrint_Init(void)
//{
//    sdo_Init();
//}

//#define dbgPrint(x) sdo_AppendText(x)
/*
int dbgPrint( const char *str )
{
    int r = 0;
    sdo_AppendText( (char*)str );
    return(r);
}
*/

// from ble_ranges.h
/*
#define BLE_EVT_INVALID        0x00       //< Invalid BLE Event. 

#define BLE_EVT_BASE           0x01       //< Common BLE Event base. 
#define BLE_EVT_LAST           0x0F       //< Total: 15. 

#define BLE_GAP_EVT_BASE       0x10       //< GAP BLE Event base.
#define BLE_GAP_EVT_LAST       0x2F       //< Total: 32. 

#define BLE_GATTC_EVT_BASE     0x30       //< GATTC BLE Event base. 
#define BLE_GATTC_EVT_LAST     0x4F       //< Total: 32.

#define BLE_GATTS_EVT_BASE     0x50       //< GATTS BLE Event base. 
#define BLE_GATTS_EVT_LAST     0x6F       //< Total: 32. 

#define BLE_L2CAP_EVT_BASE     0x70       //< L2CAP BLE Event base. 
#define BLE_L2CAP_EVT_LAST     0x8F       //< Total: 32.  
*/
// from ble_gap.h

#include "ble.h"
#include "ble_gap.h"
#include "ble_gatts.h"

const char * get_ble_evt_str( uint8_t evt_id)
{
    const char * s = "Unknown";
    
    switch( evt_id )
    {
    // ble.h BLE_EVT_BASE = 0x01
    case BLE_EVT_TX_COMPLETE:                          // 0x01 < Transmission Complete. @ref ble_evt_tx_complete_t */
        s = "BLE_EVT_TX_COMPLETE";
        break;
    case BLE_EVT_USER_MEM_REQUEST:                     // 0x02 < User Memory request. @ref ble_evt_user_mem_request_t */
        s = "BLE_EVT_USER_MEM_REQUEST";
        break;
    case BLE_EVT_USER_MEM_RELEASE:                     // 0x03 < User Memory release. @ref ble_evt_user_mem_release_t */
        s = "BLE_EVT_USER_MEM_RELEASE";
        break;
    
    // ble_gap.h BLE_GAP_EVT_BASE = 0x10
    case BLE_GAP_EVT_CONNECTED:                        // 0x10 < Connection established.                         \n See @ref ble_gap_evt_connected_t.            
        s = "BLE_GAP_EVT_CONNECTED";
        break;
    case BLE_GAP_EVT_DISCONNECTED:                     // 0x11 < Disconnected from peer.                         \n See @ref ble_gap_evt_disconnected_t.         
        s = "BLE_GAP_EVT_DISCONNECTED";
        break;
    case BLE_GAP_EVT_CONN_PARAM_UPDATE:                // 0x12 < Connection Parameters updated.                  \n See @ref ble_gap_evt_conn_param_update_t.    
        s = "BLE_GAP_EVT_CONN_PARAM_UPDATE";
        break;
    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:               // 0x13 < Request to provide security parameters.         \n Reply with @ref sd_ble_gap_sec_params_reply.  \n See @ref ble_gap_evt_sec_params_request_t. 
        s = "BLE_GAP_EVT_SEC_PARAMS_REQUEST";
        break;
    case BLE_GAP_EVT_SEC_INFO_REQUEST:                 // 0x14 < Request to provide security information.        \n Reply with @ref sd_ble_gap_sec_info_reply.    \n See @ref ble_gap_evt_sec_info_request_t.   
        s = "BLE_GAP_EVT_SEC_INFO_REQUEST";
        break;
    case BLE_GAP_EVT_PASSKEY_DISPLAY:                  // 0x15 < Request to display a passkey to the user.       \n See @ref ble_gap_evt_passkey_display_t.      
        s = "BLE_GAP_EVT_PASSKEY_DISPLAY";
        break;
    case BLE_GAP_EVT_AUTH_KEY_REQUEST:                 // 0x16 < Request to provide an authentication key.       \n Reply with @ref sd_ble_gap_auth_key_reply.    \n See @ref ble_gap_evt_auth_key_request_t.   
        s = "BLE_GAP_EVT_AUTH_KEY_REQUEST";
        break;
    case BLE_GAP_EVT_AUTH_STATUS:                      // 0x17 < Authentication procedure completed with status. \n See @ref ble_gap_evt_auth_status_t.          
        s = "BLE_GAP_EVT_AUTH_STATUS";
        break;
    case BLE_GAP_EVT_CONN_SEC_UPDATE:                  // 0x18 < Connection security updated.                    \n See @ref ble_gap_evt_conn_sec_update_t.      
        s = "BLE_GAP_EVT_CONN_SEC_UPDATE";
        break;
    case BLE_GAP_EVT_TIMEOUT:                          // 0x19 < Timeout expired.                                \n See @ref ble_gap_evt_timeout_t.              
        s = "BLE_GAP_EVT_TIMEOUT";
        break;
    case BLE_GAP_EVT_RSSI_CHANGED:                     // 0x1A < RSSI report.                                    \n See @ref ble_gap_evt_rssi_changed_t.         
        s = "BLE_GAP_EVT_RSSI_CHANGED";
        break;
    case BLE_GAP_EVT_ADV_REPORT:                       // 0x1B < Advertising report.                             \n See @ref ble_gap_evt_adv_report_t.           
        s = "BLE_GAP_EVT_ADV_REPORT";
        break;
    case BLE_GAP_EVT_SEC_REQUEST:                      // 0x1C < Security Request.                               \n See @ref ble_gap_evt_sec_request_t.          
        s = "BLE_GAP_EVT_SEC_REQUEST";
        break;    
    case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:        // 0x1D < Connection Parameter Update Request.            \n Reply with @ref sd_ble_gap_conn_param_update. \n See @ref ble_gap_evt_conn_param_update_request_t. 
        s = "BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST";
        break;    
    case BLE_GAP_EVT_SCAN_REQ_REPORT:                  // 0x1E < Scan request report.                            \n See @ref ble_gap_evt_scan_req_report_t.      
        s = "BLE_GAP_EVT_SCAN_REQ_REPORT";
        break;

    // "ble_gatts.h" BLE_GATTS_EVT_BASE = 0x50
    case BLE_GATTS_EVT_WRITE:                          // 0x50 < Write operation performed.                                           \n See @ref ble_gatts_evt_write_t.                 */
        s = "BLE_GATTS_EVT_WRITE";
        break;
    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:           // 0x51 < Read/Write Authorization request.                                    \n Reply with @ref sd_ble_gatts_rw_authorize_reply. \n See @ref ble_gatts_evt_rw_authorize_request_t. */
        s = "BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST";
        break;
    case BLE_GATTS_EVT_SYS_ATTR_MISSING:               // 0x52 < A persistent system attribute access is pending.                     \n Respond with @ref sd_ble_gatts_sys_attr_set.     \n See @ref ble_gatts_evt_sys_attr_missing_t.     */
        s = " BLE_GATTS_EVT_SYS_ATTR_MISSING";
        break;
    case BLE_GATTS_EVT_HVC:                            // 0x53 < Handle Value Confirmation.                                           \n See @ref ble_gatts_evt_hvc_t.                   */
        s = " BLE_GATTS_EVT_HVC";
        break;
    case BLE_GATTS_EVT_SC_CONFIRM:                     // 0x54 < Service Changed Confirmation. No additional event structure applies.                                                    */
        s = " BLE_GATTS_EVT_SC_CONFIRM";
        break;
    case BLE_GATTS_EVT_TIMEOUT:                 
        s = "BLE_GATTS_EVT_TIMEOUT";
        break;

    default:
        s = "Unknown";
        break;
    }
       
    return( s );
}

//------------------------- End Debug -------------------------
#endif
