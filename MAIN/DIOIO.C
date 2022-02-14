#include <stdlib.h> /* abs() */
#include <conio.h>

#include "textio.h"
#include "hrclock.h"
#include "dioio.h"

float A_D_Avg[5];

#define WDELAY 10

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

  outportb(IOPORTB, ADTRIG);
  do
    Delay(WDELAY);
  while( (inp(IOPORTC) & 0x20) == 0);

  Delay(WDELAY);
  A = inp(IOPORTA);
  
  do
    Delay(WDELAY);
  while( (inp(IOPORTC) & 0x20) == 0);
  
  Delay(WDELAY);
  B = inp(IOPORTA);
  C = A * 256 + B;
  if (C > 32767L)
    C = C - 65536L;

  A_D_Buff[0] = C;

  for (ADCHNL = 1; ADCHNL < 4; ADCHNL++)
    {
    Delay(WDELAY);
    outportb(IOPORTB, ADNEXT);
    do
      Delay(WDELAY);
    while( (inp(IOPORTC) & 0x20) == 0);

    Delay(WDELAY);
    
    A = inp(IOPORTA);
    do
      Delay(WDELAY);
    while( (inp(IOPORTC) & 0x20) == 0);
    Delay(WDELAY);
    B = inp(IOPORTA);
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
  long A_D_Buff[4];

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

  outportb(IOPORTB, MUXCHANNEL + GROUP);
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

  outportb(IOPORTB, INTRFCLATCH);
  ;
  outportb(IOPORTA, INTRFCCNTRL);
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
  outportb(IOPORTB, DAC0LOADD);
  Delay(WDELAY);
  outportb(IOPORTA, B);
  Delay(WDELAY);
  outportb(IOPORTB, DAC0HIADD);
  Delay(WDELAY);
  outportb(IOPORTA, A);
}

void DAC1OUT(int Value)
{
  int A, B;

  A = Value / 0x100;
  B = Value & 0xFF;
  outportb(IOPORTB, DAC1LOADD);
  Delay(WDELAY);
  outportb(IOPORTA, B);
  Delay(WDELAY);
  outportb(IOPORTB, DAC1HIADD);
  Delay(WDELAY);
  outportb(IOPORTA, A);
}

void LinearCalc(void)
{
  int Points = 4095, P, V, Ch1_Hi, Ch1_Lo, Ch2_Hi, Ch2_Lo,
                           Ch1_Slope, Ch2_Slope, Ch1_Intcpt, Ch2_Intcpt;    
  float Delta1, Delta2, Y1, Y2;
  
  DAC0OUT(0);
  DAC1OUT(0);

  mDelay(1000);
  AvgAD(A_D_Avg, 100);

  Ch1_Lo = A_D_Avg[0];
  Ch2_Lo = A_D_Avg[1];

  DAC0OUT(Points);
  DAC1OUT(Points);

  mDelay(1000);

  AvgAD(A_D_Avg, 100);

  Ch1_Hi = A_D_Avg[0];
  Ch2_Hi = A_D_Avg[1];
  Ch1_Slope = (Ch1_Hi - Ch1_Lo) / Points;
  Ch2_Slope = (Ch2_Hi - Ch2_Lo) / Points;
  Ch1_Intcpt = Ch1_Lo;
  Ch2_Intcpt = Ch2_Lo;

  P = 1;

  for(V = 0; V <= Points; V++)
    {
    DAC0OUT(V);
    DAC1OUT(V);
    mDelay(10);
    AvgAD(A_D_Avg, 10);

    Y1 = Ch1_Slope * V + Ch1_Intcpt;
    Delta1 = Y1 - A_D_Avg[0];

    if(abs(Delta1) > 5)
      {
      fmt_outchar(1, 20, "Ch1_Hi=    %5d   Ch1_Lo=     %5d", Ch1_Hi, Ch1_Lo);
      fmt_outchar(1, 21, "Ch1_Slope= %5d   Ch1_Intcpt= %5d", Ch1_Slope, Ch1_Intcpt);
      fmt_outchar(1, 22, "Y1=        %f", Y1);
      fmt_outchar(1, 23, "D/A VOLT=  %4.4f A/D VOLT=   %4.4f",
                          (float)(V / 4095 * 10 - 5),
                          (float)(A_D_Avg[0] / 2048 * 10));

      fmt_outchar(1, 24, "D/A 1 ERROR @%d, ERROR = %f", V, Delta1);

      V = Points;                   /* Terminate loop */
      }

    Y2 = Ch2_Slope * V + Ch2_Intcpt;
    Delta2 = Y2 - A_D_Avg[1];

    if(abs(Delta2) > 5)
      {
      fmt_outchar(1, 20, "Ch2_Hi=    %5d   Ch2_Lo=     %5d", Ch2_Hi, Ch2_Lo);
      fmt_outchar(1, 21, "Ch2_Slope= %5d   Ch2_Intcpt= %5d", Ch2_Slope, Ch2_Intcpt);
      fmt_outchar(1, 22, "Y2=        %f", Y2);
      fmt_outchar(1, 23, "D/A VOLT=  %4.4f A/D VOLT= %4.4f",
                          (float)(V / 4095 * 10 - 5),
                          (float)(A_D_Avg[1] / 2048 * 10));

      fmt_outchar(1, 24, "D/A 2 ERROR @%d ERROR= %f", V, Delta2);

      V = Points;                   /*  Terminate loop */
      }

    if(V / 20 > P)
      {
      P = P + 1;
      erase_line(1, 25, 40);
      fmt_outchar(1, 25, "V= %d   Delta 1=%3.4f  Delta 2=%3.4f", V, Delta1, Delta2);
      }

    if (kbhit())
      {
      V = Points; /* Terminate loop */
      getch();
      }
    }
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
  outportb(IOPORTB, INTRFCCOMMAND);   /* COMMAND BYTE ADDR */
  outportb(IOPORTA, Cmd);             /* COMMAND BYTE VALUE */
  LATCHOUT(DATAREQ, SET_BIT);         /* SET DATA REQ TO 8032 */
  while ((inp(IOPORTC) & 0x20) == 0)
    ;                                 /* WAIT FOR ACK FOR 8032 */
  LATCHOUT(DATAREQ, CLR_BIT);         /* CLEAR DATA REQ */
  inp(IOPORTA);                       /* DUMMY READ TO EMPTY PORTA */
  outportb(IOPORTB, IOREAD);          /* OUTPUT INTERFACE BUFF ADDR */
  while ((inp(IOPORTC) & 0x20) == 0)
    ;                                 /* WAIT FOR ACK FOR 8032 */
  
  *CmdEcho = inp(IOPORTA);            /* READ ECHO FROM 8032 */
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


void BRDGZERO(void)
{
  int DA0TRY, DA1TRY;
  
  ADMux(0);
  LATCHOUT(ZEROCAL, CLR_BIT);
  DA0TRY = DA1TRY = 2047;
  DAC0OUT(2047);
  DAC1OUT(2047);
  mDelay(100);
  do
    {
    AvgAD(A_D_Avg, 100);
    if(abs(A_D_Avg[2]) < 1)
      break;  
    else
      {
      DA0TRY = DA0TRY + A_D_Avg[2] * 3.7;
      DAC0OUT(DA0TRY);
      mDelay(100);
      }
    }
  while(!kbhit());
  getch();

  fmt_outchar(1, 20, "A/D-X VOLTS= %f  D/A-X COUNTS = %f",
                      A_D_Avg[2] / 204.8, DA0TRY);

  do
    {
    AvgAD(A_D_Avg, 100);
    if(abs(A_D_Avg[4]) < 1)
      break;
    else
      {
      DA1TRY = DA1TRY + A_D_Avg[4] * 3.7;
      DAC1OUT(DA1TRY);
      mDelay(100);
      }
    }
  while(!kbhit());
  getch();
  
  fmt_outchar(1, 21, "A/D-Y VOLTS= %f D/A-Y COUNTS= %f",
                      DA1TRY,
                      A_D_Avg[4] / 204.8);

}

/* To transfer temperature data from interface module to IBM */

void TEMPDUMP(int Mode, int delay, int max_dump)
{
  unsigned long EndTime;
  int TempSet, MSB, LSB;
  int i, j, dummy, MSBTmp, LSBTmp;

  HRReset();

  EndTime = mClock() * 1000 + delay;
  
  if(Mode == 0)
    {
    outchar(1, 20, "RUNNING TRIGGERED START OF TEMPERATURE DATA STORAGE");
    outchar(1, 21, "SETTING RUNNING BIT");

    LATCHOUT(RUNNING, SET_BIT);             /* START TEMPERATURE STORE */

    while (mClock() < EndTime)
      ;

    erase_line(1, 21, 40);
    outchar(1, 21, "CLEARING RUNNING BIT");
    LATCHOUT(RUNNING, CLR_BIT);
    }
  else
    {
    outchar(1, 20, "COMMAND 4 STARTS TEMPERATURE DATA STORAGE");
    COMMAND(4, &dummy);                        /* START TEMP STORE */

    while (mClock() < EndTime)
      ;

    }
  erase_line(1, 21, 40);
  outchar(1, 21, "COMMAND 5 TO STOP TEMPERATURE STORING");

  COMMAND(5, &dummy);                     /* STOP TEMPERATURE STORING */
  COMMAND(6, &MSB);                       /* SEND MSB COUNT */
  COMMAND(7, &LSB);                       /* SEND LSB COUNT */

  TempSet = MSB * 256 + LSB;

  fmt_outchar(1, 22, "NO OF TEMPERATURE SETS= %d", TempSet);

  if(TempSet > max_dump)
    TempSet = max_dump;

  COMMAND(1, &MSBTmp);
  COMMAND(2, &LSBTmp);

  fmt_outchar(1, 23, "%d", MSBTmp * 256 + LSBTmp);

  COMMAND(2, &MSBTmp);
  COMMAND(2, &LSBTmp);

  fmt_outchar(10, 23, "%d", MSBTmp * 256 + LSBTmp);

  COMMAND(2, &MSBTmp);
  COMMAND(2, &LSBTmp);

  fmt_outchar(20, 23, "%d", MSBTmp * 256 + LSBTmp);

  COMMAND(2, &MSBTmp);
  COMMAND(2, &LSBTmp);

  fmt_outchar(30, 23, "%d", MSBTmp * 256 + LSBTmp);

  for (i = 0; i < TempSet; i++)
    for (j = 0; j < 4; j++)
      {
      COMMAND(2, &MSBTmp);
      COMMAND(2, &LSBTmp);
      fmt_outchar(1, 23, "%d", MSBTmp * 256 + LSBTmp);
      }
}

void PORTBTEST(void)
{

}
