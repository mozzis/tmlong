/* -----------------------------------------------------------------------
/
/  error.h
/
/ ----------------------------------------------------------------------- */

#ifndef ERROR_INCLUDED
#define ERROR_INCLUDED

#ifndef PRIMTYPE_INCLUDED
   #include "primtype.h"
#endif

extern unsigned int DOS_int24_info;

void prepare_int24_trap(void);
void init_error_file(char * path);

#define MAX_MESSAGE_ROWS   14

// error code 0 means no error, see primtype.h

/* 1-99:  real primal errors */
#define  ERROR_ALLOC_MEM                  1
#define  ERROR_FLOATING_POINT             2
  
/* 300-399:  Disk I/O errors */
  
#define  ERROR_OPEN                     300
#define  ERROR_CLOSE                    301
#define  ERROR_READ                     302
#define  ERROR_WRITE                    303
#define  ERROR_BAD_FILENAME             304
#define  ERROR_BAD_DIRNAME              305
#define  ERROR_BAD_DRIVE                306
#define  ERROR_ACCESS_DENIED            307
#define  ERROR_SHARING                  308
#define  ERROR_DEV_OPEN                 309
#define  ERROR_DEV_CLOSE                310
#define  ERROR_DEV_READ                 311
#define  ERROR_DEV_WRITE                312
  
/* (320-3??: DOS Disk I/O Critical errors) */
  
#define  ERROR_DRIVE_NOT_READY          320
#define  ERROR_SEEK                     321
#define  ERROR_SECTOR_NOT_FOUND         322
#define  ERROR_BAD_SECTOR               323
#define  ERROR_GENERAL_DISK_FAILURE     324
  
/* 400-499:  I/O errors (other than Disk or GPIB I/O) */
  
#define  ERROR_DEV_IOCTL                400
  
#define  ERROR_NO_PAPER                 410

#define  ERROR_ABORTED                 1000  
/* 500-599:  Operating System errors (other than I/O) */
  
#define  ERROR_GENERIC                 9999
  
struct crit_err {
   unsigned    error_code    : 8;
   unsigned    read_or_write : 1;
   unsigned    disk_location : 2;
   unsigned    not_used      : 4;
   unsigned    disk_or_other : 7;
};
  
/* AH values: */
/*    bit 7 - 0 = disk error, 1 = other I/O error */
/*    bits 1,2 - disk location where error occurred */
/*       00 - DOS area */
/*       01 - File Allocation Table (FAT) */
/*       10 - directory */
/*       11 - file area */
/*    bit 0 - 0 = read error, 1 = write error */
  
/* DI values (DOS error code): */
#define  DOSCRIT_WRITE_PROTECT            0     /*  0 - write-protected */
#define  DOSCRIT_UNKNOWN_UNIT             1     /*  1 - unknown unit */
#define  DOSCRIT_DRIVE_NOT_READY          2     /*  2 - drive not ready */
#define  DOSCRIT_UNKNOWN_COMMAND          3     /*  3 - unknown command */
#define  DOSCRIT_DATA_CRC_ERROR           4     /*  4 - data error (bad CRC) */
#define  DOSCRIT_BAD_REQUEST              5     /*  5 - bad request structure length */
#define  DOSCRIT_SEEK_ERROR               6     /*  6 - seek error */
#define  DOSCRIT_UNKNOWN_MEDIA            7     /*  7 - unknown media type */
#define  DOSCRIT_SECTOR_NOT_FOUND         8     /*  8 - sector not found */
#define  DOSCRIT_NO_PAPER                 9     /*  9 - printer out of paper */
#define  DOSCRIT_WRITE_FAULT             10     /* 10 - write fault */
#define  DOSCRIT_READ_FAULT              11     /* 11 - read fault */
#define  DOSCRIT_GENERAL_FAILURE         12     /* 12 - general failure */
/* (including unformatted diskette) */

ERROR_CATEGORY error(ERROR_CATEGORY error_number, ...);
BOOLEAN test_for_DOS_critical_error(char * device_or_file);

void ErrorBeep(void);       
void ErrorBeepToggle(void);

#endif
