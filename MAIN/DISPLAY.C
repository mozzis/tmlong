/*****************************************************************************
 *
 * An example program to demonstrate a variety of features available in
 * the GSS*CGI Graphics Development Toolkit.  This program is designed
 * to be compiled with Microsoft C version 5.10 (or 6.0) or Turbo C.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#ifdef __TURBOC__   /* If TURBO C use farmalloc */
#include <alloc.h>
#else       /* otherwise assume MS C */
#include <malloc.h>
#endif

#include "display.h"

#ifdef __TURBOC__   /* If TURBO C use farmalloc */
#define ALLOCATE_FAR_MEMORY(x) (char far *) farmalloc(x)
#define FREE_FAR_MEMORY(x) farfree (x)
#else       /* otherwise assume MS C */
#define ALLOCATE_FAR_MEMORY(x) (char far *) halloc(x, sizeof (char))
#define FREE_FAR_MEMORY(x) hfree (x)
#endif

/**********************************************************************
 *
 * Public variables
 *
 **********************************************************************/

char far   *where;      /* Where to load drivers */
CCONFIGURATION config;                  /* GSS*CGI Configuration structure */
CDVCAPABILITY cap_display;    /* The DISPLAY device's capabilities */
CDVCAPABILITY cap_mouse;    /* The MOUSE device's capabilities */
CDVHANDLE h_display;      /* The DISPLAY device handle */
CDVHANDLE h_mouse;      /* The MOUSE device handle */
int transient;        /* Transient-drivers-loaded flag */

/*
 * The input arrays to the Open Workstation calls.
 */

CDVOPEN in_mouse = {
    CFullScreen,  /* 32K-by-32K transform mode */
    CLN_Solid,    /* Solid line type */
    1,      /* White line color */
    CMK_Star,   /* Star marker */
    1,      /* White marker color */
    1,      /* Hardware graphics text font */
    1,      /* White graphics text color */
    CHollowFill,  /* Hollow interior style */
    0,      /* Don't-care fill style index */
    1,      /* White fill color */
    CYes,   /* prompt for paper changes */
    "MOUSE"             /* MOUSE logical device */
};

CDVOPEN in_display = {
    CFullScreen,  /* 32K-by-32K transform mode */
    CLN_Solid,    /* Solid line type */
    1,      /* White line color */
    CMK_Star,   /* Star marker */
    1,      /* White marker color */
    1,      /* Hardware graphics text font */
    1,      /* White graphics text color */
    CHollowFill,  /* Hollow interior style */
    0,      /* Don't-care fill style index */
    1,      /* White fill color */
    CYes,   /* prompt for paper changes */
    "DISPLAY"           /* Display logical device */
};

/*
 * The color array used for the Output and Inquire Cell Array functions.
 */

CCOLOR  colary[] = {
    4, 5, 6, 1, 2, 3
};

/*
 * The points of the arrow filled area represented as an array of xy
 * coordinates.
 */

CXY polyxy[] = {
    {24640, 12800},
    {24640, 19200},
    {24000, 19200},
    {24960, 20800},
    {25920, 19200},
    {25280, 19200},
    {25280, 12800}
};

/*
 * The locations of the polymarkers on the grid, represented as an array
 * of xy coordinates.
 */

CXY markers[] = {
    {8000, 9600},
    {11200, 12800},
    {14400, 16000},
    {17600, 19200},
    {20800, 22400}
};

/* globals */

void output_title(char *title, int mode);

  CXY  chwh;             /* Graphics text character width */
                         /*    and height */
  CXY  cellwh;           /* Graphics text character cell */
  char cstrng[80];       /* General-purpose character buffer */
  CRGB col2sav;          /* CRGB color triple */
  CRGB color_in;         /* Input CRGB color triple */
  CRGB color_out;        /* Output CRGB color triple */
  CYESNO echo;           /* Request String echo flag */
  CCURHANDLE gcursor;    /* Graphics cursor handle */
  CHORALIGN far h_align; /* Horizontal alignment */
  int  i;                /* Loop control variable */
  int  icount;           /* Number of available line widths */
  CINTERIORFILL interior;  /* Fill interior style */
  int  itemp1;           /* Place holder variable */
  int  itemp2;           /* Place holder variable */
  int  itemp3;           /* Place holder variable */
  int  j;                /* Array index variable */
  CREQLOCATOR locator;   /* Request locator structure */
  int  mouse_stat;       /* Mouse Open Workstation status */
  CXY  ptsin[10];        /* General-purpose point input array */
  CRECT  rectangle;      /* Rectangle structure for Bar etc. */
  CROWCOL row_col;       /* Cursor text rows and columns */
  CROWCOL save_row_col;  /* Save Cursor text rows and columns */
  int  selected;         /* Variable used for returned value */
  CFILLSTYLE sel_style;  /* Selected fill style index */
  int  str_len;          /* String length variable */
  CFILLSTYLE style;      /* Fill style index */
  int  val;              /* Marker y-position value */
  CVERTALIGN far v_align;/* Vertical alignment */
  int  x;                /* Temporary x coordinate */
  CXY  xy;               /* An XY structure for a point */
  int  yoff;             /* Distance between characters */
                         /*    width and height */

  /**********************************************************************
  *
  * The grid frame.
  *
  * There is a solid, white box around the edges of the display surface.
  *
  * At the top of this frame is a row of six lines, each in a different
  * type and color.
  *
  * Immediately below that is a two-line, white, centered title.
  *
  * Immediately below that is a multi-colored line grid with a cyan
  * (pastel blue) diamond marker at five of the vertices along the
  * lower-left to upper-right diagonal of the grid.
  *
  * To the left of the grid is a column of six markers, each of a
  * different type and color.
  *
  * To the right of the grid is a filled area in the shape of a white
  * arrow, pointing up.
  *
  * Below the arrow is the version number of the driver being run.
  *
  * Finally, below the grid is a line with six markers along its length.
  * Each marker is larger than the one to its left.  The markers are
  * evenly spaced along the line.
  *
  * Colors will only appear on color-capable or gray-scale devices.  All
  * other devices will show black and white only.
  *
  **********************************************************************/
void grid_frame(void)
{

  /* draw a box around entire display surface area */
  box (0, 0, 32767, 32767);

  /* set character height */
  if (CSetGTextHeight (h_display, 800, &chwh, &cellwh))
    {
    report_error ("CSetGTextHeight", GMODE);
    }

  if (CSetGTextAlign(h_display, CTX_Center, CTX_Base,
    (CHORALIGN far *)&h_align, (CVERTALIGN far *)&v_align))
    {
    report_error ("CSetGTextAlign", GMODE);
    }

  /* label the output */
  xy.x = 16000;
  xy.y = 26880;
  CGText (h_display, xy, "Display Module");
  xy.y = 24640;
  CGText (h_display, xy, "TM Long Project");

  /* change polyline type back to index 1 */
  CSetLineType (h_display, CLN_Solid, (CLINETYPE *)&selected);

  /* test moves and draws. grid will be output */

  for (i = 1; i <= 6; i++)
    {
    x = 3200 * i + 1600;
    /* set polyline line color index to i */
    CSetLineColor (h_display, (CCOLOR)i, (CCOLOR *)&selected);

    /* draw a line */
    ptsin[0].x = x;
    ptsin[0].y = 6400;
    ptsin[1].x = x;
    ptsin[1].y = 22400;
    CPolyline (h_display, 2, ptsin);

    /* output next line */
    ptsin[0].x = 4800;
    ptsin[0].y = x + 1600;
    ptsin[1].x = 20800;
    ptsin[1].y = x + 1600;
    CPolyline (h_display, 2, ptsin);
    }

  /* set polymarker marker type to type CMK_Diamond */
  CSetMarkerType (h_display, CMK_Diamond, (CMARKERTYPE *)&selected);

  /* output the markers */
  CPolymarker (h_display, 5, markers);

  /* test draws and markers */

  /* set polyline color index to 1 */
  CSetLineColor (h_display, (CCOLOR)1, (CCOLOR *)&selected);

  /* set polymarker color index to 1 */
  CSetMarkerColor (h_display, (CCOLOR)1, (CCOLOR *)&selected);

  CSetGTextAlign(h_display,CTX_Left,CTX_Base,(void far *)&h_align,
                  (void far *)&v_align);
}


  /**********************************************************************
  *
  * Load up the GSS*CGI controller and its drivers, then open the
  * workstations used in this program.  The MOUSE workstation is opened
  * first; then the DISPLAY workstation.
  *
  * If the MOUSE open fails, then the locator cursor in the Request
  * Locator Frame below will be moved with the keyboard arrow keys via
  * the DISPLAY driver.  This will be done by setting the mouse handle to
  * the display handle.  The request locator code below uses h_mouse (the
  * mouse handle) as the input device handle.  If it's really equal to
  * the display device handle, then input will be obtained from the
  * display device.  The display device gets input from the keyboard
  * arrow keys.
  *
  **********************************************************************/

void main(void)
{

  if (load_configuration() < 0)
    {
    exit (-1);
    }

  mouse_stat = COpenWorkstation (&in_mouse, &h_mouse, &cap_mouse);

  if (COpenWorkstation (&in_display, &h_display, &cap_display))
    {
    printf ("Error %d opening device display\n", CInqCGIError ());
    exit (-1);
    }

  if (mouse_stat)
    {
    h_mouse = h_display;
    }

  grid_frame();

  newfrm ();
 

  /* restore display to normal */
  if (CEnterCTextMode (h_display))
    {
    report_error ("CEnterCTextMode", GMODE);
    }

  /* close the workstation */
  if (CCloseWorkstation (h_display))
    {
    report_error ("CCloseWorkstation", GMODE);
    }

  if (remove_configuration () < 0)
    {
    exit (-4);
    }
}


/************************************************************************
 *
 * Load GSS*CGI Configuration
 *
 ************************************************************************/

/***********************************************************************
 *
 * Find out what, if any, GSS*CGI configuration is in memory and proceed
 * to load it in, if it's not present.
 *
 ************************************************************************/

int load_configuration ()
{
  auto int    error;

  config.CGIPath = NULL;
  config.Where = NULL;
  config.Available = 0L;
  config.Required = 0L;
  error = 0;

  if (CCgiConfiguration (CStatusCGI, &config) == 0)
    {
    switch (config.Status)
      {
      case CNotLoaded:
        if (CCgiConfiguration (CLoadCGI, &config) &&
          (CInqCGIError () == MEMORY_TOO_SMALL_FOR_GSSCGI) &&
          ((config.Where = ALLOCATE_FAR_MEMORY (config.Required)) !=
          (char far *) 0))
          {
          config.Available = config.Required;
          if (CCgiConfiguration (CLoadCGI, &config))
            {
          error = CInqCGIError ();
          }
        }
      else
        {
        error = CInqCGIError ();
        }
      break;

      case CLoadedStatic:
      case CTransientLoaded:
      case CLoadedTSR:
      case CLoadedApp:
      break;                  /* No action necessary */

      case CTransient:
        error = load_drivers();
      break;
        }
    }

    return (error);
}


/************************************************************************
 *
 * Remove the GSS*CGI Configuration
 *
 ************************************************************************/

/***********************************************************************
 *
 * If the GSS*CGI configuration was loaded by the application, then
 * remove it.
 *
 ************************************************************************/

int remove_configuration (void)
{
  int error;

  config.CGIPath = NULL;
  /* Save config.Where for call to FREE_FAR_MEMORY below */
  config.Available = 0L;
  config.Required = 0L;
  error = 0;

  if (CCgiConfiguration (CStatusCGI, &config) == 0)
    {
    switch (config.Status)
      {
      case CLoadedApp:
        if (CCgiConfiguration (CRemoveCGI, &config))
          {
          error = CInqCGIError ();
          }
        FREE_FAR_MEMORY (config.Where);/* Release the configuration memory */
      break;

      case CTransientLoaded:
        remove_drivers ();
      break;

      case CNotLoaded:
      case CLoadedStatic:
      case CTransient:
      case CLoadedTSR:
      break;                  /* No action necessary */
      }
    }

  return (error);
}


/************************************************************************
 *
 * Load Drivers
 *
 ************************************************************************/

int load_drivers (void)
/***********************************************************************
 *
 * Find out what GSS*CGI configuration is currently in memory, and load
 * transient drivers if possible.  If an error occurs, display a message
 * to the user and return the error.  If no error occurs, return 0.  The
 * three possible configurations are:
 *
 *  A.  GSS*CGI not in memory
 *
 *  This means that GSS*CGI is not in memory in any form.  This can
 *  happen when the user fails to run the GSS*CGI Device Driver
 *  Management Utility to load the GSS*CGI system before running a
 *  GSS*CGI application.
 *
 *  B.  GSS*CGI in memory, drivers are loaded
 *
 *  This means that the GSS*CGI drivers listed in CGI.CFG are in
 *  memory ready to run.  This happens in one of two ways.  Either
 *  the GSS*CGI Device Driver Management Utility has been run a
 *  second time (see configuration C), or the /T switch was not
 *  given for the GSSCGI.SYS file in CGI.CFG and the GSS*CGI Device
 *  Driver Management Utility has been run at least once.
 *
 *  C.  GSS*CGI in memory, ready to load drivers
 *
 *  This means that GSS*CGI is in memory in the form of a small
 *  piece of code that loads and removes GSS*CGI device drivers.
 *  This happens when the /T switch was given for the GSSCGI.SYS
 *  file on a DRIVER= line in CGI.CFG and the GSS*CGI Device Driver
 *  Management Utility has been run once.
 *
 * The code that follows checks for and reacts to each of these
 * configurations.  Up front, it assumes that GSS*CGI is in memory ready
 * to load drivers.  Using the Load CGI function, it attempts to obtain
 * the amount of memory required to load the GSS*CGI device drivers.  A
 * functional return value greater than zero proves the assumption true.
 * A negative value means one of three things: GSS*CGI is not presently
 * in memory, drivers are already loaded or a fatal error occurred.
 *
 * If the return value is greater than zero, it represents the amount of
 * memory required to load the GSS*CGI device drivers.  This value is
 * given in bytes.  An attempt is made to allocate the required amount
 * of memory.  Succeeding in this, the program loads the drivers and
 * sets a flag as a reminder that it loaded them.  Before it terminates,
 * but after it closes all open workstations, it will remove the drivers
 * using the Kill CGI function.  Failure to obtain the required memory
 * will result in the display of a warning message and the termination
 * of the program.
 *
 * If the return from Load CGI is negative and the error value indicates
 * that GSS*CGI is not presently in memory, you will see a message on
 * the screen directing you to run the GSS*CGI Device Driver Management
 * Utility.  If GSS*CGI is in memory and drivers are loaded, then a note
 * will be made of the fact, so that later on the program won't attempt
 * to remove drivers it didn't load.
 *
 **********************************************************************/
{
  auto int  err;      /* Error variable */
  auto long bytes_needed;   /* Memory required for driver load */

  err = 0;        /* Assume no error */

  if (((CLoadCgi ((char far *) 0, 0L, &bytes_needed))))
    {
    switch ((err = CInqCGIError ())) {
      case CGI_NOT_PRESENT:
        printf ("\n%s%s%s\n",
          "GSS*CGI is not presently in memory.  Please load it by running the GSS*CGI\n",
          "Device Driver Management Utility, DRIVERS.EXE.  Then try running this\n",
          "program again.\n");
      break;

      case DRIVERS_ALREADY_LOADED:
      case CGI_NOT_TRANSIENT:
          err = 0;    /* These conditions are okay */
          transient = CFalse;
      break;

      default:
        printf ("\n%s%d.\n%s\n",
          "The following unrecognized error was returned from Load CGI: ", err,
          "Please refer to the GSS*CGI Programmer's Guide for the appropriate action.\n");
      break;
      }
    }
  else if ((where = ALLOCATE_FAR_MEMORY (bytes_needed)) != (char far *) 0)
    {
    CLoadCgi (where, bytes_needed, &bytes_needed);
    transient = CTrue;
    }
  else
    {
    printf ("\n%s%s%s%s\n",
      "The program was unable to allocate sufficient memory to load GSS*CGI\n",
      "device drivers.  Please load them manually, using the GSS*CGI Device\n",
      "Driver Management Utility, DRIVERS.EXE.  Or remove resident programs\n",
      "in order to make additional memory available.\n");
    err = INSUFFICIENT_MEMORY;
    }

    return (err);
}


/************************************************************************
 *
 * Remove Drivers
 *
 ************************************************************************/

void remove_drivers(void)
/**********************************************************************
 *
 * If the program loaded GSS*CGI device drivers with Load CGI, then
 * remove the drivers and release the memory used to contain them.
 *
 **********************************************************************/
{
  if (transient == CTrue)
    {
    CKillCgi ();
    FREE_FAR_MEMORY (where);
    }
}


/************************************************************************
 *
 * Draw a Box
 * inputs:
 * int x;        X-coordinate of lower left corner
 * int y;        Y-coordinate of lower left corner
 * int w;        Width of box
 * int h;        Height of box
 *
 *
 ************************************************************************/

void box (int x, int y, int w, int h)
{
  CXY    ptsin[5];      /* Five points to make up the box */

  /* use polyline to output the box */
  ptsin[0].x = x;
  ptsin[0].y = y;
  ptsin[1].x = x + w;
  ptsin[1].y = y;
  ptsin[2].x = x + w;
  ptsin[2].y = y + h;
  ptsin[3].x = x;
  ptsin[3].y = y + h;
  ptsin[4].x = x;
  ptsin[4].y = y;
  if (CPolyline (h_display, 5, ptsin))
    {
    report_error ("CPolyline", GMODE);
    }
}


/************************************************************************
 *
 * New Frame
 *
 ************************************************************************/

void newfrm (void)
{
  CXY     xy;
  char    cstrng[80];
  int     str_len;

  /* this routine outputs prompt, pauses for input, and clears workstation */
  /* upon receipt of the input */

  /* Do the pause if desired */
  if (in_display.Prompts && cap_display.DeviceType == CCRTDevice)
    {
    xy.x = 5600;
    xy.y = 30000;
    if (CGText (h_display, xy, "Tap `RETURN' to continue..."))
      {
      report_error ("CGText", GMODE);
      }

    /* input the string from the device */
    xy.x = 0;
    xy.y = 0;
    if (CReqString (h_display, 80, 0, xy, &str_len, cstrng))
      {
      report_error ("CReqString", GMODE);
      }
    }

  /* clear the workstation */
  if (CClearWorkstation (h_display))
    {
    report_error ("CClearWorkstation", GMODE);
    }
}


/************************************************************************
 *
 * Wait CR
 *
 ************************************************************************/

void waitcr(void)
{
  /* prompt for a CR and wait --  * *  callable from cursor mode only
    */

  CROWCOL row_col;
  CXY   xy;
  char  cstrng[80];
  int   str_len;

  /* Do the pause if desired */
  if (in_display.Prompts)
    {
    /* write the prompt */
    row_col.Row = 1;
    row_col.Col = 26;
    if (CSetCTextAddr (h_display, row_col))
      {
      report_error ("CSetCTextAddr", CMODE);
      }
    if (CCText (h_display, "Tap `RETURN' to continue..."))
      {
      report_error ("CCText", CMODE);
      }

    /* wait for string input from the device */
    xy.x = 0;
    xy.y = 0;
    if (CReqString (h_display, 80, 0, xy, &str_len, cstrng))
      {
      report_error ("CReqString", CMODE);
      }
    }
}


/************************************************************************
 *
 * Report Error
 *
 ************************************************************************/

void report_error (char *func, int mode)
{
  CGTEXTREPR  attribs;
  char  blanks[80],
        err_cstrng[80];
  CXY   xy;
  int   errnum;
  CHORALIGN horiz;
  char  i;
  CROWCOL row_col,
          save_row_col;
  int   str_len;
  CVERTALIGN  vert;

  /* initialize output strings */
  for (i = 80; i--; blanks[i] = ' ')
    {
    ;
    }

  blanks[79] = '\000';
  errnum = -(CInqCGIError ());
  sprintf (err_cstrng,
    "Error no. %d in CGI function %s.  Tap `RETURN' to continue...",
    (int) errnum, func);

  switch (mode)
    {
    case GMODE:
      if (in_display.Prompts && cap_display.DeviceType == CCRTDevice)
        {
        /* save current attributes; set alignment to {center,base} */
        if (CInqGTextRepr(h_display, &attribs) ||
          CSetGTextAlign (h_display, CTX_Center, CTX_Base,
          (void far *)&horiz, (void far *)&vert))
          {
          fatal (errnum, func);
          }

        /* fill the message area with blanks, then write the prompt */
        xy.x = 16384;
        xy.y = 31600;
        if (CGText(h_display,xy,blanks) || CGText(h_display,xy,err_cstrng))
          {
          fatal (errnum, func);
          }

        /* wait for string input */
        xy.x = 0;
        xy.y = 0;
        if (CReqString (h_display, 80, 0, xy, &str_len, err_cstrng))
          {
          fatal (errnum, func);
          }

        /* reinstate alignment */
        if (CSetGTextAlign (h_display, attribs.HorizAlign, attribs.VertAlign,
          (void far *)&horiz, (void far *)&vert))
          {
          fatal (errnum, func);
          }
        }
    break;

    case CMODE:
      {

      /* save current cursor position */
      if (CInqCTextAddr (h_display, &save_row_col))
        {
        fatal (errnum, func);
        }

      /* move cursor to position (2,0) */
      row_col.Row = 2;
      row_col.Col = 0;
      if (CSetCTextAddr (h_display, row_col))
        {
        fatal (errnum, func);
        }

      /* fill message area with blanks & overwrite error message */
      if (CCText (h_display, blanks) ||
        CSetCTextAddr (h_display, row_col) ||
        CCText (h_display, err_cstrng)
        )
        {
        fatal (errnum, func);
        }

      /* wait for string input */
      xy.x = 0;
      xy.y = 0;
      if (CReqString (h_display, 80, 0, xy, &str_len, err_cstrng))
        {
        fatal (errnum, func);
        }

      /* restore cursor position */
      if (CSetCTextAddr (h_display, save_row_col))
        {
        fatal (errnum, func);
        }
      }
    }
}


/************************************************************************
 *
 * Print title line
 *
 ************************************************************************/

void output_title(char *title, int mode)
{
  CGTEXTREPR  attribs;
  CXY   xy;
  int   errnum;
  CHORALIGN horiz;
  char  i;
  CROWCOL row_col,
          save_row_col;
  int   str_len;
  CVERTALIGN  vert;

  switch (mode)
    {
    case GMODE:
      if (in_display.Prompts && cap_display.DeviceType == CCRTDevice)
        {
        /* save current attributes; set alignment to {center,base} */
        if (CInqGTextRepr(h_display, &attribs) ||
          CSetGTextAlign (h_display, CTX_Center, CTX_Base,
          (void far *)&horiz, (void far *)&vert))
          {
          report_error("Can't print title", mode);
          }

        /* write the title */
        xy.x = 16384;
        xy.y = 32000;
        if (CGText(h_display,xy,title))
          {
          report_error("Can't print title", mode);
          }

        /* reinstate alignment */
        if (CSetGTextAlign (h_display, attribs.HorizAlign, attribs.VertAlign,
          (void far *)&horiz, (void far *)&vert))
          {
          report_error("Can't print title", mode);
          }
        }
    break;

    case CMODE:
      {
      /* save current cursor position */
      if (CInqCTextAddr (h_display, &save_row_col))
        {
        report_error("Can't print title", mode);
        }

      /* move cursor to position (2,0) */
      row_col.Row = 2;
      row_col.Col = 0;
      if (CSetCTextAddr (h_display, row_col))
        {
        report_error("Can't print title", mode);
        }

      /* write title */
      if (CSetCTextAddr(h_display, row_col) || CCText(h_display, title))
        {
        report_error("Can't print title", mode);
        }

      /* restore cursor position */
      if (CSetCTextAddr (h_display, save_row_col))
        {
        report_error("Can't print title", mode);
        }
      }
    }
}



/************************************************************************
 *
 * Fatal Error
 *
 ************************************************************************/

void  fatal (int errnum, char *func)
{
  /* close the workstation */
  CCloseWorkstation (h_display);    /* no recourse on error */

  /* now reopen and reclose it to leave it in default state */
  COpenWorkstation (&in_display, &h_display, &cap_display);
  CEnterCTextMode (h_display);    /* (make sure we exit in cursor mode) */
  CCloseWorkstation (h_display);

  remove_drivers ();      /* If they were loaded, remove them */

  /* write fatal message (using stdio) and bug out */
  printf ("Fatal error no. %d in CGI function %s.\n", (int) errnum, func);
  exit (-2);
}
