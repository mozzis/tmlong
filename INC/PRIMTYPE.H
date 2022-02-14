/***************************************************************************/
/*  PRIMTYPE.H                                                             */
/*                                                                         */
/*      Basic types, opaque types, and constants
/
/***************************************************************************/
  
#ifndef PRIMTYPE_INCLUDED
#define PRIMTYPE_INCLUDED
  
#define NOT !
  
enum { FALSE = 0, TRUE = 1 } ;
  
typedef unsigned short BOOLEAN;

#define NIL 0L
  
#define FAR     far
#define NEAR    near
#define VOID    void
  
typedef char CHAR;
typedef CHAR far *PCHAR;
typedef unsigned char UCHAR;
typedef UCHAR far *PUCHAR;
typedef int SHORT;
typedef SHORT far *PSHORT;
typedef unsigned USHORT;
typedef USHORT far *PUSHORT;
typedef long LONG;
typedef LONG far *PLONG;
typedef unsigned long ULONG;
typedef ULONG far *PULONG;
  
typedef unsigned char BYTE;
  
typedef VOID   FAR *PVOID;
  
typedef  long FIXED;
  
typedef float FLOAT;
typedef FLOAT far *PFLOAT;
typedef double DOUBLE;
typedef DOUBLE far *PDOUBLE;
  
typedef int ERROR_CATEGORY ;  /* for error codes for error reporting */

enum { ERROR_NONE = 0 } ;     /* zero means no error */

#endif
