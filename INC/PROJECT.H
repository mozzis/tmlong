/*****************************************************************************
 *
 * The include file for the GSS*CGI demonstration program, CGICTEST.C.
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * Defines
 *
 *****************************************************************************/

#define GMODE 1
#define CMODE 2

/*
 * Error value defines.
 */

#define CGI_NOT_PRESENT -3003
#define CGI_NOT_TRANSIENT   -2978
#define DRIVERS_ALREADY_LOADED  -2977
#define MEMORY_TOO_SMALL_FOR_GSSCGI -3034
#define INSUFFICIENT_MEMORY -1

/**********************************************************************
 *
 * Function prototypes for private CGITEST functions.
 *
 **********************************************************************/

int load_configuration (void);
int load_drivers (void);
int remove_configuration (void);

void remove_drivers (void);
