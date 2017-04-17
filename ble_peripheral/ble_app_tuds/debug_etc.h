#ifndef __DEBUG_ETC_H
#define __DEBUG_ETC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"

#include "stdarg.h"

// http://www.cplusplus.com/reference/cstdio/printf/
// http://www.cplusplus.com/reference/cstdio/vprintf/


#define  USE_PRINTF_OVER_SDO  0//1   
    

//------------------------- NO Debug -------------------------
#if (USE_PRINTF_OVER_SDO==0)
static inline int dbgPrintf( const char * format, ... )
{
    return(0);
}

#define dbgPrint( X )
#define dbgPrint_Init() 
#define get_ble_evt_str(X) (const char *)""
    
//------------------------- YES Debug -------------------------
#else

void sdo_Init(void);
#define dbgPrint_Init sdo_Init

int dbgPrintf( const char * format, ... );

//int dbgPrint( const char * str );
int sdo_AppendText( const char * str );
#define dbgPrint(x) sdo_AppendText(x)
const char * get_ble_evt_str( uint8_t evt_id);
#endif
//------------------------- End Debug -------------------------
        
    
#ifdef __cplusplus
}
#endif

#endif // __DEBUG_ETC_H

