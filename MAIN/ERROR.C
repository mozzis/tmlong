/* -----------------------------------------------------------------------
/
/  error.c
/
/ --------------------------------------------------------------------------
*/

#include <stdarg.h>
#include <string.h>   // strcpy(), strcat()
#include <stdlib.h>   // itoa()
#include <stdio.h>    // printf()
#include <conio.h>    // cputs()
#include <cgibind.h>
#ifdef __TURBOC__
#include <dir.h>
#endif

#include "primtype.h"
#include "error.h"
#include "cwindows.h"   // ERROR_ALLOC_MEM
#include "device.h"     // screen_handle

BOOLEAN BeepEnabled = TRUE ;

char ErrorFileName[] = "PROJECT.ERR";
char ErrorFileSpec[3+64+8+1+3];

union crit_err_map {
   struct crit_err   bits;
   unsigned int      word;
} DOS_CRITICAL_ERROR;

extern unsigned int DOS_int24_info; /* in int24.asm */
  
/* -----------------------------------------------------------------------
/
/  void init_error_file(char path)
/  establish full path+filespec of error file for use by error()
/
/-------------------------------------------------------------------------
*/
void init_error_file(char * path)
{
  char drive[3];
  char dir[64];
  char name[8];
  char exten[3];

#ifdef __TURBOC__ 
  fnsplit(path, drive, dir, name, exten);
#else
  _splitpath(path, drive, dir, name, exten);
#endif
  strcpy(ErrorFileSpec, drive);
  strcat(ErrorFileSpec, dir);
  strcat(ErrorFileSpec, ErrorFileName);
}

/* -----------------------------------------------------------------------
/
/  void error(ERROR_CATEGORY error_number, ... )
/
/  function:   Provides a common method for displaying error
/              messages in pop-up windows.  The error number
/              is used to find the associated message in the
/              error message file.  These messages may optionally
/              have embedded formatting commands starting with '%',
/              which are used in the exact same way as printf()
/              uses them.  One minor difference is that there
/              may be multi-line messages in the file, but you
/              don't have to do anything special; the parameters
/              are used as the formatting commands are found.
/              If there is no need for parameters, simply give
/              the error number.
/  requires:   (ERROR_CATEGORY) error_number - the number
/              associated with the message in the error message
/              file.
/              { ... } - (optional) parameters of any type, to
/              match the '%' formatting commands in the specific
/              message.
/  returns:    (void)
/  side effects:  consumes a fair bit of memory, so don't use
/              it to report out-of-memory errors!
/
/ ----------------------------------------------------------------------- */

ERROR_CATEGORY error(ERROR_CATEGORY error_number, ... )
{
  CRECT OldClipRect;
  CRECT ScreenArea;

  if( error_number == ERROR_ALLOC_MEM )
    {
    char no_mem_string[] = "  Out of memory. \r\n" ;
    no_mem_string[ 1 ] = '\a' ;  // make noise
    cputs( no_mem_string ) ;     // direct to screen
    }
  else
    {

    BOOLEAN message_result ;
    va_list insert_args;

    va_start(insert_args, error_number);
    ScreenArea.ll.x = 0;    
    ScreenArea.ll.y = 0;
    ScreenArea.ur.x = screen.LastVDCXY.x;
    ScreenArea.ur.y = screen.LastVDCXY.y;

    CInqClipRectangle( screen_handle, &OldClipRect ); 
    CSetClipRectangle( screen_handle, ScreenArea);

    // Check the result of va_file_message_window()
    message_result = va_file_message_window(ErrorFileSpec,
      error_number, MAX_MESSAGE_ROWS, COLORS_ERROR, insert_args ) ;

    if( message_result ) 
      {
      CSetClipRectangle( screen_handle, OldClipRect ); 
      return(error_number);
      }
    else {
      char error_string[ 50 ] ;
      char error_number_string[ 20 ] ;
      strcpy( error_string, "  Out of memory.  Error number " ) ;
      itoa( error_number, error_number_string, 10 ) ;
      strcat( error_string, error_number_string ) ;
      strcat( error_string, "\r\n" ) ;
      error_string[ 1 ] = '\a' ;             // make noise
      cputs( error_string ) ;
      }
    CSetClipRectangle( screen_handle, OldClipRect ); 
    }
  // error message has been forced to the screen, wait for a key press
  cputs( " Press a key to continue.\r\n" ) ;      
  while( ! kbhit() ) ;     // wait for a key press
  getch() ;                // read character, no echo
  return(error_number);

}

/* -----------------------------------------------------------------------
/
/  BOOLEAN test_for_DOS_critical_error(char * device_or_file)
/
/  function:   When DOS has a problem with an I/O device that
/              is difficult to handle, it calls interrupt 0x24
/              to report it (unlike a real operating system,
/              which would simply return an error to the calling
/              program).  The default int 24 service routine pops
/              up the infamous "Abort, Retry, Fail?" message, which
/              effectively allows the user to crash the program.
/              The better way to handle this interrupt is to save
/              the error information, then tell DOS to fail the
/              I/O operation.  A routine to do this is provided
/              in the module INT24.ASM.  test_for_DOS_critical_error()
/              is called after any DOS I/O function fails.
/              The critical error information is tested (if nonzero,
/              a critical error happened), and a report is made
/              indicating what caused the problem.
/              (NOTE: for this function to work, the INT24.ASM
/              module must be linked in to the program, and the
/              prepare_int24_trap() initialization function must
/              be called.)
/
/  requires:   (char *) device_or_file - a string containing the
/              device and/or file name the operation was done on.
/
/  returns:    (BOOLEAN) - TRUE if critical error detected.
/
/  side effects:  resets the global variable DOS_int24_info to zero.
/
/ ----------------------------------------------------------------------- */

BOOLEAN test_for_DOS_critical_error(char * device_or_file)
{
  BOOLEAN error_flag = FALSE;
  ERROR_CATEGORY error_number;

  if (DOS_int24_info)
    {
    switch ((int) (DOS_int24_info & 0xFF))
      {
      case DOSCRIT_WRITE_PROTECT:
        error_number = ERROR_ACCESS_DENIED;
        break;
        case DOSCRIT_DRIVE_NOT_READY:
          error_number = ERROR_DRIVE_NOT_READY;
        break;
        case DOSCRIT_SECTOR_NOT_FOUND:
          error_number = ERROR_SECTOR_NOT_FOUND;
        break;
        case DOSCRIT_DATA_CRC_ERROR:
          error_number = ERROR_BAD_SECTOR;
        break;
        case DOSCRIT_NO_PAPER:
          error_number = ERROR_NO_PAPER;
        break;
        case DOSCRIT_SEEK_ERROR:
          error_number = ERROR_SEEK;
        break;
        case DOSCRIT_UNKNOWN_UNIT:
          error_number = ERROR_BAD_DRIVE;
        break;
        case DOSCRIT_WRITE_FAULT:
          error_number = ERROR_WRITE;
        break;
        case DOSCRIT_READ_FAULT:
          error_number = ERROR_READ;
        break;
        case DOSCRIT_BAD_REQUEST:
        case DOSCRIT_UNKNOWN_MEDIA:
        case DOSCRIT_UNKNOWN_COMMAND:
        case DOSCRIT_GENERAL_FAILURE:
          error_number = ERROR_GENERAL_DISK_FAILURE;
        break;
      }
      error(error_number, device_or_file);
      DOS_int24_info = 0;
      error_flag = TRUE;
    }
  return(error_flag);
}

/* -----------------------------------------------------------------------
/
/  void ErrorBeep( void )
/
/  function:   Provides a common method for sending beeps.  Can turn off
/              by setting BeepEnabled to FALSE.
/
*/

void ErrorBeep(void)
{
  if (BeepEnabled)
    {
    printf( "\a" );
    }
}

void ErrorBeepToggle(void)
{
   BeepEnabled = !BeepEnabled;
}

