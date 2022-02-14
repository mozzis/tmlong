/* -----------------------------------------------------------------------
/
/  device.c
* 
*/

#include <string.h>
#include <stddef.h>     // NULL
#include <stdlib.h>     // NULL
#include <stdio.h>      // printf
#include <malloc.h>
#include <cgibind.h>

#include "primtype.h"
#include "device.h"
#include "doplot.h"
#include "error.h"
#include "colors.h"
#include "cwindows.h"
#include "symbol.h"

#define  FOREGROUND(x) ((int) (x & 0xF))
#define  BACKGROUND(x) ((int) ((x >> 4) & 0xF))

int screen_rows = 25;
int screen_cols = 80;
int text_cell_height;
int text_cell_width;

static SaveAreaInfo * ActiveAreas[6];
static int AreaCount;

CRECT DisplayArea;
CDVHANDLE screen_handle;

CDVCAPABILITY screen;

static CDVHANDLE printer_handle;           

static CDVCAPABILITY printer;              

static CDVHANDLE plotter_handle;  

static CDVCAPABILITY plotter;         

CDVOPEN screen_setup =
       {         
          CPreserveAspect,              // coordinate transform mode 1
          CLN_Solid,                    // initial line type
          CBRT_WHITE,                   // initial line color
          CMK_Plus,                     // marker type = Star
          CBLACK,                       // marker color, for graph cursor
          1,                            // initial graphics text font
          1,                            // initial graphics text color
          1,                            // initial fill interior style
          1,                            // initial fill style index
          1,                            // initial fill color index
          1,                            // prompting flag
          "DISPLAY"                     // driver link name
       };

static CDVOPEN printer_setup = {           
   CPreserveAspect, CLN_Solid, 1,
   CMK_Dot,1,1,1,1,1,1,0, "PRINTER"
};

static CDVOPEN plotter_setup = {
   CPreserveAspect, CLN_Solid, 1,
   CMK_Dot,1,1,1,1,1,1,0, "PLOTTER"
};

static CDVCAPABILITY const * plotDevParams = & screen;

static CDVHANDLE activeDeviceHandle;

CRGB ColorTable[16] = {                
   { 0,    0,    0 },     /* Black */
   { 1000, 1000, 1000 },  /* Bright White */             
   { 0,    500,  0 },     /* Green */
   { 600,  0,    0 },     /* Red */
   { 0,    500,  500 },   /* Cyan */
   { 500,  0,    500 },   /* Purple */
   { 500,  400,  0 },     /* Yellow (brown, really) */
   { 500,  500,  500 },   /* White (grey?) */          
   { 0,    0,    1000 },  /* Bright Blue */
   { 0,    1000, 0 },     /* Bright Green */
   { 1000, 0,    0 },     /* Bright Red */
   { 0,    1000, 1000 },  /* Bright Cyan */
   { 1000, 0,    1000 },  /* Bright Purple */
   { 1000, 1000, 0 },     /* Bright Yellow */
   { 0,    0,    600 },   /* Blue */                     
   { 1000, 500,  0 }      /* Bright Orange */
};

COLOR_SET ColorSets[MAX_COLOR] =
{
    /* foreground, backgrnd */
  {                               /* DEFAULT COLORS */
    { CBRT_WHITE, CBLUE  },         // regular;
    { CBRT_WHITE, CCYAN  },         // reverse;
    { CBRT_YELLOW, CCYAN },         // highlight;
    { CWHITE, CBLUE      },         // shaded;
  },
  {                               /* MESSAGE COLORS */
    { CBLUE, CBRT_WHITE  },         // regular;
    { CBRT_WHITE, CBLUE  },         // reverse;
    { CBRT_YELLOW, CWHITE   },      // highlight;
    { CBLACK, CWHITE     },         // shaded;
  },
  {                               /* ERROR COLORS   */
    { CBRT_WHITE, CRED   },         // regular;
    { CBLACK, CRED       },         // reverse;
    { CBLACK, CRED       },         // highlight;
    { CBLACK, CRED       }          // shaded;
  },
  {                               /* MENU COLORs    */
    { CBRT_WHITE, CBLUE  },         // regular;
    { CBRT_WHITE, CCYAN  },         // reverse;
    { CBRT_YELLOW, CBLUE },         // highlight;
    { CWHITE, CBLUE      }          // shaded;
  }
};

/***************************************************************************/
/*                                                                         */
/* return the handle of the currently active device                        */
/*                                                                         */
/***************************************************************************/
CDVHANDLE deviceHandle(void)
{
   return activeDeviceHandle;
}

/***************************************************************************/
void box(CRECT *boxrect)
{
  CXY ptsin[5]; /* Five points to make up the box */

  /* use polyline to output the box */
  ptsin[0].x = ptsin[3].x = ptsin[4].x = boxrect->ll.x;
  ptsin[0].y = ptsin[1].y = ptsin[4].y = boxrect->ll.y;
  ptsin[1].x = ptsin[2].x = boxrect->ur.x;
  ptsin[2].y = ptsin[3].y = boxrect->ur.y;
  CPolyline(activeDeviceHandle, 5, ptsin);
}

/***************************************************************************/
void erase_screen_area(UCHAR row, UCHAR column, UCHAR n_rows, UCHAR n_columns,
                       UCHAR attrib, BOOLEAN border)
{
  CRECT diagonal;
  CINTERIORFILL SelStyle;
  CCOLOR SelColor;

  diagonal.ur.x = column_to_x(column + n_columns);
  diagonal.ur.y = row_to_y(row);
  diagonal.ll.x = column_to_x(column);
  diagonal.ll.y = row_to_y(row + n_rows);

  CSetFillInterior (screen_handle, CSolidFill, (void far *)&SelStyle);
  CSetFillColor(screen_handle, BACKGROUND(attrib), &SelColor);
  CBar(screen_handle, diagonal);

  if (border)
    {
    int half_width = (text_cell_width / 6);
    int half_height = (text_cell_height / 6);

    diagonal.ur.x -= half_width;
    diagonal.ur.y -= half_height;
    diagonal.ll.x += half_width;
    diagonal.ll.y += half_height;

    CSetFillInterior (screen_handle, CHollowFill, (void far *)&SelStyle);
    CSetFillColor(screen_handle, FOREGROUND(attrib), &SelColor);
    CBar(screen_handle, diagonal);
    CSetFillInterior (screen_handle, CSolidFill, (void far *)&SelStyle);
    CSetFillColor(screen_handle, BACKGROUND(attrib), &SelColor);
    }
}

/***************************************************************/
  
SaveAreaInfo * save_screen_area(UCHAR row, UCHAR column, UCHAR n_rows,
                                  UCHAR n_columns)
{
  CBMHANDLE      screen_bitmap;
  CRECT          temp_xy;
  SaveAreaInfo far * Area;

  Area = (SaveAreaInfo far *) malloc(sizeof(SaveAreaInfo));

  if (Area)
    {
    ActiveAreas[AreaCount++] = Area;

    Area->diagonal.ur.x = column_to_x(column + n_columns);
    Area->diagonal.ur.y = row_to_y(row);
    Area->diagonal.ll.x = column_to_x(column);
    Area->diagonal.ll.y = row_to_y(row + n_rows);

    /* discover the bitmap index for the screen */
      
    CInqDrawingBitmap(screen_handle, &screen_bitmap, &temp_xy);
    /* create a bitmap to save the area that will be covered by the menu */
    if (CCreateBitmap(screen_handle, Area->diagonal, CFullDepth,
      &Area->bitmap_index) == -1)
      {
      free(Area);
      return(NULL);
      }
    else
      {
      /* select the newly created bitmap as the drawing bitmap */
      CSelectDrawingBitmap(screen_handle, Area->bitmap_index);
      /* copy the specified area from the screen into the new bitmap */
      CCopyBitmap(screen_handle, screen_bitmap, Area->diagonal,
        Area->diagonal.ll);
      /* reset so that screen bitmap is where new drawing will go */
      CSelectDrawingBitmap(screen_handle, screen_bitmap);
      }
    }
  return(Area);
}
  
/***************************************************************/
SaveAreaInfo * restore_screen_area(SaveAreaInfo * save_buffer)
{
  if (save_buffer)
    {
    /* copy from saved area bitmap onto the screen */
    CCopyBitmap(screen_handle, save_buffer->bitmap_index,
                save_buffer->diagonal, save_buffer->diagonal.ll);

    /* delete the saved area bitmap */
    CDeleteBitmap(screen_handle, save_buffer->bitmap_index);

    free(save_buffer);

    if (AreaCount > 0)
      ActiveAreas[AreaCount--] = NULL;
    save_buffer = NULL;
    }
  return(save_buffer);
}

void ReleaseAllAreas(void)
{
  int i;

  for (i = AreaCount; i > 0; i++)
    restore_screen_area(ActiveAreas[i--]);
}

/*************************************************************************/
/*                                                                         */
/* set the clipping rectangle for the screen to full screen
/*                                                                         */
/*************************************************************************/
void setClipRectToFullScreen(void)
{
  CRECT screenArea = { { 0, 0 }, { 0, 0 } };

  screenArea.ur = screen.LastVDCXY;

  CSetClipRectangle(screen_handle, screenArea);
}

// Return the scale factor for converting virtual to physical x coordinates
// for the current device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float XVirToPhysScaleFactor(void)
{
  return(float)((double)plotDevParams->LastVDCXY.x / plotDevParams->LastXY.x);
}

// Return the scale factor for converting virtual to physical y coordinates
// for the current device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
float YVirToPhysScaleFactor(void)
{
  return(float)((double)plotDevParams->LastVDCXY.y / plotDevParams->LastXY.y);
}

// given a y value in VDC space and a dcOffset in DC space, return the VDC
// value of y moved by dcOffset pixels on the device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CY adjustYbyDCOffset(CY yVal, CDCY dcOffset)
{
  double sizeDevY = (double) plotDevParams->LastXY.y;
  double sizeVDCY = (double) plotDevParams->LastVDCXY.y;

  return yVal + (CY) (dcOffset * sizeVDCY / sizeDevY + 0.5);
}

// given an x value in VDC space and a dcOffset in DC space, return the VDC
// value of x moved by dcOffset pixels on the device.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CX adjustXbyDCOffset(CX xVal, CDCX dcOffset)
{
  double sizeDevX = (double) plotDevParams->LastXY.x;
  double sizeVDCX = (double) plotDevParams->LastVDCXY.x;

  return xVal + (CX) (dcOffset * sizeVDCX / sizeDevX + 0.5);
}

// move a point in VDC space by x and y offsets in DC space.  Return the
// point in VDC space.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CXY movePointByDCOffset(CXY point, CDCX xDCOffset, CDCY yDCOffset)
{
  if(xDCOffset) point.x = adjustXbyDCOffset(point.x, xDCOffset);

  if(yDCOffset) point.y = adjustYbyDCOffset(point.y, yDCOffset);

  return point;
}

void InitTextParams(void)
{
CATEXTCAP ScreenFont;

CInqATextCap(activeDeviceHandle, &ScreenFont);

screen_rows = ScreenFont.Cells.Row;
screen_cols = ScreenFont.Cells.Col;

text_cell_height = ScreenFont.CellWidth.y / screen_rows;
text_cell_width  = ScreenFont.CellWidth.x / screen_cols;
}

//void InitTextParams(void)
//{
//  CFONTMETRICS GFntInfo;
//  float temp_height, temp_width;
//
//  CInqFontMetrics(activeDeviceHandle, &GFntInfo);
//
//  text_cell_height = GFntInfo.NominalCellSize.x;
//  text_cell_width  = GFntInfo.NominalCellSize.y;
//
//  temp_height = (float)plotDevParams->LastVDCXY.y;
//  temp_width =  (float)plotDevParams->LastVDCXY.x;
//  
//  screen_rows = (int) ((temp_height / (float)text_cell_height) + 0.5F);
//  screen_cols = (int) ((temp_width  / (float)text_cell_width) + 0.5F);
//}

void GetTextParams(int * height, int * width)
{
  *height = text_cell_height;
  *width  = text_cell_width;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ERROR_CATEGORY plotDataToDevice(char * deviceName)
{
  CRECT GraphArea;

  if(!stricmp(deviceName, "PRINTER"))
    {
    if(COpenWorkstation(&printer_setup, &printer_handle, &printer))
      {
      return error(ERROR_DEV_OPEN, deviceName);
      }
//    CClearWorkstation(printer_handle);

    activeDeviceHandle = printer_handle;
    plotDevParams  = &printer;
    GraphArea.ll.x = 0;
    GraphArea.ll.y = 0;
    GraphArea.ur   = printer.LastVDCXY;
    }
  else if(!stricmp(deviceName, "PLOTTER"))
    {
    if(COpenWorkstation(&plotter_setup, &plotter_handle, &plotter))
      {
      return error(ERROR_DEV_OPEN, deviceName);
      }

    activeDeviceHandle = plotter_handle;
    plotDevParams = & plotter;
    GraphArea.ll.x = 0;
    GraphArea.ll.y = 0;
    GraphArea.ur   = plotter.LastVDCXY;
    }
  else if(!stricmp(deviceName, "SCREEN"))
    {
    activeDeviceHandle = screen_handle;
    plotDevParams = &screen;
    GraphArea = DisplayArea;

    erase_screen_area(0, 0, screen_rows - 2, screen_cols+1,
      (UCHAR) ((ColorSets[COLORS_DEFAULT].regular.background << 4) |
      ColorSets[COLORS_DEFAULT].regular.foreground),
      FALSE);

    box(&GraphArea);
    }

  SetPlotForDevice(&GraphArea);
  InitTextParams();
  Replot();

  if(activeDeviceHandle != screen_handle)
    CCloseWorkstation(activeDeviceHandle);

  activeDeviceHandle = screen_handle;
  plotDevParams = & screen;
  SetPlotForDevice(&DisplayArea);
  InitTextParams();

  return ERROR_NONE;
}

ERROR_CATEGORY PlotScreen(void)
{
  return(plotDataToDevice("SCREEN"));
}

void ReInitScreen(void)
{
  activeDeviceHandle = screen_handle;
  plotDevParams = & screen;
  InitTextParams();
  ReleaseAllAreas();
}


/*******************************************************/
/*                                                     */
/* Initialize the display device                       */
/* The return value will always be zero for now; if an */
/* error occurs, the routine exits to DOS.             */
/* The GraphArea parameter is set to the coordinates   */
/* of the area which may be used for graphics.  The    */
/* remainder of the screen is allocated for text.      */
/* The GraphArea is calculated based on the size of the*/
/* the screen font which will be used.                 */
/*                                                     */
/*******************************************************/
int openAndClearScreen(CRECT *GraphArea)
{
  int returnVal;

  screen_setup.MarkerColor = CBRT_YELLOW;

  returnVal = COpenWorkstation(&screen_setup, &screen_handle, &screen);

  if (returnVal)
   {
   printf ("Error %d opening display device\n", CInqCGIError ());
   exit (-1);
   }

  CClearWorkstation(screen_handle);

  activeDeviceHandle = screen_handle;
  plotDevParams = & screen;

  InitTextParams();

  GraphArea->ll.x = 0;        /* position lower left of plot box */

  /* Arrange for FKey Menu on bottom row of screen */

  GraphArea->ll.y =  row_to_y((screen_rows - 2)) + (text_cell_height / 8);

  /* set upper right of plot box */
  /* (this sizes the plot area) */

  GraphArea->ur.x = screen.LastVDCXY.x;
  GraphArea->ur.y = adjustYbyDCOffset(row_to_y(1), -1);

  DisplayArea = *GraphArea;

  CSetColorTable(screen_handle, 0, 16, ColorTable);
 
  erase_screen_area(0, 0, screen_rows, screen_cols+1,
    (UCHAR) ((ColorSets[COLORS_DEFAULT].regular.background << 4) |
    ColorSets[COLORS_DEFAULT].regular.foreground),
    FALSE);

  box(GraphArea);

  return returnVal;
}


/*********************************************/
/*                                           */
/*  Character oriented functions for device  */
/*                                           */
/*********************************************/
UCHAR set_attributes(UCHAR fore, UCHAR back)
{
  return((UCHAR)((fore & 0xF) | ((back & 0xF) << 4)));
}


void display_string(char * string, int len, int row, int column, int attrib)
{
  CXY ReqXY, SelXY;                   
  CCOLOR SelColor;

  ReqXY.x = column_to_x(column);      
  ReqXY.y = row_to_y(row + 1);
  string[len] = 0;

  CSetATextPosition(screen_handle, ReqXY, &SelXY);  
  CSetATextColor(screen_handle, FOREGROUND(attrib), &SelColor);
  CSetBgColor(screen_handle, BACKGROUND(attrib), &SelColor);

  CAText(screen_handle, (UCHAR *)string, &SelXY);
}

void emit(char character, int row, int column, int attrib)
{
  char temp_string[2];

  temp_string[0] = character;

  display_string(temp_string, 1, row, column, attrib);
}

/********************************************************/
/*                                                      */
/* Convert between device (GSS) and text row/col coords */
/*                                                      */
/********************************************************/

int row_to_y(int row)
{
   return((screen_rows - row) * text_cell_height);
}

int column_to_x(int column)
{
   return(column * text_cell_width);
}

int y_to_row(int coord)
{
   return((screen_rows - 1) - (coord / text_cell_height));
}

int x_to_column(int coord)
{
   return(coord / text_cell_width);
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int closeGraphSystem(void)
{
  /* restore display to normal */
  if (CEnterCTextMode (screen_handle))
    {
    printf ("Error %d entering text mode\n", CInqCGIError ());
    exit (-1);
    }

  /* close the workstation */
  if (CCloseWorkstation (screen_handle))
    {
    printf ("Error %d closing GSS\n", CInqCGIError ());
    exit (-1);
    }
  return(0);
}
