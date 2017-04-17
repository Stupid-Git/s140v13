

#include "myapp.h"

#include "nrf_gpio.h" // nrf_gpio_cfg_output etc

#define WAKE_PIN  20
#define WAKE_PIN_MASK (1<<WAKE_PIN)

//#define WAKE_PIN_INV_MASK  WAKE_PIN_MASK

#define WAKES_MASK  WAKE_PIN_MASK

#if WAKE_ACTIVE_HIGH
#define WAKE_PIN_ON(pin_mask) do {  NRF_GPIO->OUTSET = (pin_mask) & (WAKES_MASK &  WAKE_PIN_MASK); \
                                     NRF_GPIO->OUTCLR = (pin_mask) & (WAKES_MASK & ~WAKE_PIN_MASK); } while (0)

#define WAKE_PIN_OFF(pin_mask)  do {  NRF_GPIO->OUTCLR = (pin_mask) & (WAKES_MASK &  WAKE_PIN_MASK); \
                                     NRF_GPIO->OUTSET = (pin_mask) & (WAKES_MASK & ~WAKE_PIN_MASK); } while (0)

#define WAKE_PIN_IS_ON(pin_mask) (! ((pin_mask) & (NRF_GPIO->OUT ^ WAKE_PIN_MASK) ) )

#else
#define WAKE_PIN_OFF(pin_mask) do {  NRF_GPIO->OUTSET = (pin_mask) & (WAKES_MASK &  WAKE_PIN_MASK); \
                                     NRF_GPIO->OUTCLR = (pin_mask) & (WAKES_MASK & ~WAKE_PIN_MASK); } while (0)

#define WAKE_PIN_ON(pin_mask)  do {  NRF_GPIO->OUTCLR = (pin_mask) & (WAKES_MASK &  WAKE_PIN_MASK); \
                                     NRF_GPIO->OUTSET = (pin_mask) & (WAKES_MASK & ~WAKE_PIN_MASK); } while (0)

#define WAKE_PIN_IS_ON(pin_mask) ((pin_mask) & (NRF_GPIO->OUT ^ WAKE_PIN_MASK) )
#endif
                                     
// from #define LEDS_INVERT(leds_mask) ...
#define WAKE_PIN_INVERT(pin_mask) do { uint32_t gpio_state = NRF_GPIO->OUT;      \
                                        NRF_GPIO->OUTSET = ((pin_mask) & ~gpio_state); \
                                        NRF_GPIO->OUTCLR = ((pin_mask) & gpio_state); } while (0)

// from #define LEDS_CONFIGURE(leds_mask) ...
#define WAKE_PIN_CONFIGURE(pin_mask) do { uint32_t pin;                  \
                                           for (pin = 0; pin < 32; pin++) \
                                               if ( (pin_mask) & (1 << pin) )   \
                                                   nrf_gpio_cfg_output(pin); } while (0)

void pinWakeUp_Init(void)
{
    WAKE_PIN_CONFIGURE(WAKE_PIN_MASK);
    
    WAKE_PIN_OFF(WAKE_PIN_MASK);
    NRF_GPIO->DIRSET = WAKE_PIN_MASK;

}

void pinWakeUp_Assert(void)
{
    WAKE_PIN_ON(WAKE_PIN_MASK);
}

void pinWakeUp_Release(void)
{
    WAKE_PIN_OFF(WAKE_PIN_MASK);
}

bool pinWakeUp_IsAsserted(void)
{
    return( WAKE_PIN_IS_ON(WAKE_PIN_MASK) );
}

void pinWakeUp_Deinit(void)
{
   //TODO nrf_gpio_cfg_default(WAKE_PIN);
}
    

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#define TUG_PIN25  17 // P0.17 is pin25
#define TUG_PIN26  18 // P0.18 is pin26

#define TUG_PIN25_MASK (1<<TUG_PIN25)
#define TUG_PIN26_MASK (1<<TUG_PIN26)

#define TUG_PINS_MASK ( TUG_PIN25_MASK | TUG_PIN26_MASK )

#define TUG_MASK  TUG_PINS_MASK


#define TUG_PINS_ON(pin_mask)   do {  NRF_GPIO->OUTSET = (pin_mask) & (TUG_MASK &  TUG_PINS_MASK); \
                                      NRF_GPIO->OUTCLR = (pin_mask) & (TUG_MASK & ~TUG_PINS_MASK); } while (0)

#define TUG_PINS_OFF(pin_mask)  do {  NRF_GPIO->OUTCLR = (pin_mask) & (TUG_MASK &  TUG_PINS_MASK); \
                                      NRF_GPIO->OUTSET = (pin_mask) & (TUG_MASK & ~TUG_PINS_MASK); } while (0)

#define TUG_PINS_IS_ON(pin_mask) ((pin_mask) & (NRF_GPIO->OUT ^ TUG_PINS_MASK) )

#define TUG_PINS_INVERT(pin_mask) do { uint32_t gpio_state = NRF_GPIO->OUT;      \
                                        NRF_GPIO->OUTSET = ((pin_mask) & ~gpio_state); \
                                        NRF_GPIO->OUTCLR = ((pin_mask) & gpio_state); } while (0)

#define TUG_PINS_CONFIGURE(pin_mask) do { uint32_t pin;                          \
                                          for (pin = 0; pin < 32; pin++)         \
                                              if ( (pin_mask) & (1 << pin) )     \
                                                  nrf_gpio_cfg_output(pin); } while (0)


void pinsTUG_Init(void)
{
    TUG_PINS_CONFIGURE(TUG_PINS_MASK);

    TUG_PINS_OFF(TUG_PINS_MASK);   //!!! WAKE_PIN_OFF(TUG_PINS_MASK);
    NRF_GPIO->DIRSET = TUG_PINS_MASK;
}

void pinsTUG_Deinit(void)
{
   //TODO nrf_gpio_cfg_default(BATT_ON_PIN);
   //TODO nrf_gpio_cfg_default(BATT_LD_PIN);
}

void pinsTUG_25_Assert(void){
    TUG_PINS_ON(TUG_PIN25_MASK);
}
void pinsTUG_25_Release(void){
    TUG_PINS_OFF(TUG_PIN25_MASK);
}
void pinsTUG_25_Invert(void){
    TUG_PINS_INVERT(TUG_PIN25_MASK);
}

void pinsTUG_26_Assert(void){
    TUG_PINS_ON(TUG_PIN26_MASK);
}
void pinsTUG_26_Release(void){
    TUG_PINS_OFF(TUG_PIN26_MASK);
}




//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#define BATT_ON_PIN  2
#define BATT_LD_PIN  3

#define BATT_ON_PIN_MASK (1<<BATT_ON_PIN)
#define BATT_LD_PIN_MASK (1<<BATT_LD_PIN)

#define BATT_PINS_MASK ( BATT_ON_PIN_MASK | BATT_LD_PIN_MASK )

//#define BATT_PINS_INV_MASK  BATT_PINS_MASK

#define BAT_MASK  BATT_PINS_MASK



// from #define LEDS_OFF(leds_mask) ...
#define BATT_PIN_ON(pin_mask)   do {  NRF_GPIO->OUTSET = (pin_mask) & (BAT_MASK &  BATT_PINS_MASK); \
                                      NRF_GPIO->OUTCLR = (pin_mask) & (BAT_MASK & ~BATT_PINS_MASK); } while (0)

// from #define LEDS_ON(leds_mask) ...
#define BATT_PIN_OFF(pin_mask)  do {  NRF_GPIO->OUTCLR = (pin_mask) & (BAT_MASK &  BATT_PINS_MASK); \
                                      NRF_GPIO->OUTSET = (pin_mask) & (BAT_MASK & ~BATT_PINS_MASK); } while (0)

// from #define LED_IS_ON(leds_mask) ...
#define BAT_PIN_IS_ON(pin_mask) ((pin_mask) & (NRF_GPIO->OUT ^ BATT_PINS_MASK) )

// from #define LEDS_INVERT(leds_mask) ...
#define BATT_PIN_INVERT(pin_mask) do { uint32_t gpio_state = NRF_GPIO->OUT;      \
                                        NRF_GPIO->OUTSET = ((pin_mask) & ~gpio_state); \
                                        NRF_GPIO->OUTCLR = ((pin_mask) & gpio_state); } while (0)

// from #define LEDS_CONFIGURE(leds_mask) ...
#define BATT_PIN_CONFIGURE(pin_mask) do { uint32_t pin;                          \
                                          for (pin = 0; pin < 32; pin++)         \
                                              if ( (pin_mask) & (1 << pin) )     \
                                                  nrf_gpio_cfg_output(pin); } while (0)


void pinsBatt_Init(void)
{
    BATT_PIN_CONFIGURE(BATT_PINS_MASK);
    
    BATT_PIN_OFF(BATT_PINS_MASK); // WAKE_PIN_OFF(BATT_PINS_MASK);
    NRF_GPIO->DIRSET = BATT_PINS_MASK;
}

void pinsBatt_Deinit(void)
{
   //TODO nrf_gpio_cfg_default(BATT_ON_PIN);
   //TODO nrf_gpio_cfg_default(BATT_LD_PIN);
}

void pinsBatt_ON_Assert(void){
    BATT_PIN_ON(BATT_ON_PIN_MASK);
}
void pinsBatt_ON_Release(void){
    BATT_PIN_OFF(BATT_ON_PIN_MASK);
}

void pinsBatt_LD_Assert(void){
    BATT_PIN_ON(BATT_LD_PIN_MASK);
}
void pinsBatt_LD_Release(void){
    BATT_PIN_OFF(BATT_LD_PIN_MASK);
}



//bool pinWakeUp_IsAsserted(void)
//{
//    return( WAKE_PIN_IS_ON(BATT_ON_PIN) );
//}

      

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#define  UART_TX_PIN_NUM 9
#define  UART_RTS_PIN_NUM 8

void pinsUART_TX_NotFloating(void)
{
    //nrf_gpio_cfg_output(UART_TX_PIN_NUM);
    
    nrf_gpio_cfg(
            UART_TX_PIN_NUM,
            NRF_GPIO_PIN_DIR_INPUT, // NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP, //NRF_GPIO_PIN_NOPULL,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
}

void pinsUART_RTS_NotFloating(void)
{
    nrf_gpio_cfg(
            UART_RTS_PIN_NUM,
            NRF_GPIO_PIN_DIR_INPUT, // NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP, //NRF_GPIO_PIN_NOPULL,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// TODO regarding unused pins / going into reset state

/*
https://devzone.nordicsemi.com/question/14567/unused-gpio-pins/

The default behavior of unused GPIOs (it's reset value) is set as input, 
with an internal switch (NRF_GPIO->PIN_CNF[], bit 1) set to disconnect.
This is the recommended configuration of unused GPIOs to ensure that 
they do not draw current due to floating inputs.

So, just keep the unused pins un-connected on your custom PCB and make 
sure that these pins have the reset value of NRF_GPIO->PIN_CNF[].

Cheers, Hakon

->  So the pin is kept floating on the PCB but the internal switch is disconnected?
    Imran (Aug 4 '14)
    
->  Yes, that is correct.
    Hakon Alseth (Aug 5 '14)
*/
