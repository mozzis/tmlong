/* winmenu.h */

#ifndef WINMENU_INCLUDED
#define WINMENU_INCLUDED

enum field_types {NO_DATA = 0, STRING_FIELD, FLOAT_FIELD, INT_FIELD};

enum field_index;

/* use the LABEL macro to define the label field of MENU_FIELD */

#define LABEL(x) x , sizeof(x)

/* definition of menu fields */

typedef struct {
  enum field_index index;  /* enforce field indexing via symbol */
  char * label;            /* label - may be NULL */
  int  field_offset;       /* where the field starts - set by LABEL macro */
  int  row;                /* row relative to window origin */
  int  col;                /* col relative to window origin */
  int  len;                /* length in characters */
  enum field_types dtype;  /* data type of field */
  union {
    char * str_data;       /* pointer to data of field - may be NULL... */
    float * flt_data;      /* if dtype is NO_DATA */
    int   * int_data;
    } data;
  } MENU_FIELD;

/* definition of menues */

typedef struct {
  char * title;            /* title of menu */
  MENU_FIELD * fields;     /* pointer to array of fields */
  int items;               /* number of fields in menu */
  int rows;                /* number of rows in menu */
  int cols;                /* number of cols in menu */
  int active_field;        /* active field index - no need to initialize */
  } WMENU;

BOOLEAN PopupMenu(WMENU * Menu);
void InitTextCursors(void);

#endif
