


#include "nrf_adc.h"

#include "myapp.h"

#include "ma_timers.h" // for APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE,   #define OSTIMER_WAIT_FOR_QUEUE               2                                          /**< Number of ticks to wait for the timer queue to be ready */
#include "ma_adc.h"


void sdoTE( char * S)
{
//    dbgPrint( S );
//    vTaskDelay( 42);
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
int ma_uart_timer_Reason;


//---------- UART timer ----------
APP_TIMER_DEF(ma_uart_timer_id);

static void Uart_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    uniEvent_t LEvt;
    //uint32_t err_code;    


    dbgPrint("\r\nUart_timeout_handler");
    if( ma_uart_timer_Reason == Reason_shutdown )
    {
        dbgPrint("\r\nShutdown Uart");
        ma_uart_Deinit();
    }
    if( ma_uart_timer_Reason == Reason_rxTimeout )
    {
        dbgPrint("\r\nUart Rx Timeout");
        LEvt.evtType = evt_xxxMaster_UartRxTimeout;
        core_thread_QueueSend(&LEvt);
    }
    ma_uart_timer_Reason = Reason_none;
        
}



uint32_t ma_uart_timer_init(void)
{
    uint32_t err_code;
    err_code = app_timer_create(&ma_uart_timer_id, APP_TIMER_MODE_SINGLE_SHOT, Uart_timeout_handler);  //APP_TIMER_MODE_REPEATED
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nUart: app_timer_create NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t ma_uart_timer_start(uint32_t timeout_ticks)
{
    uint32_t err_code;

//dbgPrint("\r\nma_uart_timer_start");

    // Start timer - Note: ignored if already started (freeRTOS)
    err_code = app_timer_stop(ma_uart_timer_id);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nUart: app_timer_stop NG");
    }
    err_code = app_timer_start(ma_uart_timer_id, timeout_ticks, NULL);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nUart: app_timer_start NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t ma_uart_timer_stop(void)
{
    uint32_t err_code;

//dbgPrint("\r\nma_uart_timer_stop");

    err_code = app_timer_stop(ma_uart_timer_id);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nUart: app_timer_stop NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


//---------- Holdoff timer ----------
APP_TIMER_DEF(ma_holdoff_timer_id);
extern void Holdoff_timeout_processing(void);

static void Holdoff_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
    Holdoff_timeout_processing();
}


uint32_t ma_holdoff_timer_init(void)
{
    uint32_t err_code;
    err_code = app_timer_create(&ma_holdoff_timer_id, APP_TIMER_MODE_SINGLE_SHOT, Holdoff_timeout_handler);  //APP_TIMER_MODE_REPEATED
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nHoldoff: app_timer_create NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t ma_holdoff_timer_start(uint32_t timeout_ticks)
{
    uint32_t err_code;

    dbgPrint("\r\nma_holdoff_timer_start");

    // Start timer - Note: ignored if already started (freeRTOS)
    err_code = app_timer_stop(ma_holdoff_timer_id);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nHoldoff: app_timer_stop NG");
    }
    err_code = app_timer_start(ma_holdoff_timer_id, timeout_ticks, NULL);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nHoldoff: app_timer_start NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t ma_holdoff_timer_stop(void)
{
    uint32_t err_code;

    dbgPrint("\r\nma_holdoff_timer_stop");

    err_code = app_timer_stop(ma_holdoff_timer_id);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nHoldoff: app_timer_stop NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void autoTimeout_onTick(void);

APP_TIMER_DEF(m_GP_timer_id);

static void GP_timeout_handler(void * p_context) // Effectively a 1 Second timer
{
    UNUSED_PARAMETER(p_context);
    uint32_t err_code;    

    dbgPrint(".");
    err_code = app_timer_start(m_GP_timer_id, APP_TIMER_TICKS(GP_TIMER_PERIOD_1000MS, APP_TIMER_PRESCALER), NULL);
    if (err_code != NRF_SUCCESS)
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }

    autoTimeout_onTick();

    if( g_PRE == true )
    {
        NADC_proc( NADC_action_ONE_SECOND);
        if( g_PRE == false )
        { 
            // _PRE_advertising
            err_code = ble_advertising_start(BLE_ADV_MODE_FAST);                  //$$ 10 !!!!
            dbgPrint("\r\n[6]");
            APP_ERROR_CHECK(err_code);
            dbgPrint("\r\n[14]");
        }
    }
    else
    {    
        NADC_proc( NADC_action_ONE_SECOND);
        blp_proc(BLP_PROC_TIMER_TICK);
        bln_proc(BLN_PROC_TIMER_TICK);
    }        
}


//static uint32_t m_app_ticks_per_100ms;
#define BSP_MS_TO_TICK(MS) (m_app_ticks_per_100ms * (MS / 100))

uint32_t GP_timer_init(void)
{
    uint32_t err_code;

    // Create timer
    err_code = app_timer_create(&m_GP_timer_id, APP_TIMER_MODE_SINGLE_SHOT, GP_timeout_handler);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nGP: app_timer_create NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}


uint32_t GP_timer_start(uint32_t timeout_ticks)
{
    uint32_t err_code;

    err_code = app_timer_stop(m_GP_timer_id);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nGP: app_timer_stop NG");
    }
    // Start timer - Note: ignored if already started (freeRTOS)
    //err_code = app_timer_start(m_GP_timer_id, BSP_MS_TO_TICK(timeout_ticks), NULL);
    err_code = app_timer_start(m_GP_timer_id, timeout_ticks, NULL);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nGP: app_timer_start NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t GP_timer_stop(void)
{
    uint32_t err_code;
    // Stop timer
    err_code = app_timer_stop(m_GP_timer_id);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nGP: app_timer_stop NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void autoTimeout_onTick(void);
#if USE_ADCON_TIMER
APP_TIMER_DEF(m_battLoad_timer_id);

static void battLoad_timeout_handler(void * p_context) // Aim for around 1ms
{
    UNUSED_PARAMETER(p_context);
    uint32_t err_code;    

    dbgPrint("$");

    if( g_PRE == true )
    {
        NADC_proc(NADC_action_WIDTH_TIMER);
        if( g_PRE == false )
        { 
            // _PRE_advertising
            err_code = ble_advertising_start(BLE_ADV_MODE_FAST);                  //$$ 10 !!!!
            dbgPrint("\r\n[6]");
            APP_ERROR_CHECK(err_code);
            dbgPrint("\r\n[14]");
        }
    }
    else
    {    
        NADC_proc(NADC_action_WIDTH_TIMER);
    }        
}


//static uint32_t m_app_ticks_per_100ms;
//#define BSP_MS_TO_TICK(MS) (m_app_ticks_per_100ms * (MS / 100))

uint32_t battLoad_timer_init(void)
{
    uint32_t err_code;

    // Create timer
    err_code = app_timer_create(&m_battLoad_timer_id, APP_TIMER_MODE_SINGLE_SHOT, battLoad_timeout_handler);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nbattLoad: app_timer_create NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}


uint32_t battLoad_timer_start(uint32_t timeout_ticks)
{
    uint32_t err_code;

    //err_code = app_timer_stop(m_battLoad_timer_id);
    //if( err_code != NRF_SUCCESS )
    //{
    //    sdoTE("\r\nbattLoad: app_timer_stop NG");
    //}
    // Start timer - Note: ignored if already started (freeRTOS)
    //err_code = app_timer_start(m_battLoad_timer_id, BSP_MS_TO_TICK(timeout_ticks), NULL);
    err_code = app_timer_start(m_battLoad_timer_id, timeout_ticks, NULL);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nbattLoad: app_timer_start NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t battLoad_timer_stop(void)
{
    uint32_t err_code;
    // Stop timer
    err_code = app_timer_stop(m_battLoad_timer_id);
    if( err_code != NRF_SUCCESS )
    {
        sdoTE("\r\nbattLoad: app_timer_stop NG");
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}
#endif

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
#if 0
#include "app_timer_appsh.h"
#include "app_scheduler.h"
#include "priority_scheduler.h"

uniEvent_t uniEvt_a1;
uniEvent_t uniEvt_a2;
typedef void (*uniEvent_handler_t)(void * p_context);

typedef struct
{
    uniEvent_handler_t   uniEvent_handler;
    void *               p_context;
} uniEvent_event_t;

static void uniEvent_evt_get(void * p_event_data, uint16_t event_size)
{
    uniEvent_event_t * p_Uni_event = (uniEvent_event_t *)p_event_data;
    
    APP_ERROR_CHECK_BOOL(event_size == sizeof(app_timer_event_t));
    p_Uni_event->uniEvent_handler(p_Uni_event->p_context);
}

uint32_t uniEvent_evt_schedule(uniEvent_handler_t uniEvent_handler,
                               void *             p_context)
{
    uniEvent_event_t Uni_event;

    Uni_event.uniEvent_handler = uniEvent_handler;
    Uni_event.p_context        = p_context;
    
    return app_sched_event_put(&Uni_event, sizeof(Uni_event), uniEvent_evt_get);
}

static void uniEvent_handler_A(void * p_context)
{
    dbgPrint("\r\nuniEvent_handler : ");

    uniEvent_t *pE = (uniEvent_t *)p_context ;
    dbgPrintf("\r\nEvent i = %d", pE->i );
}



static void on_uniEvent_Thread_X(void * p_event_data, uint16_t event_size)
{
    dbgPrint("\r\nThread_X -> ");

    uniEvent_t *pE = (uniEvent_t *)p_event_data ;
    dbgPrintf("\r\nEvent i = %d", pE->i );
}

static void on_uniEvent_Thread_Y(void * p_event_data, uint16_t event_size)
{
    dbgPrint("\r\nThread_Y -> ");

    uniEvent_t *pE = (uniEvent_t *)p_event_data ;
    dbgPrintf("\r\nEvent i = %d", pE->i );
}


static void on_uniEvent_PriorityThread(void * p_event_data, uint16_t event_size)
{
    dbgPrint("\r\nPriorityThread -> ");

    uniEvent_t *pE = (uniEvent_t *)p_event_data ;
    dbgPrintf("\r\nEvent i = %d", pE->i );
}
#endif

//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
#if 0
#include "app_timer_appsh.h"
#include "app_scheduler.h"

static void app_bogusT_evt_get(void * p_event_data, uint16_t event_size)
{
    app_timer_event_t * p_timer_event = (app_timer_event_t *)p_event_data;
    
    APP_ERROR_CHECK_BOOL(event_size == sizeof(app_timer_event_t));
    p_timer_event->timeout_handler(p_timer_event->p_context);
}

uint32_t app_bogusT_evt_schedule(app_timer_timeout_handler_t timeout_handler,
                                 void *                      p_context)
{
    app_timer_event_t timer_event;

    timer_event.timeout_handler = timeout_handler;
    timer_event.p_context       = p_context;
    
    return app_sched_event_put(&timer_event, sizeof(timer_event), app_bogusT_evt_get);
}

static void bogusT_timeout_handler(void * p_context)
{
    dbgPrint("\r\nbogusT_handler");
}
#endif




