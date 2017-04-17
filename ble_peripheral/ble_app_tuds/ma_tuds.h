
#ifndef MA_TUDS_H__
#define MA_TUDS_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>

#include "ble_tuds.h"

#if 0 //000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

/* Forward declaration of the ble_tuds_t type. */
struct app_tuds_s;
typedef struct app_tuds_s app_tuds_t;


typedef enum
{
    APP_TUDS_RX_PKT_0,
    APP_TUDS_TX_DONE,
} app_tuds_evt_type_t;

/**@brief Struct containing events from the UART module.
 *
 * @details The app_uart_evt_t is used to notify the application of asynchronous events when data
 * are received on the UART peripheral or in case an error occured during data reception.
 */
typedef struct
{
    app_tuds_evt_type_t evt_type; /**< Type of event. */
    union
    {
        uint32_t error_communication; /**< Field used if evt_type is: APP_UART_COMMUNICATION_ERROR. This field contains the value in the ERRORSRC register for the UART peripheral. The UART_ERRORSRC_x defines from nrf5x_bitfields.h can be used to parse the error code. See also the \nRFXX Series Reference Manual for specification. */
        uint32_t error_code;          /**< Field used if evt_type is: NRF_ERROR_x. Additional status/error code if the error event type is APP_UART_FIFO_ERROR. This error code refer to errors defined in nrf_error.h. */
        uint8_t  value;               /**< Field used if evt_type is: NRF_ERROR_x. Additional status/error code if the error event type is APP_UART_FIFO_ERROR. This error code refer to errors defined in nrf_error.h. */
    } data;
} app_tuds_evt_t;


typedef void (* app_tuds_event_handler_t) (app_tuds_evt_t * p_app_tuds_event);
typedef void (* app_tuds_packet_handler_t) (app_tuds_t * p_ma_tuds, uint8_t * p_data, uint16_t length);


typedef struct app_tuds_init_s
{
    app_tuds_packet_handler_t  packet_handler;    // Event handler to be called for handling received DCMD.

} app_tuds_init_t;

struct app_tuds_s
{
    ble_tuds_t *                p_ble_tuds;

    app_tuds_packet_handler_t    packet_handler;

};


/////////////////////////////////////////////////////////////////////////

extern app_tuds_t m_app_tuds;

uint32_t  app_tuds_init(app_tuds_t * p_app_tuds, const app_tuds_init_t * p_app_tuds_init);
void      ma_tuds_on_ble_evt(ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt); //wrapper for ble_tuds_on_ble_evt

uint32_t  ma_tuds_send_packet(app_tuds_t * p_tuds, uint8_t * buf, uint16_t* p_len16);

#endif //if 0 //000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

#endif // MA_TUDS_H__
