#ifndef __MA_TIMERS_H
#define __MA_TIMERS_H

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_bas.h"
//#include "ble_hrs.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "sensorsim.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "device_manager.h"
#include "pstorage.h"
#include "app_trace.h"
#include "bsp.h"
#include "bsp_btn_ble.h"





#define APP_TIMER_PRESCALER              0                                          /**< Value of the RTC1 PRESCALER register. */
//#define APP_TIMER_OP_QUEUE_SIZE          4                                          /**< Size of timer operation queues. */
#define APP_TIMER_OP_QUEUE_SIZE          14                                          /**< Size of timer operation queues. */


#define  GP_TIMER_PERIOD_1000MS   1000


#define  MilliSec_5000   5000
#define  MilliSec_1000   1000
#define  MilliSec_50       50

//void timers_init(void);

uint32_t ma_uart_timer_init(void);
uint32_t ma_uart_timer_start(uint32_t timeout_ticks);
uint32_t ma_uart_timer_stop(void);

uint32_t ma_holdoff_timer_init(void);
uint32_t ma_holdoff_timer_start(uint32_t timeout_ticks);
uint32_t ma_holdoff_timer_stop(void);

#if USE_ADCON_TIMER
uint32_t battLoad_timer_init(void);
uint32_t battLoad_timer_start(uint32_t timeout_ticks);
uint32_t battLoad_timer_stop(void);
#endif

uint32_t GP_timer_init(void);
uint32_t GP_timer_start(uint32_t timeout_ticks);
uint32_t GP_timer_stop(void);
void GP_timer_GBN_enable(uint32_t ticks_until);

uint32_t BlkUp_timer_init(void);
uint32_t BlkUp_timer_start(uint32_t timeout_ticks);
uint32_t BlkUp_timer_stop(void);

uint32_t BlkDn_timer_init(void);
uint32_t BlkDn_timer_start(uint32_t timeout_ticks);
uint32_t BlkDn_timer_stop(void);


enum e_ma_uart_timer_Reason
{
    Reason_none,
    Reason_rxTimeout,
    Reason_shutdown,
};

extern int ma_uart_timer_Reason;





#ifdef __cplusplus
}
#endif

#endif // __MA_TIMERS_H


