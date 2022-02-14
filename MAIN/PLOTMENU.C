/* plotmenu.c */
/* enter axis parameters and title for plot */

#include <cgibind.h>
#include <string.h>

#include "primtype.h"
#include "winmenu.h"
#include "plotbox.h"

char  PlotTitle[20];
int   PlotXMin;
int   PlotXMax;
float PlotYMin;
float PlotYMax;

/**********************************************************************/

enum field_index {FLD_TITLE = 0, FLD_XMIN, FLD_XMAX, FLD_YMIN, FLD_YMAX };

MENU_FIELD AxesFields[] =
{
  {FLD_TITLE,
   LABEL("Plot Title"), /* Field label */
   3,            /* row */
   2,            /* col */
   20,           /* length */
   STRING_FIELD,
   PlotTitle
  },

  {FLD_XMIN,
   LABEL("XMin"),/* Field label */
   5,            /* row */
   2,            /* col */
   8,            /* length */
   INT_FIELD,
   &PlotXMin,
  },

  {FLD_XMAX,
   LABEL("XMax"),/* Field label */
   5,            /* row */
   18,           /* col */
   8,            /* length */
   INT_FIELD,
   &PlotXMax,
  },

  {FLD_YMIN,
   LABEL("YMin"),/* Field label */
   7,            /* row */
   2,            /* col */
   8,            /* length */
   FLOAT_FIELD,
   &PlotYMin,
  },

  {FLD_YMAX,
   LABEL("YMax"),/* Field label */
   7,            /* row */
   18,           /* col */
   8,            /* length */
   FLOAT_FIELD,
   &PlotYMax,
  },

};

WMENU AxesMenu = {"Plot Setup Menu", AxesFields,
                  sizeof(AxesFields)/sizeof(MENU_FIELD),
                  10, 38};

int DoPlotSetupMenu(void)
{
  strcpy(PlotTitle, Plot.title);
  PlotXMin = (int)Plot.x.min_value;
  PlotXMax = (int)Plot.x.max_value;
  PlotYMin =      Plot.y.min_value;
  PlotYMax =      Plot.y.max_value;
  if (PopupMenu(&AxesMenu))
    {
    strcpy(Plot.title, PlotTitle);
    Plot.x.min_value = (float)PlotXMin;
    Plot.x.max_value = (float)PlotXMax;
    Plot.y.min_value = PlotYMin;
    Plot.y.max_value = PlotYMax;
    return (TRUE);
    }
  else
    return(FALSE);
}

