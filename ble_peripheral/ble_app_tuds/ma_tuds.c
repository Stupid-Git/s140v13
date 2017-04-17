
#include "ble.h" //ble_evt_t


#include "ble_tuds.h"
#include "ma_tuds.h"


ble_tuds_t                                  m_tuds;                                  // Structure to identify the BLKUP Service.



int32_t blk_dn_start( uint8_t *pkt );
int32_t blk_dn_add( uint8_t *pkt, uint16_t len );
int32_t blk_dn_chk(void);

#define USE_UP_STUFF 1
#include "block_proc.h"

static void tuds_Dcmd_handler(ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length)
{
    BlkDn_On_Dcmd(p_data, length);
}

static void tuds_Ddat_handler(ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length)
{
    BlkDn_On_Ddat(p_data, length);
}

static void tuds_Dcfm_tx_complete_handler(ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt)
{
    BlkDn_On_written_Dcfm(p_tuds, 0, 0);

    BlkUp_On_written_Ucmd(p_tuds, 0, 0);
    BlkUp_On_written_Udat(p_tuds, 0, 0);
}


// UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP
static void tuds_Ucfm_handler(ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length)
{
    BlkUp_On_Ucfm(p_data, length);
}






//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//
//     PUBLIC PUBLIC PUBLIC PUBLIC PUBLIC PUBLIC PUBLIC PUBLIC PUBLIC
//
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
app_tuds_t m_app_tuds;


void app_tuds_Init(void);
void app_tuds_on_ble_evt(ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt);


void tuds_service_init()
{
    uint32_t                   err_code;

    ble_tuds_init_t   tuds_init;    
    memset(&tuds_init, 0, sizeof(tuds_init));

    tuds_init.Ddat_handler = tuds_Ddat_handler;
    tuds_init.Dcmd_handler = tuds_Dcmd_handler;
    tuds_init.Dcfm_tx_complete_handler = tuds_Dcfm_tx_complete_handler;

    tuds_init.Ucfm_handler = tuds_Ucfm_handler;
    
    err_code = ble_tuds_init(&m_tuds, &tuds_init);

    //rn_BlkDn_timer_Init();//APP_TIMER_TICKS(100, APP_TIMER_PRESCALER));
    //rn_BlkUp_timer_Init();//APP_TIMER_TICKS(100, APP_TIMER_PRESCALER));
    
    //APP_ERROR_CHECK(err_code);
}

void ma_tuds_init(void)
{
    uint32_t                   err_code;

    app_tuds_init_t   ma_tuds_init;    
    memset(&ma_tuds_init, 0, sizeof(ma_tuds_init));

    tuds_service_init();
    
    //ma_tuds_init.p_tuds = &m_tuds;
    ma_tuds_init.packet_handler = 0;
    
    err_code = app_tuds_init(&m_app_tuds, &ma_tuds_init);

    //APP_ERROR_CHECK(err_code);
}

void ma_tuds_on_ble_evt(ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt)
{
    ble_tuds_on_ble_evt(p_tuds, p_ble_evt); // ble_tuds_on_ble_evt(&m_tuds, p_ble_evt);
}

void ma_tuds_set_packetHandler( app_tuds_t *p_app_tuds, app_tuds_packet_handler_t app_tuds_ph )
{
    p_app_tuds->packet_handler = app_tuds_ph;
}
    


void appcode_tuds_packet_handler (app_tuds_t * p_ma_tuds, uint8_t * p_data, uint16_t length)
{
}

void appcode1()
{
    ma_tuds_set_packetHandler(&m_app_tuds, appcode_tuds_packet_handler );
}










#include "myapp.h"
//#include "app_tuds.h"






void tuds_event_handle(app_tuds_evt_t * p_event)
{
    static uniEvent_t LEvt;

    switch (p_event->evt_type)
    {
        case APP_TUDS_RX_START_PKT_0:
            app_tuds_send_noBufferBusy(m_app_tuds);
            break;
        
        case APP_TUDS_RX_DONE_PKT_0:
            // TODO PKT_0 goes to UART
            LEvt.evtType = evt_GHJGJHGHGH_trigger;
            //LEvt.evtType = evt_Uart_RxReady;
            //uart_thread_QueueSend(&LEvt); // ..._QueueSendFromISR( ... )
        
        
            break;

        case APP_TUDS_TX_DONE:
            //LEvt.evtType = evt_Uart_TxEmpty;
            //uart_thread_QueueSend(&LEvt); // ..._QueueSendFromISR( ... )
            break;
        
        default:
            break;
    }    
}


void ma_app_tuds_Init(void)
{
    m_app_tuds.packet_handler = tuds_event_handle;
    m_app_tuds.p_ble_tuds = m_tuds;
    
}

void ma_app_tuds_Deinit() //karel
{
    //sdo_AppendText("***** ma_uart_Deinit *****\r\n");

	//app_tuds_close();
}


app_tuds_event_handler_t    m_event_handler;

uint32_t app_tuds_init(//const app_uart_comm_params_t * p_comm_params,
                       //      app_uart_buffers_t *     p_buffers,
                             app_tuds_event_handler_t event_handler
                       //      app_irq_priority_t       irq_priority
                      )
{
    uint32_t err_code;

    m_event_handler = event_handler;

    return 0;
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


void timers_init_tuds_part()
{
#if USE_TUDS_U
#endif
}

void services_init_tuds_part()
{
#if USE_TUDS_U
#endif
    uint32_t                   err_code;

    //--------------------------------------------------------
    ble_tuds_init_t   tuds_init;    
    memset(&tuds_init, 0, sizeof(tuds_init));

    tuds_init.Ddat_handler = tuds_Ddat_handler;
    tuds_init.Dcmd_handler = tuds_Dcmd_handler;
    tuds_init.Dcfm_tx_complete_handler = tuds_Dcfm_tx_complete_handler;

    tuds_init.Ucfm_handler = tuds_Ucfm_handler;
    
    err_code = ble_tuds_init(&m_tuds, &tuds_init);

    //rn_BlkDn_timer_Init();//APP_TIMER_TICKS(100, APP_TIMER_PRESCALER));
    //rn_BlkUp_timer_Init();//APP_TIMER_TICKS(100, APP_TIMER_PRESCALER));
    APP_ERROR_CHECK(err_code);


    //--------------------------------------------------------
    m_event_handler = main_app_tuds_event_handler;
    
    app_tuds_init_t   _tuds_init;    
    memset(&_tuds_init, 0, sizeof(_tuds_init));

    _tuds_init.packet_handler = main_app_tuds_packet_handler;
    _tuds_init.event_handler = main_app_tuds_event_handler;
    _tuds_init.p_ble_tuds = &m_ble_tuds;

    err_code = app_tuds_init( &m_app_tuds, &_tuds_init);    
    APP_ERROR_CHECK(err_code);
    
}

static void application_timers_start_tuds_part(void)
{
#if USE_TUDS_U
#endif
}
