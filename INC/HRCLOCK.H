/*********************************************************************
* HRCLOCK.H - The file holds the #define's, typedef's and prototypes *
*   you need to interface properly with the functions in HRCLOCK.C.  *
*                                                                    *
*   The constants mCLOCKS_PER_SEC and uCLOCKS_PER_SEC are just like  *
*   Microsoft's constant CLOCKS_PER_SEC in TIME.H. You use these     *
*   constants to obtain measurements in seconds from the return      *
*   values of mClock() and uClock(). For example, to determine the   *
*   execution time of a process in fractions of a second, you could  *
*   use the following logic:                                         *
*                                                                    *
*   unsigned long start, stop;                                       *
*                                                                    *
*   start = uClock();                                                *
*   PROCESS();                                                       *
*   stop = uClock();                                                 *
*   printf( "PROCESS TIME: %f seconds\n",                            *
*       (double)(stop - start) / (double)uCLOCKS_PER_SEC;            *
*                                                                    *
*********************************************************************/

#ifndef HRCLOCK_INCLUDED
#define HRCLOCK_INCLUDED

#ifndef PRIMTYPE_INCLUDED
#include "primtype.h"
#endif

/*----#DEFINE's-----------------------------------------------------*/
#define mCLOCKS_PER_SEC 1000    /* same as CLOCKS_PER_SEC in TIME.H */
#define uCLOCKS_PER_SEC 62500L

/*----PROTOTYPES----------------------------------------------------*/
USHORT GetPITCounterMode( USHORT Counter );
USHORT HRInit( void );
void HRReset( void );
void HRMode2( void );
void HRMode3( void );
ULONG uClock( void );
ULONG mClock( void );
void uDelay(ULONG uSecs);
void mDelay(ULONG mSecs);

#endif
