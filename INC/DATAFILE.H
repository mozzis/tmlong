/* datafile.h */
/* structure of data file for TM Long project */

#ifndef DATAFILE_INCLUDED
#define DATAFILE_INCLUDED

#ifndef PLOTBOX_INCLUDED
#include "plotbox.h"
#endif

/* structure of data file for TM Long project */

typedef struct
  {
  char FileID[10];         /* e.g. TMLONGDATA */
  float Version;
  int SzHeader;            /* number of bytes in header */
  double LinFactor;        /* elongation linearity correction factor */
  long double TimeStamp;   /* time and date of this run */
  double Duration;         /* time in seconds for this run */
  char Comment[128];
  PLOTBOX PlotInfo;
  } DATAHDR;

/* data follows... */

extern DATAHDR DefaultHdr;

ERROR_CATEGORY save_data_file(char *pathbase);
ERROR_CATEGORY load_data_file(char *pathbase);

#endif
