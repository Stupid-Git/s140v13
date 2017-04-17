
#include <stdint.h>
#include <string.h>

#include "ma_utils.h"



int32_t cb_push( uint8_t data_byte, uint8_t* buffer, uint16_t rdPtr, uint16_t* pwrPtr, uint16_t capacity )
{
    int next_wp;
    next_wp = *pwrPtr + 1;
    if( next_wp >= capacity ) // if it is equal then it is already out of bounds
        next_wp = 0;
    if( next_wp == rdPtr ) //Full
        return(0);

    buffer[*pwrPtr] = data_byte;
    *pwrPtr = next_wp;
    return(1);
}

int32_t cb_pop( uint8_t* pdata_byte, uint8_t* buffer, uint16_t* prdPtr, uint16_t wrPtr, uint16_t capacity )
{
    if( *prdPtr == wrPtr ) //Empty
        return(0);

    *pdata_byte = buffer[*prdPtr];
    (*prdPtr)++;
    if( *prdPtr >= capacity ) // if it is equal then it is already out of bounds
        *prdPtr = 0;
    return(1);
}

int32_t cb_peek( uint8_t* pdata_byte, uint8_t* buffer, uint16_t rdPtr, uint16_t wrPtr )
{
    if( rdPtr == wrPtr ) //Empty
        return(0);
    *pdata_byte = buffer[rdPtr];
    return(1);
}

 
int32_t cb_available(uint16_t rdPtr, uint16_t wrPtr, uint16_t capacity)
{
    int32_t size1;
    int32_t size2;
    int32_t size = 0;

    if( rdPtr == wrPtr ) //Empty
        return(capacity-1);

    if( rdPtr < wrPtr )
    {
        size1 = rdPtr - 0;
        size2 = (capacity - wrPtr) - 1;
        size = size1 + size2;
        return(size);
    }

    if( wrPtr < rdPtr )
    {
        size = (rdPtr - wrPtr) - 1;
        return(size);
    }
    return(size);
}

int32_t cb_count( uint16_t rdPtr, uint16_t wrPtr, uint16_t capacity )
{
    int32_t size1;
    int32_t size2;
    int32_t size = 0;

    if( rdPtr == wrPtr ) //Empty
        return(0);

    if( rdPtr < wrPtr )
    {
        size = wrPtr - rdPtr;
        return(size);
    }

    if( wrPtr < rdPtr )
    {
        size1 = capacity - rdPtr;
        size2 = wrPtr - 0;
        size = size1 + size2;
        return(size);
    }
    return(size);
}

int32_t cb_delete_range( uint8_t* buffer, uint16_t startPtr, uint16_t stop, uint16_t capacity )
{
    return(-1);
}
 


    
int32_t cb16_available(cb16_t*cb)
{
    return( cb_available(cb->rdPtr, cb->wrPtr, cb->capacity) );
}

int32_t cb16_push( cb16_t*cb, uint8_t data_byte )
{
    return( cb_push(data_byte, cb->buffer, cb->rdPtr, &cb->wrPtr, cb->capacity) );
}

int32_t cb16_push_n( cb16_t*cb, uint8_t* pD, int count)
{
    int len1, len2;
    //uint16_t p1, p2;
    
    //-----
    int next_wp;
    next_wp = cb->wrPtr + 1;
    if( next_wp >= cb->capacity ) // if it is equal then it is already out of bounds
        next_wp = 0;
    if( next_wp == cb->rdPtr ) //Full
        return(0);

    //-----
    if(cb->wrPtr < cb->rdPtr)
    {
        len1 = (cb->rdPtr - cb->wrPtr) - 1;
        if( len1 >= count)
            len1 = count;        
        memcpy(&cb->buffer[cb->wrPtr], pD, len1);
        return(len1);
    }
    else
    {
        len1 = (cb->capacity - cb->wrPtr);
        if( len1 >= count )
        {
            memcpy(&cb->buffer[cb->wrPtr], pD, count);

            next_wp = cb->wrPtr + count;
            if( next_wp >= cb->capacity ) // if it is equal then it is already out of bounds
                next_wp = 0;
            cb->wrPtr = next_wp;
            return(count);
        }
        else
        {
            len2 = count - len1;
            if(len2 > cb->rdPtr) 
                len2 = cb->rdPtr;
            memcpy(&cb->buffer[cb->wrPtr], pD, len1);
            memcpy(&cb->buffer[0], pD + len1, len2);
            cb->wrPtr = len2;
            return(len1 + len2);
        }
    }
    //unreachable return(1);
}

int32_t cb16_pop( cb16_t*cb , uint8_t* pdata_byte )
{
    return( cb_pop(pdata_byte, cb->buffer, &cb->rdPtr, cb->wrPtr, cb->capacity) );
}

int32_t cb16_peek( cb16_t*cb, uint8_t* pdata_byte )
{
    return( cb_peek(pdata_byte, cb->buffer, cb->rdPtr, cb->wrPtr) );
}

int32_t cb16_count( cb16_t*cb )
{
    return( cb_count(cb->rdPtr, cb->wrPtr, cb->capacity) );
}

int32_t cb16_delete_range( cb16_t*cb, uint16_t startPtr, uint16_t endPtr)
{
    return( cb_delete_range( cb->buffer, startPtr, endPtr, cb->capacity) );
}

int32_t cb16_clear( cb16_t*cb )
{
    cb->rdPtr = 0;
    cb->wrPtr = 0;
    return(0);
}



#include <stdio.h>
#include <string.h> //memcpy
#include <stdlib.h> //malloc

buf32_t* buf32_Create(int byteCount)
{
    buf32_t* pbuf;
    pbuf = (buf32_t*)malloc(sizeof(buf32_t));
    pbuf->buffer = (uint8_t*)malloc( byteCount );
    pbuf->length = 0;
    pbuf->capacity = byteCount;

    return(pbuf);
}

void buf32_Destroy(buf32_t** p_pbuf)
{
    buf32_t* pbuf;

    if(p_pbuf == 0)
        return;

    pbuf = *p_pbuf;
    if( pbuf != 0)
    {
        if( pbuf->buffer != 0 )
        {
            free(pbuf->buffer);
            pbuf->buffer = 0;
        }
        pbuf->length = 0;
    }
    *p_pbuf = 0;
    return;
}

void buf32_zero(buf32_t *P)
{
    P->length = 0;
}

void buf32_join(buf32_t *P, buf32_t *P2)
{
    memcpy(&P->buffer[P->length], &P2->buffer[0], P2->length);
    P->length += P2->length;
}

void buf32_cpy(buf32_t *P, void* p_bytes, int count)
{
    memcpy(&P->buffer[P->length], p_bytes, count);
    P->length += count;
}

uint32_t buf32_count(buf32_t *P, uint8_t the_byte)
{
    uint32_t r = 0;
    uint32_t i;
    for( i=0; i<P->length; i++)
    {
        if( P->buffer[i] == the_byte)
            r++;
    }
    return(r);
}

void buf32_print(char * s, buf32_t *P)
{
    uint32_t i;
    //int  j;
    printf("%s\n", s );

    for(i=0 ; i < P->length; i++)
    {
        printf("%02x ", P->buffer[i] );
        if( (i % 16) == 15 )
            printf("\n");
    }
    printf("\n");
}

void buf32_printShort(char * s, buf32_t *P)
{
    uint32_t i;
    uint32_t LO, HI;
    //int  j;

    LO = 32;
    HI = 32;
    if( P->length > 32)
    {
        //printf("P->length = %d\n", P->length); 
        HI = P->length - 32;
        //printf("    HI = %d\n", HI);
        HI = HI / 16;
        //printf("    HI = %d\n", HI);
        HI = HI * 16;
        //printf("    HI = %d\n", HI);
        if( ( P->length - HI) > 32)
            HI = HI + 16;
    }

    printf("%s\n", s );

    for(i=0 ; i < P->length; i++)
    {     
        if( (i<LO) || (i>=HI) )
        {
            printf("%02x ", P->buffer[i] );
            if( (i % 16) == 15 )
                printf("\n");
        }
        else
            if( i== LO)
            {
                //printf("------------------------------------------------\n");
                printf(" ...\n");
            }
    }
    printf("\n");
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if 0 //USE MY CRC
/* Includes ------------------------------------------------------------------*/
//#include "Global.h"

#include <stdio.h>
#include <stdarg.h>	
#include <string.h>
#include <setjmp.h>


struct b_tag
{
	uint8_t low;
	uint8_t high;
};

union i_tag
{
	struct b_tag b_acc;
	uint16_t w_acc;
};

union i_tag Crc;

const uint16_t CrcTable16[256] =			/* 1バイト分のＣＲＣ結果 */
{
	0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
	0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
	0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
	0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
	0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
	0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
	0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
	0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
	0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
	0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
	0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
	0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
	0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
	0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
	0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
	0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
	0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
	0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
	0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
	0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
	0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
	0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
	0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
	0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
	0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
	0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
	0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
	0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
	0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
	0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
	0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
	0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

union i_tag CrcTmp;

/*----------------------------------------------------------------------------*
	機能　　　：　ＣＲＣ－１６の生成（テーブル参照） １バイト毎の演算
	返り値　　：　なし
	引き数　　：　dt データ
	機能説明　：　生成多項式　Ｘ^16＋Ｘ^12＋Ｘ^5＋１ （ＣＣＩＴＴ）
		　　　処理時間：約５５us
		　　　受信側で正解チェックを行う場合は、Ｈｉ--> Ｌｏの順に演算する事
		　　　初期値を演算の前に設定すること
		　　　ＲＴＲ７１の旧タイプの演算方法とは、シフト方向が逆のため結果が異なる
	備考　　　：　
*----------------------------------------------------------------------------*/
void CrcGenerateByte(uint8_t d)
{
	CrcTmp.w_acc = CrcTable16[Crc.b_acc.high ^ d];
	Crc.b_acc.high = CrcTmp.b_acc.high ^ Crc.b_acc.low;
	Crc.b_acc.low = CrcTmp.b_acc.low;
}

/*----------------------------------------------------------------------------*
	機能　　　：　ＣＲＣ－１６の生成（テーブル参照）　バッファの演算
	返り値　　：　なし
	引き数　　：　*ptr 　計算データ列の先頭ポインタ
		　　　count　データ数
	機能説明　：　
	備考　　　：　
*----------------------------------------------------------------------------*/
void CrcGenerateBuffer(uint8_t *ptr, uint32_t count)
{
	do{
		CrcTmp.w_acc = CrcTable16[Crc.b_acc.high ^ *ptr];
		Crc.b_acc.high = CrcTmp.b_acc.high ^ Crc.b_acc.low;
		Crc.b_acc.low = CrcTmp.b_acc.low;
		ptr++;
		count--;
	}while(count);
}
#endif

uint16_t crc16_compute(uint8_t * p_data, uint16_t size, uint16_t * p_crc)
{
    uint16_t i;
    uint16_t crc = (p_crc == NULL) ? 0xffff : *p_crc;

    for (i = 0; i < size; i++)
    {
        crc  = (unsigned char)(crc >> 8) | (crc << 8);
        crc ^= p_data[i];
        crc ^= (unsigned char)(crc & 0xff) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xff) << 4) << 1;

        //printf("crc=%04x  ", crc);
    }
    return crc;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
#include <Windows.h>
void time_sleep(float time_in_seconds)
{
    uint32_t time_in_milliseconds;

    time_in_milliseconds = (uint32_t)(time_in_seconds * 1000.0);
    Sleep(time_in_milliseconds);

}
*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
uint8_t * int32_to_bytes(uint32_t nr)
{
    static uint8_t ints[4];
    ints[0] = (nr & 0x000000FF) ;
    ints[1] = (nr & 0x0000FF00) >> 8;
    ints[2] = (nr & 0x00FF0000) >> 16;
    ints[3] = (nr & 0xFF000000) >> 24;
    return(ints);
    //return ''.join(chr(b) for b in ints)
}

uint8_t * int16_to_bytes(uint32_t nr)
{
    static uint8_t ints[2];
    ints[0] = (nr & 0x000000FF) ;
    ints[1] = (nr & 0x0000FF00) >> 8;
    return(ints);
}
//def G_int16_to_bytes(nr):
//    ints = [0,0]
//    ints[0] = (nr & 0x00FF) 
//    ints[1] = (nr & 0xFF00) >> 8
//    return ''.join(chr(b) for b in ints)     
