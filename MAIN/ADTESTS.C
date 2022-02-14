/* adtests.c */

#include <stdlib.h> /* abs() */
#include <conio.h>  /* kbhit(), getch() */

#include "textio.h"
#include "hrclock.h"
#include "dio24.h"
#include "adtests.h"

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


void LinearCalc(void)
{
  int Points = 4095, V;    
  float Delta1, Delta2, Y1, Y2, Ch1_Hi, Ch1_Lo, Ch2_Hi, Ch2_Lo,
                                Ch1_Slope, Ch2_Slope, Ch1_Intcpt, Ch2_Intcpt;

  HRReset();

  DAC0OUT(0);
  DAC1OUT(0);

  uDelay(10000);
  AvgAD(A_D_Avg, 100);

  Ch1_Lo = A_D_Avg[0];
  Ch2_Lo = A_D_Avg[1];

  DAC0OUT(Points);
  DAC1OUT(Points);

  uDelay(10000);

  AvgAD(A_D_Avg, 100);

  Ch1_Hi = A_D_Avg[0];
  Ch2_Hi = A_D_Avg[1];
  Ch1_Slope = (Ch1_Hi - Ch1_Lo) / Points;
  Ch2_Slope = (Ch2_Hi - Ch2_Lo) / Points;
  Ch1_Intcpt = Ch1_Lo;
  Ch2_Intcpt = Ch2_Lo;
  
  outchar(    1,  18, "D/A 1: ");
  outchar(    40, 18, "D/A 2: ");
  fmt_outchar(1,  21, "Hi=    %-4.3f  Lo=     %-4.3f", Ch1_Hi, Ch1_Lo);
  fmt_outchar(40, 21, "Hi=    %-4.3f  Lo=     %-4.3f", Ch2_Hi, Ch2_Lo);
  fmt_outchar(1,  22, "Slope= %-4.3f     Intcpt= %-4.3f", Ch1_Slope, Ch1_Intcpt);
  fmt_outchar(40, 22, "Slope= %-4.3f     Intcpt= %-4.3f", Ch2_Slope, Ch2_Intcpt);

  for(V = 0; V <= Points; V++)
    {
    DAC0OUT(V);
    DAC1OUT(V);
    uDelay(1000);
    AvgAD(A_D_Avg, 10);

    Y1 = Ch1_Slope * (float)V + Ch1_Intcpt;
    Delta1 = Y1 - A_D_Avg[0];
    Y2 = Ch2_Slope * (float)V + Ch2_Intcpt;
    Delta2 = Y2 - A_D_Avg[1];

      fmt_outchar(8,   18, "V= %d  Y1= %-4.3f ", V, Y1);
      fmt_outchar(47,  18, "V= %d  Y2= %-4.3f ", V, Y2);
    fmt_outchar(1,  19, "Delta 1=  %-3.3f        ", Delta1);
    fmt_outchar(40, 19, "Delta 2=  %-3.3f        ", Delta2);
  
    if(abs(Delta1) > 5)
      {
      outchar(19, 19, "(ERROR) ");
      fmt_outchar(1, 20, "D/A VOLT= %4.3f A/D VOLT= %4.3f",
                          (float)((float)V * 10.0F) / 4095.0F - 5.0F,
                          (float)(A_D_Avg[0] / 2048.0 * 10.0));
      }

    if(abs(Delta2) > 5)
      {
      outchar(59, 19, "(ERROR)");
      fmt_outchar(40, 20, "D/A VOLT= %4.3f A/D VOLT= %4.3f",
                          (float)((float)V * 10.0F) / 4095.0F - 5.0F,
                          (float)(A_D_Avg[1] / 2048.0 * 10.0));
      }

    if (kbhit())
      {
      getch();
      break; /* Terminate loop */
      }
    }
}

void IntTest(int delay)
{
  int dummy;

  LATCHOUT(RUNNING, SET_BIT);
  mDelay(delay);
  LATCHOUT(RUNNING, CLR_BIT);
  Command(5, &dummy);
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
  float DA0TRY, DA1TRY;
  
  ADMux(0);
  LATCHOUT(ZEROCAL, CLR_BIT);
  DA0TRY = DA1TRY = 2047.0F;
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
      DAC0OUT((int)DA0TRY);
      mDelay(100);
      }
    }
  while(!kbhit());
  getch();

  fmt_outchar(1, 20, "A/D-X VOLTS= %-3.3f  D/A-X COUNTS = %-3.3f",
                      A_D_Avg[2] / 204.8, DA0TRY);

  do
    {
    AvgAD(A_D_Avg, 100);
    if(abs(A_D_Avg[4]) < 1)
      break;
    else
      {
      DA1TRY = DA1TRY + A_D_Avg[4] * 3.7;
      DAC1OUT((int)DA1TRY);
      mDelay(100);
      }
    }
  while(!kbhit());
  getch();
  
  fmt_outchar(1, 21, "A/D-Y VOLTS= %-3.3f D/A-Y COUNTS= %-3.3f",
                      A_D_Avg[4] / 204.8,
                      DA1TRY);

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
    Command(4, &dummy);                        /* START TEMP STORE */

    while (mClock() < EndTime)
      ;

    }
  erase_line(1, 21, 40);
  outchar(1, 21, "COMMAND 5 TO STOP TEMPERATURE STORING");

  Command(5, &dummy);                     /* STOP TEMPERATURE STORING */
  Command(6, &MSB);                       /* SEND MSB COUNT */
  Command(7, &LSB);                       /* SEND LSB COUNT */

  TempSet = MSB * 256 + LSB;

  fmt_outchar(1, 22, "NO OF TEMPERATURE SETS= %d", TempSet);

  if(TempSet > max_dump)
    TempSet = max_dump;

  Command(1, &MSBTmp);
  Command(2, &LSBTmp);

  fmt_outchar(1, 23, "%d", MSBTmp * 256 + LSBTmp);

  Command(2, &MSBTmp);
  Command(2, &LSBTmp);

  fmt_outchar(10, 23, "%d", MSBTmp * 256 + LSBTmp);

  Command(2, &MSBTmp);
  Command(2, &LSBTmp);

  fmt_outchar(20, 23, "%d", MSBTmp * 256 + LSBTmp);

  Command(2, &MSBTmp);
  Command(2, &LSBTmp);

  fmt_outchar(30, 23, "%d", MSBTmp * 256 + LSBTmp);

  for (i = 0; i < TempSet; i++)
    for (j = 0; j < 4; j++)
      {
      Command(2, &MSBTmp);
      Command(2, &LSBTmp);
      fmt_outchar(1, 23, "%d", MSBTmp * 256 + LSBTmp);
      }
}

void PORTBTEST(void)
{

}

