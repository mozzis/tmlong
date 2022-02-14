#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>

#include "dioio.h"
// #include "hrclock.h"

int _stklen = 0x2000;
float A_D_Avg[5];

static void uDelay(unsigned long delay)
{
  int dummy = 0;
  unsigned long i;

  for (i = 0; i < delay; i++)
    {
    if ((dummy++ & 0xFFFF) == 0)
      dummy &= 0xFFFF;

    }
}

int main(void)
{
  int i, j = 0;

  outportb(CONTROL, IOMODES);
  outportb(IOPORTB, ABORT);
  
   /*  Temperature A/D converter test     */
  ADMux(1);
  do
    {
    AvgAD(A_D_Avg,  100);
    printf("\r%d  ", j++);
    for (i = 0; i < 4; i++)
      {
      printf("%3.2f  ", A_D_Avg[i] / 204.8);
      }
    }
  while(!kbhit());
  getch();
  outportb(CONTROL, INPUTMODE);
  return(0);
}

void ADMux(int GROUP)
{
/* GROUP 0 = X-POS, X-STRAIN, Y-POS,   Y-STRAIN */
/* GROUP 1 = TM1,   TM2,      TM3,     TM4 */
/* GROUP 2 = V-BAT, V-EXC-X,  V-EXC-Y, V-REF */
/* GROUP 3 = DAC0,  DAC1,     ANGND,   ANGND */

  outportb(IOPORTB, MUXCHANNEL + GROUP);
}

#define WDELAY 2

void ADMeasure(long A_D_Buff[])
{
  long C;
  int A, B, ADCHNL;

  outportb(IOPORTB, ADTRIG);
  do
    uDelay(WDELAY);
  while( (inp(IOPORTC) & 0x20) == 0);

  uDelay(WDELAY);
  A = inp(IOPORTA);
  
  do
    uDelay(WDELAY);
  while( (inp(IOPORTC) & 0x20) == 0);
  
  uDelay(WDELAY);
  B = inp(IOPORTA);
  C = A * 256 + B;
  if (C > 32767L)
    C = C - 65536L;

  A_D_Buff[0] = C;

  for (ADCHNL = 1; ADCHNL < 4; ADCHNL++)
    {
    uDelay(WDELAY);
    outportb(IOPORTB, ADNEXT);
    do
      uDelay(WDELAY);
    while( (inp(IOPORTC) & 0x20) == 0);

    uDelay(WDELAY);
    
    A = inp(IOPORTA);
    do
      uDelay(WDELAY);
    while( (inp(IOPORTC) & 0x20) == 0);
    uDelay(WDELAY);
    B = inp(IOPORTA);
    uDelay(WDELAY);
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
