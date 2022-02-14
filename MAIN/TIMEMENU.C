/* timemenu.c */
/* enter run time parameter (time to run calib in sec. ) */

#include <cgibind.h>
#include <string.h>

#include "primtype.h"
#include "winmenu.h"
#include "plotbox.h"
#include "datafile.h"

float RunTime;

/**********************************************************************/

enum field_index {FLD_TIME = 0 };

MENU_FIELD TimeFields[] =
{
  {FLD_TIME,
   LABEL("Run Time (sec.)"), /* Field label */
   3,            /* row */
   2,            /* col */
   8,            /* length */
   FLOAT_FIELD,
   &RunTime
  }

};

WMENU TimeMenu = {"Run Time Setup Menu", TimeFields,
                  sizeof(TimeFields)/sizeof(MENU_FIELD),
                  8, 32};

int DoTimeMenu(void)
{
  RunTime = (float)DefaultHdr.Duration;
  if (PopupMenu(&TimeMenu))
    {
    DefaultHdr.Duration = (double)RunTime;
    return (TRUE);
    }
  else
    return(FALSE);
}

