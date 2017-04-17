
#include "myapp.h"



// 0: STM32 disable reset on WAKE assertion too long 
// 1: STM32 enable reset on WAKE assertion too long 
static uint32_t NANNY_OnOff = 1;
void prime_NANNY_OFF(void)
{
    NANNY_OnOff = 0;
}
void prime_NANNY_ON(void)
{
    NANNY_OnOff = 1;
}

void NANNY_end(void)
{
    if(NANNY_OnOff == 0) // only go here if we were turning resets OFF
        NANNY_call_after_9E_OFF_cmd();
}

int proc_timeout_NANNY( be_t *be_Req,  be_t *be_Rsp )
{
    dbgPrint("\r\nproc_timeout_NANNY_03");
    NANNY_end();
    return(0);
}

int proc_rsp_NANNY( be_t *be_Req,  be_t *be_Rsp )
{
    dbgPrint("\r\nproc_rsp_NANNY_03");
    if( be_Rsp->buffer[2] == 0x06 )
    {
        dbgPrint("\r\n ACK");
    }
        
    NANNY_end();
    return(0);
}

int make_req_NANNY( be_t *be_Req )
{
    be_Req->buffer[0] = 0x01;
    be_Req->buffer[1] = 0x9E;
    be_Req->buffer[2] = 0x03;
    
    be_Req->buffer[3] = 4; //len LSB
    be_Req->buffer[4] = 0; //len MSB
    
    be_Req->buffer[5] = ((NANNY_OnOff>>0) & 0x00ff);
    be_Req->buffer[6] = ((NANNY_OnOff>>8) & 0x00ff);
    be_Req->buffer[7] = ((NANNY_OnOff>>16) & 0x00ff);
    be_Req->buffer[8] = ((NANNY_OnOff>>24) & 0x00ff);
    
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

