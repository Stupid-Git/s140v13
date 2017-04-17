
#include "myapp.h"


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Buffers
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static uint8_t  buffer_UCm[1024];
static uint8_t  buffer_CUm[1024];

static uint8_t  buffer_UB[2048 + 7 + 64];
static uint8_t  buffer_BU[1024 + 256]; //2016/11/15 added 256 to this buffer

static uint8_t  buffer_Urx[128];
static uint8_t  buffer_Utx[128];

be_t            be_UCm;
be_t            be_CUm;

volatile be_t   be_UB;
be_t            be_BU;

be_t            be_Urx;
be_t            be_Utx;

be_t           *m_curr_beUtx;
volatile be_t  *m_curr_beUrx;

//-----------------------------------------------------------------------------
static void be_Init_be(volatile be_t *be, uint8_t * buffer, uint32_t capacity)
{
    be->buffer = buffer;
    be->capacity = capacity;
    be->length = 0;
    be->c0 = 0;
    be->c1 = 0;
    be->pktType = ePkt_Unknown;
}

static void be_Init()
{
    be_Init_be(&be_UCm, buffer_UCm,sizeof(buffer_UCm));
    be_Init_be(&be_CUm, buffer_CUm,sizeof(buffer_CUm));
    
    be_Init_be(&be_UB, buffer_UB,sizeof(buffer_UB));
    be_Init_be(&be_BU, buffer_BU,sizeof(buffer_BU));

    be_Init_be(&be_Urx, buffer_Urx,sizeof(buffer_Urx));
    be_Init_be(&be_Utx, buffer_Utx,sizeof(buffer_Utx));
    m_curr_beUrx = &be_Urx;
    m_curr_beUtx = &be_Utx;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  processing blocks
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
typedef struct coreProcBlock_s
{
    bool pending;
    bool processing;
    be_t *p_be;
    int (*make_req)( be_t *be_Req );
    int (*proc_rsp)( be_t *be_Req,  be_t *be_Rsp );
    int (*proc_timeout)( be_t *be_Req,  be_t *be_Rsp );
    
} coreProcBlock_t;



//.........................................................
coreProcBlock_t cpb_BLE;
extern int make_req_BLE( be_t *be_Req );
extern int proc_rsp_BLE( be_t *be_Req,  be_t *be_Rsp );
extern int proc_timeout_BLE( be_t *be_Req,  be_t *be_Rsp );

//.........................................................
coreProcBlock_t cpb_NANNY;
extern int make_req_NANNY( be_t *be_Req );
extern int proc_rsp_NANNY( be_t *be_Req,  be_t *be_Rsp );
extern int proc_timeout_NANNY( be_t *be_Req,  be_t *be_Rsp );

//.........................................................
coreProcBlock_t cpb_ADC;
extern int make_req_ADC( be_t *be_Req );
extern int proc_rsp_ADC( be_t *be_Req,  be_t *be_Rsp );
extern int proc_timeout_ADC( be_t *be_Req,  be_t *be_Rsp );

//.........................................................
coreProcBlock_t cpb_BLN;
extern int make_req_BLN( be_t *be_Req );
extern int proc_rsp_BLN( be_t *be_Req,  be_t *be_Rsp );
extern int proc_timeout_BLN( be_t *be_Req,  be_t *be_Rsp );

//.........................................................
coreProcBlock_t cpb_BLP;
extern int make_req_BLP( be_t *be_Req );
extern int proc_rsp_BLP( be_t *be_Req,  be_t *be_Rsp );
extern int proc_timeout_BLP( be_t *be_Req,  be_t *be_Rsp );






//=============================================================================
//=============================================================================
//=============================================================================

//-----------------------------------------------------------------------------
//===== Indicate_BufSend_Started =====
//
void be_BU_Indicate_BufSend_Started() { }
void be_CUm_Indicate_BufSend_Started() { }

void be_Cxx_Indicate_BufSend_Started()
{
    // m_curr_beUtx->vIndicateUse_Start(m_curr_beUtx);
    if( m_curr_beUtx == &be_CUm ) be_CUm_Indicate_BufSend_Started();
    if( m_curr_beUtx == &be_BU  ) be_BU_Indicate_BufSend_Started();
}



//-----------------------------------------------------------------------------
//===== Indicate_BufSend_Done =====
//
void be_BU_Indicate_BufSend_Done()
{
    /*TODO
    uniEvent_t LEvt;
    if( EntityUartTx.owner != 0)
    {
        if( EntityUartTx.owner == &OwnerBLE)
        {                
            LEvt.evtType = evt_bleMaster_UartTxDone;
            core_thread_QueueSend(&LEvt);
        }
    }
    */
}

void be_CUm_Indicate_BufSend_Done()
{
    /*TODO
    uniEvent_t LEvt;

    if( EntityUartTx.owner != 0)
    {
        if( EntityUartTx.owner == &OwnerCoreM)
        {                
            LEvt.evtType = evt_coreMaster_UartTxDone;
            core_thread_QueueSend(&LEvt);
        }
    }
    */
}

void be_Cxx_Indicate_BufSend_Done()
{
    if( m_curr_beUtx == &be_CUm ) be_CUm_Indicate_BufSend_Done();
    if( m_curr_beUtx == &be_BU  ) be_BU_Indicate_BufSend_Done();
}

//-----------------------------------------------------------------------------
//===== IndicateStartRecv =====
//
void be_UCm_IndicateStartRecv(void)
{
}
void be_UB_IndicateStartRecv(void)
{
}

//-----------------------------------------------------------------------------
//===== IndicateDoneRecv =====
//
void be_UCm_IndicateDoneRecv(void)
{
    uniEvent_t LEvt;
    LEvt.evtType = evt_coreMaster_UartRxDone;
    core_thread_QueueSend(&LEvt); // ..._QueueSendFromISR( ... )
}
void be_UB_IndicateDoneRecv(void)
{
    uniEvent_t LEvt;
    LEvt.evtType = evt_bleMaster_UartRxDone;
    core_thread_QueueSend(&LEvt); // ..._QueueSendFromISR( ... )
}

//void be_Cxx_IndicateDoneRecv(void)
//{
//}






//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void P7_timerEvent(void); // drive WakePin_Release

//---------- P7 timer ----------
APP_TIMER_DEF(m_P7_timer_id);

static void P7_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    //dbgPrint("\r\nP7_timer_handler");
    P7_timerEvent(); // drive WakePin_Release
}


uint32_t P7_timer_init(void)
{
    uint32_t err_code;

    // Create timers.
    err_code = app_timer_create(&m_P7_timer_id, APP_TIMER_MODE_SINGLE_SHOT, P7_timeout_handler);
    APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t P7_timer_start(uint32_t timeout_ticks)
{
    uint32_t err_code;
    // Start timer - Note: ignored if already started (freeRTOS)
    err_code = app_timer_start(m_P7_timer_id, timeout_ticks, NULL);
    APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t P7_timer_stop(void)
{
    uint32_t err_code;
    // Stop timer
    err_code = app_timer_stop(m_P7_timer_id);
    APP_ERROR_CHECK(err_code);
    return(err_code);
}


void P8TimerEvent(void);

//---------- P8 timer ----------
APP_TIMER_DEF(m_P8_timer_id);

static void P8_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    P8TimerEvent();
}


uint32_t P8_timer_init(void)
{
    uint32_t err_code;

    // Create timers.
    err_code = app_timer_create(&m_P8_timer_id, APP_TIMER_MODE_SINGLE_SHOT, P8_timeout_handler);
    APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t P8_timer_start(uint32_t timeout_ticks)
{
    uint32_t err_code;
    // Start timer - Note: ignored if already started (freeRTOS)
    err_code = app_timer_start(m_P8_timer_id, timeout_ticks, NULL);
    APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t P8_timer_stop(void)
{
    uint32_t err_code;
    // Stop timer
    err_code = app_timer_stop(m_P8_timer_id);
    APP_ERROR_CHECK(err_code);
    return(err_code);
}

//-----------------------------------------------------------------------------
extern bool BLN_boot;
bool BLN_boot = true;



//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//-----------------------------------------------------------------------------
typedef struct SSQ_item_s
{
    //char name[42];
    coreProcBlock_t* p_cpb;
} SSQ_item_t;

#define SSQ_LENGTH 10
SSQ_item_t SSQ[SSQ_LENGTH];

//-----------------------------------------------------------------------------
uint8_t SSQ_len()
{
    uint8_t count = 0;
    int i;
    for( i=0 ; i<SSQ_LENGTH; i++)
    {
        if( SSQ[i].p_cpb != 0 )
            count++;
    }
    return(count);
}


//-----------------------------------------------------------------------------
void  SSQ_Item_Init( SSQ_item_t *pI )
{
    //pI->name[0] = 0;
    pI->p_cpb = 0;
}

//-----------------------------------------------------------------------------
void SSQ_Init()
{
    int i;
    for( i=0 ; i<SSQ_LENGTH; i++)
    {
        SSQ_Item_Init( &SSQ[i] );
    }
}

//-----------------------------------------------------------------------------
void SSQ_AddCpb(coreProcBlock_t *p_cpb)
{
    int i;
    for( i=0 ; i<SSQ_LENGTH; i++)
    {    
        if(  (p_cpb == &cpb_BLE) && (SSQ[i].p_cpb == &cpb_BLE) ) // can't add twice
            break;
        if(  (p_cpb == &cpb_NANNY) && (SSQ[i].p_cpb == &cpb_NANNY) ) // can't add twice
            break;
        if(  (p_cpb == &cpb_ADC) && (SSQ[i].p_cpb == &cpb_ADC) ) // can't add twice
            break;
        if(  (p_cpb == &cpb_BLN) && (SSQ[i].p_cpb == &cpb_BLN) ) // can't add twice
            break;
        if(  (p_cpb == &cpb_BLP) && (SSQ[i].p_cpb == &cpb_BLP) ) // can't add twice
            break;
            
        if( SSQ[i].p_cpb == 0 )
        {
            SSQ[i].p_cpb = p_cpb;
            break;
        }
    }
}


//-----------------------------------------------------------------------------
void SSQ_Pop()
{
    int i;
    for( i=0 ; i<SSQ_LENGTH - 1; i++)
    {    
        SSQ[i] = SSQ[i+1];
    }
    SSQ_Item_Init( &SSQ[i] );
}



//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//-----------------------------------------------------------------------------
void WakePin_Assert(void)
{  
//NANNY pinWakeUp_Init();
    pinWakeUp_Init();
    pinWakeUp_Assert();

    dbgPrint("\r\n WakePin_Assert");
}

//-----------------------------------------------------------------------------
void WakePin_Release(char *S) //void WakePin_Release(void)
{
    pinWakeUp_Release();
    dbgPrint(S);
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//-----------------------------------------------------------------------------
typedef enum eSS
{
    SS_WAIT_STARTUP = 0,
    SS1_WAIT_P8 = 1,
    SS2_READY_FOR_XFER = 2,
    SS3_TXRX_BUSY = 3,
    SS4_TXRX_HOLDOFF = 4,
    SS4_TXRX_DONE = 5,
}eSS_t;
eSS_t SS = SS_WAIT_STARTUP;

//-----------------------------------------------------------------------------
void P7_timerEvent() //  P7_timer_start => ... drive WakePin_Release, 
{
    uniEvent_t LEvt;

    WakePin_Release("\r\n WakePin_Release - P7_timerEvent"); // WakePin_Release(); // Drive pin high(inactive)
    
    LEvt.evtType = evt_SSQ_Check_Queue;
    core_thread_QueueSend( &LEvt );    
}

//-----------------------------------------------------------------------------
void P8TimerEvent()//  P8_timer_start => ... process SSQ again
{
    uniEvent_t LEvt;

    SS = SS2_READY_FOR_XFER;
    LEvt.evtType = evt_SSQ_Check_Queue;
    core_thread_QueueSend( &LEvt );    
}




//-----------------------------------------------------------------------------
static uint32_t  autoTimeout_max = AUTOTIMEOUT_NONE;
static uint32_t  autoTimeout_count = 0;
void autoTimeout_Start( uint32_t timeout )
{
    autoTimeout_max = timeout;
    autoTimeout_count = 0;
}

//-----------------------------------------------------------------------------
extern uint16_t m_conn_handle; // = BLE_CONN_HANDLE_INVALID
void autoTimeout_onTick()
{
    if( autoTimeout_max == AUTOTIMEOUT_NONE)
        return;
    
    if( m_conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        autoTimeout_max = AUTOTIMEOUT_NONE;
    }
    else
    {
        autoTimeout_count++;
        if( autoTimeout_count >= autoTimeout_max)
        {
            autoTimeout_count = 0;

            sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            //uint32_t err_code;
            //err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            //if (err_code != NRF_SUCCESS)
            //{                 
            //}
         }
    }
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//-----------------------------------------------------------------------------
static void SS_thread_process_event(uniEvent_t *pEvt)
{
    uniEvent_t LEvt;

    //dbgPrintf("####  pEvt->evtType = %d\r\n", pEvt->evtType);

    
    switch( pEvt->evtType )
    {
    //-----------------------------------------------------
    //----- ConState -----
    case evt_ConState_Connected:
        
        autoTimeout_Start( AUTOTIMEOUT_STDTIME );
        m_bleIsConnected = true;
        if( m_bleIsConnected == true)
        {
        }
        break;

    case evt_ConState_Disconnected:

        autoTimeout_Start( AUTOTIMEOUT_NONE );
        m_bleIsConnected = false;    
        if( m_bleIsConnected == false )
        {
			blp_proc( BLP_PROC_FORCE_TRIGGER );
        }
        break;
        
    //-----------------------------------------------------
    //----- Triggers ----- bleMaster -----
    case evt_bleMaster_trigger: //dbgPrint("\r\nevt_bleMaster_trigger");

        //----- if operation is already pending, then fall out -----
        if( cpb_BLE.pending == true) 
            break; // no next event set?!?!
        
        cpb_BLE.pending = true;
        cpb_BLE.processing = false;
        cpb_BLE.p_be = &be_BU;
        cpb_BLE.make_req = make_req_BLE;
        cpb_BLE.proc_rsp = proc_rsp_BLE;
        cpb_BLE.proc_timeout = proc_timeout_BLE;

        SSQ_AddCpb(&cpb_BLE);
        LEvt.evtType = evt_SSQ_Check_Queue;
        core_thread_QueueSend( &LEvt );
        break;
                


    //----- Triggers ----- core_NANNY -----
    case evt_core_NANNY_trigger: //dbgPrint("\r\nevt_core_NANNY_trigger");

        //----- if operation is already pending, then fall out -----
        if( cpb_NANNY.pending == true) 
            break; // no next event set?!?!
        
        cpb_NANNY.pending = true;
        cpb_NANNY.processing = false;
        cpb_NANNY.p_be = &be_CUm;
        cpb_NANNY.make_req = make_req_NANNY;
        cpb_NANNY.proc_rsp = proc_rsp_NANNY;
        cpb_NANNY.proc_timeout = proc_timeout_NANNY;

        SSQ_AddCpb(&cpb_NANNY);
        LEvt.evtType = evt_SSQ_Check_Queue;
        core_thread_QueueSend( &LEvt );
        break;

    //----- Triggers ----- core_ADC -----
    case evt_core_ADC_trigger: //dbgPrint("\r\nevt_core_ADC_trigger");

        //----- if operation is already pending, then fall out -----
        if( cpb_ADC.pending == true) 
            break; // no next event set?!?!
        
        cpb_ADC.pending = true;
        cpb_ADC.processing = false;
        cpb_ADC.p_be = &be_CUm;
        cpb_ADC.make_req = make_req_ADC;
        cpb_ADC.proc_rsp = proc_rsp_ADC;
        cpb_ADC.proc_timeout = proc_timeout_ADC;

        SSQ_AddCpb(&cpb_ADC);
        LEvt.evtType = evt_SSQ_Check_Queue;
        core_thread_QueueSend( &LEvt );
        break;
                

    //----- Triggers ----- core_BLN -----
    case evt_core_BLN_trigger: //dbgPrint("\r\nevt_core_BLN_trigger");

        //----- if operation is already pending, then fall out -----
        if( cpb_BLN.pending == true) 
            break;

        cpb_BLN.pending = true;
        cpb_BLN.processing = false;
        cpb_BLN.p_be = &be_CUm;
        cpb_BLN.make_req = make_req_BLN;
        cpb_BLN.proc_rsp = proc_rsp_BLN;
        cpb_BLN.proc_timeout = proc_timeout_BLN;
        
        SSQ_AddCpb(&cpb_BLN);
        LEvt.evtType = evt_SSQ_Check_Queue;
        core_thread_QueueSend( &LEvt );

        break;

    case evt_core_BLP_trigger: //dbgPrint("\r\nevt_core_BLP_trigger");

        //----- if operation is already pending, then fall out -----
        if( cpb_BLP.pending == true) 
            break;

        cpb_BLP.pending = true;
        cpb_BLP.processing = false;
        cpb_BLP.p_be = &be_CUm;
        cpb_BLP.make_req = make_req_BLP;
        cpb_BLP.proc_rsp = proc_rsp_BLP;
        cpb_BLP.proc_timeout = proc_timeout_BLP;
        

        SSQ_AddCpb(&cpb_BLP);
        LEvt.evtType = evt_SSQ_Check_Queue;
        core_thread_QueueSend( &LEvt );

        break;
        


    //-----------------------------------------------------------------
    //-----------------------------------------------------------------
    case evt_xxxMaster_UartRxTimeout:

        dbgPrintf("\r\n");
        dbgPrintf("\r\nUart Timeout");
        dbgPrintf("\r\n    Uart Timeout");
        dbgPrintf("\r\n        Uart Timeout");
        dbgPrintf("\r\n");
        ma_uart_timer_stop(); // ? timer should be stopped if we get here!  called from evt_xxxMaster_UartRxTimeout:
    
        //..................................
        if( (SSQ[0].p_cpb == &cpb_ADC    ) ||
            (SSQ[0].p_cpb == &cpb_NANNY    ) ||
            (SSQ[0].p_cpb == &cpb_BLN    ) ||
            (SSQ[0].p_cpb == &cpb_BLP ) )
        {
            if( (SSQ[0].p_cpb->pending == true) && (SSQ[0].p_cpb->processing == true) )
            { 
                SSQ[0].p_cpb->proc_timeout( &be_CUm, (be_t*)&be_UCm );
                SSQ[0].p_cpb->pending = false;
                SSQ[0].p_cpb->processing = false;
            }
        }

        //..................................
        //..................................
        if( SSQ[0].p_cpb == &cpb_BLE )
        {
#if 1 //NORMAL
            if( (SSQ[0].p_cpb->pending == true) && (SSQ[0].p_cpb->processing == true) )
            { 
                SSQ[0].p_cpb->proc_timeout( &be_BU, (be_t*)&be_UB ); // added (be_t*) is this OK?
                SSQ[0].p_cpb->pending = false;
                SSQ[0].p_cpb->processing = false;
            }
#else //This is For Debug

            set_curr_beUrx_toStartPosition();

            dbgPrintf("TO:  UB_len = %d \r\n", be_UB.length);
            dbgPrintf("TO:  UB[0..3] = %02x %02x %02x %02x \r\n", be_UB.buffer[0], be_UB.buffer[1], be_UB.buffer[2], be_UB.buffer[3]);
            dbgPrintf("TO:  UB[4..7] = %02x %02x %02x %02x \r\n", be_UB.buffer[4], be_UB.buffer[5], be_UB.buffer[6], be_UB.buffer[7]);
/*
            be_UB.buffer[0] = 0x01;
            be_UB.buffer[1] = 0x9F;
            be_UB.buffer[2] = 0x15;
            be_UB.buffer[3] = 0x00;
            be_UB.buffer[4] = 0x00;
            be_UB.buffer[5] = 0xB5;
            be_UB.buffer[6] = 0x00;
            be_UB.rdPtr = 0;
            be_UB.wrPtr = 7;
            be_UB.length = 7;            
*/
            
/**/
            be_UB.buffer[0] = 0x01;
            be_UB.buffer[1] = 0x9F;
            be_UB.buffer[2] = 0x15;
            be_UB.buffer[3] = 0x01;
            be_UB.buffer[4] = 0x00;
            be_UB.buffer[5] = 0x00;
            be_UB.buffer[6] = 0xB6;
            be_UB.buffer[7] = 0x00;
            be_UB.rdPtr = 0;
            be_UB.wrPtr = 8;
            be_UB.length = 8;            
/**/
            //bleMaster_process_rsp();  //TODO  BlkUp_Go( m_curr_beUrx->buffer, m_curr_beUrx->length); //be_UB            
            if( (cpb_BLE.pending == true) && (cpb_BLE.processing == true) )
            {
                cpb_BLE.proc_rsp( &be_BU, &be_UB ); //proc_rsp_BLE
                cpb_BLE.pending = false;
                cpb_BLE.processing = false;
            }
#endif // for debug
        }
        
        // step 1 Queue Done
        LEvt.evtType = evt_SSQ_TxRxOrTimeout_Done; // After Timeout
        core_thread_QueueSend(&LEvt);


        break;


     //-----------------------------------------------------------------
    //----- coreMaster Response Processing -----
    case evt_coreMaster_UartRxDone:
    
        ma_uart_timer_stop(); //called from     case evt_coreMaster_UartRxDone
        
        //TODO - sunabaInPlace0x010x9fPacketRspExtraction( &be_UB );

        if( SSQ[0].p_cpb == &cpb_BLE ) {}
        if( SSQ[0].p_cpb == &cpb_BLN ) {}
        if( SSQ[0].p_cpb == &cpb_BLP ) {}
        if( SSQ[0].p_cpb == &cpb_ADC ) {}
        if( SSQ[0].p_cpb == &cpb_NANNY ) {}

        if( (SSQ[0].p_cpb->pending == true) && (SSQ[0].p_cpb->processing == true) )
        {
            SSQ[0].p_cpb->proc_rsp( &be_CUm, &be_UCm ); //proc_rsp_ADC
            SSQ[0].p_cpb->pending = false;
            SSQ[0].p_cpb->processing = false;
        }

        // step 1 Queue Done
        LEvt.evtType = evt_SSQ_TxRxOrTimeout_Done; // After Core Rx Done OK
        core_thread_QueueSend(&LEvt);

        // step 2 then start next
        // If the last command was BLP, then do a BLN in case the name has changed
        if( SSQ[0].p_cpb == &cpb_BLP )
        {
            //if( cpb_BLN.make_req == make_req_BLN )  this if makes no sense, because cpb_BLN.make_req is set during "evt_core_BLN_trigger" handler
            {
                LEvt.evtType = evt_core_BLN_trigger;
                core_thread_QueueSend(&LEvt);
            }
        }

        break;
            
            
            
    //-----------------------------------------------------------------
    //----- bleMaster Response Processing -----
    case evt_bleMaster_UartRxDone: //dbgPrint("\r\nevt_bleMaster_UartRxDone");
    
        ma_uart_timer_stop(); //called from     case evt_bleMaster_UartRxDone
    
        if( SSQ[0].p_cpb == &cpb_BLE ) {}
    
        if( (cpb_BLE.pending == true) && (cpb_BLE.processing == true) )
        {
            cpb_BLE.proc_rsp( &be_BU, (be_t*)&be_UB ); //proc_rsp_BLE  // added (be_t*) is this OK?
            cpb_BLE.pending = false;
            cpb_BLE.processing = false;
        }

        LEvt.evtType = evt_SSQ_TxRxOrTimeout_Done; // After Ble Rx Done OK
        core_thread_QueueSend(&LEvt);


        break;
        
    default:
        //dbgPrintf("SS_thread_process_event Event = %d not handledt\r\n", pEvt->evtType);
        break;
    }
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

//-----------------------------------------------------------------------------
void Holdoff_timeout_processing()
{
    uniEvent_t LEvt;
    SS = SS4_TXRX_DONE;
    LEvt.evtType = evt_SSQ_Check_Queue;
    core_thread_QueueSend(&LEvt);      
}


//-----------------------------------------------------------------------------
static uint32_t P7_ticks = 0;        // from  mg_7_wkUpPerX50ms 1~255 * 50ms LOW TIME FOR WAKEUP SIGNAL, 0 = WHOLE ACCESS PERIOD
static uint32_t P8_ticks = 100 * 33; // from  mg_8_wkUpDelayX1ms
void Update_P7_P8_ticks_from_mg7_mg8(void)
{
    P8_ticks = APP_TIMER_TICKS( mg_8_wkUpDelayX1ms, APP_TIMER_PRESCALER);
    P7_ticks = APP_TIMER_TICKS( mg_7_wkUpPerX50ms * 50, APP_TIMER_PRESCALER);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//-----------------------------------------------------------------------------
static void SS_process_event(uniEvent_t *pEvt)
{
    uniEvent_t LEvt;

    //dbgPrintf( "####  pEvt->evtType = %d\r\n", pEvt->evtType);
    
    switch( pEvt->evtType )
    {
        case evt_SSQ_Check_Queue:

            if( SS == SS4_TXRX_HOLDOFF ) //  cleared in Holdoff_timeout_processing;
            {
                //if( P7_ticks == 0 ) //sunaba
                //    SS = SS4_TXRX_DONE;
                
                if( P7_ticks != 0 ) //segi
                    SS = SS4_TXRX_DONE;
            }
            if( SS == SS4_TXRX_DONE )
            {
                if( P7_ticks == 0 ) //sunaba
                {
                    if(SSQ_len() != 0)
                    {
                        SS = SS2_READY_FOR_XFER;
                    }
                    else
                    {
                        if( m_bleIsConnected == false)
                        {
                            WakePin_Release("\r\n WakePin_Release - Sunaba Last Transfer");
                            SS = SS_WAIT_STARTUP;
                        }
                        else
                        {
                            SS = SS2_READY_FOR_XFER;
                        }
                    }
                }
                if( P7_ticks != 0 ) //segi
                    SS = SS_WAIT_STARTUP;
            }
        // 
            if( SS == SS_WAIT_STARTUP) // reset state
            {
                if( SSQ_len() == 0)
                {
                    break; //return;
                }
                if( P7_ticks == 0 ) //sunaba
                {
                    WakePin_Assert();
                    SS = SS1_WAIT_P8;
                    P8_timer_start( P8_ticks ); // -> SS2_READY_FOR_XFER
                    break;
                }
                if( P7_ticks != 0 ) //segi
                {
                    WakePin_Assert();
                    SS = SS1_WAIT_P8;
                    P7_timer_start( P7_ticks ); // -> WakePin_Release();
                    P8_timer_start( P8_ticks ); // -> SS2_READY_FOR_XFER
                    break;
                }
                
            }
            if( SS == SS1_WAIT_P8) // Waiting for P8_ticks counter
            {
                //P8 Timer will force state -> SS2_READY_FOR_XFER
                break; // we are starting up already
            }
            if( SS == SS2_READY_FOR_XFER) // P8_ticks timeout | TxRx DOne | ....
            {
                if( P7_ticks == 0 ) //sunaba
                {
                    if( SSQ_len() == 0)
                    {
                        if( m_bleIsConnected == false)
                        {
                            WakePin_Release("\r\n WakePin_Release -  SS2_READY_FOR_.."); //WakePin_Release();
                            SS = SS_WAIT_STARTUP;
                            break; // return
                        }
                        if( m_bleIsConnected == true)
                            break; // return
                    }

                    SS = SS3_TXRX_BUSY;
                    LEvt.evtType = evt_SSQ_StartNext_TxRx;
                    core_thread_QueueSend( &LEvt );
                    break;
                }
                if( P7_ticks != 0 ) //segi
                {
                    if( SSQ_len() == 0)
                    {
                        SS = SS_WAIT_STARTUP;
                        break; // return 
                    }
                    SS = SS3_TXRX_BUSY;
                    LEvt.evtType = evt_SSQ_StartNext_TxRx;
                    core_thread_QueueSend( &LEvt );
                    break;
                }
            }
            
            if( SS == SS3_TXRX_BUSY)
            {
                // SS3_TXRX_BUSY doing TX - RX 
                return;
            }
        
        
            break;


        case evt_SSQ_StartNext_TxRx:
            
            /*
            dbgPrintf("\r\nSSQ_len = %d", SSQ_len() );
            if( SSQ[0].p_cpb == &cpb_NANNY) dbgPrint( "\r\ncpb_NANNY");
            if( SSQ[0].p_cpb == &cpb_ADC) dbgPrint( "\r\ncpb_ADC");
            if( SSQ[0].p_cpb == &cpb_BLN) dbgPrint( "\r\ncpb_BLN");
            if( SSQ[0].p_cpb == &cpb_BLP) dbgPrint( "\r\ncpb_BLP");

            if( SSQ[0].p_cpb == &cpb_BLE) dbgPrint( "\r\ncpb_BLE");
            */
        
            if( SSQ[0].p_cpb->pending == true) 
            {
                if( SSQ[0].p_cpb->processing == true)  //TODO : pending -> processing rethink ?!?
                    return;                            //TODO : pending -> processing rethink ?!? 
                SSQ[0].p_cpb->processing = true;
                
                SSQ[0].p_cpb->make_req( SSQ[0].p_cpb->p_be ); // Every single make_req is called here
                
                m_curr_beUtx =  SSQ[0].p_cpb->p_be ; //&be_CUm; or &be_BU;
                
                ma_uart_Init();
                ma_uart_timer_stop();
                ma_uart_packetTx_start();

                ma_uart_timer_Reason = Reason_rxTimeout;
                ma_uart_timer_start( APP_TIMER_TICKS( 2000,  APP_TIMER_PRESCALER) ); // 2000 -> 2000 milli Seconds

            }
            break;

            
            
        case evt_SSQ_TxRxOrTimeout_Done:

            if( P7_ticks != 0) // segi
            {
                SS = SS4_TXRX_DONE;
                WakePin_Release("\r\n WakePin_Release -  2/ evt_SSQ_TxRxOrTimeout_Done"); //WakePin_Release();
            }

            
            if( P7_ticks == 0) // sunaba
            {
                SS = SS4_TXRX_HOLDOFF;
                ma_holdoff_timer_start( APP_TIMER_TICKS( 2,  APP_TIMER_PRESCALER) );
            }


            if( (SSQ[0].p_cpb == &cpb_BLP ) && ( (BLN_boot == true) || ( m_bleIsConnected == false ) ) )
            {
                //WakePin_Release("\r\n WakePin_Release -  1/ evt_SSQ_TxRxOrTimeout_Done"); //WakePin_Release();
                BLN_boot = false;
            }


            SSQ_Pop(); // Get next command in Q if any
            
            if( SSQ_len() == 0 )
            {
                ma_uart_Deinit();
            }

            LEvt.evtType = evt_SSQ_Check_Queue;
            core_thread_QueueSend( &LEvt );

            break;
        
            
        default:
            //dbgPrintf("SS_process_event Event = %d not handledt\r\n", pEvt->evtType);
            break;
    }
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#include "app_scheduler.h"


//-----------------------------------------------------------------------------
void core_thread_init()
{
    be_Init();
    
    SSQ_Init();

    cpb_NANNY.pending = false;
    cpb_NANNY.processing = false;

    cpb_ADC.pending = false;
    cpb_ADC.processing = false;
    cpb_BLE.pending = false;
    cpb_BLE.processing = false;
    cpb_BLN.pending = false;
    cpb_BLN.processing = false;
    cpb_BLP.pending = false;
    cpb_BLP.processing = false;
}

//-----------------------------------------------------------------------------
static void core_thread_process_queue_item(void * p_event_data, uint16_t event_size)
{
    //dbgPrint("\r\nPriorityThread -> ");
    //uniEvent_t *pE = (uniEvent_t *)p_event_data ;
    //dbgPrintf("Event i = %d\r\n", pE->i );
    
    //core_thread_process_event((uniEvent_t *)p_event_data);

    dbgPrintf("\r\n####  pEvt->evtType = %d", ((uniEvent_t *)p_event_data)->evtType);

    SS_thread_process_event((uniEvent_t *)p_event_data);
    SS_process_event((uniEvent_t *)p_event_data);
}

//-----------------------------------------------------------------------------
void core_thread_QueueSend(uniEvent_t *pEvt)
{
    app_sched_event_put(pEvt, sizeof(uniEvent_t), core_thread_process_queue_item);
    //priority_sched_event_put(pEvt, sizeof(uniEvent_t), core_thread_process_queue_item);
}



