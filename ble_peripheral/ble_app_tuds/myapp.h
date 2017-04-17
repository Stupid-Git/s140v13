#ifndef __MYAPP_H
#define __MYAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

    
#include "debug_etc.h"

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
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "sensorsim.h"
#include "softdevice_handler.h"
#include "app_timer.h"
//+V13 TODO  #include "device_manager.h"
//+V13 TODO  #include "pstorage.h"
//+V13 TODO  #include "app_trace.h"
#include "bsp.h"
#include "bsp_btn_ble.h"

//#define FIRMWARE_REV_LSB  0x91  // 2016/12/09  Release/Tag TUDS_V0.91
//#define FIRMWARE_REV_MSB  0x00 // e.g. 0x0091 -> Version 0.91

//#define FIRMWARE_REV_LSB  0x00  // 2017/01/06  Release/Tag TUDS_V1.00 <- TODO
//#define FIRMWARE_REV_MSB  0x01  // e.g. 0x0100 -> Version 1.00

#define FIRMWARE_REV_LSB  0x01  // 2017/01/12  Release/Tag TUDS_V1.01 <- TODO "added bool bShortenedNameDidChange = false;
#define FIRMWARE_REV_MSB  0x01  // e.g. 0x0101 -> Version 1.01

//---------------------------------------------------------
#define  USE_SDK_V11 0
#if USE_SDK_V11
#endif

//---------------------------------------------------------
#define  USE_SDK_V13 1
#if USE_SDK_V13
#endif


//---------------------------------------------------------
// Use NEW ADC scheme
#define USE_NADC   1
#define BOOT_V_LIMIT 2.60 // 2.50

//---------------------------------------------------------
// Timer to time the ON period for the ADC load
#define USE_ADCON_TIMER    1


//---------------------------------------------------------
// Use CRC instead of checksum
#define _USE_CRC  1 //0

#if _USE_CRC
#define _ALLOW_REVERSE_CRC  0 //0
//uint16_t crc16_compute(uint8_t * p_data, uint16_t size, uint16_t * p_crc);
//+V13 TODO  #include "ma_utils.h"
#endif



//---------------------------------------------------------
// Wake pin output to micro controller 
// Set  WAKE_ACTIVE_HIGH  to 1 for active high (rising edge)  signalling 
// Set  WAKE_ACTIVE_HIGH  to 0 for active low  (falling edge) signalling 
#define WAKE_ACTIVE_HIGH  1//1 

// must be a multiple of 2
#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */
//As expected, makeing the FIFO bigger did not stop the byte swapping
//#define UART_TX_BUF_SIZE                4096                                         /**< UART TX buffer size. */
//#define UART_RX_BUF_SIZE                4096                                         /**< UART RX buffer size. *///

//---------------------------------------------------------
// For debugging, for 9E command reponses to test values
#define USE_FORCE_9E_5_TO_1  0       //  mg_5_dengen = 1; // Force No sleep_mode
#define USE_FORCE_9E_6_ADV   0       //  if(mg_6_advX50ms >  20) mg_6_advX50ms =  20; // > 1000ms is too Slow ?
#define USE_FORCE_9E_9_ADC   0//1       //  



//---------------------------------------------------------
// New Feature, Automatically Disconnect if there has been 
// no activity for a certain period 
#define AUTOTIMEOUT_STDTIME 60       //  Standard Timeout period until Disconnect
#define AUTOTIMEOUT_NONE     0       //  Disable AutoTimeout
void autoTimeout_Start( uint32_t timeout );


//-----------------------------------------------------------------------------
// BLN - 0x9E 0x00 Name Get processing
#define BLN_PROC_INIT_TRIGGER    0
#define BLN_PROC_TIMER_TICK      1
#define BLN_PROC_UNPARK          2
#define BLN_PROC_FORCE_TRIGGER   3
void bln_proc(int param);

//-----------------------------------------------------------------------------
// BLP - 0x9E_01 Parameter Get processing
#define BLP_PROC_INIT_TRIGGER    0
#define BLP_PROC_TIMER_TICK      1
#define BLP_PROC_UNPARK          2
#define BLP_PROC_FORCE_TRIGGER   3
void blp_proc(int param);


//=============================================================================
// TYPES


typedef enum epktType
{
    ePkt_Unknown,
//    ePkt_0xC0,
    ePkt_0x01,
//    ePkt_T2,
//    ePkt_K2,
} epktType_t;


struct be_s;
typedef struct be_s be_t;

typedef struct be_s
{
    void      (*vIndicateUse_Start)(be_t* this_be);
    void      (*vIndicateUse_End)(be_t* this_be);
    void*      vpSemaphore;
    epktType_t pktType;

    uint8_t  c0;
    uint8_t  c1;
    uint8_t  *buffer;
    uint32_t capacity;
    uint32_t rdPtr;
    uint32_t wrPtr;
    uint32_t length;
} be_t;



void NANNY_call_after_9E_OFF_cmd(void);

void prime_NANNY_OFF(void);
void prime_NANNY_ON(void);
void NANNY_normalBoot_schedule_9E_ON_cmd(void);


extern bool m_bleIsConnected;

//=============================================================================
typedef enum eEvtType
{
    evt_ConState_Connected        =  0,         // 0
    evt_ConState_Disconnected     =  1,         // 1
    
    // Event
    evt_Uart_RxReady              = 10,                // 10
    evt_Uart_TxEmpty              = 11,                // 11
    
    evt_core_BLN_trigger          = 20,                // 4
    evt_core_BLP_trigger          = 21,                // 5
    evt_core_ADC_trigger          = 22,                // 6
    evt_core_NANNY_trigger        = 23,                // 7
    
    // Core Master
    evt_coreMaster_UartTxDone     = 30,                // 7
    evt_coreMaster_UartRxDone     = 31,                // 8

    evt_xxxMaster_UartRxTimeout   = 32,                // 9
    
    // bleMaster
    evt_bleMaster_trigger         = 40,                // 10
    evt_bleMaster_UartTxDone      = 41,                // 11
    evt_bleMaster_UartRxDone      = 42,                // 12
    evt_bleMaster_BleRspDone      = 43,                // 13

    // SS
    evt_SSQ_Check_Queue           = 50,                // 14
    evt_SSQ_StartNext_TxRx        = 51,                // 15
    evt_SSQ_TxRxOrTimeout_Done    = 52,                // 16

} eEvtType_t;

typedef struct uniEvent_s
{
    eEvtType_t evtType;
    int i;
} uniEvent_t;


//=============================================================================
// predefines
// Buffers
extern           be_t  be_CUm;
extern           be_t  be_UCm;

extern           be_t  be_BU;
extern volatile  be_t  be_UB;

extern           be_t  be_Urx;

extern volatile  be_t  *m_curr_beUrx;
extern           be_t  *m_curr_beUtx;


void core_thread(void * arg);
void uart_thread(void * arg);



//+V13 TODO  #include "ma_timers.h"
#include "app_tuds.h"


void ma_adc_config(void);
    
    
void ma_uart_Init(void);
void ma_uart_Deinit(void);


int DEVT_uartRxReady(void);
int DEVT_uartTxEmpty(void);

    

bool ma_uart_packetTx_start(void);

extern char m_s1[256];

//-----------------------------------------------------------------------------
// ma_uart.c
void uart_thread_init(void);
void uart_thread_QueueSend(uniEvent_t *pEvt);

void set_curr_beUrx_toStartPosition(void);

//-----------------------------------------------------------------------------
// ma_thread.c
void core_thread_init(void);
void core_thread_QueueSend(uniEvent_t *pEvt);


//-----------------------------------------------------------------------------
// ma_constate.c

void pinWakeUp_Init(void);
void pinWakeUp_Assert(void);
void pinWakeUp_Release(void);
void pinWakeUp_Deinit(void);
bool pinWakeUp_IsAsserted(void);
 

extern uint8_t mg_5_dengen;        // system power 1:On ,  0:Off
extern uint8_t mg_6_advX50ms;      // advertising period x 50ms (1~255) -> 50ms ~ 12.75sec
extern uint8_t mg_7_wkUpPerX50ms;  // period for Wake Signal x 50ms (1~255) -> 50ms ~ 12.75sec
extern uint8_t mg_8_wkUpDelayX1ms; // delay to SOH for Wake Signal x 1ms (1~255) -> 1ms ~ 255ms
extern uint8_t mg_9_ADC_rate;      // do Battery power measurement  0:Off, every 1~255 seconds
extern uint8_t mg_10_loadADC;      // do Battery Loaded measurement 0:Off, every 1~255 minutes
extern int8_t  mg_11_power;        // set output power level (signed byte) e.g. -40, -30, -20, -16, -1, -8, -4, 0 , +4 dBm
extern uint8_t mg_12_9E00_rate;    // the rate to read (9E_00) and update Advertising Info (0-once only after reset, 0x01~0x3F:1~63minutes, 0x81~0xBF: 1~63seconds) 

extern uint32_t mg_6_advX50ms_inTicks;

//-----------------------------------------------------------------------------
void advertising_init_mg_new(uint32_t param_APP_ADV_INTERVAL); //mg_6_advX50ms_inTicks


extern uint8_t mg_ManufacturerSpecific_rsp26[26];
extern uint8_t mg_ShortenedName_rsp26[26 + 2]; // Added 2 bytes for two NULLS
void mg_ManufacturerSpecific_rsp26_setInitialValue(void);
void mg_ShortenedName_rsp26_set(char * device_name);


void gap_device_name_only_set(char * nextName);
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif // __MYAPP_H


