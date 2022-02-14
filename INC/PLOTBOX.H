/* -----------------------------------------------------------------------
/
/  plotbox.h
/
*/

#ifndef PLOTBOX_INCLUDED
#define PLOTBOX_INCLUDED

#ifndef COpenWorkstation
#include <cgibind.h>
#endif

#ifndef PRIMTYPE_INCLUDED
#include "primtype.h"
#endif

enum { LEGEND_SIZE = 30 } ;

struct axis_data
{
   char    legend[ LEGEND_SIZE ] ;  /* preallocated */
   CXY     axis_end_offset;
   CXY     axis_zero;               /* XY coord of zero in VDC? */
   float   max_value;
   float   min_value;
   float   original_max_value;      /* for restoring from zooms */
   float   original_min_value;
   float   inv_range;               /* total length in VDC's */
   BOOLEAN ascending;
   USHORT  units;                   /* same as in datahdr's */
} ;
  
typedef struct axis_data AXISDATA;

// Add data structure for automatic peak labels.

struct curve_peak_label
{
   float curve_number ;
   float threshold_value ;
   BOOLEAN  enabled ;
} ;

// Store peak label info for at most 10 curves.
#define MAX_LABELLED_CURVES 10

// values for PLOTBOX.z_position
typedef enum { NOSIDE, RIGHTSIDE, LEFTSIDE } ZPosition ;

#define TITLE_SIZE 30

struct plot_box
{
   char              title[ TITLE_SIZE ] ;
   float             xscale;
   float             yscale;
   CRECT             fullarea;
   CRECT             plotarea;
   AXISDATA          x;
   AXISDATA          y;
   AXISDATA          z;
   int               xz_percent;
   int               yz_percent;
   ZPosition         z_position ;
   int               background_color;
   CCOLOR            box_color;
   CCOLOR            text_color;
   CCOLOR            grid_color;
   CLINETYPE         grid_line_type;
   CCOLOR            plot_color;
   CLINETYPE         plot_line_type;
   int               style;
} ;

typedef struct plot_box PLOTBOX ;

// data units for X and Y axes of plot
enum { COUNTS =  0, ANGSTROM     =  1, NM     =  2, // nanometers
       UM     =  3,                                 // micrometers
       MM     =  4,                                 // millimeters
       CM     =  5,                                 // centimeters
       METER  =  6, WAVENUMBER   =  7,
       RSHIFT =  8,                                 // raman shift
       EV     =  9,                                 // electron volts
       JOULE  = 10, ERG          = 11, HZ     = 12,
       ADJ_NM = 13, WLEN_UNITNUM = 14, SECOND = 20,
       MSEC   = 21,                                 // millisecond
       USEC   = 22,                                 // microsecond
       NSEC   = 23,                                 // nanoseconds
       PSEC   = 24,                                 // picoseconds
       FSEC   = 25,                                 // femtoseconds
       TIME_DIS_UNITNUM = 26,
       AMP  = 30, VOLT = 31, OHM = 32, COULOMB = 33, MHO = 34,
       WATT = 35
     } ; 

#define OVERLAPPED_CURVES 0
#define HIDDENLINE_CURVES 1

extern PLOTBOX Plot;          /* plot area */

extern CRECT DisplayGraphArea;  /* screen graphing window */

void CalcOffsetForZ(PLOTBOX *plot, FLOAT ZValue, PLONG pXOffset,
                    PLONG pYOffset);

void CopyPlotboxToHeader(void);

BOOLEAN CalcClipRect( PLOTBOX *, FLOAT, CRECT *);
  
void initAxisToOriginal( AXISDATA * axis ) ;

void plotboxOutline(PLOTBOX * plot, CXY outline[], short * pointCount,
                     int offset);

void drawPlotboxOutline( PLOTBOX * plotbox ) ;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ResizePlotboxForDevice(SHORT theWindow);

void draw_plotbox(void);

void create_plotbox(void);

#endif  //  PLOTBOX_INCLUDED


