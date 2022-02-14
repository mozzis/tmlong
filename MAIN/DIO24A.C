/* dio24.c */

#include <stdlib.h> /* abs() */
#include <conio.h>

#include "textio.h"
#include "hrclock.h"
#include "dio24.h"

float A_D_Avg[5];

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


#define WDELAY 5

static void Delay(unsigned long delay)
{
  int dummy = 0;
  unsigned long i;

  for (i = 0; i < delay; i++)
    {
    if ((dummy++ & 0xFFFF) == 0)
      dummy &= 0xFFFF;

    }
}

void ADMeasure(long A_D_Buff[])
{
  long C;
  int A, B, ADCHNL;

  putout(IOPORTB, ADTRIG);
  do
    Delay(WDELAY);
  while( (getin(IOPORTC) & 0x20) == 0);

  Delay(WDELAY);
  A = getin(IOPORTA);
  
  do
    Delay(WDELAY);
  while( (getin(IOPORTC) & 0x20) == 0);
  
  Delay(WDELAY);
  B = getin(IOPORTA);
  C = A * 256 + B;
  if (C > 32767L)
    C = C - 65536L;

  A_D_Buff[0] = C;

  for (ADCHNL = 1; ADCHNL < 4; ADCHNL++)
    {
    Delay(WDELAY);
    putout(IOPORTB, ADNEXT);
    do
      Delay(WDELAY);
    while( (getin(IOPORTC) & 0x20) == 0);

    Delay(WDELAY);
    
    A = getin(IOPORTA);
    do
      Delay(WDELAY);
    while( (getin(IOPORTC) & 0x20) == 0);
    Delay(WDELAY);
    B = getin(IOPORTA);
    Delay(WDELAY);
    C = A * 256 + B;
    if (C > 32767L)
      C = C - 65536L;

    A_D_Buff[ADCHNL] = C;
    }
}

void AvgAD(float *A_D_Avg, int Count)
{
  int i, j;
  long A_D_Buff[6];

  for (i = 0; i < Count; i++)
    {
    ADMeasure(A_D_Buff);
    for (j = 0; j < 4; j++)
      A_D_Avg[j] = (A_D_Avg[j] + (float)A_D_Buff[j]);
    }

  for (i = 0; i < 4; i++)
    A_D_Avg[i] = A_D_Avg[i] / Count;
}

void ADMux(int GROUP)
{
/* GROUP 0 = X-POS, X-STRAIN, Y-POS,   Y-STRAIN */
/* GROUP 1 = TM1,   TM2,      TM3,     TM4 */
/* GROUP 2 = V-BAT, V-EXC-X,  V-EXC-Y, V-REF */
/* GROUP 3 = DAC0,  DAC1,     ANGND,   ANGND */

  putout(IOPORTB, MUXCHANNEL + GROUP);
}

/* This sub sets or clear the specified bitvalue in the interface control */
/* latch. BITVAL  must be the bit value ie. 1,2,4,,8,16,32,64,128 corres- */
/* ponding to D0 - D7 of the latch. */

void LATCHOUT(const int BITVAL, int set_clr)
{
  static int INTRFCCNTRL = 0,
             INTRFCLATCH = 0x90;

  if(set_clr == 0)
     INTRFCCNTRL = INTRFCCNTRL & ~BITVAL;
  else
     INTRFCCNTRL = INTRFCCNTRL | BITVAL;

  putout(IOPORTB, INTRFCLATCH);
  Delay(WDELAY);
  putout(IOPORTA, INTRFCCNTRL);
}

void LEDTest(void)
{
  int i;
  int repdel = 50;
  
  HRMode2();
  HRInit();

  for (i = 0; i < 10; i ++)
    {
    LATCHOUT(READY, SET_BIT);
    mDelay(repdel);
    LATCHOUT(READY, CLR_BIT);
    mDelay(repdel);
    }
  for (i = 0; i < 10; i ++)
    {
    LATCHOUT(MARK, SET_BIT);
    mDelay(repdel);
    LATCHOUT(MARK, CLR_BIT);
    mDelay(repdel);
    }
  for (i = 0; i < 10; i ++)
    {
    LATCHOUT(RUNNING, SET_BIT);
    mDelay(repdel);
    LATCHOUT(RUNNING, CLR_BIT);
    mDelay(repdel);
    }
}

void DSPTest(void)
{
  LATCHOUT(CPURESET, SET_BIT);
  LATCHOUT(CPURESET, CLR_BIT);
  mDelay(3000);
  LATCHOUT(CPURESET, SET_BIT);
}


/* Output a count to offset D/A converter number # in counts.  */
/* 0 count=-5V, and 4095 count=+5V.                            */

void DAC0OUT(int Value)
{
  int A, B;

  A = Value / 0x100;
  B = Value & 0xFF;
  putout(IOPORTB, DAC0LOADD);
  Delay(WDELAY);
  putout(IOPORTA, B);
  Delay(WDELAY);
  putout(IOPORTB, DAC0HIADD);
  Delay(WDELAY);
  putout(IOPORTA, A);
}

void DAC1OUT(int Value)
{
  int A, B;

  A = Value / 0x100;
  B = Value & 0xFF;
  putout(IOPORTB, DAC1LOADD);
  Delay(WDELAY);
  putout(IOPORTA, B);
  Delay(WDELAY);
  putout(IOPORTB, DAC1HIADD);
  Delay(WDELAY);
  putout(IOPORTA, A);
}


void IntTest(int delay)
{
  int dummy;

  LATCHOUT(RUNNING, SET_BIT);
  mDelay(delay);
  LATCHOUT(RUNNING, CLR_BIT);
  COMMAND(5, &dummy);
}

void COMMAND(int Cmd, int *CmdEcho)
{
  putout(IOPORTB, INTRFCCOMMAND);   /* COMMAND BYTE ADDR */
  putout(IOPORTA, Cmd);             /* COMMAND BYTE VALUE */
  LATCHOUT(DATAREQ, SET_BIT);         /* SET DATA REQ TO 8032 */
  while ((getin(IOPORTC) & 0x20) == 0)
    Delay(WDELAY);
  LATCHOUT(DATAREQ, CLR_BIT);         /* CLEAR DATA REQ */
  getin(IOPORTA);                       /* DUMMY READ TO EMPTY PORTA */
  Delay(WDELAY);
  putout(IOPORTB, IOREAD);          /* OUTPUT INTERFACE BUFF ADDR */
  Delay(WDELAY);
  while ((getin(IOPORTC) & 0x20) == 0)
    Delay(WDELAY);
  
  *CmdEcho = getin(IOPORTA);            /* READ ECHO FROM 8032 */
}

void GAIN(float * GainMD, float * GainTD)
{
  float HiMD, HiTD, LowMD, LowTD;

  ADMux(0);                          /* SET MUX */
  LATCHOUT(ZEROCAL, SET_BIT);        /* SET RELAY TO ZEROCAL POS */
  DAC0OUT(0);                        /* SET DAC TO -5V OUT */
  DAC1OUT(0);
  mDelay(100);                       /* WAIT FOR 10MS TO SETTLE */
  AvgAD(A_D_Avg, 100);               /* GET 100 READINGS */
  LowMD = A_D_Avg[1];
  LowTD = A_D_Avg[3];
  DAC0OUT(4095);                     /* SET DAC'S TO +5V OUT */
  DAC1OUT(4095);
  mDelay(100);
  AvgAD(A_D_Avg, 100);
  HiMD = A_D_Avg[1];
  HiTD = A_D_Avg[3];
  *GainMD = (HiMD - LowMD) * 0.244141F;
  *GainTD = (HiTD - LowTD) * 0.244141F;
}

