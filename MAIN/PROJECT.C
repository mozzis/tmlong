#include <stdlib.h>
#include <stdio.h>
//#include <conio.h>
#ifdef __TURBOC__   /* If TURBO C use farmalloc */
#include <alloc.h>
#else       /* otherwise assume MS C */
#include <malloc.h>
#endif
#include <cgibind.h>
#ifdef USE_D16M
#include <dos16.h>
#endif

#include <setjmp.h>
#include <float.h>
#include <signal.h>

#include "primtype.h"
#include "data.h"
#include "datafile.h"
#include "project.h"
#include "device.h"
#include "error.h"       /* init_error_file() */
#include "doplot.h"      /* Replot() */
#include "colors.h"
#include "cwindows.h"
#include "keymenu.h"
#include "winmenu.h"
#include "error.h"

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

char far   *where;            /* Where to load drivers */
CCONFIGURATION config;        /* GSS*CGI Configuration structure */
int transient;                /* Transient-drivers-loaded flag */

static char * TitleText[] = {"TM Long Calibration Software",
                             " ",
                             "     Copyright (C) 1993",
                             " ",
                             "     Maynframe Software",
                             " ",
                             "       (Press ENTER)",
                             NULL
                            };

int _stklen = 0x6000;

static jmp_buf reentry_context;

/**********************************************************************/
void control_C_trap()
{
   /* any code to reset, kill outstanding I/O, and */
   /* restabilize program will go here... */

   longjmp(reentry_context, 1);
}

/**********************************************************************/
void floating_point_error_trap()
{
   _fpreset();
   longjmp(reentry_context, 2);
}

  /**********************************************************************
  *
  * Load up the GSS*CGI controller and its drivers, then open the
  * workstations used in this program.
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

extern void prepare_int24_trap(void);  /* in int24.asm */

int main(int argc, char * argv[])
{
  CRECT GraphArea;
  ERROR_CATEGORY err;

  init_error_file(argv[0]);

  if (load_configuration() < 0 && argc)
    {
    return (-1);
    }

  prepare_int24_trap();

  signal(SIGABRT, control_C_trap );
  signal(SIGINT, control_C_trap );
  signal(SIGFPE, floating_point_error_trap );

  err = setjmp(reentry_context);

  if (err)
    {
    ReInitScreen();
    ReleaseAllWindows();
    if (err == 1)
      error(ERROR_ABORTED);
    else if (err == 2)
      error(ERROR_FLOATING_POINT);
    else
      error(ERROR_GENERIC);
    }
  else
    {
    openAndClearScreen(&GraphArea);
    InitTextCursors();
    allocDataPoints();
    InitializePlot(&GraphArea);
    ShowFKeys();

    err = load_data_file(argv[0]);
    }

  if (!err)
    InitializePlot(&GraphArea);
  
  if (!err)
    plotDataToDevice("SCREEN");
  else
    PutUpPlotBox();

  message_window(TitleText, COLORS_MESSAGE);
  
  err = setjmp(reentry_context);
  
  if (err)
    {
    ReInitScreen();
    ReleaseAllWindows();
    if (err == 1)
      error(ERROR_ABORTED);
    else if (err == 2)
      error(ERROR_FLOATING_POINT);
    else
      error(ERROR_GENERIC);
    }
  
  RunFKeyForm();
  
  save_data_file(argv[0]);
  
  closeGraphSystem();

  if (remove_configuration () < 0)
    {
    return(-4);
    }
  return(0);
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
  auto int error;
#ifdef USE_D16M
  int PrevStrategy;
#endif

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
#ifdef USE_D16M
        PrevStrategy = D16MemStrategy(MTransparent);
#endif
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
#ifdef USE_D16M
        D16MemStrategy(PrevStrategy);
#endif
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
#ifdef USE_D16M
  int PrevStrat;
#endif

  err = 0;        /* Assume no error */

#ifdef USE_D16M
    PrevStrat = D16MemStrategy(MTransparent);
#endif
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
    CLoadCgi ((UCHAR *)where, bytes_needed, &bytes_needed);
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
#ifdef USE_D16M
    D16MemStrategy(PrevStrat);
#endif
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

