
#include "myapp.h" // #include "nrf_soc.h" -> NRF_APP_PRIORITY_HIGH
#include "nrf_adc.h"
#include "nrf_delay.h"

#include "ma_adc.h"


volatile int32_t adc_sample;


void pinsBatt_Init(void);
void pinsBatt_ON_Assert(void);
void pinsBatt_LD_Assert(void);
void pinsBatt_ON_Release(void);
void pinsBatt_LD_Release(void);


//-------------------------------------
static void BattPins_ON_LO(void)
{  
    pinsBatt_Init();
    pinsBatt_ON_Assert();
}

static void BattPins_ON_HI(void)
{  
    //pinsBatt_Init();
    //pinsBatt_ON_Assert();
    pinsBatt_LD_Assert();
}
static void BattPins_OFF(void)
{
    pinsBatt_ON_Release();
    pinsBatt_LD_Release();
}

#if USE_SDK_V11
#include "app_util_platform.h"
#include "nrf_drv_config.h"
#endif

void ma_adc_config(void)
{
    const nrf_adc_config_t nrf_adc_config = NRF_ADC_CONFIG_DEFAULT;

    // Initialize and configure ADC
    nrf_adc_configure( (nrf_adc_config_t *)&nrf_adc_config);
    nrf_adc_input_select(NRF_ADC_CONFIG_INPUT_2);
    nrf_adc_int_enable(ADC_INTENSET_END_Enabled << ADC_INTENSET_END_Pos);
#if USE_SDK_V11
    NVIC_SetPriority(ADC_IRQn, APP_IRQ_PRIORITY_HIGH);
#else
    NVIC_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_HIGH);
#endif
    NVIC_EnableIRQ(ADC_IRQn);
   
}
/*
    See also Zip file attached to
        https://devzone.nordicsemi.com/question/1771/high-sample-rate-with-adc-and-softdevice/
*/


void ADC_IRQHandler(void)
{
    nrf_adc_conversion_event_clean();

    adc_sample = nrf_adc_result_get();

    NADC_proc(NADC_action_ADC_DONE);
}



static uint16_t batVoltage78;
bool g_PRE = true;


static float voltsFromAdcValue(int v)
{
    float dv = 0.0;
    
    //dv = 0.006349 * v + 0.240635;
    dv = 0.007914 * v - 0.1741;
    
    return(dv);
}


//mg_9_ADC_rate = be_Rsp->buffer[9];
//mg_10_loadADC = be_Rsp->buffer[10];
static uint8_t cnt_9 = 0;
static uint8_t cnt_10s = 0;
static uint8_t cnt_10m = 0;


int adc_count()
{
    int adcMeasureType = 0;
    
    //Hammer
    if(mg_9_ADC_rate==0)
    {
        cnt_9 = 0;
        cnt_10s = 0;
        cnt_10m = 0;
        
        adcMeasureType = 0;
        return(adcMeasureType);
    }
    //Hammer
    
    if( cnt_9 < mg_9_ADC_rate )
        cnt_9++;
    
    if( cnt_10m < mg_10_loadADC )
    {
        cnt_10s++;
        if( cnt_10s >= 60 )
        {
            cnt_10s = 0;
            if( cnt_10m < mg_10_loadADC ) 
                cnt_10m++;
        }
    }
    
    adcMeasureType = 0;
    if( cnt_9 >= mg_9_ADC_rate )
    {
        adcMeasureType = 1;
        if( cnt_10m >= mg_10_loadADC ) 
        {
            adcMeasureType = 2;
        }
    }
    return(adcMeasureType);
}

//    6-> 505uS
// 351 -> aaa
//#define TIMERTogA        6
//#define TIMERTogB      317
//uint32_t timerTog = TIMERTogA;

#define TIMERTog_6       6
#define TIMERTog_317   317



//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
typedef enum _eNADC_state
{
    NADC_state_IDLE,        // 0
    NADC_state_START,       // 1
    NADC_state_WIDTH_WAIT,  // 2
    NADC_state_WAIT_ADC,    // 3
    NADC_state_DONE_ADC,    // 4
    NADC_state_QUEUE_9E02,  // 5
} eNADC_state;

typedef struct ADC_entity_s
{
    eNADC_state  state;
    uint16_t     u16_value;
} ADC_entity_t;

ADC_entity_t m_ADC_ENT_L;
ADC_entity_t m_ADC_ENT_H;

eNADC_mode NADC_mode;

/* not used*/
void NADC_set_ADC_L_GO(void)
{
    if( m_ADC_ENT_L.state == NADC_state_IDLE)
    {
        m_ADC_ENT_L.state = NADC_state_START;
    }
}
void NADC_set_ADC_H_GO(void)
{
    if( m_ADC_ENT_H.state == NADC_state_IDLE)
    {
        m_ADC_ENT_H.state = NADC_state_START;
    }
}
/*not used */

int NADC_proc( eNADC_action action)
{
    
    dbgPrintf("\r\n               NADC_proc %d", action);
    dbgPrintf(",  L.state = %d", m_ADC_ENT_L.state);
    dbgPrintf(",  H.state = %d", m_ADC_ENT_H.state);
    dbgPrintf(",  NADC_mode = %d", NADC_mode);
    
    
    int adcMeasureType = 0;
    if(action == NADC_action_NONE)
    {
    }
    
    if(action == NADC_action_ONE_SECOND)
    {
        if( (NADC_mode == NADC_mode_FORCE_2) || (NADC_mode == NADC_mode_FORCE_1) )
        {
            return(0); // FORCE does not use timer
        }
        
        // After voltage > BOOT_V_LIMIT(2.5) is detected, we remain in NADC_mode_VCHECK mode
        // with mg_9_ADC_rate = 0; so no new ADC's will be done.
        
        // After the 9E_01 read of mg_9_ADC_rate 
        // if mg_9_ADC_rate == 0
        //    go straight to NADC_mode_NORMAL 
        // else
        //    go to NADC_mode_FORCE_1
        if(NADC_mode == NADC_mode_VCHECK)
        {
        }
        if(NADC_mode == NADC_mode_NORMAL)
        {
        }

        adcMeasureType = adc_count();
        if (adcMeasureType == 1)
        {
            if(m_ADC_ENT_L.state < NADC_state_START)  // == NADC_state_IDLE
                m_ADC_ENT_L.state = NADC_state_START;
            action = NADC_action_CHECK; // get rolling
            
            cnt_9 = 0; // this and parking needs to be sorted out
        }
        if (adcMeasureType == 2)
        {
            if(m_ADC_ENT_H.state < NADC_state_START)
                m_ADC_ENT_H.state = NADC_state_START;
            action = NADC_action_CHECK; // get rolling
            
            cnt_9 = 0; // this and parking needs to be sorted out
            cnt_10s = 0; // this and parking needs to be sorted out
            cnt_10m = 0; // this and parking needs to be sorted out
        }

        //if (adcMeasureType != 0) // != 0 means doing an ADC
        //{
        //    adc_sm = ADC_PARKED;
        //}                
        if (adcMeasureType != 0)
        {
        }
    }
    
    
    if(action == NADC_action_9E_TIMEOUT)
    {
        action = NADC_action_9E_DONE; // fall through to DONE for the moment
        /*
        if(m_ADC_ENT_L.state == NADC_state_QUEUE_9E02)
        {
            //TODO : try and resend?  Other timeouts? .....
        }
        if(m_ADC_ENT_H.state == NADC_state_QUEUE_9E02)
        {
            //TODO : try and resend?  Other timeouts? .....
        }
        */
    }
    if(action == NADC_action_9E_DONE)
    {
        if(m_ADC_ENT_L.state == NADC_state_QUEUE_9E02)
        {
            m_ADC_ENT_L.state = NADC_state_IDLE;
        }
        
        if(m_ADC_ENT_H.state == NADC_state_QUEUE_9E02)
        {
            m_ADC_ENT_H.state = NADC_state_IDLE;
        }

        if(NADC_mode == NADC_mode_FORCE_1)
        {
            NADC_set_ADC_H_GO();
            NADC_mode = NADC_mode_FORCE_2;
            action = NADC_action_CHECK;
        }
        else
        if(NADC_mode == NADC_mode_FORCE_2)
        {
            NADC_mode = NADC_mode_NORMAL;
            cnt_9 = 0; // this and parking needs to be sorted out
            cnt_10s = 0; // this and parking needs to be sorted out
            cnt_10m = 0; // this and parking needs to be sorted out
        }
    }
    
    if(action == NADC_action_CHECK)
    {
        if( m_ADC_ENT_L.state > NADC_state_START)
            return(0);
        if( m_ADC_ENT_H.state > NADC_state_START)
            return(0);

        if( m_ADC_ENT_L.state == NADC_state_START)
        {
            BattPins_ON_LO();                       //DRIVE_BAT_LOW()
            battLoad_timer_start( TIMERTog_6 );     //START_TIMER(L);
            m_ADC_ENT_L.state = NADC_state_WIDTH_WAIT;
            return(0);
        }
        else
        if( m_ADC_ENT_H.state == NADC_state_START)
        {
            BattPins_ON_LO();                       //DRIVE_BAT_HIGH()
            BattPins_ON_HI();                       //DRIVE_BAT_HIGH()
            battLoad_timer_start( TIMERTog_317 );   //START_TIMER(HIGH);
            m_ADC_ENT_H.state = NADC_state_WIDTH_WAIT;
            return(0);
        }
        return(0);
    }
    
    
    if(action == NADC_action_WIDTH_TIMER)
    {
        if( m_ADC_ENT_L.state == NADC_state_WIDTH_WAIT)
        {
            nrf_adc_start(); //START_ADC_CONVERSION()
            m_ADC_ENT_L.state = NADC_state_WAIT_ADC;
            return(0);
        }
        else
        if( m_ADC_ENT_H.state == NADC_state_WIDTH_WAIT)
        {
            nrf_adc_start(); //START_ADC_CONVERSION()
            m_ADC_ENT_H.state = NADC_state_WAIT_ADC;
            return(0);
        }
        return(0);
    }

    if(action == NADC_action_ADC_DONE)
    {
        BattPins_OFF(); //RELEASE_DRIVE_BAT()

        float    _dv_new = voltsFromAdcValue(adc_sample);
        uint16_t _u16_bat = (uint16_t)(_dv_new * 1000.0);

        if( m_ADC_ENT_L.state == NADC_state_WAIT_ADC)
        {
            m_ADC_ENT_L.u16_value = _u16_bat;
            m_ADC_ENT_L.state = NADC_state_DONE_ADC;
            if(NADC_mode == NADC_mode_VCHECK)
                m_ADC_ENT_L.state = NADC_state_IDLE;
        }
        else
        if( m_ADC_ENT_H.state == NADC_state_WAIT_ADC)
        {
            m_ADC_ENT_H.u16_value = _u16_bat | 0x8000; // | 0x8000 to indicated loaded value
            m_ADC_ENT_H.state = NADC_state_DONE_ADC;
            if(NADC_mode == NADC_mode_VCHECK)
                m_ADC_ENT_H.state = NADC_state_IDLE;
        }

        //NADC_mode = NADC_mode_VCHECK;
        //NADC_mode_VCHECK,   // do ADC but don't send via 9E_02
        //NADC_mode_FORCE_1,  // after reading 9E_01 and timer non zero: Force first ADC->9E_02
        //NADC_mode_FORCE_2,  // after reading 9E_01 and timer non zero: Force first ADC->9E_02
        //NADC_mode_NORMAL,   // after FORCE_1, FORCE_2 just our normal mode

        if(NADC_mode == NADC_mode_VCHECK)
        {
            if (_dv_new > BOOT_V_LIMIT) //2.50)
            {
                mg_9_ADC_rate = 0; // Zero - disable until mg_9_ADC_rate is set again
                mg_10_loadADC = 0; // Not Used
                cnt_9 = 0;
                cnt_10s = 0;
                cnt_10m = 0;

                g_PRE = false; // OLD - get out of PRE mode
                             
                NANNY_normalBoot_schedule_9E_ON_cmd();                                        
            }
        }

        if(NADC_mode == NADC_mode_NORMAL)
        {
            uniEvent_t LEvt;
            LEvt.evtType = evt_core_ADC_trigger;
            core_thread_QueueSend(&LEvt);
        }
        
        if(NADC_mode == NADC_mode_FORCE_1)
        {
            uniEvent_t LEvt;
            LEvt.evtType = evt_core_ADC_trigger;
            core_thread_QueueSend(&LEvt);
        }
        
        if(NADC_mode == NADC_mode_FORCE_2)
        {
            uniEvent_t LEvt;
            LEvt.evtType = evt_core_ADC_trigger;
            core_thread_QueueSend(&LEvt);
        }
        
        
            
        return(0);
    }



    if(action == NADC_action_RESET)
    {
        m_ADC_ENT_L.state = NADC_state_IDLE;
        m_ADC_ENT_H.state = NADC_state_IDLE;
        BattPins_OFF(); //RELEASE_DRIVE_BAT();
        
        // Set Voltage check rate
        NADC_mode = NADC_mode_VCHECK;        
        mg_9_ADC_rate = 1; // Once a second
        mg_10_loadADC = 0; // Not Used
        cnt_9 = 0;
        cnt_10s = 0;
        cnt_10m = 0;
        
        //mg_9_ADC_rate = be_Rsp->buffer[9];
        //mg_10_loadADC = be_Rsp->buffer[10];
        //static uint8_t cnt_9 = 0;
        //static uint8_t cnt_10s = 0;
        //static uint8_t cnt_10m = 0;
    }
    return(0);
}



int proc_timeout_ADC( be_t *be_Req,  be_t *be_Rsp )
{
    dbgPrint("\r\nproc_timeout_ADC_02");
    NADC_proc(NADC_action_9E_TIMEOUT);
    return(0);
}


int proc_rsp_ADC( be_t *be_Req,  be_t *be_Rsp )
{
    dbgPrint("\r\nproc_rsp_ADC_02");
    NADC_proc(NADC_action_9E_DONE);
    return(0);
}



int make_req_ADC( be_t *be_Req )
{
    if( m_ADC_ENT_L.state == NADC_state_DONE_ADC)
    {
        batVoltage78 = m_ADC_ENT_L.u16_value;
        m_ADC_ENT_L.state = NADC_state_QUEUE_9E02;
    }
    else
    if( m_ADC_ENT_H.state == NADC_state_DONE_ADC)
    {
        batVoltage78 = m_ADC_ENT_H.u16_value;
        m_ADC_ENT_H.state = NADC_state_QUEUE_9E02;
    }    
    
    be_Req->buffer[0] = 0x01;
    be_Req->buffer[1] = 0x9E;
    be_Req->buffer[2] = 0x02;
    be_Req->buffer[3] = 4; //len LSB
    be_Req->buffer[4] = 0; //len MSB
    be_Req->buffer[5] = FIRMWARE_REV_LSB; //0x91;
    be_Req->buffer[6] = FIRMWARE_REV_MSB; //0x00; // 0x0100 -> Version 1.00
    be_Req->buffer[7] = (batVoltage78 >> 0) & 0x00ff;
    be_Req->buffer[8] = (batVoltage78 >> 8) & 0x00ff;
    
    uint16_t crc;
    crc = CRC_START_SEED; //0x0000;//0xFFFF;
    crc = crc16_compute (be_Req->buffer, 9, &crc);
    be_Req->buffer[ 9] = (crc >> 8) & 0x00ff; // CRC MSB first
    be_Req->buffer[10] = (crc >> 0) & 0x00ff;

    be_Req->rdPtr = 0;
    be_Req->wrPtr = 11;
    be_Req->length = 11;

    /* DEBUG*/
    dbgPrintf("\r\n===============================================================");
    dbgPrintf("\r\n");
    int i;
    for(i=0; i<11;i++)
    {
        if( i==7 ) dbgPrintf("<%02x ", be_Req->buffer[i] ); else
        if( i==8 ) dbgPrintf("%02x> ", be_Req->buffer[i] ); else
        dbgPrintf("%02x ", be_Req->buffer[i] );
    }
    dbgPrintf("\r\n===============================================================");

    // 01 9e 02 04 00 00 01 f0 0a a0 01
    /**/
    
    return(0);
}

