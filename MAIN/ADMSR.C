#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <dir.h>

#include "textio.h"
#include "hrclock.h"
#include "dio24.h"
#include "adtests.h"

unsigned _stklen = 0x6000;

void DoMenu(void)
{
  outchar(1, 1,  " 1) +10V REF ADJUST");
  outchar(1, 2,  " 2) TC F.S. ADJUST");
  outchar(1, 3,  " 3) DISP. SEG. TEST");
  outchar(1, 4,  " 4) LED BLINK TEST");
  outchar(1, 5,  " 5) COMMAND TEST");
  outchar(1, 6,  " 6) BATTERY TEST");
  outchar(1, 7,  " 7) RS232 TEST");
  outchar(1, 8,  " 8) A/D CLOCK TEST");
  outchar(1, 9,  " 9) TC A/D CONV TEST");
  outchar(1, 10, "10) X & Y STRAIN ZERO ADJ");
  outchar(40, 1, "11) X & Y GAIN ADJUST");
  outchar(40, 2, "12) TC ZERO/GAIN CAL");
  outchar(40, 3, "13) D/A-0 SCOPE SWEEP");
  outchar(40, 4, "14) D/A-1 SCOPE SWEEP");
  outchar(40, 5, "15) A/D -> D/A LIN TEST");
  outchar(40, 6, "16) V-EXC ADJ & TEST");
  outchar(40, 7, "17) A/D ZERO TEST");
  outchar(40, 8, "18) A/D F.S. TEST");
  outchar(40, 9, "19) STRAIN,POS A/D TEST");
  outchar(40, 10,"20) BRIDGE ZERO TEST");
  outchar(40, 11,"21) TEMPERATURE DUMP TEST");
  outchar(40, 12,"22) QUIT");
}

static int BreakAbort(void)
{
  return(0);
}

/* This is the main test procedure part. You will require a DVM, Oscilliscope */
int main(int argc, char * argv[])
{
  int quit_flag = 0, Request, i;
  unsigned InitCounter0Mode;        /* Initial PIT counter 0 mode */
  float X_Zero, Y_Zero, A_D;
  char inbuf[10];

  outportb(CONTROL, IOMODES);
  outportb(IOPORTB, ABORT);
  
  InitCounter0Mode = GetPITCounterMode(0);
  
  HRInit();
  HRMode2();

  ctrlbrk(BreakAbort);

  if (argc)
    {
    char drive[MAXDRIVE];
    char dir[MAXDIR];
    char file[MAXFILE];
    char ext[MAXEXT];
    fnsplit(argv[0], drive, dir, file, ext);
    ScreenSetup(dir);
    }

  DoMenu();
  do
    {
    outchar(1, 12, "YOUR REQUEST PLEASE");
    if(RequestInput(21, 12, inbuf, 3))
      Request = atoi(inbuf);
    else
      Request = 0;
    /* Clear variable text screen below */
    for (i = 13; i < 26; i++)
      {
      erase_line(1, i, 75);
      }
    switch(Request)
      {
      case 1:
        {
         /*  +10V Ref. Adj routine */
        outchar(5, 16, "CONNECT DVM + TO TP27 AND - TO ANGND.");
        outchar(5, 17, "ADJUST R106 +10V REF ADJ. POT FOR +10.00 VOLTS ON THE DVM");
        outchar(5, 18, "I WILL MONITOR WITH MY A/D CONVERTER");
        outchar(5, 19, "HIT ANY KEY TO ABORT");
        ADMux((2));
        mDelay(1000);
        do
          {
          AvgAD(A_D_Avg, 100);
          A_D =  A_D_Avg[3] / 204.8;
          fmt_outchar(1, 23, "A/D READ %f", A_D);
          }
        while(!kbhit());
        getch();
        A_D = abs(A_D - 10.0F);
        if (A_D > .01)
          {
          outchar(1, 21, "SOMETHING WRONG IN A/D MUX CHAIN, FIND AND FIX");
          }
        else
          {
          outchar(1, 21, "A/D READ IS ACCEPTABLE");
          }
        }
      break;
      case 2:
         /*  Thermocouple calibrate full scale adjust. */
        outchar(1, 16, "CONNECT DVM TO TP29 AND ANGND");
        outchar(1, 17, "ADJUST R107 FOR 1.7 FOR 1.7V");
        outchar(1, 18, "CYCLE POWER ON UNIT AND CHECK IF IT POWERS UP AND READS");
        out_cur_char(1, 19, "HIT ANY KEY TO CONTINUE");
        while(!kbhit());
        getch();
        eeol(1, 19, "HIT ANY KEY TO CONTINUE");
      break;
      case 3:
         /*  Front panel display segment test. */
        outchar(1, 16, "CYCLE POWER ON UNIT AND CHECK IF IT POWERS UP AND READS");
        outchar(1, 17, "CHECK ALL LED SEGMENTS AND FIX IF OUT");
        out_cur_char(1, 18, "HIT ANY KEY TO ABORT");
        DSPTest();
        while(!kbhit());
        getch();
        LATCHOUT(CPURESET, CLR_BIT);
      break;
      case 4:
         /*  Front panel ready,mark,run led test. */
        outchar(1, 16, "PLEASE OBSERVE FRONT PANEL READY,MARK & RUNNING LED'S");
        out_cur_char(1, 17, "HIT ANY KEY TO CONTINUE");
        while(!kbhit());
        getch();
        out_cur_char(1, 17, "WORKING");
        LEDTest();
        erase_line(1, 17, 25);
      break;
      case 5:
        {
        int HITEMP, LOTEMP;
         /* Temperature transfer command test */
        outchar(1, 16, "TEMPERATURE FROM INTERFACE");
        outchar(1, 17, "HIT ANY KEY TO ABORT");
        do
          {
          IntTest(1000);           /*  WAIT FOR 1 SECOND APPROX */
          Command(1, &HITEMP);
          Command(2, &LOTEMP);
          fmt_outchar(1, 18, "%3.2f", (float)(HITEMP * 256 + LOTEMP) / 10.0F);
          for (i = 1; i < 4; i++)
            {
            Command(2, &HITEMP);
            Command(2, &LOTEMP);
            fmt_outchar(1, 18, "%3.2f", (float)(HITEMP * 256 + LOTEMP) / 10.0F);
            }
          }
        while(!kbhit());
        getch();
        }
      break;
      case 6:
       /*  Battery test. */
        outchar(1, 16, "CONNECT A JUMPER FROM D/A-0 TP3 TO BATTERY JP30");
        outchar(1, 17, "HIT ANY KEY TO CONTINUE");
        ADMux((2));                      /*  SET TO GROUP 1 */
        LATCHOUT(BATSEL, SET_BIT);
        mDelay((1000));
        DAC0OUT((4000));                /*  SET D/A TO +5V */
        do
          {
          AvgAD(A_D_Avg,  100);
          A_D = A_D_Avg[0] / 204.8;
          fmt_outchar(1, 18, "D/A=4.76V, A/D READS %f ", A_D);
          }
        while(!kbhit());
        getch();
        LATCHOUT(BATSEL, CLR_BIT);
      break;
      case 7:
         outchar(1, 16, " TEST THE RS232 USING PROCOMM AND COM2");
         out_cur_char(1, 17, " OF THE HOST CPU");
         eeol(1, 17, " OF THE HOST CPU");
      break;
      case 8:
        outchar(1, 16, "CONNECT FLUKE DVM TO TP33 A/D CLOCK AND VERIFY");
        outchar(1, 17, "FREQ SETTING TO BE 65 KHZ");
        outchar(1, 18, "HIT ANY KEY TO ABORT ");
        while(!kbhit());
        getch();
      break;
      case 9:
        {
        int j = 0;
         /*  Temperature A/D converter test     */
        outchar(1, 16, "TEMPERATURE A/D TEST");
        outchar(1, 17, "HIT ANY KEY TO ABORT");
        ADMux(1);
        do
          {
          fmt_outchar(4, 19, "%d  ", j++);
          AvgAD(A_D_Avg,  100);
          for (i = 0; i < 4; i++)
            {
            fmt_outchar(12 + (i*10), 19, "%3.2f ", A_D_Avg[i] / 204.8);
            }
          }
        while(!kbhit());
        getch();
        }
      break;
      case 10:
        LATCHOUT(ZEROCAL, SET_BIT);
        DAC0OUT(2048);
        DAC1OUT(2048);
        ADMux((0));
        outchar(1, 16, "PLEASE ADJ. X&Y-STRAIN DC ZERO R1 & R2, FOR ZERO");
        outchar(1, 17, "MUST BE LESS THAN .005");
        outchar(1, 19, "X-STRAIN ZERO:            Y-STRAIN ZERO:");
        do
          {
          AvgAD(A_D_Avg,  100);
          X_Zero = A_D_Avg[1] / 204.8;
          Y_Zero = A_D_Avg[3] / 204.8;
          fmt_outchar(17, 19, "%3.4f  ", X_Zero);
          fmt_outchar(45, 19, "%3.4f  ", Y_Zero);
          }
        while(!kbhit());
        getch();
      break;
      case 11:
        {
        float MDGAIN, TDGAIN;

        outchar(1, 16, "PLEASE ADJ. X&Y-STRAIN DC GAIN R42 & R82, FOR 270");
        outchar(1, 19, "X-STRAIN GAIN:             Y-STRAIN GAIN:");
        do
          {
          GAIN(&MDGAIN, &TDGAIN);
          fmt_outchar(17, 19, "%4.3f ", MDGAIN);
          fmt_outchar(45, 19, "%4.3f ", TDGAIN);
          }
        while(!kbhit());
        getch();
        }
      break;
      case 12:
        outchar(1, 16, "CALIBRATE THE 4 CHANNEL THERMOCOUPLE WITH");
        outchar(1, 17, "OMEGA CALIBRATOR TO 0.1 DEG C.");
        outchar(1, 18, "USE FRONT PANEL DISPLAYS FOR CALIBRATION");
      break;
      case 13:
        outchar(1, 16, "CONNECT SCOPE TO TP3 AND ANGND. OBSERVE THE");
        outchar(1, 17, "TRIANGLE WAVEFORM");
        outchar(1, 18, "HIT ANY KEY TO ABORT");
       do
        {
         for (i = 0; i < 4096; i++)
           DAC0OUT(i);
        }
        while(!kbhit());
        getch();
      break;
      case 14:
        outchar(1, 16, "CONNECT SCOPE TO TP8 AND ANGND. OBSERVE THE");
        outchar(1, 17, "TRIANGLE WAVEFORM");
        outchar(1, 18, "HIT ANY KEY TO ABORT");
       do
        {
         for (i = 0; i < 4096; i++)
           DAC1OUT(i);
        }
        while(!kbhit());
        getch();
      break;
      case 15:
         /*  A/D D/A LIN TEST   */
        outchar(1, 14, "D/A IS SWEPT FROM -5V TO +5V AND A/D CONVERTED");
        outchar(1, 15, "A STRAIGHT LINE EQUATION IS CALCULATED FOR THE TEST");
        outchar(1, 16, "WORKING ");
        ADMux(3);
        mDelay(2000);
        LinearCalc();
        outchar(1, 16, "ALL DONE");
      break;
      case 16:
        {
        float X_EXC, Y_EXC;
        outchar(1, 16, "ADJUST X & Y V-EXC R8 & R62 FOR +5.000V");
        outchar(1, 17, "USE DVM AT TP7 & TP2 FOR ACCURACY");
        outchar(1, 18, "VERIFY A/D TO BE WITHIN 0.1%");
        outchar(1, 19, "HIT ANY KEY TO ABORT");
        ADMux(2);
        outchar(1, 20, "X-EXC VOLT:                Y-EXC VOLT:");
        do
          {
          AvgAD(A_D_Avg, 100);
          X_EXC = A_D_Avg[1] / 204.8;
          Y_EXC = A_D_Avg[2] / 204.8;
          fmt_outchar(17, 20, "%3.4f  ", X_EXC);
          fmt_outchar(41, 20, "%3.4f  ", Y_EXC);
          }
        while(!kbhit());
        getch();
        }
      break;
      case 17:
        outchar(1, 16, "A/D ZERO TEST. SHOULD BE LESS THAN .005");
        outchar(1, 17, "CAN ONLY TEST CHANNEL 3 & 4");
        outchar(1, 18, "HIT ANY KEY TO ABORT");
        ADMux(3);
        outchar(1, 20, "A/D ZERO CHN 3:            A/D ZERO CHN 4:");
        do
          {
          AvgAD(A_D_Avg,  (100));
          X_Zero = A_D_Avg[2] / 204.8;
          Y_Zero = A_D_Avg[3] / 204.8;
          fmt_outchar(17, 20, "%3.4f  ", X_Zero);
          fmt_outchar(45, 20, "%3.4f  ", Y_Zero);
          }
        while(!kbhit());
        getch();
      break;
      case 18:
        outchar(1, 16, "A/D FULL SCALE TEST. SHOULD BE LESS THAN .005");
        outchar(1, 17, "CAN ONLY TEST CHANNEL 3 & 4");
        outchar(1, 18, "HIT ANY KEY TO ABORT");
        ADMux(2);
        outchar(1, 20, "A/D FULL SCALE CHN 3:");
        do
          {
          AvgAD(A_D_Avg,  100);
          X_Zero = A_D_Avg[3] / 204.8;
          fmt_outchar(25, 20, "%3.4f  ", X_Zero);
          }
        while(!kbhit());
        getch();
      break;
      case 19:
        ADMux(0);
        LATCHOUT(ZEROCAL, CLR_BIT);
        do
          {
          AvgAD(A_D_Avg,  100);
          outchar(1, 16, "HIT ANY KEY TO ABORT");
          outchar(5, 18,  "X-POS");
          outchar(13, 18, "X-STRAIN");
          outchar(24, 18, "Y-POS");
          outchar(33, 18, "Y-STRAIN");
          for (i = 0; i < 4; i ++)
            {
            fmt_outchar(4 + (i * 10), 20, "%3.4f  ", A_D_Avg[i] / 204.8);
            }
          }
        while(!kbhit());
        getch();
      break;
      case 20:
        outchar(1, 16, "HIT ANY KEY TO ABORT");
        BRDGZERO();
      break;
      case 21:
        {
        int MODE, DELAY_SEC, MAX_DUMP;

        outchar(1, 16, "HIT ANY KEY TO ABORT");
        outchar(1, 17, "Input RUN/CMD: 0,1; #SEC; # SET DUMP");
        scanf("%d %d %d", &MODE, &DELAY_SEC, &MAX_DUMP);
        erase_line(1, 18, 80);
        do
          TEMPDUMP(MODE, DELAY_SEC, MAX_DUMP);
        while(!kbhit());
        getch();
        DoMenu();
        }
      break;
      case 22:
        quit_flag = 1;
      break;
      case 23:
        /* PORTBTEST */
      break;
      default:
        outchar(1, 13, "INCORRECT REQUEST");
      break;
      }
    }
    while (quit_flag == 0);
    outportb(CONTROL, INPUTMODE);
    EndScreen();
    (InitCounter0Mode == 2) ? HRMode2() : HRMode3();
    return 0;
}

