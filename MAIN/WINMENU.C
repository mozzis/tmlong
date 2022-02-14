/* winmenu.c */
/* produce and manage popup menues */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cgibind.h>

#include "primtype.h"
#include "cwindows.h"
#include "device.h"
#include "colors.h"
#include "userin.h"
#include "keys.h"
#include "winmenu.h"

WINDOW * W = 0;              /* Window of popup menu */

typedef struct {
  MENU_FIELD * field;
  int        dispCurPos;
  int        editCurPos;
  int        charCnt;
  enum ctype CurType;
  char       editStr[80];
  char       dispStr[80];
} CONTEXT;

CONTEXT Current;

static unsigned char OverwriteTextCursorData[] =
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static unsigned char InsertTextCursorData[] =
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

CXY CursorBMSize = {16,16};

enum ctype { CURSORTYPE_NORMAL = 0, CURSORTYPE_OVERSTRIKE };

CBMHANDLE bmOverwriteCursor, bmInsertCursor;

CMARKERTYPE TextCursor = CURSORTYPE_NORMAL;

BOOLEAN TextCursorOn = FALSE;

static CXY LastTextCursorLoc;

/* -----------------------------------------------------------------------
/
/-----------------------------------------------------------------------*/
void InitTextCursors(void)
{
  CBMHANDLE bmScreen;
  CRECT box;
  CRECT xy;
  CMINMAX val_wid, val_hgt;
  float PixToVDC[2];

  CInqDrawingBitmap(screen_handle, &bmScreen, &box);

  val_wid.Min = 0;
  val_wid.Max = (CursorBMSize.x / 2) - 1;
  val_hgt.Min = 0;
  val_hgt.Max = CursorBMSize.y - 1;

  PixToVDC[0] = (float) screen.LastVDCXY.x /(float)screen.LastXY.x;
  PixToVDC[1] = (float) screen.LastVDCXY.y /(float)screen.LastXY.y;
  xy.ll.x = 0;
  xy.ll.y = 0;
  xy.ur.x = CursorBMSize.x * (int) PixToVDC[0];
  xy.ur.y = CursorBMSize.y * (int) PixToVDC[1];
  CCreateBitmap(screen_handle, xy, CFullDepth, &bmInsertCursor);
  CCreateBitmap(screen_handle, xy, CFullDepth, &bmOverwriteCursor);
  CSelectDrawingBitmap(screen_handle, bmInsertCursor);

  CBytePixels(screen_handle, xy.ll, CursorBMSize.x, CursorBMSize.y,
    val_wid, val_hgt, InsertTextCursorData);

  CSelectDrawingBitmap(screen_handle, bmOverwriteCursor);

  CBytePixels(screen_handle, xy.ll, CursorBMSize.x, CursorBMSize.y,
    val_wid, val_hgt, OverwriteTextCursorData);

  /* SELECT THE SCREEN BITMAP.  CREATE THE CURSOR. */
  CSelectDrawingBitmap(screen_handle, bmScreen);
}

/* -----------------------------------------------------------------------
/  Select display of either the insert or the overstrike cursor
/-----------------------------------------------------------------------*/
static void setMarkerRep(enum CursorType ctype)
{
  CMARKERREPR markerRep;

  markerRep.Type      = CMK_UserDefined;
  markerRep.Color     = CBRT_WHITE;
  markerRep.Height    = 0;      // dummy number for an unused parameter
  markerRep.HotSpot.x = 0;
  markerRep.HotSpot.y = 0;

  if (ctype == CURSORTYPE_OVERSTRIKE)
    markerRep.Handle = bmOverwriteCursor;
  else
    markerRep.Handle = bmInsertCursor;

  CSetMarkerRepr(screen_handle, & markerRep);
}

/*-----------------------------------------------------------------------
/
/-----------------------------------------------------------------------*/

void set_cursor_type(enum CursorType ctype)
//---------------------------------------------------------------------
{
  CPIXOPS SelMode;

  if(TextCursorOn)
    {
    // Set the drawing mode to XOR

    CSetWritingMode(screen_handle, CdXORs, &SelMode);

    CPolymarker(screen_handle, 1, &LastTextCursorLoc);
    }

  setMarkerRep(ctype);

  if(TextCursorOn)
    {
    CPolymarker(screen_handle, 1, &LastTextCursorLoc);

    // Set the drawing mode back to replace the current pixels

    CSetWritingMode(screen_handle, CReplace, &SelMode);
    }
}

/* -----------------------------------------------------------------------
/
/-----------------------------------------------------------------------*/
void set_cursor_type_default(void)
{
  set_cursor_type(CURSORTYPE_NORMAL);
}

/* -----------------------------------------------------------------------
/
/  function:   sets the cursor position on the screen.
/  requires:   (int) row - the row on the screen
/              (int) column - the column on the screen
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void set_cursor(int row, int col)
{
  CPIXOPS SelMode;
  CXY CursorLoc;

  GetWinRowCol(W, &row, &col);

  CursorLoc.x = column_to_x(col);
  CursorLoc.y = row_to_y(row + 1);

  // Set the drawing mode to XOR
  CSetWritingMode(screen_handle, CdXORs, &SelMode);

  // if cursor is showing, erase it from this previous position
  if (TextCursorOn)
    CPolymarker(screen_handle, 1, &LastTextCursorLoc);

  // output cursor to new position
  CPolymarker(screen_handle, 1, &CursorLoc);
  LastTextCursorLoc = CursorLoc;

  // Set the drawing mode to replace the current pixels
  CSetWritingMode(screen_handle, CReplace, &SelMode);

  TextCursorOn = TRUE;
}

/*-----------------------------------------------------------------------
/ if cursor is showing, erase it.
/-----------------------------------------------------------------------*/
void erase_cursor(void)
{
  CPIXOPS SelMode;

  // if cursor is showing, erase it from this previous position
  if (TextCursorOn)
    {
    // Set the drawing mode to XOR
    CSetWritingMode(screen_handle, CdXORs, &SelMode);

    CPolymarker(screen_handle, 1, &LastTextCursorLoc);

    // Set the drawing mode back to replace the current pixels
    CSetWritingMode(screen_handle, CReplace, &SelMode);
    }
  TextCursorOn = FALSE;
}

/* -----------------------------------------------------------------------
/  function:   called to make sure the display image for the field
/              window is current after modifying the field string
/              or taking some other action, such as moving the cursor.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void update_display_string(void)
{
  int space_count = Current.field->len - Current.charCnt;

  strncpy(Current.dispStr, Current.editStr, Current.charCnt);

  if (space_count > 0)
    memset(&(Current.dispStr[Current.charCnt]), ' ', space_count);
  Current.dispStr[Current.field->len] = 0;
}

/* -----------------------------------------------------------------------
/  function:   Copies the Edit string into
/              the Display string, sets the variables associated with
/              the Display string, and also causes the display
/              string to be initialized.
/  requires:   (char *) source_string - the new text for the field.
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void string_to_field_string(char * source_string)
{
  int source_string_len = strlen(source_string);

  if (Current.field->dtype == STRING_FIELD)
    {
    int limit = Current.field->len;

    if((limit > 0) && (source_string_len > limit))
      source_string_len = limit - 1;
    }

  strncpy(Current.editStr, source_string, source_string_len);
  Current.editStr[source_string_len] = 0;

  Current.charCnt = source_string_len;

  Current.editCurPos = source_string_len;

  Current.dispCurPos = Current.charCnt;

  update_display_string();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void display_cursor_right(void)
{
  int max_display_cursor = min(Current.field->len, Current.charCnt);

  if (++Current.dispCurPos > max_display_cursor)
    --Current.dispCurPos;
}

/* -----------------------------------------------------------------------
/  function:   If possible, moves the string cursor to the right,
/              then moves the display cursor to the right.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void cursor_right(void)
{
  if (++Current.editCurPos > Current.charCnt)
    --Current.editCurPos;

  display_cursor_right();
  update_display_string();
}

/*-----------------------------------------------------------------------
------------------------------------------------------------------------*/
void display_cursor_left(void)
{
  if (Current.dispCurPos > 0)
    --Current.dispCurPos;
}

/* -----------------------------------------------------------------------
/  function:   If possible, moves the string cursor to the left, then
/              moves the display cursor to the left.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void cursor_left(void)
{
  if (Current.editCurPos > 0)
    --Current.editCurPos;
  display_cursor_left();
  update_display_string();
}

/* -----------------------------------------------------------------------
/  function:   force the cursor to the beginning of the field text.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void cursor_full_left(void)
{
  Current.dispCurPos = 0;
  Current.editCurPos = 0;

  update_display_string();
}

/* -----------------------------------------------------------------------
/  function:   force the cursor to the end of the field text.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void cursor_full_right(void)
{
  Current.dispCurPos = min(Current.field->len, Current.charCnt);
  Current.editCurPos = Current.charCnt;

  update_display_string();
}

/* -----------------------------------------------------------------------
/  function:   removes a character from the field text string
/              if possible.
/  requires:   (BOOLEAN) go_left - TRUE if the character that should
/              be deleted is to the left of the cursor, otherwise
/              the character at the cursor is deleted
/  returns:    (BOOLEAN) - TRUE if character successfully deleted
/  side effects:
/ ----------------------------------------------------------------------- */
BOOLEAN delete_char_from_field(BOOLEAN go_left)
{
  char *   field_at_cursor;
  int      move_count;

  if (Current.charCnt > 0)
    {
    move_count = Current.charCnt - Current.editCurPos;

    if (go_left)
      {
      if (Current.editCurPos == 0)
        return(FALSE);
      else
        Current.editCurPos--;
      }
    else
      {
      if (Current.editCurPos == Current.charCnt)
        return(FALSE);
      else
        move_count--;
      }

    if (move_count > 0)
      {
      field_at_cursor = &(Current.editStr[Current.editCurPos]);
      memcpy(field_at_cursor, (field_at_cursor + 1), move_count);
      }

    Current.charCnt--;
    Current.editStr[Current.charCnt] = 0;

    return(TRUE);
    }
  else
    return(FALSE);
}

/* -----------------------------------------------------------------------
/  function:   deletes the character to the left of the cursor,
/              usually after the backspace key is pressed.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void char_delete_left(void)
{
  if (!delete_char_from_field(TRUE))
    return;

  if (Current.charCnt == Current.field->len)
    {
    Current.dispCurPos = Current.editCurPos;
    }
  else
    {
    display_cursor_left();
    }
  update_display_string();
}

/* -----------------------------------------------------------------------
/  function:   deletes the character at the cursor, usually after
/              the delete key is pressed.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void char_delete_at_cursor(void)
{
  if (delete_char_from_field(FALSE))
    update_display_string();
}

/* -----------------------------------------------------------------------
/  function:   deletes all text in the field at one blow.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */

void delete_entire_field(void)
{
  Current.charCnt = 0;
  Current.editCurPos = 0;
  Current.editStr[0] = 0;               /* in case field is zoomed */
  Current.dispCurPos = 0;
  update_display_string();
}

/* -----------------------------------------------------------------------
/  function:   called to add a character to the field string at
/              the current position.
/  requires:   (unsigned char) key - the character to insert
/  returns:    (BOOLEAN) - TRUE if character successfully added to string
/  side effects:
/ ----------------------------------------------------------------------- */
BOOLEAN insert_char_into_field(unsigned char key)
{
  char *   editPos;
  BOOLEAN  at_last_position;

  at_last_position = (Current.editCurPos >= Current.charCnt);

  /* is there NO room for another character? */
  if (Current.charCnt > Current.field->len)
    {
    if (Current.CurType == CURSORTYPE_OVERSTRIKE)
      {                          /* in overstrike mode characters are */
      if (at_last_position)   /* not really "inserted" unless the  */
        return(FALSE);       /* cursor is at the end of the text  */
      }
    else
      return(FALSE);
    }
  editPos = &(Current.editStr[Current.editCurPos]);

  if (Current.CurType == CURSORTYPE_OVERSTRIKE)
    {
    if (at_last_position)
      Current.charCnt++;
    }
  else
    {
    if (!at_last_position)
      {
      memmove((editPos + 1), editPos, (Current.charCnt - Current.editCurPos));
      }
    Current.charCnt++;
    }
  *editPos = key;

  Current.editCurPos++;
  Current.editStr[Current.charCnt] = 0;

  return(TRUE);
}

/* -----------------------------------------------------------------------
/  function:   called to put a key into the field.  If it can
/              be inserted into the field text, the cursor is
/              adjusted in the appropriate direction and the
/              field display string is updated.
/  requires:   (unsigned char) key - the character to insert
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
void char_insert(unsigned char key)
{
  if (insert_char_into_field(key))
    {
    if (Current.dispCurPos < Current.field->len)
      display_cursor_right();
    update_display_string();
    }
}

/* -----------------------------------------------------------------------
/  function:   switches between overstrike and insert mode.
/              The cursor is also modified to show which mode is active.
/  requires:   (void)
/  returns:    (void)
/  side effects:
/ ----------------------------------------------------------------------- */
static void toggle_insert_mode(void)
{
  if (Current.CurType == CURSORTYPE_OVERSTRIKE)
    Current.CurType = CURSORTYPE_NORMAL;
  else
    Current.CurType = CURSORTYPE_OVERSTRIKE;

  set_cursor_type(Current.CurType);
}

static void FieldToDisplayString(MENU_FIELD * Field)
{
  int fldlen;
  if (Field->dtype)
    {
    switch (Field->dtype)
      {
      case STRING_FIELD:
        sprintf(Current.dispStr,"%s", Field->data.str_data);
      break;
      case FLOAT_FIELD:
        sprintf(Current.dispStr, "%#-*g", Field->len, *Field->data.flt_data);
      break;
      case INT_FIELD:
        sprintf(Current.dispStr, "%-*d", Field->len, *Field->data.int_data);
      break;
      }
    fldlen = strlen(Current.dispStr);
    if (fldlen < Field->len)
      memset(&Current.dispStr[fldlen], ' ', Field->len - fldlen);
    Current.dispStr[Field->len] = '\0';
    }
}

static void ConvertDispToField(MENU_FIELD * Field)
{
  switch (Field->dtype)
    {
    case STRING_FIELD:
      strncpy(Field->data.str_data, Current.dispStr, Field->len);
      Field->data.str_data[Field->len] = '\0';
    break;
    case FLOAT_FIELD:
      *Field->data.flt_data = (float)atof(Current.dispStr);
    break;
    case INT_FIELD:
      *Field->data.int_data = (int)atoi(Current.dispStr);
    break;
    }
}

/*******************************************************************/

static void OutdispStr(MENU_FIELD * Field, enum colordex Color)
{
  int fldpos = Field->field_offset;

  erase_cursor();

  txt_to_win(W, Current.dispStr, Field->row, Field->col + fldpos, Color);
}

/*******************************************************************/

static void DisplayField(MENU_FIELD * Field, BOOLEAN Hot)
{
  enum colordex Fcolor;

  Current.field = Field;

  if (Hot)                   /* if cursor is on field, draw reversed */
    Fcolor = REVERSE_COLOR;
  else
    Fcolor = REGULAR_COLOR;

  if (Field->label)
    {
    txt_to_win(W, Field->label, Field->row, Field->col, REGULAR_COLOR);
    }

  FieldToDisplayString(Field);

  OutdispStr(Field, Fcolor);
}

/* -----------------------------------------------------------------------
/  function:   Takes appropriate action for the various cursor and
/              editing keys, and inserts any characters.
/  requires:   (unsigned char) key - key from keyboard input
/              routine (which translates special keys into
/              single byte codes with the high bit set)
/              NOTE: if key value is NULL, this routine performs
/              self-initialization, as all field type character
/              action routines are expected to do
/  returns:    (BOOLEAN) - TRUE if character was recognized
/              and used by this routine
/  side effects:
/ ----------------------------------------------------------------------- */

BOOLEAN default_char_action(unsigned char key)
{
  BOOLEAN key_was_used = TRUE;
  static  BOOLEAN erase_trigger = TRUE;
  int     row, col;

  if (key)
    {
    if (key & KEYS_HIGH_BIT)
      {
      switch (key)
        {
        case KEY_BACKSPACE:
          char_delete_left();
        break;
        case KEY_RIGHT:
          cursor_right();
        break;
        case KEY_LEFT:
          cursor_left();
        break;
        case KEY_HOME:
          cursor_full_left();
        break;
        case KEY_END:
          cursor_full_right();
        break;
        case KEY_DELETE:
          char_delete_at_cursor();
        break;
        case KEY_DELETE_FAR:
          delete_entire_field();
        break;
        case KEY_INSERT:
          toggle_insert_mode();
        break;
        default:
          key_was_used = FALSE;
        }
      if (key_was_used)
        erase_trigger = FALSE;
      }
    else
      {
      if (erase_trigger != FALSE)
        {
        delete_entire_field();
        erase_trigger = FALSE;
        }
      char_insert(key);
      }

    OutdispStr(Current.field, REVERSE_COLOR);

    // Show the cursor position
    row = Current.field->row;
    col = Current.field->col + Current.field->field_offset;
    set_cursor(row, col + Current.dispCurPos);
    }
  else     /* function was called to do self-initialization */
    {
    erase_trigger = TRUE;
    }
  return(key_was_used);
}

void ManageField(MENU_FIELD * Field, int key)
{
  BOOLEAN Done = FALSE;

  Current.field = Field;

  set_cursor_type(CURSORTYPE_NORMAL);

  default_char_action(0);

  default_char_action(key);

  do
    {
    key = get_key_input();
    switch (key)
      {
      case KEY_ENTER:
      case KEY_ESCAPE:
        Done = TRUE;
      break;
      default:
        default_char_action(key);
      }
    }
  while (!Done);

  if (key != KEY_ESCAPE)
    {
    ConvertDispToField(Field);
    }
  
  DisplayField(Field, TRUE);

}

/**********************************************************************/
/*                                                                    */
/* Look for next field vertically (DOWN KEY)                          */
/*                                                                    */
/**********************************************************************/
static void next_vert_field(WMENU * Menu)
{
  int hdiff = 0x7FFF, tdiff,
  oldIndex = Menu->active_field, j;

  DisplayField(&Menu->fields[oldIndex], FALSE); /* unhilite current field */
  for (j = 0; j < Menu->items; j++)
    {
    tdiff = abs(Menu->fields[j].col - Menu->fields[oldIndex].col);
    if (tdiff < hdiff )
      if (Menu->fields[j].row > Menu->fields[oldIndex].row)
        {
        hdiff = tdiff;
        Menu->active_field = j;
        }
    }
  if (Menu->active_field == oldIndex) /* No prev vert field, find topmost */
    for (j = Menu->items - 1; j >= 0; j--)
      {
      if (Menu->fields[j].row < Menu->fields[oldIndex].row)
        Menu->active_field = j;
      }
  DisplayField(&Menu->fields[Menu->active_field], TRUE); /* hilite field */
}

/**********************************************************************/
/*                                                                    */
/* Look for prev field vertically (UP KEY)                            */
/*                                                                    */
/**********************************************************************/
static void prev_vert_field(WMENU * Menu)
{
  int hdiff = 0x7FFF, tdiff,
  oldIndex = Menu->active_field, j;

  DisplayField(&Menu->fields[oldIndex], FALSE); /* unhighlight this field */
  for (j = Menu->items - 1; j >= 0; j--)
    {
    tdiff = abs(Menu->fields[j].col - Menu->fields[oldIndex].col);
    if (tdiff < hdiff )
      if (Menu->fields[j].row < Menu->fields[oldIndex].row)
        {
        hdiff = tdiff;
        Menu->active_field = j;
        }
    }
  if (Menu->active_field == oldIndex) /* no next vert field, find topmost */
    for (j = 0; j < Menu->items; j++)
      {
      if (Menu->fields[j].row > Menu->fields[oldIndex].row)
        Menu->active_field = j;
      }
  DisplayField(&Menu->fields[Menu->active_field], TRUE); /* highlite field */
}


/**********************************************************************/
/*                                                                    */
/* Look for next field in same horizontal row (RARROW KEY)            */
/*                                                                    */
/**********************************************************************/
static void next_horiz_field(WMENU * Menu)
{
  int j, i = Menu->active_field;
  BOOLEAN success = FALSE;

  DisplayField(&Menu->fields[i], FALSE);    /* unhighlight current field */
  for (j = 0; j < Menu->items; j++)
    if (Menu->fields[j].row == Menu->fields[i].row)
      if (Menu->fields[j].col > Menu->fields[i].col)
        {
        success = TRUE;
        break;
        }
  if (!success) /* no succeeding horiz field, look for leftmost */
    for (j = Menu->items - 1; j >= 0; j--)
      if (Menu->fields[j].row == Menu->fields[i].row)
        if (Menu->fields[j].col < Menu->fields[i].col)
          {
          i = j;
          }
  if (success)
    i= j;
  Menu->active_field = i;
  DisplayField(&Menu->fields[i], TRUE); /* highlight new current field */
}

/**********************************************************************/
/*                                                                    */
/* Look for prev field in same horizontal row (LARROW KEY)            */
/*                                                                    */
/**********************************************************************/
static void prev_horiz_field(WMENU * Menu)
{
  int j, i = Menu->active_field;
  BOOLEAN success = FALSE;

  DisplayField(&Menu->fields[i], FALSE);    /* unhighlight current field */
  for (j = Menu->items - 1; j >= 0; j--)
    if (Menu->fields[j].row == Menu->fields[i].row)
      if (Menu->fields[j].col < Menu->fields[i].col)
        {
        success = TRUE;
        break;
        }
  if (!success) /* no preceding horiz field, look for rightmost */
    for (j = 0; j < Menu->items; j++)
      if (Menu->fields[j].row == Menu->fields[i].row)
        if (Menu->fields[j].col > Menu->fields[i].col)
          {
          i = j;
          }
  if (success)
    i= j;
  Menu->active_field = i;
  DisplayField(&Menu->fields[i], TRUE);
}

/**********************************************************************/
/*                                                                    */
/* Look for next field in menu - uses declaration order (TAB KEY)     */
/*                                                                    */
/**********************************************************************/
static void next_field(WMENU * Menu)
{
  int i = Menu->active_field;

  DisplayField(&Menu->fields[i++], FALSE);
  if (i > Menu->items - 1)
    i = 0;
  Menu->active_field = i;
  DisplayField(&Menu->fields[i], TRUE);
}

/**********************************************************************/
/*                                                                    */
/* Look for prev field in menu - uses declaration order (BACKTAB KEY) */
/*                                                                    */
/**********************************************************************/
static void prev_field(WMENU * Menu)
{
  int i = Menu->active_field;

  if (i == 0)
    i = Menu->items - 1;

  DisplayField(&Menu->fields[i--], FALSE);
  Menu->active_field = i;
  DisplayField(&Menu->fields[i], TRUE);
}


/**********************************************************************/
/*                                                                    */
/*                                                                    */
/*                                                                    */
/**********************************************************************/
static void manage_menu(WMENU * Menu)
{
  BOOLEAN done = FALSE;
  UCHAR key;

  while (!done)
    {
    key = get_key_input();
    switch (key)
      {
      case KEY_ESCAPE:
      case KEY_ENTER:
        done = TRUE;
      break;
      case KEY_TAB:
        next_field(Menu);
      break;
      case KEY_BACK_TAB:
        prev_field(Menu);
      break;
      case KEY_RIGHT:
        next_horiz_field(Menu);
      break;
      case KEY_LEFT:
        prev_horiz_field(Menu);
      break;
      case KEY_DOWN:
        next_vert_field(Menu);
      break;
      case KEY_UP:
        prev_vert_field(Menu);
      break;
      default:
        ManageField(&Menu->fields[Menu->active_field], key);
      }
    }
}

/**********************************************************************/
/*                                                                    */
/* Draw a popup window, then place the text from the menu definition  */
/* within it. Returns whether was able to create window.              */
/*                                                                    */
/**********************************************************************/

BOOLEAN PopupMenu(WMENU * Menu)
{
  int i;
  MENU_FIELD * Fields = Menu->fields;

  Menu->active_field = 0;

  if ((Menu->rows < 4) || (Menu->rows > (UCHAR)screen_rows))
    Menu->rows = (UCHAR)screen_rows;

  W = define_transient_window(0, 0, Menu->rows, Menu->cols, (char)0);

  if (W)
    {
    autocenter_window(W);

    open_window(W, COLORS_MESSAGE);

    title_into_window(W, Menu->title, HIGHLIGHT_COLOR);

    for (i = 0; i < Menu->items; i++)
      {
      DisplayField(&Fields[i], i == Menu->active_field);
      }

    manage_menu(Menu);

    close_window(W);

    destroy_transient_window(W);

    return(TRUE);
    }
  else
    return(FALSE);
}

