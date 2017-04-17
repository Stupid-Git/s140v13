
#ifndef BLE_TUDS_H__
#define BLE_TUDS_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>


//+V13 //OLD->NEW 
#define GATT_MTU_SIZE_DEFAULT /*OLD*/  BLE_GATT_ATT_MTU_DEFAULT/*NEW*/ //+V13
//+V13

#define BLE_UUID_TUDS_SERVICE 0x0001                      /**< The UUID of the Nordic UART Service. */
#define BLE_TUDS_MAX_DATA_LEN (GATT_MTU_SIZE_DEFAULT - 3) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */

/* Forward declaration of the ble_tuds_t type. */
typedef struct ble_tuds_s ble_tuds_t;

/**@brief Nordic UART Service event handler type. */
typedef void (*ble_tuds_Ddat_handler_t) (ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length);
typedef void (*ble_tuds_Dcmd_handler_t) (ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length);
typedef void (*ble_tuds_tx_complete_handler) (ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt);


typedef void (*ble_tuds_Ucfm_handler_t) (ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length);


/**@brief Nordic UART Service initialization structure.
 *
 * @details This structure contains the initialization information for the service. The application
 * must fill this structure and pass it to the service using the @ref ble_tuds_init
 *          function.
 */
typedef struct ble_tuds_init_s
{
    ble_tuds_Dcmd_handler_t  Dcmd_handler;    // Event handler to be called for handling received DCMD.
    ble_tuds_Ddat_handler_t  Ddat_handler;    // Event handler to be called for handling received DDAT.
    ble_tuds_tx_complete_handler  tx_complete_handler;

    ble_tuds_Ucfm_handler_t  Ucfm_handler;    // Event handler to be called for handling received UCFM.

} ble_tuds_init_t;

/**@brief Nordic UART Service structure.
 *
 * @details This structure contains status information related to the service.
 */
struct ble_tuds_s
{
    uint8_t                    uuid_type;               // UUID type for Nordic UART Service Base UUID.
    uint16_t                   service_handle;          // Handle of Nordic UART Service (as provided by the S110 SoftDevice).
    uint16_t                   conn_handle;             // Handle of the current connection (as provided by the S110 SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection.

    ble_gatts_char_handles_t   Dcmd_handles;            // Handles related to the DCMD characteristic (as provided by the S110 SoftDevice).
    ble_gatts_char_handles_t   Ddat_handles;            // Handles related to the DDAT characteristic (as provided by the S110 SoftDevice).
    ble_gatts_char_handles_t   Dcfm_handles;            // Handles related to the DCFM characteristic (as provided by the S110 SoftDevice).
    ble_tuds_Dcmd_handler_t    Dcmd_handler;            // Event handler to be called for handling received Command.
    ble_tuds_Ddat_handler_t    Ddat_handler;            // Event handler to be called for handling received Data.
    bool                       is_DCFM_notify_enabled;  // Variable to indicate if the peer has enabled notification of the DCFM characteristic.
    ble_tuds_tx_complete_handler  tx_complete_handler;

    ble_gatts_char_handles_t   Ucmd_handles;            // Handles related to the UCMD characteristic (as provided by the S110 SoftDevice).
    ble_gatts_char_handles_t   Udat_handles;            // Handles related to the UDAT characteristic (as provided by the S110 SoftDevice).
    ble_gatts_char_handles_t   Ucfm_handles;            // Handles related to the UCFM characteristic (as provided by the S110 SoftDevice).
    bool                       is_UCMD_notify_enabled;  // Variable to indicate if the peer has enabled notification of the DCFM characteristic.
    bool                       is_UDAT_notify_enabled;  // Variable to indicate if the peer has enabled notification of the DCFM characteristic.
    ble_tuds_Ucfm_handler_t    Ucfm_handler;            // Event handler to be called for handling received CFM.
/*    
    ble_gatts_char_handles_t   Wctrl_handles;           // Handles related to the WCTRL characteristic (as provided by the S110 SoftDevice).
    ble_gatts_char_handles_t   Rctrl_handles;           // Handles related to the RCTRL characteristic (as provided by the S110 SoftDevice).
*/ 

    uint16_t Boguslen16;
};

/**@brief Function for initializing the Nordic UART Service.
 *
 * @param[out] p_tuds      Nordic UART Service structure. This structure must be supplied
 *                        by the application. It is initialized by this function and will
 *                        later be used to identify this particular service instance.
 * @param[in] p_tuds_init  Information needed to initialize the service.
 *
 * @retval NRF_SUCCESS If the service was successfully initialized. Otherwise, an error code is returned.
 * @retval NRF_ERROR_NULL If either of the pointers p_tuds or p_tuds_init is NULL.
 */
uint32_t ble_tuds_init(ble_tuds_t * p_tuds, const ble_tuds_init_t * p_tuds_init);

/**@brief Function for handling the Nordic UART Service's BLE events.
 *
 * @details The Nordic UART Service expects the application to call this function each time an
 * event is received from the S110 SoftDevice. This function processes the event if it
 * is relevant and calls the Nordic UART Service event handler of the
 * application if necessary.
 *
 * @param[in] p_tuds       Nordic UART Service structure.
 * @param[in] p_ble_evt   Event received from the S110 SoftDevice.
 */
void ble_tuds_on_ble_evt(ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt);

/**@brief Function for sending a string to the peer.
 *
 * @details This function sends the input string as an RX characteristic notification to the
 *          peer.
 *
 * @param[in] p_tuds    Pointer to the T&D UD Service structure.
 * @param[in] buf       Buffer to be sent.
 * @param[in] p_len16   Pointer to Length of the buffer.
 *
 * @retval NRF_SUCCESS If the string was sent successfully. Otherwise, an error code is returned.
 */
uint32_t ble_tuds_notify_Dcfm(ble_tuds_t * p_tuds, uint8_t * buf, uint16_t* p_len16);

uint32_t ble_tuds_notify_Ucmd(ble_tuds_t * p_tuds, uint8_t * buf, uint16_t* p_len16);
uint32_t ble_tuds_notify_Udat(ble_tuds_t * p_tuds, uint8_t * buf, uint16_t* p_len16);

#endif // BLE_TUDS_H__
