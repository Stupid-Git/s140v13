
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


#define APP_TIMER_PRESCALER 0
#define BLE_ERROR_NO_TX_BUFFERS BLE_ERROR_NO_TX_PACKETS

app_tuds_event_handler_t    m_event_handler;

extern ble_tuds_t  m_ble_tuds;                                  // Structure to identify the BLKUP Service.



#if !SPLIT_UP_DN //SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN


#define _EXT

void BlkDn_unlockStateMachine(void);


//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================




/* ----------------------------------------------------------------------------
*  BSS
*/

// block_dn.c
#define BLK_DN_COUNT (128 + 8)

_EXT uint8_t  m_blkDn_buf[ 16 * BLK_DN_COUNT ]; // 2048 @ BLK_DN_COUNT = 128
_EXT uint8_t  m_blkDn_chk[  1 * BLK_DN_COUNT ]; //  128 @ BLK_DN_COUNT = 128
_EXT uint16_t m_blkDn_len;
_EXT uint16_t m_blkDn_blkCnt;
_EXT uint16_t m_blkDn_rxBlkCnt;

static uint8_t  m_Dcfm_buf[20];
static uint16_t m_Dcfm_len;

// block_up.c
#define BLK_UP_COUNT (128 + 8)
//_EXT uint8_t  m_blkUp_buf[ 16 * BLK_UP_COUNT ]; // 2048 @ BLK_UP_COUNT = 128
_EXT   uint8_t* m_blkUp_p_buf;//[ 16 * BLK_UP_COUNT ]; // 2048 @ BLK_UP_COUNT = 128
_EXT   uint8_t  m_blkUp_chk[  1 * BLK_UP_COUNT ]; //  128 @ BLK_UP_COUNT = 128
_EXT   uint16_t m_blkUp_len;
_EXT   uint16_t m_blkUp_blkCnt;
_EXT   uint16_t m_blkUp_txBlkCnt;
_EXT   uint8_t  m_blkUp_chkSumLSB;
_EXT   uint8_t  m_blkUp_chkSumMSB;


/* ----------------------------------------------------------------------------
*  FUNCTIONS
*/

void      ma_tuds_on_ble_evt(ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt); //wrapper for ble_tuds_on_ble_evt

uint32_t  ma_tuds_send_packet(app_tuds_t * p_tuds, uint8_t * buf, uint16_t* p_len16);

void unused_function_calls_m_event_handler()
{
    app_tuds_evt_t app_tuds_event;
    
    m_event_handler(&app_tuds_event);
}
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================

typedef enum eBlkDnState
{
    eBlkDn_WAIT_CMD,
    eBlkDn_WAIT_PACKET,
    eBlkDn_MISSING_PRESEND,
    eBlkDn_MISSING_SEND,
    eBlkDn_GOT_PACKET,
    eBlkDn_CFM_PRESEND, // set before CFM packet sent
    eBlkDn_CFM_SEND, // set after CFM packet sent
    eBlkDn_CFM_SENT, // set after CFM packet sent (Write Done or ACK) received

    //eBlkDn_PROCESS_PACKET,
    //eBlkDn_SEND_CMD,
    //eBlkDn_SEND_DATA,
    //eBlkDn_SEND_WAITCFM,

} eBlkDnState_t;


static eBlkDnState_t m_BlkDn_sm = eBlkDn_WAIT_CMD;



//-----------------------------------------------------------------------------
static int32_t blk_dn_start( uint8_t *pkt )
{
    uint8_t  j;
    uint16_t i;

    m_blkDn_rxBlkCnt = 0;
    m_blkDn_blkCnt = 0;    
    
    m_blkDn_len = pkt[2] | (pkt[3]<<8);
    if( m_blkDn_len == 0)
        return(1);
    
    m_blkDn_blkCnt = ((m_blkDn_len - 1) / 16) + 1;
    
    
    for(i=0 ; i < m_blkDn_blkCnt; i++)
    {
        m_blkDn_chk[i] = 0x00;
        for(j=0 ; j < 16; j++)
        {
            m_blkDn_buf[i*16 + j] = 0x00;
        }
    }
    
    return(0);
}


static int32_t blk_dn_add( uint8_t *pkt, uint16_t len )
{
    uint8_t  position;
    uint8_t  j;
//    uint16_t i;
    
    if(len != 20)
        return(1);
    
    position = pkt[0];
    if( position >= BLK_DN_COUNT)
        return(1);
    
    m_blkDn_rxBlkCnt++;
    m_blkDn_chk[position] += 1;
    for(j=0 ; j < 16; j++)
    {
        m_blkDn_buf[position*16 + j] = pkt[4 + j];
    }
    return(0);
}

#define DN_CHK_OK 0
#define DN_CHK_CHKSUM_NG 3

static int32_t blk_dn_chk()
{
//    uint8_t  j;
    uint16_t i;
    uint16_t missing_blk_cnt;
    uint16_t cs_pkt;
    uint16_t cs_now;
 
    if(m_blkDn_blkCnt == 0)
        return(1);

    if(m_blkDn_rxBlkCnt < m_blkDn_blkCnt)
        return(1);
    
    missing_blk_cnt = 0;
    for(i=0 ; i < m_blkDn_blkCnt; i++)
    {
        if(m_blkDn_chk[i] == 0x00)
            missing_blk_cnt++;
    }
    if(missing_blk_cnt>0)
        return(2);

    cs_pkt = m_blkDn_buf[m_blkDn_len - 2] | (m_blkDn_buf[m_blkDn_len - 1]<<8);
#if 0 // _CRC
    //cs_now = CRC_START_SEED; //0x0000;//0xFFFF;
    //c_now = crc16_compute( &m_blkDn_buf[0], m_blkDn_len - 2, cs_now);
#else    
    cs_now = 0;
    for(i=0 ; i < m_blkDn_len - 2; i++)
    {
        cs_now += m_blkDn_buf[i];
    }
#endif 
    if( cs_now != cs_pkt)
        return( DN_CHK_CHKSUM_NG );
    
    return(DN_CHK_OK);
}

static int32_t blk_dn_missing_count()
{
    uint16_t i;
    uint16_t missing_blk_cnt;
 
    if(m_blkDn_blkCnt == 0)
        return(0);
   
    missing_blk_cnt = 0;
    for(i=0 ; i < m_blkDn_blkCnt; i++)
    {
        if(m_blkDn_chk[i] == 0x00)
            missing_blk_cnt++;
    }
    return(missing_blk_cnt);
}

static int32_t blk_dn_get_missing(uint8_t* buf, uint8_t count)
{
    uint16_t i;
    uint16_t missing_blk_cnt;
 
    if(m_blkDn_blkCnt == 0)
        return(0);
   
    missing_blk_cnt = 0;
    for(i=0 ; i < m_blkDn_blkCnt; i++)
    {
        if(m_blkDn_chk[i] == 0x00)
        {
            missing_blk_cnt++;
            if( missing_blk_cnt <= count )
            {
                buf[missing_blk_cnt-1] = (uint8_t)i;
            }
            if( missing_blk_cnt == count )
                break;
        }
    }
    return(missing_blk_cnt);
}



//-----------------------------------------------------------------------------
//
//  Timer
//
//-----------------------------------------------------------------------------
static void BlkDn_timeout_handler(void * p_context);
#include "myapp.h"


APP_TIMER_DEF(m_BlkDn_timer_id);


//static uint32_t m_app_ticks_per_100ms;
#define BSP_MS_TO_TICK(MS) (m_app_ticks_per_100ms * (MS / 100))

uint32_t BlkDn_timer_init(void)
{
    uint32_t err_code;

    // Create timer
    err_code = app_timer_create(&m_BlkDn_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT, //APP_TIMER_MODE_REPEATED,
                                BlkDn_timeout_handler);
    if (err_code != NRF_SUCCESS)
    {
        ;
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}

uint32_t BlkDn_timer_start(uint32_t timeout_ticks)
{
    uint32_t err_code;

    err_code = app_timer_stop(m_BlkDn_timer_id);
    err_code = app_timer_start(m_BlkDn_timer_id, timeout_ticks, NULL);
    if (err_code != NRF_SUCCESS)
    {
        ;
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}

/*static*/ uint32_t BlkDn_timer_stop(void)
{
    uint32_t err_code;
    // Stop timer
    err_code = app_timer_stop(m_BlkDn_timer_id);
    if (err_code != NRF_SUCCESS)
    {
        ;
    }
    //APP_ERROR_CHECK(err_code);
    return(err_code);
}

//-----------------------------------------------------------------------------
//
//  Controls
//
//-----------------------------------------------------------------------------

static uint32_t blk_dn_startSend_Dcfm_OK()
{
    uint32_t err_code;
    
    m_Dcfm_buf[0] = 1;
    m_Dcfm_buf[1] = 0; // 0 = OK
    m_Dcfm_len = 2;
    
    // should be this mode m_BlkDn_sm = eBlkDn_GOT_PACKET;
    
    err_code = ble_tuds_notify_Dcfm( &m_ble_tuds, m_Dcfm_buf, &m_Dcfm_len);
    if(err_code == NRF_SUCCESS)
    {
        dbgPrint("o");
        m_BlkDn_sm = eBlkDn_CFM_SEND;
        BlkDn_timer_start( APP_TIMER_TICKS(222, APP_TIMER_PRESCALER) );
    }
    else
    if(err_code == BLE_ERROR_NO_TX_PACKETS)
    {
        dbgPrint("n");
        // should be this mode m_BlkDn_sm = eBlkDn_GOT_PACKET;
        BlkDn_timer_start( APP_TIMER_TICKS(222, APP_TIMER_PRESCALER) );
    }
    else
    {
        dbgPrint("x");
    }

    return(err_code);    
}

static uint32_t blk_dn_startSend_Dcfm_NG()
{
    uint32_t err_code;
    
    m_Dcfm_buf[0] = 1;
    m_Dcfm_buf[1] = 1; // 1 = NG (Checksum NG)
    m_Dcfm_len = 2;
    
    // should be this mode m_BlkDn_sm = eBlkDn_GOT_PACKET;
    
    err_code = ble_tuds_notify_Dcfm( &m_ble_tuds, m_Dcfm_buf, &m_Dcfm_len);
    if(err_code == NRF_SUCCESS)
    {
        m_BlkDn_sm = eBlkDn_CFM_SEND;
        BlkDn_timer_start( APP_TIMER_TICKS(222, APP_TIMER_PRESCALER) );
    }
    if(err_code == BLE_ERROR_NO_TX_BUFFERS)
    {
        // should be this mode m_BlkDn_sm = eBlkDn_GOT_PACKET;
        BlkDn_timer_start( APP_TIMER_TICKS(222, APP_TIMER_PRESCALER) );
    }

    return(err_code);    
}

static uint32_t blk_dn_startSend_Dcfm_missing(int max_entries)
{
    uint32_t err_code;
    
    m_Dcfm_buf[0] = 1;
    m_Dcfm_buf[1] = 2; // 2 = Missing
    
    int n = blk_dn_missing_count();
    if( n > max_entries )
        n = max_entries;
    m_Dcfm_buf[2] = n;
    blk_dn_get_missing( &m_Dcfm_buf[3], n );
    m_Dcfm_len = 3 + n;
    
    // should be this mode m_BlkDn_sm = eBlkDn_GOT_PACKET;
    
    err_code = ble_tuds_notify_Dcfm( &m_ble_tuds, m_Dcfm_buf, &m_Dcfm_len);
    if(err_code != NRF_SUCCESS)
    {
        dbgPrint("ble_tuds_notify_Dcfm=<TODO not OK>\r\n");
        //dbgPrint("ble_tuds_notify_Dcfm=%d\r\n", err_code);
    }

    if(err_code == NRF_SUCCESS)
    {
        BlkDn_timer_stop();
        m_BlkDn_sm = eBlkDn_MISSING_SEND;
        BlkDn_timer_start( APP_TIMER_TICKS(222, APP_TIMER_PRESCALER) );
    }
    if(err_code == BLE_ERROR_NO_TX_BUFFERS)
    {
        // should be this mode m_BlkDn_sm = eBlkDn_GOT_PACKET;
        BlkDn_timer_stop();
        m_BlkDn_sm = eBlkDn_MISSING_PRESEND;
        BlkDn_timer_start( APP_TIMER_TICKS(222, APP_TIMER_PRESCALER) );
    }

    return(err_code);    
}

//-----------------------------------------------------------------------------
//
//  Events
//
//-----------------------------------------------------------------------------
void BlkUp_Go_Test(void);

void  BlkUart_Purge(void);
void  BlkUp_Purge(void);
void  BlkDn_Purge(void)
{
    m_BlkDn_sm = eBlkDn_WAIT_CMD;
    BlkUp_Purge();
#if USE_DRCT
    BlkUart_Purge();
#endif
}
//-----------------------------------------------------------------------------
//
//  Events
//
//-----------------------------------------------------------------------------

static int m_BlkDn_packetWaitTimeCount = 0;

int32_t app_tuds_Dcmd_handler(app_tuds_t *p_app_tuds, uint8_t *buf, uint8_t len) //int32_t BlkDn_On_Dcmd( uint8_t *buf, uint8_t len)
{
    int32_t r;
    uint8_t  byteZero;
    app_tuds_evt_t app_tuds_event;

    if( (buf[0] == 1) && (buf[1] == 1) )
    {
        m_BlkDn_packetWaitTimeCount = 0;
        dbgPrint("S");
        r = blk_dn_start(buf);
        m_BlkDn_sm = eBlkDn_WAIT_PACKET;

    }
    if( (buf[0] == 1) && (buf[1] == 2) )
    {
        byteZero = 0;
        
        m_blkDn_len = 3;
        m_blkDn_buf[0] = byteZero;
        m_blkDn_buf[1] = 0x00; // totalCheckSum
        m_blkDn_buf[2] = 0x00; // totalCheckSum
        
    
        app_tuds_event.evt_type = APP_TUDS_RX_PKT_1; // Dcmd[1,2] event
        app_tuds_event.data.value = 42;//dummy
        m_event_handler(&app_tuds_event);    
        
        //callThisWhenBlePacketIsRecieved();
        //m_app_tuds.OnEvent();
        //BlkUp_Go_Test();
    
    
    }
    
    
    if( (buf[0] == 1) && (buf[1] == 3) )
    {
        BlkDn_Purge();
    }
    return(r);
}

int32_t app_tuds_Ddat_handler(app_tuds_t *p_app_tuds, uint8_t *buf, uint8_t len) //int32_t BlkDn_On_Ddat(uint8_t *buf, uint8_t len)
{
    int32_t r;

    BlkDn_timer_stop();
    m_BlkDn_packetWaitTimeCount = 0;
    r = blk_dn_add( buf, len);
    
    r = blk_dn_chk();
    if(r == DN_CHK_OK)
    {
        dbgPrint("O");
        m_BlkDn_sm = eBlkDn_GOT_PACKET;
        blk_dn_startSend_Dcfm_OK();
    }
    else
    if(r == DN_CHK_CHKSUM_NG)
    {
        dbgPrint("N");
        m_BlkDn_sm = eBlkDn_GOT_PACKET;
        blk_dn_startSend_Dcfm_NG();
    }
    else
    {
        dbgPrint("D");
        BlkDn_timer_start( APP_TIMER_TICKS(222, APP_TIMER_PRESCALER) );
    }
    
    return(r);
}

bool m_BlkDn_stateMachineLocked = false;


void BlkDn_lockStateMachine(void)
{
    m_BlkDn_stateMachineLocked = true;
}
void BlkDn_unlockStateMachine(void)
{
    m_BlkDn_stateMachineLocked = false;
}
static void on_blk_dn_packetReceived()
{
    app_tuds_evt_t app_tuds_event;

    // Finally, let the SM accept new packets
    BlkDn_lockStateMachine();
    m_BlkDn_sm = eBlkDn_WAIT_CMD;

#if 0
    BlkUp_Go_Test();
    BlkDn_unlockStateMachine();
#else
    app_tuds_event.evt_type = APP_TUDS_RX_PKT_0; // Dcmd[1,1] ... packet received event
    app_tuds_event.data.value = 42;//dummy
    m_event_handler(&app_tuds_event);    
    //callThisWhenBlePacketIsRecieved();
#endif
}

int32_t app_tuds_OnWrittenComplete_Dcfm_handler(app_tuds_t *p_app_tuds,  uint8_t *buf, uint8_t len) //int32_t BlkDn_On_written_Dcfm(ble_tuds_t *p_tuds,  uint8_t *buf, uint8_t len)
{
    int32_t r;

    // Check if in proper state
    if( (m_BlkDn_sm != eBlkDn_CFM_SEND) &&  (m_BlkDn_sm != eBlkDn_MISSING_SEND)   )
        return(0);

    dbgPrint("C");
    dbgPrint("C");

    BlkDn_timer_stop();
    if( m_BlkDn_sm == eBlkDn_CFM_SEND)
    {
        m_BlkDn_sm = eBlkDn_CFM_SENT; 
        on_blk_dn_packetReceived();
    }
    if( m_BlkDn_sm == eBlkDn_MISSING_SEND)
    {
        m_BlkDn_sm = eBlkDn_WAIT_PACKET;
        BlkDn_timer_start( APP_TIMER_TICKS(222, APP_TIMER_PRESCALER) );
    }
    return(r);
}

static void blk_dn_printState(void)
{
    if(m_BlkDn_sm == eBlkDn_WAIT_CMD       ) dbgPrint("[W_C]");
    if(m_BlkDn_sm == eBlkDn_WAIT_PACKET    ) dbgPrint("[W_P]");
    if(m_BlkDn_sm == eBlkDn_MISSING_PRESEND) dbgPrint("[M_P]");
    if(m_BlkDn_sm == eBlkDn_MISSING_SEND   ) dbgPrint("[M_S]");
    if(m_BlkDn_sm == eBlkDn_GOT_PACKET     ) dbgPrint("[G_P]");
    if(m_BlkDn_sm == eBlkDn_CFM_PRESEND    ) dbgPrint("[C_P]"); // set before CFM packet sent
    if(m_BlkDn_sm == eBlkDn_CFM_SEND       ) dbgPrint("[C_S]"); // set after CFM packet sent
    if(m_BlkDn_sm == eBlkDn_CFM_SENT       ) dbgPrint("[C_T]"); // set after CFM packet sent (Write Done or ACK) received
}

static void BlkDn_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
#if 0
    dbgPrint("*");
#else    
    dbgPrint("*");
    blk_dn_printState();
    if(m_BlkDn_sm == eBlkDn_WAIT_CMD) // Do nothing
    { } //Just waiting for a start up packet
    else
    if(m_BlkDn_sm == eBlkDn_WAIT_PACKET) // Should have got the next data packet by now
    {
        if(++m_BlkDn_packetWaitTimeCount > 3)
        {
            m_BlkDn_packetWaitTimeCount = 0;
            m_BlkDn_sm = eBlkDn_WAIT_CMD; // Reset
        }
        else
        {
            dbgPrint("[WP]");
            blk_dn_startSend_Dcfm_missing(4);
        }
    }
    else
    if(m_BlkDn_sm == eBlkDn_GOT_PACKET) // Do nothing
    { } // Just got the packet so I should have cancelled the timer
    else
    if(m_BlkDn_sm == eBlkDn_MISSING_PRESEND) // buffer was full last time, so try again
    {
            dbgPrint("[MPre]");
            blk_dn_startSend_Dcfm_missing(4);
    }
    else
    if(m_BlkDn_sm == eBlkDn_MISSING_SEND) // we didn't get a write done
    {
            dbgPrint("[MSend]");
            blk_dn_startSend_Dcfm_missing(4);
    }
    else
    if(m_BlkDn_sm == eBlkDn_CFM_PRESEND) // buffer was full last time, so try again
    {
        blk_dn_startSend_Dcfm_OK();
    }
    else
    if(m_BlkDn_sm == eBlkDn_CFM_SEND) // we didn't get a write done
    {
        blk_dn_startSend_Dcfm_OK();
    }
    else
    if(m_BlkDn_sm == eBlkDn_CFM_SENT)
    {
    }
    else
    {
    }
#endif
}


//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
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
        BlkUp_timer_start( APP_TIMER_TICKS(224, APP_TIMER_PRESCALER) );
    }
    else
    if(err_code == BLE_ERROR_NO_TX_BUFFERS)
    {
        dbgPrint("n");
        // should be this mode m_BlkUp_sm = eBlkDn_GOT_PACKET;
        BlkUp_timer_start( APP_TIMER_TICKS(224, APP_TIMER_PRESCALER) );
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
        BlkUp_timer_start( APP_TIMER_TICKS(224, APP_TIMER_PRESCALER) );
    }
    else
    if(err_code == BLE_ERROR_NO_TX_BUFFERS)
    {
        dbgPrint("n");
        // should be this mode m_BlkUp_sm = eBlkDn_GOT_PACKET;
        BlkUp_timer_start( APP_TIMER_TICKS(224, APP_TIMER_PRESCALER) );
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
            BlkUp_timer_start( APP_TIMER_TICKS(224, APP_TIMER_PRESCALER) );
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
            BlkUp_timer_start( APP_TIMER_TICKS(224, APP_TIMER_PRESCALER) );
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




//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================


#else // #if !SPLIT_UP_DN //SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN


// -- app_tuds_d.h --
int32_t app_tuds_Dcmd_handler(app_tuds_t *p_app_tuds, uint8_t *buf, uint8_t len);
int32_t app_tuds_Ddat_handler(app_tuds_t *p_app_tuds, uint8_t *buf, uint8_t len);

int32_t app_tuds_OnWrittenComplete_Dcfm_handler(app_tuds_t *p_app_tuds,  uint8_t *buf, uint8_t len);

uint32_t BlkDn_timer_init(void);


// -- app_tuds_u.h --
int32_t app_tuds_Ucfm_handler(app_tuds_t *p_app_tuds, uint8_t *buf, uint8_t len);

int32_t app_tuds_OnWrittenComplete_Ucmd_handler(app_tuds_t *p_app_tuds,  uint8_t *buf, uint8_t len);
int32_t app_tuds_OnWrittenComplete_Udat_handler(app_tuds_t *p_app_tuds,  uint8_t *buf, uint8_t len);

uint32_t BlkUp_timer_init(void);


// -- app_tuds.h --
void tuds_Dcmd_data_handler(ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length);
void tuds_Ddat_data_handler(ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length);
void tuds_Ucfm_data_handler(ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length);
void tuds_tx_complete_handler(ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt);


#endif // #if !SPLIT_UP_DN //SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN SPLIT_UP_DN

app_tuds_t          m_app_tuds;

static void tuds_Dcmd_data_handler(ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length)
{
    //int32_t r;
    //r = 
    app_tuds_Dcmd_handler(&m_app_tuds, p_data, length); //BlkDn_On_Dcmd(p_data, length);
    //
    //BlkDn_On_Dcmd(p_data, length);
}

static void tuds_Ddat_data_handler(ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length)
{
    //int32_t r;
    //r = 
    app_tuds_Ddat_handler(&m_app_tuds, p_data, length); //BlkDn_On_Ddat(p_data, length);
    //
    //BlkDn_On_Ddat(p_data, length);
}

static void tuds_tx_complete_handler(ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt)
{
    app_tuds_OnWrittenComplete_Dcfm_handler(&m_app_tuds, 0, 0); // BlkDn_On_written_Dcfm(p_tuds, 0, 0);

    app_tuds_OnWrittenComplete_Ucmd_handler(&m_app_tuds, 0, 0); // BlkUp_On_written_Ucmd(p_tuds, 0, 0);
    app_tuds_OnWrittenComplete_Udat_handler(&m_app_tuds, 0, 0); // BlkUp_On_written_Udat(p_tuds, 0, 0);
}


// UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP
static void tuds_Ucfm_data_handler(ble_tuds_t * p_tuds, uint8_t * p_data, uint16_t length)
{
    app_tuds_Ucfm_handler(&m_app_tuds, p_data, length); // BlkUp_On_Ucfm(p_data, length);
    //
    //BlkUp_On_Ucfm(p_data, length);
}








//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
uint32_t  app_tuds_init(app_tuds_t * p_app_tuds, const app_tuds_init_t * p_app_tuds_init);
uint32_t  app_tuds_init(app_tuds_t * p_app_tuds, const app_tuds_init_t * p_app_tuds_init)
{
    //p_app_tuds->event_handler = p_app_tuds_init->event_handler;
    p_app_tuds->packet_handler = p_app_tuds_init->packet_handler;
    p_app_tuds->p_ble_tuds = p_app_tuds_init->p_ble_tuds;
    
    return(0);    
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static void main_app_tuds_packet_handler( app_tuds_t * p_ma_tuds, uint8_t * p_data, uint16_t length)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static void main_app_tuds_event_handler(app_tuds_evt_t * p_app_tuds_event)
{
    callThisWhenBlePacketIsRecieved(p_app_tuds_event);
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
#include "myapp.h"
#include "app_tuds.h"



void app_tuds_Init(void);
void app_tuds_on_ble_evt(ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt);

uint32_t timers_init_tuds_part(void)
{
    uint32_t rv;
    
    rv = BlkDn_timer_init();
    if( rv == 0 )
    {
        rv = BlkUp_timer_init();
    }
    return(rv);
}


void services_init_tuds_part(void)
{

    uint32_t                   err_code;

    //--------------------------------------------------------
    ble_tuds_init_t   tuds_init;    
    memset(&tuds_init, 0, sizeof(tuds_init));

    tuds_init.Ddat_handler = tuds_Ddat_data_handler;
    tuds_init.Dcmd_handler = tuds_Dcmd_data_handler;
    tuds_init.tx_complete_handler = tuds_tx_complete_handler;

    tuds_init.Ucfm_handler = tuds_Ucfm_data_handler;
    
    err_code = ble_tuds_init(&m_ble_tuds, &tuds_init);

    //rn_BlkDn_timer_Init();//APP_TIMER_TICKS(100, APP_TIMER_PRESCALER));
    //rn_BlkUp_timer_Init();//APP_TIMER_TICKS(100, APP_TIMER_PRESCALER));
    APP_ERROR_CHECK(err_code);


    //--------------------------------------------------------
    m_event_handler = main_app_tuds_event_handler; // NOTE42: m_event_handler is used.
    
    app_tuds_init_t   _tuds_init;    
    memset(&_tuds_init, 0, sizeof(_tuds_init));

    _tuds_init.packet_handler = main_app_tuds_packet_handler;
    _tuds_init.event_handler = main_app_tuds_event_handler;    // NOTE42: this event_handler is not used, m_event_handler is used.
    _tuds_init.p_ble_tuds = &m_ble_tuds;

    err_code = app_tuds_init( &m_app_tuds, &_tuds_init);    
    APP_ERROR_CHECK(err_code);
    
}
/*
void tuds_service_init()
{
    uint32_t                   err_code;

    ble_tuds_init_t   tuds_init;    
    memset(&tuds_init, 0, sizeof(tuds_init));

    tuds_init.Ddat_handler = tuds_Ddat_data_handler;
    tuds_init.Dcmd_handler = tuds_Dcmd_data_handler;
    tuds_init.tx_complete_handler = tuds_tx_complete_handler;

    tuds_init.Ucfm_handler = tuds_Ucfm_data_handler;
    
    err_code = ble_tuds_init(&m_ble_tuds, &tuds_init);

    //rn_BlkDn_timer_Init();//APP_TIMER_TICKS(100, APP_TIMER_PRESCALER));
    //rn_BlkUp_timer_Init();//APP_TIMER_TICKS(100, APP_TIMER_PRESCALER));
    
    //APP_ERROR_CHECK(err_code);
}
*/
void application_timers_start_tuds_part(void)
{
}




/*
void ma_tuds_init(void)
{
    uint32_t                   err_code;

    app_tuds_init_t   ma_tuds_init;    
    memset(&ma_tuds_init, 0, sizeof(ma_tuds_init));

    tuds_service_init();
    
    //ma_tuds_init.p_tuds = &m_ble_tuds;
    ma_tuds_init.packet_handler = 0;
    
    err_code = app_tuds_init(&m_app_tuds, &ma_tuds_init);

    //APP_ERROR_CHECK(err_code);
}

void ma_tuds_on_ble_evt(ble_tuds_t * p_tuds, ble_evt_t * p_ble_evt)
{
    ble_tuds_on_ble_evt(p_tuds, p_ble_evt); // ble_tuds_on_ble_evt(&m_ble_tuds, p_ble_evt);
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



void tuds_event_handle(app_tuds_evt_t * p_event)
{
    static uniEvent_t LEvt;

    switch (p_event->evt_type)
    {
        case APP_TUDS_RX_START_PKT_0:
           // app_tuds_send_noBufferBusy(m_app_tuds);
            break;
        
        case APP_TUDS_RX_DONE_PKT_0:
            // TODO PKT_0 goes to UART
        //TODO    LEvt.evtType = evt_GHJGJHGHGH_trigger;
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
    m_app_tuds.p_ble_tuds = &m_ble_tuds;
    
}

void ma_app_tuds_Deinit() //karel
{
    //dbgPrint("***** ma_app_tuds_Deinit *****\r\n");

	//app_tuds_close();
}


app_tuds_event_handler_t    m_event_handler;

uint32_t app_tuds_init__uarttype(//const app_uart_comm_params_t * p_comm_params,
                       //      app_uart_buffers_t *     p_buffers,
                             app_tuds_event_handler_t event_handler
                       //      app_irq_priority_t       irq_priority
                      )
{
    uint32_t err_code;

    m_event_handler = event_handler;

    return 0;
}
*/

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


