/* dio24.c */

#include <conio.h>

#include "hrclock.h"
#include "dio24.h"

float far A_D_Avg[5];

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


void ADMeasure(long A_D_Buff[])
{
  long C;
  int A, ADCHNL;

  putout(IOPORTB, ADTRIG);
  while((getin(IOPORTC) & 0x20) == 0)
    ;

  A = getin(IOPORTA);
  
  while(!(getin(IOPORTC) & 0x20))
    ;
  
  C = (A << 8) + getin(IOPORTA);

  if (C > 32767L)
    C = C - 65536L;

  A_D_Buff[0] = C;

  for (ADCHNL = 1; ADCHNL < 4; ADCHNL++)
    {
    putout(IOPORTB, ADNEXT);
    while(!(getin(IOPORTC) & 0x20))
      ;

    A = getin(IOPORTA);
    while(!(getin(IOPORTC) & 0x20))
      ;
    
    C = (A << 8) + getin(IOPORTA);

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
    for (j = 3; j >= 0; j--)
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
  putout(IOPORTA, INTRFCCNTRL);
}

/* Output a count to offset D/A converter number # in counts.  */
/* 0 count=-5V, and 4095 count=+5V.                            */

void DAC0OUT(int Value)
{
  int A, B;

  A = Value / 0x100;
  B = Value & 0xFF;
  putout(IOPORTB, DAC0LOADD);
  putout(IOPORTA, B);
  putout(IOPORTB, DAC0HIADD);
  putout(IOPORTA, A);
}

void DAC1OUT(int Value)
{
  int A, B;

  A = Value / 0x100;
  B = Value & 0xFF;
  putout(IOPORTB, DAC1LOADD);
  putout(IOPORTA, B);
  putout(IOPORTB, DAC1HIADD);
  putout(IOPORTA, A);
}

void Command(int Cmd, int *CmdEcho)
{
  putout(IOPORTB, INTRFCCOMMAND);   /* COMMAND BYTE ADDR */
  putout(IOPORTA, Cmd);             /* COMMAND BYTE VALUE */
  LATCHOUT(DATAREQ, SET_BIT);         /* SET DATA REQ TO 8032 */
  while ((getin(IOPORTC) & 0x20) == 0)
    ;                                 /* WAIT FOR ACK FOR 8032 */
  LATCHOUT(DATAREQ, CLR_BIT);         /* CLEAR DATA REQ */
  getin(IOPORTA);                       /* DUMMY READ TO EMPTY PORTA */
  putout(IOPORTB, IOREAD);          /* OUTPUT INTERFACE BUFF ADDR */
  while ((getin(IOPORTC) & 0x20) == 0)
    ;                                 /* WAIT FOR ACK FOR 8032 */
  
  *CmdEcho = getin(IOPORTA);            /* READ ECHO FROM 8032 */
}

