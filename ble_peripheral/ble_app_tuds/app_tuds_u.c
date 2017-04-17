
#include "stdint.h"

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"

#include "debug_etc.h"

#include <stdlib.h>
#include "nordic_common.h"
#include "app_timer.h"

#include "ble_tuds.h"
#include "app_tuds.h"

#if SPLIT_UP_DN


#define APP_TIMER_PRESCALER 0
#define BLE_ERROR_NO_TX_BUFFERS BLE_ERROR_NO_TX_PACKETS

extern ble_tuds_t  m_ble_tuds;                                  // Structure to identify the BLKUP Service.


#define BLK_UP_COUNT (128 + 8)
static uint8_t* m_blkUp_p_buf;//[ 16 * BLK_UP_COUNT ]; // 2048 @ BLK_UP_COUNT = 128
static uint8_t  m_blkUp_chk[  1 * BLK_UP_COUNT ]; //  128 @ BLK_UP_COUNT = 128
static uint16_t m_blkUp_len;
static uint16_t m_blkUp_blkCnt;
static uint16_t m_blkUp_txBlkCnt;
static uint8_t  m_blkUp_chkSumLSB;
static uint8_t  m_blkUp_chkSumMSB;



//=============================================================================
//=============================================================================
//=============================================================================
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//===== UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP   ======
//=============================================================================
//=============================================================================
//=============================================================================


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
typedef enum eBlkUpState
{
    eBlkUp_IDLE,
    eBlkUp_CMD_PRESEND,
    eBlkUp_CMD_SEND,
    eBlkUp_CMD_SENT,
    eBlkUp_DAT_PRESEND,
    eBlkUp_DAT_SEND,
    eBlkUp_DAT_SENT,
    
    eBlkUp_WAIT_CFM,

} eBlkUpState_t;

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------
static  eBlkUpState_t m_BlkUp_sm = eBlkUp_IDLE;


void  BlkUp_Purge()
{
    m_BlkUp_sm = eBlkUp_IDLE;
}


//-----------------------------------------------------------------------------
static int32_t blk_up_set( uint8_t *pkt, uint16_t len)
{
//    uint8_t  j;
    uint16_t i;
    uint16_t cs;

    m_blkUp_txBlkCnt = 0;
    m_blkUp_blkCnt = 0;    
    
    if( len > (16 * BLK_UP_COUNT) )
    {
        //Error - data too long 
        return(1);
    }
#if 0 // _CRC
    //cs = CRC_START_SEED; //0x0000;//0xFFFF;
    //cs = crc16_compute (&pkt[0], len, &cs);
#else
    cs = 0;
    for(i=0 ; i < len; i++)
        cs = cs + pkt[i];
#endif   
    m_blkUp_chkSumLSB = (uint8_t)((cs>>0) & 0x00FF);
    m_blkUp_chkSumMSB = (uint8_t)((cs>>8) & 0x00FF);
    
    
    m_blkUp_len = len + 2;
    
    m_blkUp_blkCnt = ((m_blkUp_len - 1) / 16) + 1;

    m_blkUp_p_buf = pkt;
    
    for(i=0 ; i < m_blkUp_blkCnt; i++)
    {
        m_blkUp_chk[i] = 0x00;
    }
    
    return(0);
}

//-----------------------------------------------------------------------------
static int32_t blk_up_get_Ucmd( uint8_t *buf20 )
{
    uint8_t  i;

    for( i = 0; i< 20; i++)
        buf20[i] = 0x00; 

    buf20[0] = 0x01;
    buf20[1] = 0x01;
    buf20[2] = (uint8_t)((m_blkUp_len>>0) & 0x00FF);
    buf20[3] = (uint8_t)((m_blkUp_len>>8) & 0x00FF);
    
    return(0);
}

//-----------------------------------------------------------------------------
static int32_t blk_up_get_blk_No( uint8_t *buf20, uint8_t blk_No )
{
    uint16_t position;
    uint8_t  i;

    for( i = 0; i< 20; i++)
        buf20[i] = 0x00; 

    position = blk_No;
    if( ( (position * 16) + 1) > (m_blkUp_len))
        return(-1);
    
    buf20[0] = (uint8_t)((blk_No>>0) & 0x00FF);
    buf20[1] = 0x00;
    buf20[2] = 0x00;
    buf20[3] = 0x00;
    for(i=0 ; i < 16; i++)
    {
        if( (position*16 + i) < (m_blkUp_len - 2))
            buf20[4 + i] = m_blkUp_p_buf[position*16 + i];

        if( (position*16 + i) == (m_blkUp_len - 2))
            buf20[4 + i] = m_blkUp_chkSumLSB;
        
        if( (position*16 + i) == (m_blkUp_len - 1))
            buf20[4 + i] = m_blkUp_chkSumMSB;
    }

    return(0);
}


static int32_t blk_up_setSent( uint8_t blk_No )
{
    uint16_t position;
    position = blk_No;
    if( ( (position * 16) + 1) > (m_blkUp_len))
        return(-1);

    m_blkUp_txBlkCnt++;
    m_blkUp_chk[position] += 1;
    return(0);
}

#define UP_CHK_OK 0

static int32_t blk_up_getNextBlkno(uint8_t current_blk_No)
{
    int32_t r;
    uint16_t i;
 
    // Look forward first
    for(i = current_blk_No ; i < m_blkUp_blkCnt; i++)
    {
        if(m_blkUp_chk[i] == 0x00)
        {
            r = i;
            return(r);
        }
    }

    // Next, look from begining again
    for( i = 0 ; i < current_blk_No ; i++)
    {
        if(m_blkUp_chk[i] == 0x00)
        {
            r = i;
            return(r);
        }
    }

    r = -1;
    return(r);
}
//-----------------------------------------------------------------------------
//
//  Timer
//
//-----------------------------------------------------------------------------
static void BlkUp_timeout_handler(void * p_context);


 
APP_TIMER_DEF(m_BlkUp_timer_id);



//static uint32_t m_app_ticks_per_100ms;
#define BSP_MS_TO_TICK(MS) (m_app_ticks_per_100ms * (MS / 100))

  

uint32_t BlkUp_timer_init()//uint32_t ticks_per_100ms)
{
    uint32_t  err_code;

    err_code = app_timer_create(&m_BlkUp_timer_id, APP_TIMER_MODE_SINGLE_SHOT, BlkUp_timeout_handler);
    if (err_code != NRF_SUCCESS)
    {
    }
    return( err_code);
}


uint32_t  BlkUp_timer_start(uint32_t timeout_ticks)
{
    uint32_t  err_code;
   
    //timeout_ticks = 1000;
    
    err_code = app_timer_stop(m_BlkUp_timer_id);
    err_code = app_timer_start(m_BlkUp_timer_id, timeout_ticks, NULL);
    if (err_code != NRF_SUCCESS)
    {
        ;
    }
    return(err_code);    
}

uint32_t BlkUp_timer_stop()
{
    uint32_t err_code;
    err_code = app_timer_stop(m_BlkUp_timer_id);
    if (err_code != NRF_SUCCESS)
    {
        ;
    }
    return(err_code);    
}



//-----------------------------------------------------------------------------
//
//  Controls
//
//-----------------------------------------------------------------------------
static uint8_t  m_Ucmd_buf[20];
static uint8_t  m_Udat_buf[20];
static uint16_t m_Ucmd_len;
static uint16_t m_Udat_len;
static uint8_t  m_current_blk_No;

static uint32_t blk_up_startSend_Ucmd()
{
    uint32_t err_code;

    blk_up_get_Ucmd(m_Ucmd_buf);    

    m_Ucmd_len = 20;

    //m_BlkUp_sm = eBlkUp_CMD_PRESEND;
    //BlkUp_timer_stop();

    dbgPrintf("blk_up_startSend_Ucmd\r\n");

    err_code = ble_tuds_notify_Ucmd( &m_ble_tuds, m_Ucmd_buf, &m_Ucmd_len);
    if(err_code == NRF_SUCCESS)
    {
        dbgPrint("o");
        m_BlkUp_sm = eBlkUp_CMD_SEND;
        BlkUp_timer_start( APP_TT_224 );
    }
    else
    if(err_code == BLE_ERROR_NO_TX_BUFFERS)
    {
        dbgPrint("n");
        // should be this mode m_BlkUp_sm = eBlkDn_GOT_PACKET;
        BlkUp_timer_start( APP_TT_224 );
    }
    else
    {
        dbgPrint("x");
    }

    return(err_code);    
}

static uint32_t blk_up_startSend_Udat(uint8_t blk_No)
{
    uint32_t err_code;
    //int32_t r;

    //r = 
    blk_up_get_blk_No(m_Udat_buf, blk_No);    

    m_Udat_len = 20;

    //m_BlkUp_sm = eBlkUp_DAT_PRESEND;
    //BlkUp_timer_stop();
    err_code = ble_tuds_notify_Udat( &m_ble_tuds, m_Udat_buf, &m_Udat_len);
    if(err_code == NRF_SUCCESS)
    {
        dbgPrint("o");
        m_BlkUp_sm = eBlkUp_DAT_SEND;
        BlkUp_timer_start( APP_TT_224 );
    }
    else
    if(err_code == BLE_ERROR_NO_TX_BUFFERS)
    {
        dbgPrint("n");
        // should be this mode m_BlkUp_sm = eBlkDn_GOT_PACKET;
        BlkUp_timer_start( APP_TT_224 );
    }
    else
    {
        dbgPrint("x");
    }

    return(err_code);    
}


//-----------------------------------------------------------------------------
//
//  Events
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Callback for BLE write to Ucfm (register)
//-----------------------------------------------------------------------------
int32_t app_tuds_Ucfm_handler( app_tuds_t *p_app_tuds, uint8_t *buf, uint8_t len) //int32_t BlkUp_On_Ucfm( uint8_t *buf, uint8_t len)
{
    int32_t r;
    r = 0;

    if( buf[0] == 1 )
    {
        if( buf[1] == 0 ) // 1,0 OK
        {
            dbgPrint("S");
            m_BlkUp_sm = eBlkUp_IDLE;
        }

        if( buf[1] == 1 ) // 1,1 NG
        {
            dbgPrint("N");
            m_BlkUp_sm = eBlkUp_IDLE;
        }

        if( buf[1] == 2 ) // 1,2 Resend
        {
        }

    }
    return(r);
}

//-----------------------------------------------------------------------------
// Callback for write done event
//-----------------------------------------------------------------------------
int32_t app_tuds_OnWrittenComplete_Ucmd_handler(app_tuds_t *p_app_tuds,  uint8_t *buf, uint8_t len) //int32_t BlkUp_On_written_Ucmd(ble_tuds_t *p_tuds,  uint8_t *buf, uint8_t len)
{
    int32_t r;

    // Check if in proper state
    if( m_BlkUp_sm != eBlkUp_CMD_SEND)
        return(0);

   
    if( m_BlkUp_sm == eBlkUp_CMD_SEND)
    {
        BlkUp_timer_stop();
        m_BlkUp_sm = eBlkUp_CMD_SENT; 

        m_current_blk_No = 0;
        blk_up_startSend_Udat(m_current_blk_No);
    }
   
    
    return(r);
}

//-----------------------------------------------------------------------------
// Callback for write done event
//-----------------------------------------------------------------------------
int32_t app_tuds_OnWrittenComplete_Udat_handler(app_tuds_t *p_app_tuds,  uint8_t *buf, uint8_t len) //int32_t BlkUp_On_written_Udat(ble_tuds_t *p_tuds,  uint8_t *buf, uint8_t len)
{
    int32_t r;

    // Check if in proper state
    if( m_BlkUp_sm != eBlkUp_DAT_SEND)
        return(0);

    //dbgPrint("C");

    if( m_BlkUp_sm == eBlkUp_DAT_SEND)    
    {
        BlkUp_timer_stop();
        m_BlkUp_sm = eBlkUp_DAT_SENT; 
        blk_up_setSent(m_current_blk_No);
        r = blk_up_getNextBlkno(m_current_blk_No);
        dbgPrintf("Udat r = %d\r\n", r);
        if(r >= 0)
        {
            m_current_blk_No = (uint8_t)(r);
            blk_up_startSend_Udat(m_current_blk_No);
        }
        else
        {
            // r < 0 => Not more to send => wait for Ucfm
            //start_wait_Ucfm();
            m_BlkUp_sm = eBlkUp_WAIT_CFM;
            BlkUp_timer_start( APP_TT_224 );
        }
    }
    
    return(r);
}



//-----------------------------------------------------------------------------
//
//  Timer
//
//-----------------------------------------------------------------------------
static void blk_up_printState(void)
{
    if(m_BlkUp_sm == eBlkUp_IDLE           ) dbgPrintf("[I_I]");
    if(m_BlkUp_sm == eBlkUp_CMD_PRESEND    ) dbgPrintf("[C_P]");
    if(m_BlkUp_sm == eBlkUp_CMD_SEND       ) dbgPrintf("[C_S]");
    if(m_BlkUp_sm == eBlkUp_CMD_SENT       ) dbgPrintf("[C_T]");
    if(m_BlkUp_sm == eBlkUp_DAT_PRESEND    ) dbgPrintf("[D_P]");
    if(m_BlkUp_sm == eBlkUp_DAT_SEND       ) dbgPrintf("[D_S]");
    if(m_BlkUp_sm == eBlkUp_DAT_SENT       ) dbgPrintf("[D_T]");
    
    if(m_BlkUp_sm == eBlkUp_WAIT_CFM       ) dbgPrintf("[W_C]");
}

static int m_BlkUp_cfm_WaitTimeCount = 0;

static void BlkUp_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
#if 0
    dbgPrintf("*");
#else    
    dbgPrintf("*");
    blk_up_printState();
    if(m_BlkUp_sm == eBlkUp_IDLE) // Do nothing
    { } //Just waiting for new data to send
    else
        
    if(m_BlkUp_sm == eBlkUp_WAIT_CFM) // Should have got the next data packet by now
    {
        if(++m_BlkUp_cfm_WaitTimeCount > 3)
        {
            m_BlkUp_cfm_WaitTimeCount = 0;
            m_BlkUp_sm = eBlkUp_IDLE; // Reset
        }
        else
        {
            //start_wait_Ucfm();
            m_BlkUp_sm = eBlkUp_WAIT_CFM;
            BlkUp_timer_start( APP_TT_224 );
        }
    }
    else
/*
    if(m_BlkUp_sm == eBlkDn_GOT_PACKET) // Do nothing
    { } // Just got the packet so I should have cancelled the timer
    else
    if(m_BlkUp_sm == eBlkDn_MISSING_PRESEND) // buffer was full last time, so try again
    {
            dbgPrintf("[MPre]");
            start_send_Dcfm_missing(4);
    }
    else
    if(m_BlkUp_sm == eBlkDn_MISSING_SEND) // we didn't get a write done
    {
            dbgPrintf("[MSend]");
            start_send_Dcfm_missing(4);
    }
    else
*/        
    
    if(m_BlkUp_sm == eBlkUp_CMD_PRESEND) // buffer was full last time, so try again
    {
        blk_up_startSend_Ucmd();
    }
    else
    if(m_BlkUp_sm == eBlkUp_CMD_SEND) // we didn't get a write done
    {
        blk_up_startSend_Ucmd();
    }
    else
    if(m_BlkUp_sm == eBlkUp_CMD_SENT)
    {
    }
    
    if(m_BlkUp_sm == eBlkUp_DAT_PRESEND) // buffer was full last time, so try again
    {
        blk_up_startSend_Udat(m_current_blk_No);
    }
    else
    if(m_BlkUp_sm == eBlkUp_DAT_SEND) // we didn't get a write done
    {
        blk_up_startSend_Udat(m_current_blk_No);
    }
    else
    if(m_BlkUp_sm == eBlkUp_DAT_SENT)
    {
    }
    else
    {
    }
#endif
}


//-----------------------------------------------------------------------------
//
//  Triggers to start BlkUp process
//
//-----------------------------------------------------------------------------

void BlkUp_Go_Test(void)
{

    static uint8_t upbuf[256];
    static uint16_t upbuflen;
    uint16_t i;
    

    dbgPrintf("BlkUp_Go_Test\r\n");

    upbuflen = 42;
    
    for( i = 0 ; i< upbuflen; i++)
        upbuf[i] = (uint8_t)i;
    
    blk_up_set(upbuf, upbuflen);
    
    //------------------------

    m_BlkUp_sm = eBlkUp_CMD_PRESEND;
    BlkUp_timer_stop();    
    blk_up_startSend_Ucmd();
    // 1 -> writedone or    (BlkUp_On_written_Ucmd)
    // 2 -> timer -> retry  (BlkUp_timeout_handler)
    
}


void BlkUp_Go( uint8_t *pkt, uint16_t len)
{

    dbgPrintf("BlkUp_Go\r\n");

    if( len > (16 * BLK_UP_COUNT) )
    {
        //Error - data too long 
        return;
    }

    
    blk_up_set(pkt, len);
    
    //------------------------

    m_BlkUp_sm = eBlkUp_CMD_PRESEND;
    BlkUp_timer_stop();    
    blk_up_startSend_Ucmd();
    // 1 -> writedone or    (BlkUp_On_written_Ucmd)
    // 2 -> timer -> retry  (BlkUp_timeout_handler)
    
}


#endif //#if SPLIT_UP_DN

