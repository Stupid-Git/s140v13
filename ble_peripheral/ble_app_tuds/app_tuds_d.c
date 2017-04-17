
#include "ble_tuds.h"

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

#include "app_tuds.h"

#if SPLIT_UP_DN

//-----------------------------------------------------------------------------
#define APP_TIMER_PRESCALER 0
#define BLE_ERROR_NO_TX_BUFFERS BLE_ERROR_NO_TX_PACKETS

//#define APP_TIMER_DEF(timer_id)                                  \
//    static app_timer_t timer_id##_data = { {0} };                  \
//    static const app_timer_id_t timer_id = &timer_id##_data
		
//static app_timer_id_t         m_Dn_timer_id;  //LNEW
// static app_timer_t m_Dn_timer_id_data = { {0} };  //LNEW
// static const app_timer_id_t m_Dn_timer_id = &m_Dn_timer_id_data;  //LNEW

//-----------------------------------------------------------------------------
extern ble_tuds_t  m_ble_tuds;                                  // Structure to identify the BLKUP Service.


static uint8_t  m_Dcfm_buf[20];
static uint16_t m_Dcfm_len;


//-----------------------------------------------------------------------------
void BlkDn_unlockStateMachine(void);


//=============================================================================
//=============================================================================
//=============================================================================
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//===== DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN ======
//=============================================================================
//=============================================================================
//=============================================================================




/* ----------------------------------------------------------------------------
*  BSS
*/

#define BLK_DN_COUNT (128 + 8)

uint8_t  m_blkDn_buf[ 16 * BLK_DN_COUNT ]; // 2048 @ BLK_DN_COUNT = 128
uint8_t  m_blkDn_chk[  1 * BLK_DN_COUNT ]; //  128 @ BLK_DN_COUNT = 128
uint16_t m_blkDn_len;
uint16_t m_blkDn_blkCnt;
uint16_t m_blkDn_rxBlkCnt;


//extern app_tuds_t                  m_app_tuds;
extern app_tuds_event_handler_t    m_event_handler;
//app_tuds_evt_t

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
uint32_t BlkDn_timer_init(void);
uint32_t BlkDn_timer_start(uint32_t timeout_ticks); //, void* p_context);
uint32_t BlkDn_timer_stop(void);
static void BlkDn_timeout_handler(void * p_context);
//#include "myapp.h"



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
        BlkDn_timer_start( APP_TT_222 );
    }
    else
    if(err_code == BLE_ERROR_NO_TX_PACKETS)
    {
        dbgPrint("n");
        // should be this mode m_BlkDn_sm = eBlkDn_GOT_PACKET;
        BlkDn_timer_start( APP_TT_222 );
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
        BlkDn_timer_start( APP_TT_222 );
    }
    if(err_code == BLE_ERROR_NO_TX_BUFFERS)
    {
        // should be this mode m_BlkDn_sm = eBlkDn_GOT_PACKET;
        BlkDn_timer_start( APP_TT_222 );
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
        dbgPrint("ble_tuds_notify_Dcfm=<TODO not OK>\n\r");
        //dbgPrint("ble_tuds_notify_Dcfm=%d\n\r", err_code);
    }

    if(err_code == NRF_SUCCESS)
    {
        BlkDn_timer_stop();
        m_BlkDn_sm = eBlkDn_MISSING_SEND;
        BlkDn_timer_start( APP_TT_222 );
    }
    if(err_code == BLE_ERROR_NO_TX_BUFFERS)
    {
        // should be this mode m_BlkDn_sm = eBlkDn_GOT_PACKET;
        BlkDn_timer_stop();
        m_BlkDn_sm = eBlkDn_MISSING_PRESEND;
        BlkDn_timer_start( APP_TT_222 );
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
        BlkDn_timer_start( APP_TT_222 );
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
        BlkDn_timer_start( APP_TT_222 );
    }
    return(r);
}
//-----------------------------------------------------------------------------
//
//  Timer
//
//-----------------------------------------------------------------------------

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


#endif //#if SPLIT_UP_DN
