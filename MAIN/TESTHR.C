#include <stdio.h>
#include <stdlib.h>          /* EXIT_SUCCESS, EXIT_FAILURE          */
#include <ctype.h>           /* toupper()
#include <stdio.h>           /* puts(), printf()                    */
#include <conio.h>           /* getch()                             */

#include "hrclock.h"         /* HRCLOCK specific stuff              */

int _stklen = 0x2000;

int main(int argc, char * argv[])
{
  USHORT InitCounter0Mode;        /* Initial PIT counter 0 mode     */
  ULONG  doits = 1000;
  int which = 0, verbose = 0, mode = 2;
  char * dummy;

  while (--argc)
    {
    if (argv[argc][0] == '-' || argv[argc][0] == '/')
      {
      switch (toupper((argv[argc][1])))
        {
        case 'D':
          doits = strtoul(&argv[argc][2], dummy, 10);
        break;
        case 'U':
          which = 1;
        break;
        case 'M':
          mode = atoi(&argv[argc][2]);
        break;
        case 'V':
          verbose = 1;
        break;
        default:
          printf("\aIncorrect option: %s\n", argv[argc]);
        case '?':
        case 'H':
          printf("Options:   -D<num> - specify delay loop count\n"
                 "           -U      - use uDelay instead of mDelay\n"
                 "           -M2     - use PIT Mode 2\n"
                 "           -M3     - use PIT Mode 3\n"
                 "           -V      - Verbose mode\n");
          return(1);
        }
      }
    }
  
  if (mode != 2 && mode != 3)
    {
    printf("\aMode must be 2 or 3!\n");
    return(2);
    }

  if (verbose)
    printf("Using loop counter: %ld\n", doits);

  InitCounter0Mode = GetPITCounterMode(0);
  if (verbose)
    printf("\nPIT Counter 0 default mode: %d\n", InitCounter0Mode);

  if (verbose)
    printf("Using PIT mode %d\n", mode);

  if (mode == 2)
    HRMode2();
  else
    HRMode3();

  if (!HRInit())
    return(EXIT_FAILURE);

  if (which)
    {
    printf("Starting uDelay\n");
    uDelay(doits);
    }
  else
    {
    printf("Starting mDelay\n");
    mDelay(doits);
    }

  printf("\nDone\a\n");

  (InitCounter0Mode == 2) ? HRMode2() : HRMode3();

  return(0);

}
