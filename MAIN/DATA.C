#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <time.h>
#include <conio.h>
#include <cgibind.h>

#include "error.h"
#include "data.h"
#include "colors.h"
#include "cwindows.h"
#include "hrclock.h"
#include "dio24.h"
#include "symbol.h"
#include "device.h"

DATA RunData = {
               2,              /* CurveCount */
               0x24,           /* typeData */
               DEFAULT_POINTS, /* Number of Points */
               0               /* Pointer to data */
               };

ERROR_CATEGORY allocDataPoints(void)
{
  float * RunPoints = 0;

  if (RunData.Points)
    {
    free(RunData.Points);
    RunData.Points = 0;
    }

  if (!(RunPoints = malloc(RunData.Count * RunData.CurveCount * sizeof(float))))
    return(error(ERROR_ALLOC_MEM));

  RunData.Points = RunPoints;
  return(ERROR_NONE);
}

ERROR_CATEGORY freeDataPoints(void)
{
  if (RunData.Points)
    {
    free(RunData.Points);
    RunData.Points = 0;
    }
  return(ERROR_NONE);
}


char text[80];

char * AcquireMessage[] = {
                          "                                              ",
                          "     Working to acquire linearity data",
                          "           Press any key to abort",
                          "                                              ",
                          "                                              ",
                          NULL
                          };
                          

int GenRealData(void)
{
  WINDOW * MessageWindow = NULL;
  int Points = RunData.Count, V;    
  float Y1, Ch1_Hi, Ch1_Lo, Ch1_Slope, Ch1_Intcpt, Delta1;
  float * Points0, *Points1;
//  unsigned char WinAtt;

  if (allocDataPoints())
    return(1);

  Points0 = RunData.Points;
  Points1 = &(RunData.Points[RunData.Count]);

  outportb(CONTROL, IOMODES);
  outportb(IOPORTB, ABORT);
  
  put_up_message_window(AcquireMessage, COLORS_MESSAGE, &MessageWindow);

  HRMode2();
  HRInit();

  mDelay(1000);
  
  ADMux(3);

  mDelay(1000);

  DAC0OUT(0);
  DAC1OUT(0);

  uDelay(100000L);
  AvgAD(A_D_Avg, 100);

  Ch1_Lo = A_D_Avg[0];
  
  DAC0OUT(Points);
  DAC1OUT(Points);

  uDelay(10000);
  AvgAD(A_D_Avg, 100);

  Ch1_Hi = A_D_Avg[0];
  
  Ch1_Slope = (Ch1_Hi - Ch1_Lo) / Points;
  Ch1_Intcpt = Ch1_Lo;

  for(V = 0; V < Points; V++)
    {
    DAC0OUT(V);
    DAC1OUT(V);
    uDelay(1000);
    AvgAD(A_D_Avg, 10);

    Y1 = Ch1_Slope * (float)V + Ch1_Intcpt;
    Delta1 = Y1 - A_D_Avg[0];

    Points0[V] = A_D_Avg[0];
    Points1[V] = A_D_Avg[1];
    
    if (!(V % 20))
      {
      sprintf(text, "Point %-4.4d Y1= %-4.4f D1= %-#3.4g ", V, Y1, Delta1);
      txt_to_win(MessageWindow, text, 5, 6, REGULAR_COLOR);
      }

    if (kbhit())
      {
      getch();
      break; /* Terminate loop */
      }
    }
  release_message_window(MessageWindow);
  outportb(CONTROL, INPUTMODE);
  return(0);
}
