

#include "stdint.h"
#include "string.h"
#include "ble_gap.h"

#include "myapp.h"

#include "ma_adc.h"


//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//  9E_00
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
#define BLE_NAME_MAX_SIZE  (1 + BLE_GAP_DEVNAME_MAX_LEN)    // "1 +" to hold the NULL

uint8_t mg_ManufacturerSpecific_rsp26[26];
uint8_t mg_ShortenedName_rsp26[26+2];


void mg_ManufacturerSpecific_rsp26_setInitialValue(void)
{
    int i;
    for(i=0;i<26;i++)
    {
        mg_ManufacturerSpecific_rsp26[i] = 0;
    }
    mg_ManufacturerSpecific_rsp26[0] = 0xFF;
    mg_ManufacturerSpecific_rsp26[1] = 0xFF;
}

bool mg_ShortenedName_rsp26_IsUnchanged(char * device_name)
{
    int i;
    if( device_name == 0) // null pointer
        return(false);
    if( *device_name == 0) // null string
        return(false);

    for(i=0;i<26;i++)
    {
        if(mg_ShortenedName_rsp26[i] != device_name[i])
        {
            return(false);
        }
        if( device_name[i] == 0 )
            break;
    }

    for( ; i<26 + 2;i++)
    {
        if(mg_ShortenedName_rsp26[i] != 0)
            return(false);
    }
    return(true);
}

void mg_ShortenedName_rsp26_set(char * device_name)
{
    int i;
    for(i=0;i<26 + 2;i++)
    {
        mg_ShortenedName_rsp26[i] = 0;
    }
    if( device_name == 0) // null pointer
        return;
    if( *device_name == 0) // null string
        return;

    for(i=0;i<26;i++)
    {
        mg_ShortenedName_rsp26[i] = device_name[i];
        if( device_name[i] == 0 )
            break;
    }
    
}

bool mg_ManufacturerSpecific_rsp26_IsUpdated(uint8_t * buffer) 
{  
    int i;
    for( i=35; i<61;i++)
    {
        if( mg_ManufacturerSpecific_rsp26[i-35] != buffer[i] )
            break;
    }   
    if( i==61)
        return(false); // all the same, Is Updated is false

    for( i=35; i<61;i++)
    {
        mg_ManufacturerSpecific_rsp26[i-35] = buffer[i];
    }   
    return(true);
}

int make_req_BLN( be_t *be_Req )
{
    uint16_t cs;
    
    be_Req->buffer[0] = 0x01;
    be_Req->buffer[1] = 0x9E;
    be_Req->buffer[2] = 0x00;
    be_Req->buffer[3] = 0; //Len LSB
    be_Req->buffer[4] = 0; //Len MSB
#if _USE_CRC
//nerror    
    cs = CRC_START_SEED; //0x0000;//0xFFFF;
    cs = crc16_compute (be_Req->buffer, 5, &cs);
    be_Req->buffer[5] = (cs >> 8) & 0x00ff; // CRC MSB first
    be_Req->buffer[6] = (cs >> 0) & 0x00ff;
#else
    cs = get_checksum  (be_Req->buffer, 0, 5 );
    //be_Req->buffer[5] = 0x9F;
    //be_Req->buffer[6] = 0x00;
    be_Req->buffer[5] = (cs >> 0) & 0x00ff;
    be_Req->buffer[6] = (cs >> 8) & 0x00ff;
#endif

    be_Req->rdPtr = 0;
    be_Req->wrPtr = 7;
    be_Req->length = 7;
    return(0);
}

int proc_rsp_BLN( be_t *be_Req,  be_t *be_Rsp )
{
    //int i;

    dbgPrint("proc_rsp_BLN\r\n");

    //be->buffer[5..8] toroku code (4Bytes)
    
    // [26BYTES] BlueToothLE advertising Shortened Local Name (SCAN RESPONSE)
    //be->buffer[9..34] Advertising Name (26Bytes)
    
    // [26BYTES] BlueToothLE advertising Manufaturer Specific Data (PASSIVE RESPONSE)
    //be->buffer[35..36] comapany ID (2Bytes) (0x00FE)
    //be->buffer[37..40] serial number (4Bytes)
    //be->buffer[41] BLE security Lock 0: No lock, 1: Lock (1Byte)
    //be->buffer[42] Format Number, 0: RTR-500BLE, 1: TR-4, 2: ...  (1Byte)
    //be->buffer[43..60] Format Data (18Bytes)

    if( mg_ShortenedName_rsp26_IsUnchanged( (char*)&be_Rsp->buffer[9] ) == false)
    {
        mg_ShortenedName_rsp26_set( (char*)&be_Rsp->buffer[9] );
        gap_device_name_only_set((char*)mg_ShortenedName_rsp26);
    }
    
    
    if( mg_ManufacturerSpecific_rsp26_IsUpdated(be_Rsp->buffer) == true)
    {
        advertising_init_mg_new(mg_6_advX50ms_inTicks);  // This is called to set new mg_ManufacturerSpecific_rsp26, (mg_ShortenedName_rsp26,)
    }
    
    //for( i=35; i<61;i++)
    //{
    //    mg_ManufacturerSpecific_rsp26[i-35] = be_Rsp->buffer[i];
    //}   
    //advertising_init_mg_new(mg_6_advX50ms_inTicks);  // This is called to set new mg_ManufacturerSpecific_rsp26, (mg_ShortenedName_rsp26,)
    

    bln_proc(BLN_PROC_UNPARK);

    return(0);
}

int proc_timeout_BLN( be_t *be_Req,  be_t *be_Rsp )
{
    dbgPrint("proc_timeout_BLN\r\n");

    bln_proc(BLN_PROC_UNPARK);

    return(0);
}








typedef enum 
{
    BLN_COUNTING,
    BLN_PARKED,
} bln_state_t;
static bln_state_t bln_sm = BLN_PARKED; //BLN_BOOT;


static uint32_t bln_cnt = 0;
uint32_t mg_12_9E00_rate_inSeconds = 0;


void bln_proc(int param) // ref adc_proc(int param) blp_proc(int param)
{
    static bool bForceSet = false;
    
    if(param == BLN_PROC_INIT_TRIGGER) // Basically called on power up
    {
        bln_cnt = 0;
        //mg_12_9E00_rate_inSeconds = 0; // default: updateded by result of blp (9E_01) response
        bln_sm = BLN_PARKED;
        return;
    }

    if(param == BLN_PROC_UNPARK)
    {
        bForceSet = false; //
        bln_sm = BLN_COUNTING;
        return;
    }

    if(param == BLN_PROC_FORCE_TRIGGER) // After blp_proc (after Disconnect), check for updated parameters
    {
        //mg_12_9E00_rate_inSeconds = BLP_NORM_RATE;
        //bln_cnt = mg_12_9E00_rate_inSeconds - 1;
        bForceSet = true;
        bln_sm = BLN_COUNTING;
        param = BLN_PROC_TIMER_TICK; // force
    }

    
    if(param == BLN_PROC_TIMER_TICK)
    {
        
        switch(bln_sm)
        {
        //case BLN_BOOT: // Power-up count-down to wait for microprocessor to boot

        case BLN_COUNTING:
            if (bln_cnt < mg_12_9E00_rate_inSeconds)
                bln_cnt++;
            if ( ((bln_cnt >= mg_12_9E00_rate_inSeconds) && (mg_12_9E00_rate_inSeconds != 0) ) || (bForceSet))
            {
                /*
                dbgPrintf("----------------------> bForceSet                 = %d\r\n", bForceSet);
                dbgPrintf("----------------------> bln_cnt                   = %d\r\n", bln_cnt);
                dbgPrintf("----------------------> mg_12_9E00_rate_inSeconds = %d\r\n", mg_12_9E00_rate_inSeconds);
                */

                if( m_bleIsConnected == false) 
                {
                    bln_cnt = 0;
                    uniEvent_t LEvt;
                    LEvt.evtType = evt_core_BLN_trigger;
                    core_thread_QueueSend(&LEvt);
                    bln_sm = BLN_PARKED;
                }                
            }
            break;

        case BLN_PARKED:
            bForceSet = false; // bForceSet = false; will basically not get executed because  BLN_PROC_UNPARK usually arrives first.
            if (bln_cnt < mg_12_9E00_rate_inSeconds)
                bln_cnt++;
            break;
            
        default:
            bln_sm = BLN_PARKED;
            break;
        }
    }
    
    
}






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
int8_t mg_11_power         = 0;    // set output power level (signed byte) e.g. -40, -30, -20, -16, -1, -8, -4, 0 , +4 dBm
uint8_t mg_12_9E00_rate     = 0;    // the rate to read (9E_00) and update Advertising Info (0-once only after reset, 0x01~0x3F:1~63minutes, 0x81~0xBF: 1~63seconds) 

uint32_t mg_6_advX50ms_inTicks;
uint32_t OLD_mg_6_advX50ms_inTicks = 0;

//uint32_t mg_12_9E00_rate_inSeconds;
static uint32_t blp_rate = 60;

void P7_P8_ticks_from_mg7_mg8(void);
void WakePin_Release(char *S); //void  WakePin_Release(void);

int make_req_BLP( be_t *be_Req )
{
    uint16_t cs;
    
    be_Req->buffer[0] = 0x01;
    be_Req->buffer[1] = 0x9E;
    be_Req->buffer[2] = 0x01;
    be_Req->buffer[3] = 0;
    be_Req->buffer[4] = 0;
    //be_Req->buffer[5] = 0xA0;
    //be_Req->buffer[6] = 0x00;
    
    
    
#if _USE_CRC
//nerror
    cs = CRC_START_SEED; //0x0000;//0xFFFF;
    cs = crc16_compute (be_Req->buffer, 5, &cs);
    be_Req->buffer[5] = (cs >> 8) & 0x00ff; //CRC MSB first
    be_Req->buffer[6] = (cs >> 0) & 0x00ff;
#else
    cs = get_checksum  (be_Req->buffer, 0, 5 );
    be_Req->buffer[5] = (cs >> 0) & 0x00ff;
    be_Req->buffer[6] = (cs >> 8) & 0x00ff;
    //be_Req->buffer[5] = 0xA0;
    //be_Req->buffer[6] = 0x00;
#endif

    be_Req->rdPtr = 0;
    be_Req->wrPtr = 7;
    be_Req->length = 7;
    return(0);
}



int proc_rsp_BLP( be_t *be_Req,  be_t *be_Rsp )
{
    dbgPrint("proc_rsp_BLP\r\n");
    
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
    
    mg_9_ADC_rate = 5; // 30 seconds
    //mg_10_loadADC = 2;  //  2 minutes
    
    //mg_11_power = ;
    
    //mg_12_9E00_rate = 0x80 + 10; // 10 seconds
#endif

    

//-----------------------------------------------------------------------------    
//-----------------------------------------------------------------------------    

    dbgPrintf("9E_01 Length = %d\r\n", be_Rsp->wrPtr);
    dbgPrintf("B[0] = 0x%02x SleepMode\r\n", be_Rsp->buffer[5]);
    dbgPrintf("B[1] = %d x 50mS Advert Rate\r\n", be_Rsp->buffer[6]);
    dbgPrintf("B[2] = %d x 50mS WakeUp period\r\n", be_Rsp->buffer[7]);
    dbgPrintf("B[3] = %d mS  Wakeup delay \r\n", be_Rsp->buffer[8]);
    dbgPrintf("B[4] = %d Seconds, ADC cycle\r\n", be_Rsp->buffer[9]);
    dbgPrintf("B[5] = %d Minutes LoadCycle\r\n", be_Rsp->buffer[10]);
    dbgPrintf("B[6] = %d dBm\r\n", be_Rsp->buffer[11]);
    dbgPrintf("B[7] = 0x%02x BLN_time RAW\r\n", be_Rsp->buffer[12]);

    //----- Process mg_5 ----- 
    // Done last?
    
    //----- Process mg_6 ----- 
    if(mg_6_advX50ms < 1) mg_6_advX50ms =   1; // 0 is invalid

    OLD_mg_6_advX50ms_inTicks = mg_6_advX50ms_inTicks;
    mg_6_advX50ms_inTicks = ((mg_6_advX50ms * 50 * 1000)/625);

    //----- Process mg_7, mg_8 ----- 
    P7_P8_ticks_from_mg7_mg8();

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
    if( mg_6_advX50ms_inTicks != OLD_mg_6_advX50ms_inTicks)
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

    m_ADC_notEnabled = false;


    return(0);
}

int proc_timeout_BLP( be_t *be_Req,  be_t *be_Rsp )
{
    dbgPrint("proc_timeout_BLP\r\n");


    blp_proc(BLP_PROC_UNPARK);

    m_ADC_notEnabled = false;

    return(0);
}


//=============================================================================
typedef enum 
{
    BLP_BOOT,
    BLP_ENDED,
    //BLP_COUNTING,
    BLP_PARKED,
} blp_state_t;
static blp_state_t blp_sm = BLP_BOOT;

static uint32_t blp_cnt = 0;
//static uint32_t blp_rate = 60;

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



/*
void blp_proc_OLD(int param)
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
        blp_sm = BLP_COUNTING;
        return;
    }

    if(param == BLP_PROC_FORCE_TRIGGER) // After Disconnect, check for updated parameters
    {
        blp_rate = BLP_NORM_RATE;
        blp_cnt = blp_rate - 1;
        blp_sm = BLP_COUNTING;
        param = BLP_PROC_TIMER_TICK; // force
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
                blp_rate = BLP_NORM_RATE;
                blp_cnt = blp_rate - 1;
                blp_sm = BLP_COUNTING;
            }
            else
            {
                break;
            }

        case BLP_COUNTING:
            if (blp_cnt < blp_rate)
                blp_cnt++;

            if ( (blp_cnt >= blp_rate) && (blp_rate != 0) )
            {
                if( m_bleIsConnected == false) 
                {
                    blp_cnt = 0;
                    uniEvent_t LEvt;
                    LEvt.evtType = evt_core_BLP_trigger;
                    core_thread_QueueSend(&LEvt);
                    blp_sm = BLP_PARKED;
                }                
            }
            break;

        case BLP_PARKED:
            if (blp_cnt < blp_rate)
                blp_cnt++;
            break;
            
        default:
            blp_sm = BLP_BOOT;
            break;
        }
    }
    
}
*/

