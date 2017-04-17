
#include "debug_etc.h"


#include "app_util_platform.h" //identifier "APP_IRQ_PRIORITY_LOW"
#include "nrf_drv_spi.h"




#include "ma_timers.h" // for APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE

#define DELAY_MS                 1000                   ///< Timer Delay in milli-seconds.

/** @def  TX_RX_MSG_LENGTH
 * number of bytes to transmit and receive. This amount of bytes will also be tested to see that
 * the received bytes from slave are the same as the transmitted bytes from the master */
#define TX_RX_MSG_LENGTH         100
typedef enum
{
    #if (SPI0_ENABLED == 1)
    TEST_STATE_SPI0_LSB,    ///< Test SPI0, bits order LSB
    TEST_STATE_SPI0_MSB,    ///< Test SPI0, bits order MSB
    #endif
    /*
    #if (SPI1_ENABLED == 1)
    TEST_STATE_SPI1_LSB,    ///< Test SPI1, bits order LSB
    TEST_STATE_SPI1_MSB,    ///< Test SPI1, bits order MSB
    #endif
    #if (SPI2_ENABLED == 1)
    TEST_STATE_SPI2_LSB,    ///< Test SPI2, bits order LSB
    TEST_STATE_SPI2_MSB,    ///< Test SPI2, bits order MSB
    #endif
    */
    END_OF_TEST_SEQUENCE
} spi_master_ex_state_t;

#if USE_PRINTF_OVER_SDO
static uint8_t m_tx_data_spi[TX_RX_MSG_LENGTH]; ///< SPI master TX buffer.
static uint8_t m_rx_data_spi[TX_RX_MSG_LENGTH]; ///< SPI master RX buffer.


static volatile bool m_transfer_completed = true;
//static spi_master_ex_state_t m_spi_master_ex_state = (spi_master_ex_state_t)0;

#if (SPI0_ENABLED == 1)
static const nrf_drv_spi_t m_spi_master_0 = NRF_DRV_SPI_INSTANCE(0);
#endif
/*
#if (SPI1_ENABLED == 1)
static const nrf_drv_spi_t m_spi_master_1 = NRF_DRV_SPI_INSTANCE(1);
#endif
#if (SPI2_ENABLED == 1)
static const nrf_drv_spi_t m_spi_master_2 = NRF_DRV_SPI_INSTANCE(2);
#endif
*/

#include "ma_utils.h"
#define SPIM_BUF_SIZE 1024
static uint8_t  m_sdo_Buf[SPIM_BUF_SIZE + (sizeof(cb16_t) - 1 )];
static cb16_t  *m_sdo_Cb = (cb16_t *)m_sdo_Buf;
static bool     m_sdo_RTS = true;
static bool     m_sdo_isInitialised = false;
#endif


#define KS_TX_RX_MSG_LENGTH         8 //100

//static bool  skip_pop = false;

void sdo_next(void);

//void sdo_master_0_event_handler(nrf_drv_spi_event_t event) //  <= V10
void sdo_master_0_event_handler(nrf_drv_spi_evt_t const * p_event) //  <= V11                            
{    
#if (USE_PRINTF_OVER_SDO == 0)
    return;    
#else
    //uint32_t err_code = NRF_SUCCESS;
    //bool result = false;

    nrf_drv_spi_evt_type_t event; //V11
    event = p_event->type; //V11
    
    switch (event)
    {
        case NRF_DRV_SPI_EVENT_DONE:
            m_sdo_RTS = true;
        
            //if( skip_pop == false)
                sdo_next();
            /*
            // Check if received data is correct.
            result = check_buf_equal(m_tx_data_spi, m_rx_data_spi, KS_TX_RX_MSG_LENGTH);
            //karel APP_ERROR_CHECK_BOOL(result);

            nrf_drv_spi_uninit(&m_spi_master_0);

            err_code = bsp_indication_set(BSP_INDICATE_RCV_OK);
            APP_ERROR_CHECK(err_code);

            m_transfer_completed = true;
            */
            break;

        default:
            // No implementation needed.
            break;
    }
#endif

}



void sdo_Init(void)
{
#if (USE_PRINTF_OVER_SDO == 0)
    return;    
#else
    nrf_drv_spi_t const * p_instance;

    if( m_sdo_isInitialised == true)
        return;
    m_sdo_isInitialised = true;

#if (SPI0_ENABLED == 1)
    p_instance = &m_spi_master_0;
    
    uint32_t err_code = NRF_SUCCESS;

    nrf_drv_spi_config_t config =
    {
        .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,
        .irq_priority = APP_IRQ_PRIORITY_LOW,
        .orc          = 0xCC,
        .frequency    = NRF_DRV_SPI_FREQ_1M,
      //.frequency    = NRF_DRV_SPI_FREQ_2M,
        .mode         = NRF_DRV_SPI_MODE_0,
      //.mode         = NRF_DRV_SPI_MODE_3,
      //.bit_order    = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST,
        .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,
    };


    config.sck_pin  = 29; //karel SPIM0_SCK_PIN;
    config.mosi_pin = 25; //karel SPIM0_MOSI_PIN;
    config.miso_pin = 28; //karel SPIM0_MISO_PIN;
    err_code = nrf_drv_spi_init(p_instance, &config, sdo_master_0_event_handler);
    APP_ERROR_CHECK(err_code);

#endif
    
    m_sdo_Cb = (cb16_t *)m_sdo_Buf;
    m_sdo_Cb->buffer = m_sdo_Cb->autoBuf;
    m_sdo_Cb->capacity = sizeof(m_sdo_Buf) - sizeof(cb16_t) + 1;
    m_sdo_Cb->rdPtr = 0;
    m_sdo_Cb->wrPtr = 0;
    m_sdo_Cb->length = 0;
#endif
   
}

void sdo_Deinit(void)
{
#if (USE_PRINTF_OVER_SDO == 0)
    return;
#else
    nrf_drv_spi_uninit(&m_spi_master_0);
    m_sdo_isInitialised = false;
#endif
}

void sdo_test(void)
{
#if (USE_PRINTF_OVER_SDO == 0)
    return;
#else
    nrf_drv_spi_t const * p_instance;
    #if (SPI0_ENABLED == 1)
    p_instance = &m_spi_master_0;
    #endif
    
    uint16_t i;
    uint16_t  len = KS_TX_RX_MSG_LENGTH;
    for (i = 0; i < len; i++)
    {
        m_tx_data_spi[i] = 10+i;
        m_rx_data_spi[i] = 0;
    }
   
    uint32_t err_code = nrf_drv_spi_transfer(p_instance, m_tx_data_spi, len, m_rx_data_spi, len);
    APP_ERROR_CHECK(err_code);
#endif
}


void sdo_next(void)
{
#if (USE_PRINTF_OVER_SDO == 0)
    return;
#else
    nrf_drv_spi_t const * p_instance;
    p_instance = &m_spi_master_0;

    uint8_t data;
    int32_t dataCount = cb16_count(m_sdo_Cb);
    
    if( dataCount == 0 )
        return;
    if( m_sdo_RTS == false )
        return;
    m_sdo_RTS = false;

//nrf_gpio_pin_toggle(BSP_LED_3);
    
    if( dataCount < (KS_TX_RX_MSG_LENGTH - 1) )
        m_tx_data_spi[0] = (uint8_t)dataCount;
    else
    {
        m_tx_data_spi[0] = (uint8_t)(KS_TX_RX_MSG_LENGTH - 1);
        dataCount = (KS_TX_RX_MSG_LENGTH - 1);
    }
    
    uint16_t i;
    /*A*/uint16_t  len = dataCount+1; //KS_TX_RX_MSG_LENGTH;
    /*B*///uint16_t  len = KS_TX_RX_MSG_LENGTH;
    for (i = 1; i < len; i++)
    {
        cb16_pop(m_sdo_Cb, &data);
        m_tx_data_spi[i] = data;
        m_rx_data_spi[i] = 0;
    }
   
    /*A*/uint32_t err_code = nrf_drv_spi_transfer(p_instance, m_tx_data_spi, KS_TX_RX_MSG_LENGTH, m_rx_data_spi, KS_TX_RX_MSG_LENGTH);//len);
    /*B*///uint32_t err_code = nrf_drv_spi_transfer(p_instance, m_tx_data_spi, len, m_rx_data_spi, len);
    APP_ERROR_CHECK(err_code);
//nrf_gpio_pin_toggle(BSP_LED_3);
#endif

}

#include <string.h>

int sdo_AppendText( const char * s )
{
#if (USE_PRINTF_OVER_SDO == 0)
    return(0);
#else
    //int32_t r;
//nrf_gpio_pin_toggle(BSP_LED_3);
    int len = strlen( s );
//nrf_gpio_pin_toggle(BSP_LED_3);

    //skip_pop = true;
    // https://devzone.nordicsemi.com/question/39453/how-to-enable-and-disable-all-interrupts/
    // ...  You can use the sd_nvic_critical_region_enter()/exit() functions to turn off all interrupts the softdevice isn't using. That is the most you can do. 
    __disable_irq();
    /*r = */cb16_push_n( m_sdo_Cb, (uint8_t*)s, len );
    __enable_irq();
    //m_sdo_RTS = true;
    //skip_pop = false;
    /*
    while( len-- > 0)
    {
        r = cb16_push( m_sdo_Cb, (uint8_t)*s++ );
        if( r == 0 )
            break;
    }
    */
//nrf_gpio_pin_toggle(BSP_LED_3);    
    sdo_next();
//nrf_gpio_pin_toggle(BSP_LED_3);
    return(0);
#endif
}

/* TESTING
int sdo_AppendText_( const char * s )
{
#if (USE_PRINTF_OVER_SDO == 0)
return(0);    
#endif
    int32_t r;
    int len = strlen( s );
    while( len-- > 0)
    {
        r = cb16_push( m_sdo_Cb, (uint8_t)*s++ );
        if( r == 0 )
            break;
    }
    
    sdo_next();
}
*/

