#ifndef __TYPEDEFINE_H
#define __TYPEDEFINE_H


//#define _BIG_ENDIAN
//#define DBG01

#define Z80_MEM_SIZE    0x6000      // 16kbyte
/*----------------------------------------------------------------------*/
/*                                                                      */
/*----------------------------------------------------------------------*/
typedef unsigned char    uchar;
typedef unsigned short    ushort;
typedef unsigned int    uint;
typedef    unsigned long    ulong;

typedef    volatile char    vchar;
typedef    volatile short  vshort;
typedef    volatile int    vint;
typedef    volatile long    vlong;

typedef    volatile unsigned char    vuchar;
typedef    volatile unsigned short    vushort;
typedef    volatile unsigned int    vuint;
typedef    volatile unsigned long    vulong;


// bz80
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;


extern void WSendZ80( char *data );
extern void WRecvZ80( char *buf );
extern void DispReg( void );

#endif


