/* -----------------------------------------------------------------------
/
/  doplot.c
/
*/

#include <string.h>       // memmove
#include <cgibind.h>

#include "primtype.h"
#include "plotbox.h"
#include "datafile.h"
#include "curvdraw.h"
#include "doplot.h"
#include "device.h"      // deviceHandle()


void SetPlotForDevice(CRECT * GraphArea)
{
  Plot.fullarea.ll = GraphArea->ll;
  Plot.fullarea.ur = GraphArea->ur;
}

// use DefaultHdr for data file header info
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void InitializePlot(CRECT * GraphArea)
{
  DATAHDR *pDatahdr = &DefaultHdr;

  memmove(&Plot, &pDatahdr->PlotInfo, sizeof(PLOTBOX));

  Plot.fullarea.ll = GraphArea->ll;
  Plot.fullarea.ur = GraphArea->ur;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void CopyPlotToHeader(void)
{
  DATAHDR *pDatahdr = &DefaultHdr;

  memmove(&pDatahdr->PlotInfo, &Plot, sizeof(PLOTBOX));

  return;
}

/**************************************************/
/*                                                */
/* Put up the plot box without drawing any curves */
/*                                                */
/**************************************************/
int PutUpPlotBox(void)
{
  create_plotbox();
  return FALSE;
}

/***************************************************************************/
/* Clr the interior of a plot box and then replot the curves in it without */
/* redrawing all the axis labels, tick marks, etc. The plotbox outline is  */
/* redrawn since previous curves may have drawn on top of it.              */
/***************************************************************************/
void ReplotCurvesOnly(void)
{
   CXY       polygon[7] ;       // vertices of area to fill for erasing
   SHORT     number_vertices ;  // number of vertices in polygon[]
   CCOLOR    selColor ;
   PLOTBOX * plotbox = & Plot;

   // define polygon to erase curves only.
   // use offset of 1 to move polygon one device pixel inside the axes.
   plotboxOutline(plotbox, polygon, (void far *)& number_vertices, 1);
   
   // erase everything inside the plotbox outline
   CSetFillColor( deviceHandle(), plotbox->background_color,
		            & selColor ) ;
   CFillArea(deviceHandle(), number_vertices, polygon);

   drawPlotboxOutline(plotbox);

   plot_curves();
}

/**************************************************/
/*                                                */
/* Draw both the plot box and the curves          */
/*                                                */
/**************************************************/
BOOLEAN Replot(void)
{
   create_plotbox();

   return plot_curves();
}




