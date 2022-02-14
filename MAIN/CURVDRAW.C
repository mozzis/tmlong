/* -----------------------------------------------------------------------
/
/  curvdraw.c
*/


#include <conio.h>     // kbhit()
#include <malloc.h>
#include <math.h>
#include <cgibind.h>

#include "primtype.h"
#include "colors.h"
#include "plotbox.h"
#include "data.h"
#include "curvdraw.h"
#include "device.h"    // screen_handle
#include "keys.h"      // ESCAPE
#include "error.h"

CXY far *plotlines = 0;


int FactorPoint(float Value, float Factor, float BasePoint,
                long OffsetValue, int ascends)
{
  float temp;

  if (ascends)
    temp = Value - BasePoint;
  else
    temp = BasePoint - Value;

  temp *= Factor;
  temp += (float)OffsetValue;

  return ((int)temp);

}

static BOOLEAN startLinePlot(USHORT numPoints)
{
  static unsigned int plotSize = 0;
  long bufsize;

  if (plotlines && plotSize >= numPoints)
    return (TRUE);

  if (plotlines  && plotSize < numPoints)
    {
    free(plotlines);
    plotlines = NULL;
    }

  plotSize = numPoints;

  bufsize = plotSize * sizeof(CXY);

  plotlines = (CXY far *) malloc((size_t)bufsize);
  
  if (!plotlines || !plotSize)
    return(FALSE);
  else
    return(TRUE);

}

/* ----------------------------------------------------------------------- */
ERROR_CATEGORY array_plot(int CurveIndex, int StartPoint, int Count)
{
  CRECT          ClipRect, OldClipRect;
  USHORT         i, j;
  FLOAT          x, y, ZValue = 0.0F;
  FLOAT XBasePt, XFactor;
  FLOAT YBasePt, YFactor;
  LONG XOffset,  YOffset;
  CLINETYPE      SelType;
  CCOLOR         SelColor;
  int            xVal, yVal;

  /* set clip rect */
  /* if CalcClip returns TRUE, Zvalue won't fit in rectangle */
  if (CalcClipRect(&Plot, ZValue, &ClipRect))
    return ERROR_NONE;

  if (!startLinePlot(RunData.Count))
    {
    return(error(ERROR_ALLOC_MEM));
    }

  CInqClipRectangle(deviceHandle(), &OldClipRect);
  CSetClipRectangle(deviceHandle(), ClipRect);

  XBasePt = Plot.x.min_value;

  XFactor = Plot.x.inv_range * (float) Plot.x.axis_end_offset.x;

  YBasePt = Plot.y.min_value;

  YFactor = Plot.y.inv_range * (float) Plot.y.axis_end_offset.y;

  CalcOffsetForZ(&Plot, ZValue, &XOffset, &YOffset);

  XOffset += Plot.x.axis_zero.x;
  YOffset += Plot.y.axis_zero.y;

  CSetLineType(deviceHandle(), Plot.plot_line_type, (void far *)&SelType);
  CSetLineColor(deviceHandle(), Plot.plot_color, (void far *)&SelColor);

  for (i = 0; i < Count; i++)
    {
    j = StartPoint + i;
    x = (FLOAT) j;
    if (j < RunData.Count)
      y = RunData.Points[CurveIndex * Count + j];
    else
      y = 0.0F;

    xVal = FactorPoint(x, XFactor, XBasePt, XOffset, Plot.x.ascending);
    yVal = FactorPoint(y, YFactor, YBasePt, YOffset, Plot.y.ascending);

    plotlines[i].x = xVal;
    plotlines[i].y = yVal;
    }

  if(i == 1)     // be sure to draw a dot if only one point
    {
    plotlines[1] = plotlines[0];
    i = 2;
    }

  CPolyline(deviceHandle(), Count, plotlines);

  CSetClipRectangle(deviceHandle(), OldClipRect);

  return ERROR_NONE;
}

/* -----------------------------------------------------------------------*/
//
//  BOOLEAN plot_curves(void)
//
//  returns:    TRUE if terminated before completion
//
/* ----------------------------------------------------------------------- */
BOOLEAN plot_curves(void)
{
  ERROR_CATEGORY err;
  int Key, CurveNumber;
  BOOLEAN Abort = FALSE;

  for (CurveNumber = 0; CurveNumber < RunData.CurveCount; CurveNumber++)
    {
    /* assume x axis in counts for now */
    err = array_plot(CurveNumber, (int)Plot.x.min_value,
                                  (int)(Plot.x.max_value - 
                                        Plot.x.min_value) + 1);

    // now check for error.   
    if(err)
      return TRUE;

    /* check for escape from curve draw */
    if(kbhit())
      {
      Key = getch();
      if(Key == ESCAPE)
        {
        Abort = TRUE ;
        break ;
        }
      }
    }
  return(Abort);
}
