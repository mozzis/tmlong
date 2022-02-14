/* -----------------------------------------------------------------------
/
/  device.h
/
*/

#ifndef DEVICE_INCLUDED
#define DEVICE_INCLUDED

#ifndef PRIMTYPE_INCLUDED
#include "primtype.h"
#endif

#ifndef COLORS_INCLUDED
#include "colors.h"
#endif

extern CDVHANDLE screen_handle ;

extern CDVCAPABILITY screen ;

extern int screen_rows;
extern int screen_cols;
extern COLOR_SET ColorSets[];

typedef struct save_area_info {
   CBMHANDLE   bitmap_index;        
   CRECT       diagonal;
};

typedef struct save_area_info SaveAreaInfo;

/*********************************************************************/
/* return the handle of the currently active device                  */
/*********************************************************************/
CDVHANDLE deviceHandle( void ) ;

/*********************************************************************/
/* return the dimensions of the current font                         */
/*********************************************************************/
void GetTextParams(int * height, int * width);

/*********************************************************************/
void erase_screen_area(UCHAR row, UCHAR column, UCHAR rows, UCHAR columns,
                       UCHAR attrib, BOOLEAN border);

/*********************************************************************/
SaveAreaInfo * save_screen_area(UCHAR row, UCHAR col, UCHAR rows, UCHAR cols);
/*********************************************************************/
SaveAreaInfo * restore_screen_area(SaveAreaInfo * save_buffer );

// set the clipping rectangle for the screen to the whole screen
/*********************************************************************/
void setClipRectToFullScreen( void ) ;

ERROR_CATEGORY PlotScreen(void);

// Return the scale factor for converting virtual to physical x coordinates
// for the current device.
/*********************************************************************/
float XVirToPhysScaleFactor( void ) ;

// Return the scale factor for converting virtual to physical y coordinates
// for the current device.
/*********************************************************************/
float YVirToPhysScaleFactor( void ) ;

// given a y value in VDC space and a dcOffset in DC space, return the VDC
// value of y moved by dcOffset pixels on the device.
/*********************************************************************/
CY adjustYbyDCOffset( CY yVal, CDCY dcOffset ) ;

// given an x value in VDC space and a dcOffset in DC space, return the VDC
// value of x moved by dcOffset pixels on the device.
/*********************************************************************/
CX adjustXbyDCOffset( CX xVal, CDCX dcOffset ) ;

// move a point in VDC space by x and y offsets in DC space.  Return the
// point in VDC space.
/*********************************************************************/
CXY movePointByDCOffset( CXY point, CDCX xDCOffset, CDCY yDCOffset ) ;

// plot the window to the device named in deviceName.
// Device names are printer, plotter, and screen ( NOT case sensitive )
// Does nothing if deviceName doesn't match one of the above
/*********************************************************************/
ERROR_CATEGORY plotDataToDevice(char * deviceName);

/* Reset the screen in case of an error */

void ReInitScreen(void);

// GSS Open Workstation for the screen and then clear it.  Returns the
// result of the GSSOpenWorkstation call.
/*********************************************************************/
int openAndClearScreen(CRECT * GraphARea);

/*********************************************************************/
int closeGraphSystem(void);

/*********************************************************************/
UCHAR set_attributes(UCHAR fore, UCHAR back);
/*********************************************************************/
void display_string(char * string, int len, int row, int column, int attrib);
/*********************************************************************/
void emit(char character, int row, int column, int attrib);
/*********************************************************************/
int row_to_y(int row);
/*********************************************************************/
int column_to_x(int column);
/*********************************************************************/
int y_to_row(int coord);
/*********************************************************************/
int x_to_column(int coord);


#endif   // DEVICE_INCLUDED
  

