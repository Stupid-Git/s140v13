

#include "stdint.h"
#include "string.h"

#include "myapp.h"



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
    uint16_t crc;
    
    be_Req->buffer[0] = 0x01;
    be_Req->buffer[1] = 0x9E;
    be_Req->buffer[2] = 0x00;
    be_Req->buffer[3] = 0; //Len LSB
    be_Req->buffer[4] = 0; //Len MSB
    crc = CRC_START_SEED; //0x0000;//0xFFFF;
    crc = crc16_compute (be_Req->buffer, 5, &crc);
    be_Req->buffer[5] = (crc >> 8) & 0x00ff; // CRC MSB first
    be_Req->buffer[6] = (crc >> 0) & 0x00ff;

    be_Req->rdPtr = 0;
    be_Req->wrPtr = 7;
    be_Req->length = 7;
    return(0);
}

int proc_rsp_BLN( be_t *be_Req,  be_t *be_Rsp )
{
    //int i;

    dbgPrint("\r\nproc_rsp_BLN_00");
    bool bShortenedNameDidChange = false;  //Change 2017/01/12

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
        bShortenedNameDidChange = true;
    }
    
    
    //if(mg_ManufacturerSpecific_rsp26_IsUpdated(be_Rsp->buffer) == true)  //Change 2017/01/12
    if( (mg_ManufacturerSpecific_rsp26_IsUpdated(be_Rsp->buffer) == true) || (bShortenedNameDidChange == true) )  //Change 2017/01/12
    {
        advertising_init_mg_new(mg_6_advX50ms_inTicks);  // This is called to set new mg_ManufacturerSpecific_rsp26, (mg_ShortenedName_rsp26,)
    }
    
    bln_proc(BLN_PROC_UNPARK);

    return(0);
}

int proc_timeout_BLN( be_t *be_Req,  be_t *be_Rsp )
{
    dbgPrint("\r\nproc_timeout_BLN");

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
extern uint32_t mg_12_9E00_rate_inSeconds;// = 0;


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
                dbgPrintf("\r\n----------------------> bForceSet                 = %d", bForceSet);
                dbgPrintf("\r\n----------------------> bln_cnt                   = %d", bln_cnt);
                dbgPrintf("\r\n----------------------> mg_12_9E00_rate_inSeconds = %d", mg_12_9E00_rate_inSeconds);
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


