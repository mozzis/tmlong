/* data.h */

#ifndef DATA_INCLUDED
#define DATA_INCLUDED

struct data {
  int CurveCount;     /* number of curves in block, usually 1 */
  int typeData;       /* e.g. 0x0014 for long int, or 0x0024 for float */
  unsigned  Count;    /* number of points in a curve */
  float *   Points;   /* one or more arrays of points */
} ;

typedef struct data DATA;

extern DATA RunData;

#define DEFAULT_POINTS 4095

ERROR_CATEGORY allocDataPoints(void);
ERROR_CATEGORY freeDataPoints(void);
int GenFakeData(void);
int GenRealData(void);

#endif
