/* Useful constants */

#ifndef CONSTANT_INCLUDED
#define CONSTANT_INCLUDED

#define SZFLOAT sizeof( float )
#define SZLONG  sizeof( long )

#define MAXFLOAT     3.4E19
#define MINFLOAT     -3.4E19
  
#define MAXDOUBLE    1.7E154
#define MINDOUBLE    -1.7E154
  
#define MAX_DOUBLE_DIFF (1e-9)  /* 1e-9, for double float comparisons */
#define MAX_FLOAT_DIFF  (1e-9)  /* 1e-9, for single float comparisons */

/* shift status port address and flags for DOS */
enum { RIGHTSHIFT = 1, LEFTSHIFT = 2, CNTRL_FLAG = 4, ALT_FLAG = 8 } ;

#define SHIFT_STATUS_ADDR  0x00400017

#define PLANCK       6.626176e-34 /* Planck's constant in Joules * seconds */
#define LIGHTSPEED   2.99792458e8 /* speed of light in meters/sec */

#define  PI  3.1415926535

enum { DOSFILESIZE = 12, DOSPATHSIZE = 64 } ;

enum { FNAME_LENGTH = DOSFILESIZE + DOSPATHSIZE + 1 } ;

enum { DESCRIPTION_LENGTH = 81 } ;

enum { FTIDLEN = 40 } ;

#endif
