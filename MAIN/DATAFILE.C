#include <stdio.h>
#ifdef __TURBOC__
#include <dir.h>
#else
#include <stdlib.h>
#endif
#include <dos.h>
#include <string.h>
#include <cgibind.h>

#include "primtype.h"
#include "plotbox.h"
#include "datafile.h"
#include "data.h"
#include "doplot.h"
#include "colors.h"
#include "error.h"

char DataFileName[] = "TMLONG.DAT";
char DataFileSpec[MAXDRIVE+MAXDIR+MAXFILE+MAXEXT+1];

DATAHDR DefaultHdr =
  {
  {"TMLONGDATA"},              /* FileID[10] */
   0.1F,                       /* Version */
   sizeof(DATAHDR),            /* SzHeader */
   100.0,                      /* LinFactor */
   (long double)0.0,           /* TimeStamp (long double) */
   60.0,                       /* Duration */
   {"Default setup"},          /* Comment[128] */
   {                           /* Plot Box */
     {"TM Long Calibration"},  /*  title  */
     1.0F,                     /*  xscale */
     1.0F,                     /*  yscale */
     {                         /*  fullarea */
       {0, 0},                 /* these values work for VGA 640X480 */
       {32767, 24575},         /* device.c will adjust to hardware */
     },
     {                         /*  plotarea */
       {0, 0},                   /* these values = fullscreen VGA 640X480 */
       {32767, 24575},           /* plotstuff will adjust to hardware */
     },
     {                         /*  x axis */
       {"Time (Sec.)"},          /*  legend */
       {10, 10},                 /*  axis_end_offset */
       {10, 10},                 /*  axis_zero */
       DEFAULT_POINTS - 1,       /*  max_value */
       0.0F,                     /*  min_value */
       DEFAULT_POINTS - 1,       /*  original_max_value */
       0.0F,                     /*  original_min_value */
       1.0F,                     /*  inv_range */
       TRUE,                     /*  ascending */
       COUNTS                    /*  units */
     }, 
     {                         /*  y axis */
       {"Elongation (%)"},       /*  legend */
       {10, 10},                 /*  axis_end_offset */
       {10, 10},                 /*  axis_zero */
       100.0F,                   /*  max_value */
       0.0F,                     /*  min_value */
       100.0F,                   /*  original_max_value */
       0.0F,                     /*  original_min_value */
       1.0F,                     /*  inv_range */
       TRUE,                     /*  ascending */
       COUNTS                    /*  units */
     }, 
     {                         /*  z axis */
       {""},                     /*  legend */
       {0, 0},                   /*  axis_end_offset */
       {0, 0},                   /*  axis_zero */
       0.0F,                     /*  max_value */
       0.0F,                     /*  min_value */
       0.0F,                     /*  original_max_value */
       0.0F,                     /*  original_min_value */
       1.0F,                     /*  inv_range */
       TRUE,                     /*  ascending */
       COUNTS                    /*  units */
     }, 
       
     0,                         /*  xz_percent */
     0,                         /*  yz_percent */
     NOSIDE,                    /*  z_position  */
     CBLUE,                     /*  background_color */
     CBRT_WHITE,                /*  box_color */
     CBRT_WHITE,                /*  text_color */
     CBRT_WHITE,                /*  grid_color */
     CLN_LongDashed,            /*  grid_line_type */
     CBRT_YELLOW,               /*  plot_color */
     CLN_Solid,                 /*  plot_line_type */
    }

  };

void make_file_spec(char * pathbase)
{
  char drive[3];
  char dir[64];
  char name[8];
  char exten[3];

#ifdef __TURBOC__ 
  fnsplit(pathbase, drive, dir, name, exten);
#else
  _splitpath(pathbase, drive, dir, name, exten);
#endif
  strcpy(DataFileSpec, drive);
  strcat(DataFileSpec, dir);
  strcat(DataFileSpec, DataFileName);
}

ERROR_CATEGORY save_data_file(char * pathbase)
{
  FILE * outfile;
  int i;
       
  make_file_spec(pathbase);

  if (!(outfile = fopen(DataFileSpec, "wb")))
    return(error(ERROR_OPEN, DataFileName));
 
  CopyPlotToHeader();

  if (fwrite(&DefaultHdr, sizeof(DATAHDR), 1, outfile) != 1)
    {
    fclose(outfile);
    return(error(ERROR_WRITE, DataFileName)); 
    }
  
  if (fwrite(&RunData, sizeof(DATA), 1, outfile) != 1)
    {
    fclose(outfile);
    return(error(ERROR_WRITE, DataFileName)); 
    }

  for (i = 0; i < RunData.CurveCount; i++)
    {
    float * Dptr = &(RunData.Points[i * RunData.Count]);
    if (fwrite(Dptr, sizeof(float), (int)RunData.Count, outfile) !=
          RunData.Count)
      {
      fclose(outfile);
      return(error(ERROR_WRITE, DataFileName)); 
      }
    }
  fclose(outfile);
  return(ERROR_NONE);
}

ERROR_CATEGORY load_data_file(char * pathbase)
{
  FILE * infile;
  ERROR_CATEGORY err;
  int i;

  make_file_spec(pathbase);

  if (!(infile = fopen(DataFileSpec, "rb")))
    return(error(ERROR_OPEN, DataFileName));
 
  if (fread(&DefaultHdr, sizeof(DATAHDR), 1, infile) != 1)
    {
    fclose(infile);
    return(error(ERROR_READ, DataFileName)); 
    }
  
  freeDataPoints();

  if (fread(&RunData, sizeof(DATA), 1, infile) != 1)
    {
    fclose(infile);
    return(error(ERROR_READ, DataFileName)); 
    }

  RunData.Points = NULL;
  
  err = allocDataPoints();

  if (err)
    return(err);
    
  for (i = 0; i < RunData.CurveCount; i++)
    {
    float * Dptr = &(RunData.Points[i * RunData.Count]);
    if (fread(Dptr, sizeof(float), (int)RunData.Count, infile) !=
          RunData.Count)
      {
      fclose(infile);
      return(error(ERROR_READ, DataFileName)); 
      }
  }
  fclose(infile);
  return(ERROR_NONE);
}

