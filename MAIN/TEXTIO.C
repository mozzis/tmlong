/* textio.c */
/* part of admsr program */
/* provide RequestString and Outchar functions */
/* al a Basic's INPUT and LOCATE : PRINT stuff */

#include <graphics.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <conio.h>
#include <dos.h>

#include "textio.h"

static struct viewporttype vport;
static int theight, twidth, xorg, yorg, tyinc;
static char outbuf[81];


void cursor_out(int x, int y)
{
  int xcrd = xorg + (x * twidth),
      ycrd = yorg + (y+1) * theight - tyinc;

  line(xcrd, ycrd, xcrd + twidth, ycrd);
}
    
void erase_line(int x, int y, int len)
{
  struct fillsettingstype oldstyle;

  getfillsettings(&oldstyle);

  setfillstyle(EMPTY_FILL, getbkcolor());
  bar(xorg + (x * twidth),                   /* LEFT */
      yorg + (y * theight),                  /* TOP */
      xorg + ((x+len) * twidth),             /* RIGHT */
      yorg + ((y+1) * theight) - tyinc);     /* BOTTOM */

  setfillstyle(oldstyle.pattern, oldstyle.color);
}

void outchar(int x, int y, char * Outstring)
{
  erase_line(x, y, strlen(Outstring));
  outtextxy(xorg + (x * twidth), yorg + (y * theight), Outstring);
}

void fmt_outchar(int x, int y, const char * FmtStr, ...)
{
  va_list insert_args;
  
  va_start(insert_args, FmtStr);
  vsprintf(outbuf, FmtStr, insert_args);
  va_end(insert_args);
  outchar(x, y, outbuf);
}

void out_cur_char(int x, int y, char * Outstring)
{
  outchar(x, y, Outstring);
  cursor_out(x + strlen(Outstring), y);
}

void eeol(int x, int y, char * Outstring)
{
  int len = strlen(Outstring);

  erase_line(x+len, y, 80 - (x + len));
}

int RequestInput(int x, int y, char * Instring, int maxchar)
{
  int cpos = 0, done = 0;
  char inchar;

  Instring[0] = '\0';

  do
    {
    erase_line(x, y, maxchar);
  
    if (cpos)
      out_cur_char(x, y, Instring);
    
    cursor_out(x+cpos, y);
    
    inchar = getch();
    switch (inchar)
      {
      case 0x0d:
        done = 1;
      break;
      case 0x08:
        if (cpos)
          {
          cpos--;
          Instring[cpos] = '\0';
          }
      break;
      default:
        Instring[cpos++] = inchar;
        Instring[cpos] = '\0';
      }
    }
  while(!done && cpos < maxchar);

  erase_line(x+cpos, y, maxchar);
  return(cpos);
}


void ScreenSetup(char * driverpath)
{
  /* request auto detection */
  int gdriver = EGA, gmode = EGAHI, errorcode;

  /* initialize graphics and local variables */
  initgraph(&gdriver, &gmode, driverpath);

  /* read result of initialization */
  errorcode = graphresult();
  
  if (errorcode != grOk)  /* an error occurred */
  {
     printf("Graphics error: %s\n", grapherrormsg(errorcode));
     printf("Press any key to halt:");
     getch();
     exit(1); /* terminate with an error code */
  }

  window(1, 1, 80,12);
  setcolor(WHITE);
  setbkcolor(EGA_BLUE);
  settextjustify(LEFT_TEXT, TOP_TEXT);

  /* clear the screen */
  cleardevice();

  getviewsettings(&vport);
//  setviewport(vport.left, vport.top,
//                  vport.left + twidth * 80, vport.top + theight * 23, 1);
//
  theight = textheight("Zgpi");
  tyinc = theight / 4;
  theight += tyinc;
  twidth  = textwidth("X");

  xorg = vport.left;
  yorg = vport.top;
}

void EndScreen(void)
{
  closegraph();
}
