/*********************************************************************
* HRCLOCK.C - This file contains all functions needed by the high-   *
*   resolution clock functions mClock() and uClock(). These functions*
*   return millisecond and microsecond timestamps. They build their  *
*   timestamps from the data in the Master Clock Count and the 8254's*
*   counter registers. During initialization, the function HRInit()  *
*   determines what mode PIT counter 0 is in and stores the mode in  *
*   the variable Counter0Mode. When the function GetTimerData() takes*
*   samples from the PIT's registers, it interprets the value from   *
*   counter 0 based on that counter's mode. In addition, uses modulo *
*   arithmetic to determine when the BIOS Master Clock Count is out  *
*   of phase with (lagging behind) the PIT.                          *
*                                                                    *
*   This file contains no main() function -- it is designed to be    *
*   linked with existing programs. See the file HRDEMO.C for         *
*   compilation instructions.                                        *
*                                                                    *
* Author: David Reid, Last Revised: 08/08/91                         *
*********************************************************************/
#pragma function(inp, outp)  /* No intrinsic inp and outp functions */

#include <stdio.h>           /* puts(), printf()                    */
#include <conio.h>           /* outp(), inp()                       */
#include <dos.h>             /* disable(), enable()                 */
#include "hrclock.h"         /* HRCLOCK specific stuff              */

/*----#DEFINE's-----------------------------------------------------*/
#define TICKS_PER_DAY   1573040L
#define COUNTS_PER_SEC  (double)1193180
#define COUNTS_PER_mSEC (double)1193.18
#define COUNTS_PER_uSEC (double)1.19318

typedef union                /* This union gives direct access to   */
    {                        /* both the high and low bytes of the  */
    USHORT i;                  /* PIT counter values.                 */
    struct { UCHAR l, h; } c;
    } CNTR;

/*----PROTOTYPES----------------------------------------------------*/
USHORT HRInit(void);
void HRReset(void);
void HRMode2(void);
void HRMode3(void);
USHORT GetPITCounterMode(USHORT Counter);
static USHORT GetTimerData(void);
ULONG uClock(void);
ULONG mClock(void);

/*----GLOBAL VARIABLES----------------------------------------------*/
static const volatile ULONG far *MCC = (ULONG far *)0x0040006cL;
static CNTR   C0;                 /* Value from PIT counter 0       */
static CNTR   InitC0;             /* Initial value from PIT cntr 0  */
static CNTR   C1;                 /* Value from PIT counter 1       */
static USHORT   Counter0Mode;       /* Current PIT counter 0 mode     */
static UCHAR  Status;             /* Status word for PIT counter 0  */
static USHORT   PhaseCycle;         /* Last calc'd phase cycle        */
static USHORT   PhaseDiff;          /* Last calc'd phase difference   */
static USHORT   ControlDiff;        /* Phase difference at init.      */
static ULONG  InitTicker;         /* Master clock count at init time*/
static ULONG  CurrTicker;         /* Curr master clock count value  */
static ULONG  LastTicker;         /* Last master clock count value  */

static unsigned char getin(int port)
{
  unsigned char retval;

  asm {
    mov dx, port;
    in  al, dx;
    jmp short l1:
      }
    l1:
  asm mov retval, al;

  return retval;
}

static void putout(int port, unsigned char val)
{
  asm {
    mov al, val;
    mov dx, port;
    out dx, al;
    jmp short l2:
      }
l2:
    return;
}

/*********************************************************************
* GetPITCounterMode - Returns the mode of the specified PIT counter. *
*********************************************************************/
USHORT GetPITCounterMode(USHORT Counter)
    {
    int InquireBit;

    if (Counter > 2)
        return(0);

    for(InquireBit=2; Counter>0; Counter--)
        InquireBit <<= 1;

    _disable();
    putout(0x43, 0xE0 | InquireBit);
    Status = getin(0x40 | Counter);
    _enable();
    return((Status >> 1) & 0x07);
    }

/*********************************************************************
* HRMode2 - Switch counter 0 to mode 2 and load initial value 65536. *
*********************************************************************/
void HRMode2(void)
    {
    _disable();                   /* Disable interrupts             */
    putout(0x43, 0x34);         /* Counter 0, Mode 2, LSB/MSB     */
    putout(0x40, 0x00);         /* Load low word of value         */
    putout(0x40, 0x00);         /* Load high word of value        */
    _enable();                    /* Re-enable interrupts           */
    }

/*********************************************************************
* HRMode3 - Switch counter 0 to mode 3 and load initial value 65536  *
*********************************************************************/
void HRMode3(void)
    {
    _disable();                   /* Disable interrupts             */
    putout(0x43, 0x36);         /* Counter 0, Mode 3, LSB/MSB     */
    putout(0x40, 0x00);         /* Load low word of value         */
    putout(0x40, 0x00);         /* Load high word of value        */
    _enable();                    /* Re-enable interrupts           */
    }

/*********************************************************************
* HRReset -                                                         *
*********************************************************************/
void HRReset(void)
{
  int i = 0;

  LastTicker = 0L;
  while (!GetTimerData() && i < 1000)
    i++;
  InitTicker = CurrTicker;
  InitC0.i = C0.i;
}

/*********************************************************************
* HRInit - Initialization routine for HRCLOCK routines.              *
*********************************************************************/
USHORT HRInit(void)
    {
    int i;

    Counter0Mode = GetPITCounterMode(0);
    if ((Counter0Mode != 2) && (Counter0Mode != 3))
        {
        puts("PIT counter 0 in invalid mode");
        return(FALSE);
        }

    /*----Find the constant value ControlDiff-----------------------*/
    /* (If we get the same difference between the MCC and PIT phase */
    /* phase cycle 64 times in a row, we  accept it as correct.)    */
    for (i=0; i<64; )
        {
        GetTimerData();
        i = (ControlDiff == PhaseDiff) ? i+1 : 0;
        ControlDiff = PhaseDiff;
        }
    HRReset();
    return(TRUE);
    }

/*********************************************************************
* GetTimerData - This function reads the current PIT and MCC values. *
*   It normalizes the value in PIT counter 0 so the functions uClock *
*   and mClock may generate time stamps with a minimum of effort.    *
*********************************************************************/
static USHORT GetTimerData(void)
{
  USHORT C0mod, C1mod;

  /*----Disable interrupts and read 8254 -------------------------*/
  _disable();              /* Disable interrupts                  */
  CurrTicker = *MCC;       /* Get Master Clock Count at 0040:006C */
  putout(0x43, 0xC6);        /* Read-Back Command: status & counter */
                           /*   regsters for PIT counters 0 and 1 */
  Status = getin(0x41);      /* Throw away C1 status byte           */
  asm mov ax,ax;
  asm mov ax,ax;
  C1.c.l = getin(0x41);      /* Get low byte of counter 1           */
  asm mov ax,ax;
  asm mov ax,ax;
  Status = getin(0x40);      /* Get status word for counter 0       */
  asm mov ax,ax;
  asm mov ax,ax;
  C0.c.l = getin(0x40);      /* Get low byte of counter 0           */
  asm mov ax,ax;
  asm mov ax,ax;
  C0.c.h = getin(0x40);      /* Get high byte of counter 0          */
  _enable();               /* Re-enable interrupts                */

  /*----Normalize counter 0 value to range 0 -> 65535-------------*/
  if (Counter0Mode == 2)
      --C0.i;
  else                     /* Counter0Mode == 3                   */
    if (C0.i == 0)
      C0.i = (Status & 0x80) ? 0xffff : 0x7fff;
    else
      {
      C0.i >>= 1;
      --C0.i;
      C0.i |= (Status & 0x80) ? 0x8000 : 0x0000;
      }
  C0.i = 0xFFFF - C0.i;    /* Invert counter 0 value              */

  C0mod = C0.i % 18;       /* Get counter 0 MOD(18)               */
  C1mod = 18 - C1.c.l;     /* Get counter 1 MOD(18)               */

  /*----Determine counter0/counter1 phase cycle-------------------*/
  /*    (Convert from:  16,14,12,10, 8, 6, 4, 2, 0 )              */
  /*    (         to:   0, 1, 2, 3, 4, 5, 6, 7, 8 )               */
  PhaseCycle = 8 - (((18+C1mod-C0mod) % 18) >> 1);

  /*----Account for midnight rollover-----------------------------*/
  if (CurrTicker < LastTicker)
    {
    LastTicker = CurrTicker;
    CurrTicker += TICKS_PER_DAY;
    }
  else
    LastTicker = CurrTicker;

  /*----Get difference between (MCC%9) and PhaseCycle-------------*/
  PhaseDiff = (9 + (USHORT)((CurrTicker%9)) - PhaseCycle) % 9;

  if (PhaseDiff != ControlDiff)
    /*----Test for slow BIOS ticker-----------------------------*/
    if ((PhaseDiff+1)%9 == ControlDiff)
      ++CurrTicker;    /* Increment slow ticker value           */
    else
      return(0);       /* Return FALSE - out of sync readings   */

  /*----If we made it here, everything's in sync------------------*/
  return(1);             /* Return TRUE - all readings in sync    */
}

/*********************************************************************
* uClock - This calls GetTimerData and returns the current time in   *
*   uSecs. The return value has a granularity of 16 uSecs. Because   *
*   this function may occasionally have to call GetTimerData twice   *
*   to get an in-sync reading from the PIT, this function is         *
*   accurate to only around 50 uSecs (2*16uSec + function overhead). *
*   The return values from this function roll over every ~19 hours.  *
*********************************************************************/
ULONG uClock(void)
{
  ULONG uSecs;
  int i = 0;

  while (!GetTimerData() && i < 1000)
      i++;

  /*--------Combine BIOS ticker with top 12 bits of timer count---*/
  uSecs = ((CurrTicker - InitTicker) << 12) | (C0.i >> 4);
  uSecs -= (ULONG)(InitC0.i >> 4);

  /*--------Convert to 16 microsecond increments------------------*/
  uSecs /= COUNTS_PER_uSEC;

  return(uSecs);
}

/*********************************************************************
* mClock - This function calls GetTimerData and then builds a 32 bit *
*   time stamp that represents the number of elapsed mSecs since     *
*   HRInit() was last called.
*********************************************************************/
ULONG mClock(void)
{
  ULONG mSecs;
  int i = 0;

  while (!GetTimerData() && i < 1000)
      i++;

  /*--------Combine BIOS ticker with top 6 bits of timer count----*/
  mSecs = ((CurrTicker - InitTicker) << 6) | (C0.i >> 10);

  if (C0.i & 0x0200)   /* Round up if truncated fraction >= 0.5   */
      ++mSecs;

  /*--------Convert to milliseconds-------------------------------*/
  mSecs /= COUNTS_PER_mSEC / 1024;

  return(mSecs);
}																	

void uDelay(ULONG uSecs)
{
  ULONG EndTime, StartTime = uClock();

  do
    EndTime = uClock();
  while((EndTime - StartTime) < uSecs);
}


void mDelay(ULONG mSecs)
{
  ULONG EndTime, StartTime = mClock();
  
  do
    EndTime = mClock();
  while((EndTime - StartTime) < mSecs);
}
