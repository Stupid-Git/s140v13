

#include "stdint.h"
#include "string.h"

#include "myapp.h"

#include "ma_adc.h"


//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//  9E_01
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================

uint8_t mg_5_dengen         = 1;    // system power 1:On ,  0:Off
uint8_t mg_6_advX50ms       = 10;   // advertising period x 50ms (1~255) -> 50ms ~ 12.75sec
uint8_t mg_7_wkUpPerX50ms   = 0;    // period for Wake Signal x 50ms (1~255) -> 50ms ~ 12.75sec
uint8_t mg_8_wkUpDelayX1ms  = 100;  // delay to SOH for Wake Signal x 1ms (1~255) -> 1ms ~ 255ms
uint8_t mg_9_ADC_rate       = 0;    // do Battery power measurement  0:Off, every 1~255 seconds
uint8_t mg_10_loadADC       = 0;    // do Battery Loaded measurement 0:Off, every 1~255 minutes
int8_t  mg_11_power         = 0;    // set output power level (signed byte) e.g. -40, -30, -20, -16, -1, -8, -4, 0 , +4 dBm
uint8_t mg_12_9E00_rate     = 0;    // the rate to read (9E_00) and update Advertising Info (0-once only after reset, 0x01~0x3F:1~63minutes, 0x81~0xBF: 1~63seconds) 

uint32_t mg_12_9E00_rate_inSeconds = 0;

uint32_t mg_6_advX50ms_inTicks;
static uint32_t mg_6_advX50ms_inTicks_Temp = 0;

static uint32_t blp_rate = 60;

void Update_P7_P8_ticks_from_mg7_mg8(void);
void WakePin_Release(char *S); //void  WakePin_Release(void);

int make_req_BLP( be_t *be_Req )
{
    uint16_t crc;
    
    be_Req->buffer[0] = 0x01;
    be_Req->buffer[1] = 0x9E;
    be_Req->buffer[2] = 0x01;
    be_Req->buffer[3] = 0;
    be_Req->buffer[4] = 0;

    crc = CRC_START_SEED; //0x0000;//0xFFFF;
    crc = crc16_compute (be_Req->buffer, 5, &crc);
    be_Req->buffer[5] = (crc >> 8) & 0x00ff; //CRC MSB first
    be_Req->buffer[6] = (crc >> 0) & 0x00ff;

    be_Req->rdPtr = 0;
    be_Req->wrPtr = 7;
    be_Req->length = 7;
    return(0);
}



int proc_rsp_BLP( be_t *be_Req,  be_t *be_Rsp )
{
    dbgPrint("\r\nproc_rsp_BLP_01");
    
    //be_Rsp->buffer[7] = 4;   //mg_7_wkUpPerX50ms  =   4; //TESTING
    //be_Rsp->buffer[8] = 100; //mg_8_wkUpDelayX1ms = 100; //TESTING

    
    if((mg_7_wkUpPerX50ms == 0) && (be_Rsp->buffer[7] != 0) ) // If old was 0, but new is not zero
    {
        WakePin_Release("\r\n WakePin_Release - proc_rsp_BLP");   //WakePin_Release();
    }
    
    mg_5_dengen        = be_Rsp->buffer[5];
    mg_6_advX50ms      = be_Rsp->buffer[6];
    mg_7_wkUpPerX50ms  = be_Rsp->buffer[7];
    mg_8_wkUpDelayX1ms = be_Rsp->buffer[8];
    mg_9_ADC_rate      = be_Rsp->buffer[9];
    mg_10_loadADC      = be_Rsp->buffer[10];
    mg_11_power        = (int8_t)be_Rsp->buffer[11];
    mg_12_9E00_rate    = be_Rsp->buffer[12];
    
    if ((mg_12_9E00_rate == 0) && (mg_9_ADC_rate == 0))
    {
        blp_rate = 60;
    }

//-----------------------------------------------------------------------------    
//-----------------------------------------------------------------------------    
#if USE_FORCE_9E_5_TO_1
    mg_5_dengen = 1; // Force No sleep_mode
#endif

#if USE_FORCE_9E_6_ADV
    //mg_6_advX50ms = 
    //if(mg_6_advX50ms > 100) mg_6_advX50ms = 100; // > 5000ms is too Slow ?
    //if(mg_6_advX50ms >  50) mg_6_advX50ms =  50; // > 2500ms is too Slow ?
    if(mg_6_advX50ms >  20) mg_6_advX50ms =  20; // > 1000ms is too Slow ?
    ///if(mg_6_advX50ms >  10) mg_6_advX50ms =  10; // > 500ms is too Slow ?
#endif

#if USE_FORCE_9E_9_ADC
    //mg_7_wkUpPerX50ms = be_Rsp->buffer[7];
    //mg_8_wkUpDelayX1ms = be_Rsp->buffer[8];
    
    mg_9_ADC_rate = 35; // 30 seconds
    mg_10_loadADC = 2;  //  2 minutes
    
    //mg_11_power = ;
    
    //mg_12_9E00_rate = 0x80 + 10; // 10 seconds
#endif

    

    if(NADC_mode == NADC_mode_VCHECK)
    {
        //if ( (mg_12_9E00_rate == 0) && (mg_9_ADC_rate == 0))
        if (mg_9_ADC_rate == 0)
        {
            NADC_mode = NADC_mode_NORMAL; // Do NOT send 9E_02
        }
        else
        {
            NADC_mode = NADC_mode_FORCE_1; // FORCE TWO ADC READINGS BEFORE going to NADC_mode_NORMAL
        }
    }
//-----------------------------------------------------------------------------    
//-----------------------------------------------------------------------------    

    dbgPrintf("\r\n9E_01 Length = %d", be_Rsp->wrPtr);
    dbgPrintf("\r\nB[0] = 0x%02x SleepMode", be_Rsp->buffer[5]);
    dbgPrintf("\r\nB[1] = %d x 50mS Advert Rate", be_Rsp->buffer[6]);
    dbgPrintf("\r\nB[2] = %d x 50mS WakeUp period", be_Rsp->buffer[7]);
    dbgPrintf("\r\nB[3] = %d mS  Wakeup delay ", be_Rsp->buffer[8]);
    dbgPrintf("\r\nB[4] = %d Seconds, ADC cycle", be_Rsp->buffer[9]);
    dbgPrintf("\r\nB[5] = %d Minutes LoadCycle", be_Rsp->buffer[10]);
    dbgPrintf("\r\nB[6] = %d dBm", be_Rsp->buffer[11]);
    dbgPrintf("\r\nB[7] = 0x%02x BLN_time RAW", be_Rsp->buffer[12]);

    //----- Process mg_5 ----- 
    // Done last?
    
    //----- Process mg_6 ----- 
    if(mg_6_advX50ms < 1) mg_6_advX50ms =   1; // 0 is invalid

    mg_6_advX50ms_inTicks_Temp = mg_6_advX50ms_inTicks;
    mg_6_advX50ms_inTicks = ((mg_6_advX50ms * 50 * 1000)/625);

    //----- Process mg_7, mg_8 ----- 
    Update_P7_P8_ticks_from_mg7_mg8();

    //----- Process mg_9 -----
    // mg_9_ADC_rate
    
    //----- Process mg_10 -----
    // mg_10_loadADC
    
    //----- Process mg_11 -----
    //
    /*
    if( 
        (mg_11_power != 4) &&
        (mg_11_power != 0) &&
        (mg_11_power != -4) &&
        (mg_11_power != -8) &&
        (mg_11_power != -12) &&
        (mg_11_power != -16) &&
        (mg_11_power != -20) &&
        (mg_11_power != -30)
    )
    {
        sd_ble_gap_tx_power_set(0); //tx_power Radio transmit power in dBm (accepted values are -40, -30, -20, -16, -12, -8, -4, 0, and 4 dBm). (in ble_gap.h)
    }
    else
    {
        sd_ble_gap_tx_power_set(mg_11_power);
    }
    */
    uint32_t err_code = sd_ble_gap_tx_power_set(mg_11_power);
    if( err_code != NRF_SUCCESS) // == NRF_ERROR_INVALID_PARAM
    {
        sd_ble_gap_tx_power_set(0);
    }
    
    
    //----- Process mg_12 -----
    if(mg_12_9E00_rate == 0)
    {
        mg_12_9E00_rate_inSeconds = 0;
    } else
    if( (mg_12_9E00_rate >= 0x01) && (mg_12_9E00_rate <= 0x3f)) // minutes
    {
        mg_12_9E00_rate_inSeconds = mg_12_9E00_rate * 60;
    } else
    if( (mg_12_9E00_rate >= 0x81) && (mg_12_9E00_rate <= 0xbf)) // seconds
    {
        mg_12_9E00_rate_inSeconds = mg_12_9E00_rate - 0x80;
    } else
    {
        mg_12_9E00_rate_inSeconds = 0;
    }

    //gap_device_name_only_set("mg_newBleName");
    if( mg_6_advX50ms_inTicks != mg_6_advX50ms_inTicks_Temp)
    {
        advertising_init_mg_new(mg_6_advX50ms_inTicks);      // set new advertising rate
    }
    
    sd_ble_gap_adv_stop();                    // these lines are needed to get
    ble_advertising_start(BLE_ADV_MODE_FAST); // advertising rate changed

    //----- Process mg_5 ----- 
    //Finally if Power off signal
    // This is very simple. May need more houskeeping to be done
    if( mg_5_dengen == 0 ) // 0 OFF, 1 ON
    {
        //TODO - shutdown ADC and Control pins
        //TODO - shutdown UART and WakeUp pins
        uint32_t err_code;
        // Go to system-off mode (this function will not return; wakeup will cause a reset).
        err_code = sd_power_system_off();
        
        //if we get to here we failed to go into sytem off mode
        APP_ERROR_CHECK(err_code);
    }
    
    

    blp_proc(BLP_PROC_UNPARK);
    
    //if (BLN_boot == true)
    // changed to always force a BLN  after BLP
    //-> TODO, if Man. Specific Data unchanged, dont change it
    //-> TODO, if Advertsing Name unchanged, dont change it
    {
        bln_proc(BLN_PROC_FORCE_TRIGGER);
    }


    if( NADC_mode == NADC_mode_FORCE_1 )
    {
        NADC_set_ADC_L_GO();
        NADC_proc(NADC_action_CHECK);
    }
    else
    if( NADC_mode == NADC_mode_NORMAL )
    {
        //reset counters
        //cnt_9 = 0; //Hammer see fix called Hammer
    }

    return(0);
}

int proc_timeout_BLP( be_t *be_Req,  be_t *be_Rsp )
{
    dbgPrint("\r\nproc_timeout_BLP");
    blp_proc(BLP_PROC_UNPARK);
    return(0);
}


//=============================================================================
typedef enum 
{
    BLP_BOOT,
    BLP_ENDED,
    BLP_PARKED,
} blp_state_t;
static blp_state_t blp_sm = BLP_BOOT;

static uint32_t blp_cnt = 0;

#define BLP_BOOT_RATE  1 //was 5 before ADC chech on boot up  //2016/11/04 with ADC voltage test this is too long ?
//#define BLP_BOOT_RATE  1
#define BLP_NULL_RATE  0
//#define BLP_NORM_RATE  0 //70


void blp_proc(int param)
{
    if(param == BLP_PROC_INIT_TRIGGER) // Basically called on power up
    {
        blp_cnt = 0;
        blp_rate = BLP_BOOT_RATE;
        blp_sm = BLP_BOOT;
        return;
    }
    if(param == BLP_PROC_UNPARK)
    {
        blp_sm = BLP_ENDED;
        return;
    }

    if(param == BLP_PROC_FORCE_TRIGGER) // After Disconnect, check for updated parameters
    {
        blp_rate = BLP_NULL_RATE;
        blp_cnt = 0;

        uniEvent_t LEvt;
        LEvt.evtType = evt_core_BLP_trigger;
        core_thread_QueueSend(&LEvt);
        blp_sm = BLP_PARKED;
    }

    
    if(param == BLP_PROC_TIMER_TICK)
    {
        
        switch(blp_sm)
        {
        case BLP_BOOT: // Power-up count-down to wait for microprocessor to boot
            if (blp_cnt < blp_rate)
                blp_cnt++;

            if( blp_cnt >= blp_rate)
            {
                blp_rate = BLP_NULL_RATE;
                blp_cnt = 0;

                uniEvent_t LEvt;
                LEvt.evtType = evt_core_BLP_trigger;
                core_thread_QueueSend(&LEvt);

                blp_sm = BLP_PARKED;
            }
            else
            {
                break;
            }

        case BLP_ENDED:
            break;

        case BLP_PARKED:
            break;
            
        default:
            blp_sm = BLP_BOOT;
            break;
        }
    }
    
}


