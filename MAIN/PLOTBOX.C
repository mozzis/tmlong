/* -----------------------------------------------------------------------
/
/  plotbox.c
*/

#include <stdio.h>    // sprintf
#include <stdlib.h>   // itoa
#include <string.h>
#include <math.h>     // sqrt()
#include <limits.h>
#include <cgibind.h>

#include "primtype.h"
#include "constant.h"
#include "plotbox.h"
#include "symbol.h"
#include "datafile.h"
#include "device.h"   // deviceHandle()

PLOTBOX Plot;

CRECT DisplayGraphArea;  /* screen graphing window */

/* -----------------------------------------------------------------------
/
/  function:   used to find a percentage of an int value.
/  requires:   (int) full - the amount the percentage is taken from
/              (int) percentage - the percent value; usually 0 - 100
/  returns:    (int) 'percentage' % of 'full'
/  side effects:
/
/ ----------------------------------------------------------------------- */
int percent(int full, int percentage)
{
  return (int) ((long) full * (long) percentage / 100L);
}

void CalcOffsetForZ(PLOTBOX *plot, FLOAT ZValue, PLONG pXOffset,
                     PLONG pYOffset)
{
  register float temp1, temp2;
  long ltemp1, ltemp2;

   /* calculate the relative magnitude of the ZValue along the Z axis in */
   /* measurement units */
  if (plot->z.min_value == ZValue)
    ltemp1 = ltemp2 = 0L;
  else
    {
    if (plot->z.ascending)
      temp1 =  ZValue - plot->z.min_value;
    else
      temp1 = plot->z.min_value - ZValue;

    /* convert Z distance to virtual plotting units to get X */
    /* and Y offset */
  
    temp1 *= plot->z.inv_range;
    temp2 = temp1;
    temp2 *= plot->z.axis_end_offset.y;
    temp1 *= plot->z.axis_end_offset.x;
 
    ltemp1 = (long)temp1;
    ltemp2 = (long)temp2;

    if (ltemp1 < INT_MIN) ltemp1 = INT_MIN;
    if (ltemp1 > INT_MAX) ltemp1 = INT_MAX;

    if (ltemp2 < INT_MIN) ltemp2 = INT_MIN;
    if (ltemp2 > INT_MAX) ltemp2 = INT_MAX;
    }  
  *pXOffset = ltemp1;
  *pYOffset = ltemp2;
}


/* -----------------------------------------------------------------------
/
/  function:   translates the input X value into an X  coordinate
/              position on the output device, in VDC units.  The position
/              is relative to the minimum position for the axis, and
/              scaled to the length of the axis.  Note that the
/              .magnitude value used to scale the axis to a limited
/              range is also applied here to the input data (see
/              scale_axis() above).
/  requires:   (PLOTBOX *) plot - a pointer to the plotbox structure
/              (FLOAT) xvalue - the raw, unscaled input value of x-coord.
/              (int) XOffset - this curves X offset from the left side of
/                              the plotbox
/  returns:    (int) - the output X position in VDC units
/
/ ----------------------------------------------------------------------- */

int GssPosX(PLOTBOX * ppbox, float xvalue, int XOffset)
{
  float XLen;

  if (ppbox->x.ascending)
    XLen = xvalue - ppbox->x.min_value;
  else
    XLen = ppbox->x.min_value - xvalue;

  XLen *= ppbox->x.inv_range;
  XLen *= ppbox->x.axis_end_offset.x;
  XLen += ppbox->x.axis_zero.x;
  XLen += XOffset;

  return (int)XLen;
}

/* -----------------------------------------------------------------------
/
/  function:   translates the input Y value into a y coordinate
/              position on the output device, in VDC units.  The position
/              is relative to the minimum position for the axis, and
/              scaled to the length of the axis.  Note that the
/              .magnitude value used to scale the axis to a limited
/              range is also applied here to the input data (see
/              scale_axis() above).
/  requires:   (PLOTBOX *) plot - a pointer to the plotbox structure
/              (FLOAT) yvalue - the raw, unscaled input value of x-coord.
/              (int) YOffset - this curves Y offset from the bottom of
/                              the plotbox
/  returns:    (int) - the output Y position in VDC units
/
/ ----------------------------------------------------------------------- */

int GssPosY(PLOTBOX * ppbox, float yvalue, int YOffset)
{
  float YLen;

  if (ppbox->y.ascending)
    YLen = yvalue - ppbox->y.min_value;
  else
    YLen = ppbox->y.min_value - yvalue;

  YLen *= ppbox->y.inv_range;
  YLen *= ppbox->y.axis_end_offset.y;
  YLen += ppbox->y.axis_zero.y;
  YLen += YOffset;

  return (int)YLen;
}

/* -----------------------------------------------------------------------
/
/  function:   translates the input values into an x and y coordinate
/              position on the output device, in VDC units.  The position
/              is relative to the minimum position for the axis, and
/              scaled to the length of the axis.  Note that the
/              .magnitude value used to scale the axis to a limited
/              range is also applied here to the input data (see
/              scale_axis() above).
/  requires:   (PLOTBOX *) plot - a pointer to the plotbox structure
/              (float) xvalue - the raw, unscaled input value of x-coord.
/              (float) yvalue - the raw, unscaled input value of x-coord.
/              (float) zvalue - the raw, unscaled input value of x-coord.
/                               (usually the plot's line number (overlay))
/  returns:    (CXY) pos  - the output position in VDC units
/                     pos.x - coordinate x
/                     pos.y - coordinate y
/
/ ----------------------------------------------------------------------- */
CXY gss_position(PLOTBOX *plot, float xvalue, float yvalue, float zvalue)
{
  LONG XOffset = 0L, YOffset = 0L;
  CXY pos;

  CalcOffsetForZ(plot, zvalue, &XOffset, &YOffset);

  pos.x = GssPosX(plot, xvalue, (int) XOffset);
  pos.y = GssPosY(plot, yvalue, (int) YOffset);

  return pos;
}

/* -----------------------------------------------------------------------
/
/  function:   Calculates a cliprectangle for the given plotbox and Z value
/
/  requires:   (PLOTBOX *) Plot - the current plotbox structure.
/              (FLOAT) ZValue - the z coordinate (graph layer number)
/              (CRECT *) ClipRect - pointer to clip rectangle structure
/                                      to be filled
/
/  returns:    FALSE if OK
/              TRUE if Clip rectangle has no volume
/
/ ----------------------------------------------------------------------- */

BOOLEAN CalcClipRect(PLOTBOX * Plot, float ZValue, CRECT *ClipRect)
{
   LONG XOffset;
   LONG YOffset;

   ClipRect->ll = Plot->x.axis_zero;
   ClipRect->ur.x = Plot->x.axis_end_offset.x + Plot->x.axis_zero.x;
   ClipRect->ur.y = Plot->y.axis_end_offset.y + Plot->y.axis_zero.y;

   if((ZValue < Plot->z.min_value) && (ZValue < Plot->z.max_value))
      return TRUE;
   if((ZValue > Plot->z.min_value) && (ZValue > Plot->z.max_value))
      return TRUE;
   
   CalcOffsetForZ(Plot, ZValue, & XOffset, & YOffset);
      
   ClipRect->ll.x += (int)XOffset;
   ClipRect->ur.x += (int)XOffset;
   ClipRect->ll.y += (int)YOffset;
   ClipRect->ur.y += (int)YOffset;

   return FALSE;
}

/* -----------------------------------------------------------------------
/ Set up axis values based on axis->min and axis->max. End points of the
/ axis will be exactly what the user entered (or what autoscale produces).
/ ------------------------------------------------------------------------*/
void scale_axis(AXISDATA * axis)
{
   double axisRange = fabs(axis->max_value - axis->min_value);
   
   if(axisRange < 1e-12)            /* want to know if max & min equal, */
     axisRange = 1.0;               /* but if axisRange < range of float */
                                     /* disaster occurs unless '<' used */

   axis->inv_range = (float) (1.0 / axisRange);
   
   if(axis->max_value < axis->min_value)
     axis->ascending = FALSE;
   else
     axis->ascending = TRUE;
}

/*------------------------------------------------------------------------
/ return the margin size and the axis angle for the given plotbox and axis.
/ margin is in VDC for label side margin.
/-------------------------------------------------------------------------*/
void axisMarginAngle(PLOTBOX * plot, char which_one, int * margin,
                      double * angle)
{
  switch(which_one)
    {
    case 'X' :
      * angle  = 0.0;
      * margin = plot->plotarea.ll.y - plot->fullarea.ll.y; // bottom
    break;

    case 'Y' :
      if(plot->z_position != LEFTSIDE)
        {
        * angle  = PI * 0.5;
        * margin = plot->plotarea.ll.x - plot->fullarea.ll.x; // left
        }
      else
        {
        * angle  = PI * 1.5;
        * margin = plot->fullarea.ur.x - plot->plotarea.ur.x; // right
        }
    break;

    case 'Z' :
      if(plot->yz_percent == 0)
        * angle = 0.0;
      else
        * angle = atan(((double) plot->yz_percent
        * (double) (plot->fullarea.ur.y-plot->fullarea.ll.y))
        / ((double) plot->xz_percent
          * (double) (plot->fullarea.ur.x-plot->fullarea.ll.x)));

      if(plot->z_position == RIGHTSIDE)
        * margin = plot->fullarea.ur.x - plot->plotarea.ur.x; // right
      else
        {
        * margin = plot->plotarea.ll.x - plot->fullarea.ll.x; // left
        * angle = - * angle;
        }

      if(* angle < PI * 0.25)
        * margin = plot->plotarea.ll.y - plot->fullarea.ll.y; // bottom

    break;
    }
}

/*-----------------------------------------------------------------------*/
void initAxisToOriginal(AXISDATA * axis)
{
  axis->min_value = axis->original_min_value;
  axis->max_value = axis->original_max_value;
}

// values for maxMins arg of gssPlotPosition
enum { MINMINMIN, MINMINMAX, MINMAXMIN, MINMAXMAX, MAXMINMIN, MAXMINMAX,
       MAXMAXMIN, MAXMAXMAX };

// bit mask for extracting x,y, or z axis MIN or MAX from above enum
// bit = 0 iff MIN, bit = 1 iff MAX
enum { XMAXMIN = 4, YMAXMIN = 2, ZMAXMIN = 1 };

/*------------------------------------------------------------------------
/ Determine gss position with correction for min and max axis values
/ being identical. Only for x,y,z at axis end points.
/ This is a helper function for drawPlotboxOutline().
/------------------------------------------------------------------------*/
static CXY gssPlotPosition(PLOTBOX * plotbox, int maxMins)
{
  float xVal = plotbox->x.min_value;
  float yVal = plotbox->y.min_value;
  float zVal = plotbox->z.min_value;
  CXY position;

  if(maxMins & XMAXMIN)
    xVal = plotbox->x.max_value;

  if(maxMins & YMAXMIN)
    yVal = plotbox->y.max_value;

  if(maxMins & ZMAXMIN)
    zVal = plotbox->z.max_value;

  if(plotbox->z_position == NOSIDE)
    zVal = (float) 0.0;

  position = gss_position(plotbox, xVal, yVal, zVal);

  if(plotbox->x.min_value == plotbox->x.max_value)
    if(maxMins & XMAXMIN)
      position.x += plotbox->x.axis_end_offset.x;

  if(plotbox->y.min_value == plotbox->y.max_value)
    if(maxMins & YMAXMIN)
      position.y += plotbox->y.axis_end_offset.y;

   if(plotbox->z.min_value == plotbox->z.max_value)
    if(maxMins & ZMAXMIN)
      {
      position.x += plotbox->z.axis_end_offset.x;
      position.y += plotbox->z.axis_end_offset.y;
      }
   return position;
}

/*-----------------------------------------------------------------------
/ Return an array of points and a point count corresponding to the
/ plot box outline (includes the axis lines). A maximum of 7 points
/ will be provided, the first point and last point are identical.
/ REQUIRES that outline[] be big enough to hold seven points.
/ Each point returned in outline[] will be modified by offset, where
/ offset > 0 moves the point toward the center of the plotbox and
/ offset < 0 moves the point away from the center of the plotbox.
/ offset is in DEVICE COORDINATES !!!  +1 means move one device pixel
/ towards the center.
/------------------------------------------------------------------------*/
void plotboxOutline(PLOTBOX * plot, CXY outline[], short * pointCount,
                     int offset)
{
  switch(plot->z_position)
    {
    case NOSIDE :
      outline[0] = gssPlotPosition(plot, MINMAXMIN);
      outline[1] = gssPlotPosition(plot, MAXMAXMIN);
      outline[2] = gssPlotPosition(plot, MAXMINMIN);
      outline[3] = gssPlotPosition(plot, MINMINMIN);
      * pointCount = 5;
      if(offset)
        {
        outline[0] = movePointByDCOffset(outline[0], +offset, -offset);
        outline[1] = movePointByDCOffset(outline[1], -offset, -offset);
        outline[2] = movePointByDCOffset(outline[2], -offset, +offset);
        outline[3] = movePointByDCOffset(outline[3], +offset, +offset);
        }
      outline[4] = outline[0];
    break;

    case RIGHTSIDE :
      outline[0] = gssPlotPosition(plot, MINMAXMIN);
      outline[1] = gssPlotPosition(plot, MINMAXMAX);
      outline[2] = gssPlotPosition(plot, MAXMAXMAX);
      outline[3] = gssPlotPosition(plot, MAXMINMAX);
      outline[4] = gssPlotPosition(plot, MAXMINMIN);
      outline[5] = gssPlotPosition(plot, MINMINMIN);
      * pointCount = 7;
      if(offset)
        {
        outline[0] = movePointByDCOffset(outline[0], +offset, 0      );
        outline[1] = movePointByDCOffset(outline[1], 0,       -offset);
        outline[2] = movePointByDCOffset(outline[2], -offset, -offset);
        outline[3] = movePointByDCOffset(outline[3], -offset, 0      );
        outline[4] = movePointByDCOffset(outline[4], 0,       +offset);
        outline[5] = movePointByDCOffset(outline[5], +offset, +offset);
        }
      outline[6] = outline[0];
    break;

    case LEFTSIDE :
      outline[0] = gssPlotPosition(plot, MAXMAXMIN);
      outline[1] = gssPlotPosition(plot, MAXMAXMAX);
      outline[2] = gssPlotPosition(plot, MINMAXMAX);
      outline[3] = gssPlotPosition(plot, MINMINMAX);
      outline[4] = gssPlotPosition(plot, MINMINMIN);
      outline[5] = gssPlotPosition(plot, MAXMINMIN);
      * pointCount = 7;
      if(offset)
        {
        outline[0] = movePointByDCOffset(outline[0], -offset, 0      );
        outline[1] = movePointByDCOffset(outline[1], 0,       -offset);
        outline[2] = movePointByDCOffset(outline[2], +offset, -offset);
        outline[3] = movePointByDCOffset(outline[3], +offset, 0      );
        outline[4] = movePointByDCOffset(outline[4], 0,       +offset);
        outline[5] = movePointByDCOffset(outline[5], -offset, +offset);
        }
      outline[6] = outline[0];
    break;
  }
}

/*-------------------------------------------------------------------------
/ draw the outline of the plot box, including the x,y, and z axis lines.
/------------------------------------------------------------------------*/
void drawPlotboxOutline(PLOTBOX * plot)
{
  CXY drawset[7];
  SHORT pointCount;
  CCOLOR SelColor;

  plotboxOutline(plot, drawset, (void far *)& pointCount, 0);
  CSetLineColor(deviceHandle(), plot->box_color, &SelColor);
  CPolyline(deviceHandle(), pointCount, drawset);
}

/* -----------------------------------------------------------------------
/
/  function:   Set size of plotbox structure (15% of fullarea)
/  requires:   (PLOTBOX *) plot - the structure describing
/              the current plotbox.
/  returns:    (void)
/  side effects: Alters contents of plotbox variables .plotarea[].
/
/ ----------------------------------------------------------------------- */
void set_plotbox_size(PLOTBOX * plot)
{
  int fullwidth  = plot->fullarea.ur.x - plot->fullarea.ll.x;
  int fullheight = plot->fullarea.ur.y - plot->fullarea.ll.y;

  /* lower left is plot 0,0;  upper right is upper right of fullarea */
  if (plot->z_position != LEFTSIDE)
    {
    plot->plotarea.ll.x = (plot->fullarea.ll.x + percent(fullwidth, 8));
    plot->plotarea.ur.x = plot->fullarea.ur.x - percent(fullwidth, 5);
    }
  else
    {
    plot->plotarea.ll.x = plot->fullarea.ll.x + percent(fullwidth, 5);
    plot->plotarea.ur.x = plot->fullarea.ur.x - percent(fullwidth, 8);
    }

  plot->plotarea.ll.y = plot->fullarea.ll.y + percent(fullheight, 12); /* 8->10 MLM */
  plot->plotarea.ur.y = plot->fullarea.ur.y - percent(fullheight, 8);

  /* find 0,0 location for plots */
  plot->x.axis_zero.x = plot->plotarea.ll.x;
  plot->y.axis_zero.y = plot->plotarea.ll.y;

  plot->x.axis_end_offset.x = plot->plotarea.ur.x - plot->x.axis_zero.x;
  plot->y.axis_end_offset.y = plot->plotarea.ur.y - plot->y.axis_zero.y;
}

/* -----------------------------------------------------------------------
/   function:   Sets the drawing sizes for the axis lines
/   requires:
/               (PLOTBOX *) plot - the structure describing
/                                  the current plotting region, or "plotbox".
/               char which_one - which axis is being plotted, 'X', 'Y', 'Z'
/
/               (AXISDATA * *)axis - returns pointer to proper axis
/
/               (int *) axis_length - length of axis vector in VDC units
/
/   returns:      (void)
/ ----------------------------------------------------------------------- */
void SizeAxis(PLOTBOX *plot, char which_one, AXISDATA * *axis,
               int *axis_length)
{
  int xplot_size,yplot_size,zy_size,zx_size;
  int xaxis_size, yaxis_size;

  /* calculate best angle and char height based on Height space */
  /* will need to check length space later */
  switch(which_one)
    {
    case 'X':
      *axis=&(plot->x);
    break;

    case 'Y':
      *axis=&(plot->y);
    break;

    case 'Z':
      *axis=&(plot->z);
    break;
    }

  xplot_size = plot->plotarea.ur.x - plot->plotarea.ll.x;
  yplot_size = plot->plotarea.ur.y - plot->plotarea.ll.y;

  if (plot->z_position != NOSIDE)
    {
    zy_size = (int) (((LONG) yplot_size * (LONG) plot->yz_percent) /
      100L);
    zx_size = (int) (((LONG) xplot_size * (LONG) plot->xz_percent) /
      100L);

    xaxis_size = xplot_size - zx_size;
    yaxis_size = yplot_size - zy_size;
    }
  else
    {
    xaxis_size = xplot_size;
    yaxis_size = yplot_size;
    }

  switch(which_one)
    {
    case 'X':
      if (plot->z_position != LEFTSIDE)
        {     /* keep it to the left */
        (* axis)->axis_zero = plot->plotarea.ll;
        (*axis)->axis_end_offset.x = xaxis_size + plot->plotarea.ll.x;
        (*axis)->axis_end_offset.y = plot->plotarea.ll.y;
        }
      else
        {     /* keep it to the right */
        (*axis)->axis_zero.x = plot->plotarea.ur.x - xaxis_size;
        (*axis)->axis_zero.y = plot->plotarea.ll.y;
        (*axis)->axis_end_offset.x = plot->plotarea.ur.x;
        (*axis)->axis_end_offset.y = plot->plotarea.ll.y;
        }
      *axis_length = xaxis_size;
      break;

      case 'Y':
        /* check to see which side to put Y axis on */
        (*axis)->axis_zero.y = plot->plotarea.ll.y;
        (*axis)->axis_end_offset.y = yaxis_size + plot->plotarea.ll.y;
        if (plot->z_position != LEFTSIDE)
          {  /* put it on the left hand side */
          (*axis)->axis_zero.x = plot->plotarea.ll.x;
          (*axis)->axis_end_offset.x = plot->plotarea.ll.x;
          }
        else
          {  /* put it on the right hand side */
          (*axis)->axis_zero.x = plot->plotarea.ur.x;
          (*axis)->axis_end_offset.x = plot->plotarea.ur.x;
          }
        *axis_length = yaxis_size;
      break;
      case 'Z':
        (*axis)->axis_zero.y = plot->plotarea.ll.y;
        if (plot->z_position == RIGHTSIDE)
          {     /* put z axis on the right hand side */
          (*axis)->axis_zero.x = xaxis_size + plot->plotarea.ll.x;
          /* end of Z axis at plotarea.ur.x for any angle <= PI/2 */
          (*axis)->axis_end_offset.x = (*axis)->axis_zero.x + zx_size;
          (*axis)->axis_end_offset.y = (*axis)->axis_zero.y + zy_size;
          }
        else
          {        /* put it on the left hand side */
          (*axis)->axis_zero.x = plot->plotarea.ur.x - xaxis_size;
          /* end of Z axis at plotarea.ll.x for any angle > PI/2 and < PI*/
          (*axis)->axis_end_offset.x = (*axis)->axis_zero.x - zx_size;
          (*axis)->axis_end_offset.y = (*axis)->axis_zero.y + zy_size;
          }
        *axis_length = (int) sqrt(((double)zx_size * (double)zx_size) +
          ((double)zy_size * (double)zy_size));
      break;
      }
    (*axis)->axis_end_offset.x -= (*axis)->axis_zero.x;
    (*axis)->axis_end_offset.y -= (*axis)->axis_zero.y;
}

/* -----------------------------------------------------------------------
/  function:   formats a tick value to mark the ticks on the axis
/              which has no trailing zeros and, if the tick value is
/              less than one, no zero before the decimal point.
/              If the tick value is zero, the string simply is set
/              to "0" (which avoids feeding 0.0 to the code which
/              strips off the zeroes).
/  requires:   (float) value - the tick value
/              (char *) string - the string to put the clean tick value into.
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

static void format_clean_tick_value(double value, char * string)
{
  if (value == 0.0)
    strcpy(string, "0");
  else
    {
    char tick_str[20];
    int  len;

    sprintf(tick_str, "%.3f", value);
    len = strlen(tick_str);
    while (len-- > 0)    /* strip off trailing zeroes */
      {
      if (tick_str[len] == '.')
        {
        tick_str[len] = '\0';
        break;
        }
      else if (tick_str[len] == '0')
        {
        tick_str[len] = '\0';
        }
      else
        break;
      }
    if (tick_str[0] == '0')             /* remove leading zero (only if */
      strcpy(string, &tick_str[1]);                /* 0.nnn or -0.nnn) */
    else if (tick_str[0] == '-' && tick_str[1] == '0')
      {
      tick_str[1] = '-';
      strcpy(string, &tick_str[1]);
      }
    else
      strcpy(string, tick_str);

    /* limit to 6 chars */
    if (strlen(string) > 6)
      {
      char * temp = strchr((const char *)string, '.');
      if (temp)
        *temp = '\0';
      }
    }
}
/*---------------------------------------------------------------------
/ draw the axis title
/--------------------------------------------------------------------*/
static void draw_axis_legend(PLOTBOX * plot, AXISDATA * axis,
                             int legendExponent,
                             char which_one, double axis_angle,
                             int tick_line_length, int char_height,
                             int char_width, int axis_length)
{
  char text[80], expstr[20];
  int  textlen, explen, i;
  CVERTALIGN y_align;
  CXY textpt, rel_exp_pt;
  CCOLOR SelColor;
  float dTick = 0.5F / axis->inv_range;  // dTick is 1/2 of the axis range
  double cos_axis_angle = cos(axis_angle);
  double sin_axis_angle = sin(axis_angle);
  float XAxisScale;
  float YAxisScale;

  if (which_one == 'Z' && plot->z_position == NOSIDE)
    return;

  strcpy(text, axis->legend);
  textlen = strlen(text);
  if(legendExponent)
    {
    strcat(text, " x 10");
    textlen = strlen(text);
    itoa(legendExponent, expstr, 10);
    explen = strlen(expstr);
    for (i=textlen; i<(textlen + explen); i++)
      text[i] = ' ';    /* save space for exponent */

    text[i] = '\0';  /* put in terminating null */
    }

  switch (which_one)
    {
    case 'X':
      XAxisScale = (float) axis_length * axis->inv_range;
      textpt.x = axis->axis_zero.x + (int) (dTick * XAxisScale);

      textpt.y = axis->axis_zero.y - tick_line_length;
      /* include space for the tick value */
      textpt.y -= char_height;

      /* put it in the middle of the remaining space */
      textpt.y = ((textpt.y - plot->fullarea.ll.y) >> 1) +
        plot->fullarea.ll.y;

      if(legendExponent)
        {
        rel_exp_pt.x = (int)
          ((double)(textlen * char_width) * plot->xscale);
        rel_exp_pt.y = (int) ((double)char_height * plot->yscale * 0.4);
        }
      y_align = CTX_Center;  // for later text alignment purposes
    break;

    case 'Y':
      YAxisScale = (float) axis_length * axis->inv_range;
      textpt.y = axis->axis_zero.y + (int) (dTick * YAxisScale);
      if(legendExponent)
        {
        rel_exp_pt.y = (int)
          ((double)(textlen * char_height) * plot->xscale);
        rel_exp_pt.x = (int) ((double)char_height * plot->yscale * 0.3);
        }

      if (plot->z_position != LEFTSIDE)
        {
        textpt.x = axis->axis_zero.x - tick_line_length;
        /* include space for the tick value */
        textpt.x -= (tick_line_length) + char_height;

        /* put it in the middle of the remaining space */

        textpt.x = ((float)(textpt.x - plot->fullarea.ll.x) * 0.75F) +
                     plot->fullarea.ll.x;

        if(legendExponent)
          rel_exp_pt.x = -rel_exp_pt.x;
        y_align = CTX_Center;
        }
      else
        {
        textpt.x = axis->axis_zero.x + tick_line_length;
        /* include space for the tick value */
        textpt.x += (tick_line_length) + char_height;

        /* put it in the middle of the remaining space */
        textpt.x = plot->fullarea.ur.x -
          ((float)(plot->fullarea.ur.x - textpt.x) * 0.75F);

        if(legendExponent)
          rel_exp_pt.y = -rel_exp_pt.y;
        }
      y_align = CTX_Bottom;  // for later text alignment purposes
    break;
    case 'Z':
      {
      int zy_size = 0;
      int zx_size = 0;

      zy_size = (int) (((LONG)(plot->plotarea.ur.y - plot->plotarea.ll.y) *
          (LONG) plot->yz_percent) / 100L);
      zx_size = (int) (((LONG)(plot->plotarea.ur.x - plot->plotarea.ll.x) *
          (LONG) plot->xz_percent) / 100L);

      XAxisScale = (float) zx_size * axis->inv_range;
      YAxisScale = (float) zy_size * axis->inv_range;

      textpt.y = axis->axis_zero.y + (int) (dTick * YAxisScale) -
        (int) (cos_axis_angle * (double) tick_line_length);
      /* space for tick value */
      textpt.y -= (int) (cos_axis_angle *
        (double) (tick_line_length) + (1.5 * (double) char_height));

      if(legendExponent)
        {
        rel_exp_pt.y = (int) (sin_axis_angle * (double) (textlen *
          char_height) * plot->xscale);
        rel_exp_pt.x = (int) (cos_axis_angle * (double) (textlen *
          char_height) * plot->xscale);
        }

      if (plot->z_position == RIGHTSIDE)
        {
        textpt.x = axis->axis_zero.x + (int) (dTick * XAxisScale) +
          (int) (sin_axis_angle * (double) tick_line_length);
        /* space for tick value */
        textpt.x += (int) (sin_axis_angle *
          (double) (tick_line_length) + (1.4 * (double) char_height));
        if(legendExponent)
          {
          rel_exp_pt.y += (int) (cos_axis_angle * (double) char_height *
            plot->yscale * 0.4);
          rel_exp_pt.x -= (int) (sin_axis_angle * (double) char_height *
            plot->yscale * 0.4);
          }
        }
      else
        {
        textpt.x = axis->axis_zero.x - (int) (dTick * XAxisScale) -
          (int) (sin_axis_angle * (double) tick_line_length);
        /* space for tick value */
        textpt.x += ((int) (sin_axis_angle * (double) tick_line_length -
          (2.3 * (double) char_height)));
        rel_exp_pt.y += (int) (cos_axis_angle * (double) char_height *
          plot->yscale * 0.4);
        rel_exp_pt.x -= (int) (sin_axis_angle * (double) char_height *
          plot->yscale * 0.4);
        }
      y_align = CTX_Top;  // for later text alignment purposes
      }
    break;
    }

  AlignText((FLOAT)char_width, (FLOAT)char_height,
    CTX_Center, y_align, text, &textpt, (FLOAT)axis_angle);
  
  CSetLineColor(deviceHandle(), plot->text_color, &SelColor);
  
  symbol(&textpt, text, (float) axis_angle,
         (float)char_width, (float)char_height,  plot->text_color);

  /* print the exponent */
  if(legendExponent)
    {
    textpt.x += rel_exp_pt.x;
    textpt.y += rel_exp_pt.y;

    char_height = (char_height * 3) / 4;

    AlignText((FLOAT)char_width, (FLOAT)char_height,
      CTX_Left, CTX_Bottom, expstr, &textpt, (FLOAT)axis_angle);
    
    symbol(& textpt, expstr, (float) axis_angle,
      (float)char_width, (float)char_height, plot->text_color);
    }
}

// Return the value of the exponent to use for scaling. All tick
// value numeric strings should be scaled by the proper power or ten when
// drawing the axis. axisEnd1 and axisEnd2 are the two endpoints of the
// axis in any order. Returned value will be a multiple of three.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static int scientificExponent(double axisEnd1, double axisEnd2)
{
  int exponent;
  axisEnd1 = fabs(axisEnd1);
  axisEnd2 = fabs(axisEnd2);

  // make axisEnd1 the largest absolute value end point.
  if(axisEnd1 < axisEnd2) axisEnd1 = axisEnd2;

  // "cheat" by one order of magnitude so that values such as 1024 don't
  // require an exponent. Up to 9999 only requires 4 digits.
  if(axisEnd1 >= 1.0 && axisEnd1 < 10000.0) return 0;

  // Don't try to take the log of zero.
  if(axisEnd1 == 0.0) return 0;

  exponent = (int) floor(log10(axisEnd1)); // exponent of axisEnd1

  // return a multiple of three
  return exponent - (exponent % 3);
}

// find a rounded "nice" number approximately equal to x.
// ASSUME x is not zero !!!    REFERENCE : "Graphics Gems" page 657 ff.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static double nicenum(double x)
{
  int    exp  = (int) floor(log10(x)); // exponent of x
  double base = pow(10.0, (double) exp); // base value of x, power of 10
  double f    = x / base;                  // between 1 and 10

  if(f < 1.5) return        base;

  if(f < 3.0) return  2.0 * base;

  if(f < 7.0) return  5.0 * base;

  return 10.0 * base;
}

// Compute tight labels from min and max (and NTICKS). Return the tick
// values in tickVal[] and the number of ticks in tickCount. Tight labels
// have the min and max at each end with "nice" values in between.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void tightLabel(double tickVal[], double min, double max,
int * tickCount)
{
  enum { NTICK = 5 };    // desired number of tick marks
  double d;              // tick mark spacing
  double x;
  double start;
  double end;

  if(max == min)
    {
    * tickCount = 1;
    tickVal[ 0 ] = min;
    return;
    }

  d = nicenum(fabs(max - min) / (NTICK - 1));

  * tickCount = 0;

  tickVal[ (* tickCount) ++ ] = min;

  if(min > max)
    {
    start = max;
    end   = min;
    }
  else
    {
    start = min;
    end   = max;
    }

  for(x = (floor(start / d) + 1) * d; x < end; x += d)
    {
    // don't let a tick value get too close to the min or the max
    if(fabs(x - start) < d * 0.67)
      continue;
    if(fabs(x - end) < d * 0.67)
      continue;

    tickVal[ (* tickCount) ++ ] = x;
    }

  tickVal[ (* tickCount) ++ ] = max;
}

/* -----------------------------------------------------------------------
/   void draw_axis(PLOTBOX *plot, char which_one)
/
/   function:   Displays the tick marks, tick values, text label,
/               and the scale magnitude for the axis specified.
/               Does not draw the axis line itself.
/   requires:
/               (PLOTBOX *) plot - the structure describing
/                                  the current plotting region, or "plotbox".
/               char which_one - which axis is being plotted, 'X', 'Y', 'Z'
/ ----------------------------------------------------------------------- */
static void draw_axis(PLOTBOX *plot, char which_one)
{
  char        text[80];
  int         char_height, char_width, i;
  int         legendExponent;
  CHORALIGN   x_align;
  CVERTALIGN  y_align;
  double      axis_angle,sin_axis_angle,cos_axis_angle;
  double      scale_to_exponent;
  int         tick_line_length;
  int         axis_length;
  int         Margin;
  AXISDATA *  axis;
  CXY         tickline[2];
  CXY         textpt;
  CCOLOR      SelColor;
  CLINETYPE   SelLType;
  double      tickVal[ 15 ];  // where the ticks should go
  int         tickCount;      // the number of tick values in tickVal

  if (which_one == 'Z' && plot->z_position == NOSIDE)
    return;

  SizeAxis(plot, which_one, & axis, & axis_length);
  axisMarginAngle(plot, which_one, & Margin, & axis_angle);

  GetTextParams(&char_height, &char_width);

  switch (which_one)
    {
    case 'X':
    char_height = percent(Margin,20);
      y_align = CTX_Top;
    break;

    case 'Y':
      y_align = CTX_Bottom;
    break;

    case 'Z':
      y_align = CTX_Top;
    break;
    }

  legendExponent = scientificExponent(axis->min_value, axis->max_value);
  scale_to_exponent = pow(10.0, - legendExponent);

  // determine where the tick marks belong and how many there are
  tightLabel(tickVal, axis->min_value, axis->max_value, & tickCount);

  /* set up tick lines so that they reflect the size of shortest screen */
  /* axis */
  tick_line_length = Margin / 6;

  // plot tickmarks and tick values
  CSetLineType(deviceHandle(), CLN_Solid, (CLINETYPE far *)&SelLType); 
  CSetLineColor(deviceHandle(), plot->text_color, &SelColor);
   
  for(i = 0; i < tickCount; i ++)
    {
    switch (which_one)  
      {
      case 'X' :
        tickline[0] = tickline[1] = gss_position(plot, (float)tickVal[i],
                                                 plot->y.min_value,
                                                 plot->z.min_value);
        tickline[1].y -= tick_line_length;
      break;

      case 'Y':
        {
        float xIntercept    = plot->x.min_value;
        int   tickEndOffset = - tick_line_length;

        if(plot->z_position == LEFTSIDE)
          {
          xIntercept    = plot->x.max_value;
          tickEndOffset = tick_line_length;
          }

        tickline[0] = tickline[1] = gss_position(plot, xIntercept,
                                                 (float)tickVal[i],
                                                 plot->z.min_value);
        if(plot->z_position == LEFTSIDE)
          if(plot->x.max_value == plot->x.min_value)
            {
            tickline[0].x += plot->x.axis_end_offset.x;
            tickline[1].x += plot->x.axis_end_offset.x;
            }

        tickline[1].x += tickEndOffset;
        }
      break;

      case 'Z':
        {
        float xIntercept = plot->x.min_value;

        sin_axis_angle = sin(axis_angle);
        cos_axis_angle = cos(axis_angle);

        if(plot->z_position == RIGHTSIDE)
          xIntercept = plot->x.max_value;

        tickline[0] = tickline[1] = gss_position(plot, xIntercept,
                                                  plot->y.min_value,
                                                  (float) tickVal[i]);
        if(plot->z_position == RIGHTSIDE)
          if(plot->x.max_value == plot->x.min_value)
            {
            tickline[0].x += plot->x.axis_end_offset.x;
            tickline[1].x += plot->x.axis_end_offset.x;
            }

        tickline[1].x += (int) (sin_axis_angle
                                 * (double) tick_line_length);
        tickline[1].y -= (int) (cos_axis_angle
                                 * (double) tick_line_length);
        }
      break;
      } /* end of switch */

    CPolyline(deviceHandle(), 2, tickline);

    textpt = tickline[1];
    switch (which_one)
      {
      case 'X':
        textpt.y -= tick_line_length / 2;
      break;

      case 'Y':
        if (plot->z_position != LEFTSIDE)
           textpt.x -= tick_line_length/2;
        else
          textpt.x += tick_line_length/2;

      break;

      case 'Z':
        textpt.x += (int) (sin_axis_angle * 0.5
                            * (double) tick_line_length);
        textpt.y -= (int) (cos_axis_angle * 0.5
                            * (double) tick_line_length);
      break;
      }

    if (i == 0)
      {
      switch (which_one)
        {
        case 'Y':
          if (plot->z_position == LEFTSIDE)
            x_align = CTX_Right; 
        break;
        case 'X':
          x_align = CTX_Left; 
        break;
        case 'Z':
          if (plot->z_position == RIGHTSIDE)
            x_align = CTX_Left; 
          else
            x_align = CTX_Right; 
        break;
        }
      }
    else
      {
      if(i == (tickCount - 1))
        {
        if ((which_one == 'X') && (plot->z_position != LEFTSIDE))
          x_align = CTX_Right; /* note this - MLM */
        }
      else
        x_align = CTX_Center;
      }
    
    format_clean_tick_value(tickVal[ i ] * scale_to_exponent, text);
    
    AlignText((FLOAT)char_width, (FLOAT)char_height, x_align, y_align, text,
               &textpt, (FLOAT) axis_angle);

    symbol(&textpt, text, (float)axis_angle, (float)char_width,
           (float)char_height, plot->text_color);
    }

  /* plot the axis legend in the center of the axis */
  draw_axis_legend(plot, axis, legendExponent, which_one, axis_angle,
                    tick_line_length, char_height, char_width, axis_length);
}

/* ----------------------------------------------------------------------- */
void draw_plotbox(void)
{
  CCOLOR SelColor;        

  Plot.xscale = 0.7F;         
  Plot.yscale = 1.0F;

  CSetFillColor(deviceHandle(), Plot.box_color, &SelColor);
  CSetBgColor(deviceHandle(), Plot.background_color, &SelColor);
  
  draw_axis(&Plot,'X');    /* draw axis x*/
  draw_axis(&Plot,'Y');    /* draw axis y*/
  draw_axis(&Plot,'Z');    /* draw axis z*/

  if (Plot.title != NULL)
    {
    int TopMargin = Plot.fullarea.ur.y - Plot.plotarea.ur.y;
    float char_height, char_width;
    CGTEXTREPR TextInfo;
    CXY textpt;

    CInqGTextRepr(deviceHandle(), &TextInfo);
    char_height = (float)TextInfo.CharSize.x * Plot.xscale;
    char_width =  (float)TextInfo.CharSize.y * Plot.yscale;

    textpt.x = ((Plot.fullarea.ur.x - Plot.fullarea.ll.x) >> 1)
                + Plot.fullarea.ll.x;

    textpt.y = (TopMargin >> 1) + Plot.plotarea.ur.y;

    AlignText(char_width, char_height,  CTX_Center, CTX_Center,                         
               Plot.title, &textpt, 0.0F);
    
    CSetLineColor(deviceHandle(), Plot.text_color, &SelColor);
    symbol(&textpt, Plot.title, 0.0F, char_width, char_height, Plot.text_color);
    }

  drawPlotboxOutline(&Plot);
}

/* ----------------------------------------------------------------------- */
void create_plotbox(void)
{
  set_plotbox_size(&Plot);
  scale_axis(&Plot.x);
  scale_axis(&Plot.y);
  scale_axis(&Plot.z);
  draw_plotbox();
}
