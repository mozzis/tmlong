/* -----------------------------------------------------------------------
/
/   doplot.h
/
/ ----------------------------------------------------------------------- */
  
#ifndef DOPLOT_INCLUDED
#define DOPLOT_INCLUDED

void SetPlotForDevice(CRECT * GraphArea);
void InitializePlot(CRECT * GraphArea);
void CopyPlotToHeader(void);

void ReplotCurvesOnly(void) ;

BOOLEAN Replot(void);

int PutUpPlotBox(void);

void UpdatePlotscreenString(SHORT Row, SHORT Column, PCHAR String);

#endif  //  DOPLOT_INCLUDED
