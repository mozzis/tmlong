/* -----------------------------------------------------------------------
/
/  cwindows.c
/
/ ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <cgibind.h>

#include "primtype.h"
#include "colors.h"
#include "keys.h"
#include "cwindows.h"
#include "userin.h"
#include "device.h"

typedef struct ndxcolorset {
   COLOR_PAIR  colors[5];  /* and others as needed */
} INDEX_COLOR_SET;

struct wind {
   unsigned int            Attrib;
   unsigned char           Row;
   unsigned char           Column;
   unsigned char           SizeInRows;
   unsigned char           SizeInColumns;
   int                     TextStringSetCount;
   char **                 TextStringSet;
   INDEX_COLOR_SET *       ColorSet;
   struct save_area_info * SavedArea;
};

typedef struct {
   char *        text;
   unsigned char text_length;
   unsigned char column_offset;
} OPTION;

char * BusyWorking[] = { "Working...", NULL };
char * BusyWorkingEsc[] = { "Working...", "Press <ESC> to abort", NULL };
char * DataFileOverwritePrompt[] = {
  "This option will irretrievably ",
  "overwrite an existing data file. ",
  "Continue?",
  NULL
};

#define MAX_CHOICES 10

#define STRING_SET_GRANULES 32

static char * yes_no_choice_text[] = { "No", "Yes", NULL };

static UCHAR PopupChoiceRow;
static OPTION * PopupChoiceOptions = NULL;
static int PopupChoiceCount = 0;

static WINDOW * PopupWindow;

static int WindowCount = 0;
#define MAX_SAVED_WIN 6
static WINDOW * ActiveWindows[MAX_SAVED_WIN];

typedef SHORT (CharAction)(WINDOW *, UCHAR Row, SHORT Rows, USHORT *attr,
                           UCHAR *ItemRows, USHORT Items, USHORT Itdex,
                           BOOLEAN Ptr, UCHAR Key);

/* private functions */

void  txt_to_win(WINDOW * W, char * s, UCHAR rset, UCHAR cset, UCHAR coldex);
UCHAR get_color_index(USHORT StringAttr);
void  strings_to_win(WINDOW * W, UCHAR rset, int sdex, int cnt, USHORT * attrs);
void  display_choice(WINDOW * W, OPTION * opts, int optdex, UCHAR coldex);
int   offer_choices_in_window(WINDOW * W, char ** cstrings, int deflt_choice);
char ** extend_string_set(char ** current_set, int * elements);

void manage_window(WINDOW * W, UCHAR row, SHORT rows, PUSHORT attrs,
                   UCHAR * ItemRowCount, USHORT ItemCount, BOOLEAN Ptr,
                   CharAction * DoThis);

void title_into_window(WINDOW * W, CHAR *title, UCHAR coldex);

BOOLEAN title_message_window(CHAR *title, CHAR **msg, USHORT *attr,
                             UCHAR *ItemRows, USHORT Items, UCHAR MaxRows,
                             UCHAR color_set_index, BOOLEAN Pointer,
                             CharAction *);

BOOLEAN put_up_title_message_window(PCHAR title_text,
                                    PCHAR * message_text,
                                    PUSHORT attribute_set,
                                    UCHAR MaxRows,
                                    UCHAR color_set_index,
                                    BOOLEAN Dynamic,
                                    WINDOW * *MessageWindow);

// -----------------------------------------------------------------------
WINDOW * define_transient_window(unsigned char Row, unsigned char Column,
                                 unsigned char SizeInRows,
                                 unsigned char SizeInColumns,
                                 unsigned int  Attrib)
{
  WINDOW * WindowPtr = NULL;

  if ((WindowPtr = malloc(sizeof(WINDOW))) != NULL)
    {
    memset(WindowPtr, 0, sizeof(WINDOW));

    WindowPtr->Row = Row;
    WindowPtr->Column = Column;
    WindowPtr->SizeInRows = SizeInRows;
    WindowPtr->SizeInColumns = SizeInColumns;
    WindowPtr->Attrib = Attrib;
    ActiveWindows[WindowCount++] = WindowPtr;
    if (WindowCount >= MAX_SAVED_WIN)
      WindowCount = MAX_SAVED_WIN - 1;
    }
  return (WindowPtr);
}

// -----------------------------------------------------------------------

WINDOW * destroy_transient_window(WINDOW * WindowPtr)
{
  free(WindowPtr);
  if (WindowCount > 0)
    ActiveWindows[WindowCount--] = NULL;
  return(0);
}

void ReleaseAllWindows(void)
{
  int i;
  for (i = WindowCount; i > 0; i--)
    destroy_transient_window(ActiveWindows[i]);
}

// -----------------------------------------------------------------------
void attach_strings_to_window(WINDOW * WindowPtr, char ** StringSet)
{
  WindowPtr->TextStringSetCount = 0;
  WindowPtr->TextStringSet = StringSet;

  if(StringSet)
    {
    int count = 0;

    while(StringSet[ count ])
      count++;

    WindowPtr->TextStringSetCount = count;
    }
}

// -----------------------------------------------------------------------
void autosize_window(WINDOW * WindowPtr)
{
  int            i;
  int            width;
  int            max_width = 0;
  unsigned char  required_columns;
  unsigned char  required_rows;

  for (i=0; i < WindowPtr->TextStringSetCount; i++)
    {
    width = strlen(WindowPtr->TextStringSet[i]);
    if (max_width < width)
      max_width = width;
    }
  required_columns = (unsigned char) (max_width + 2);
  required_rows = (unsigned char) (WindowPtr->TextStringSetCount + 2);

  if (WindowPtr->SizeInRows > required_rows)
    WindowPtr->SizeInRows = required_rows;

  if (WindowPtr->SizeInColumns > required_columns)
    WindowPtr->SizeInColumns = required_columns;
}

// -----------------------------------------------------------------------
void autocenter_window(WINDOW * WindowPtr)
{
  WindowPtr->Row =
    (unsigned char)((screen_rows / 2) - (WindowPtr->SizeInRows / 2));

  WindowPtr->Column =
    (unsigned char)((screen_cols / 2) - (WindowPtr->SizeInColumns / 2));
}

// -----------------------------------------------------------------------
void open_window(WINDOW * W, unsigned char color_set_index)
{
  unsigned char attribute;

  W->ColorSet = (INDEX_COLOR_SET *) &ColorSets[color_set_index];

  if ((W->SavedArea = save_screen_area(W->Row, W->Column,
    W->SizeInRows, W->SizeInColumns)) != NULL)
    {
    attribute =
      set_attributes(W->ColorSet->colors[REGULAR_COLOR].foreground,
      W->ColorSet->colors[REGULAR_COLOR].background);
    }

  erase_screen_area(W->Row, W->Column,
    W->SizeInRows, W->SizeInColumns, attribute, TRUE);
}

// -----------------------------------------------------------------------
void close_window(WINDOW * W)
{
   if ((W->SavedArea != NULL))
      restore_screen_area(W->SavedArea);
}

unsigned char get_window_attribute(WINDOW * W, UCHAR color_index)
{
  return(set_attributes(W->ColorSet->colors[color_index].foreground,
         W->ColorSet->colors[color_index].background));
}

// -----------------------------------------------------------------------
void txt_to_win(WINDOW * W, char * string, UCHAR row_offset,
                UCHAR column_offset, UCHAR color_index)
{
  unsigned char attribute;
  unsigned char string_len = (unsigned char) strlen(string);
  unsigned char max_column_offset;

  if (row_offset >= W->SizeInRows)
    row_offset = (W->SizeInRows - (char)1);

  if (string_len > W->SizeInColumns)
    string_len = W->SizeInColumns;

  max_column_offset = (W->SizeInColumns - string_len);
  if (column_offset > max_column_offset)
    column_offset = max_column_offset;

  if (color_index > MAX_COLOR)
    color_index = REGULAR_COLOR;

  attribute =
    set_attributes(W->ColorSet->colors[color_index].foreground,
      W->ColorSet->colors[color_index].background);

  display_string(string, string_len, (W->Row + row_offset),
    (W->Column + column_offset), attribute);
}

// -----------------------------------------------------------------------
BOOLEAN GetWinRowCol(WINDOW * W, int *Row, int *Col)
{
  if (W)
    {
    *Row += W->Row;
    *Col += W->Column;

    if (*Row >= W->Row + W->SizeInRows)
      *Row = (W->Row + W->SizeInRows - 1);

    if (*Col > W->Column + W->SizeInColumns)
      *Col = W->Column + W->SizeInColumns;
    return(TRUE);
    }
  return(FALSE);
}

/* -----------------------------------------------------------------------
/  function:   UCHAR get_color_index(USHORT StringAttr)
/  requires:   (USHORT) StringAttr - can be any of the attributes
/                 STRATTR_REGULAR
/                 STRATTR_REV_VID
/                 STRATTR_HIGHLIGHT
/                 STRATTR_SHADED
/                 STRATTR_ERROR
/              along with other flags not associated with color
/
/  returns:    One of the color indices:
/                 REGULAR_COLOR
/                 REVERSE_COLOR
/                 HIGHLIGHT_COLOR
/                 SHADED_COLOR
/                 ERROR_COLOR
/  side effects:
/
/ ----------------------------------------------------------------------- */
static UCHAR get_color_index(USHORT StringAttr)
{
  UCHAR i;

  for (i=0; i< MAX_COLOR; i++)
    {
    if ((1 << i) & (UCHAR) StringAttr)
      return (UCHAR) (i+1);
    }

  // if got this far, there is no special color condition
  return REGULAR_COLOR;
}

// -----------------------------------------------------------------------
static void strings_to_win(WINDOW * WindowPtr,
                                    unsigned char row_offset,
                                    int string_index, int string_count,
                                    PUSHORT attribute_set)
{
  int            i;
  unsigned char  column_offset = 1;
  char           buffer[85];
  int            line_width;
  UCHAR          color_index = REGULAR_COLOR;

  line_width = (int) (WindowPtr->SizeInColumns - column_offset - 1);

  for (i=0; i < string_count; i++)
    {
    if ((string_index + i) >= WindowPtr->TextStringSetCount)
      break;

    sprintf(buffer, "%-*s", line_width,
      WindowPtr->TextStringSet[string_index + i]);

    // get the color type for this line
    if (attribute_set != NULL)
      color_index = get_color_index(attribute_set[string_index + i]);

    txt_to_win(WindowPtr, buffer, row_offset, column_offset,
      color_index);

    if (++row_offset >= WindowPtr->SizeInRows)
      break;
    }
}

// -----------------------------------------------------------------------
void manage_dynamic_window(WINDOW * W, unsigned char row_offset,
                           int delta_row_count)
{
  manage_window(W, row_offset, W->SizeInRows + delta_row_count,
                NULL, NULL, 0, FALSE, NULL);
}

// -----------------------------------------------------------------------
static void display_choice(WINDOW * W, OPTION * options,
                             int option_index, unsigned char color_index)
{
  txt_to_win(W, options[option_index].text,
    W->SizeInRows, options[option_index].column_offset, color_index);
}

// -----------------------------------------------------------------------
static int offer_choices_in_window(WINDOW * W, char ** choice_strings,
                                   int default_choice)
{
  int            i;
  int            choice_count = 0;
  int            current_choice;
  BOOLEAN        done = FALSE;
  unsigned char  key;
  unsigned char  offset = 0;
  unsigned char  start_offset;
  OPTION         options[MAX_CHOICES];

  while ((choice_count < MAX_CHOICES)
    && (choice_strings[choice_count] != NULL))
    {
    options[choice_count].text = choice_strings[choice_count];
    options[choice_count].text_length = (unsigned char)
      strlen(options[choice_count].text);

    options[choice_count].column_offset = (offset + (unsigned char)1);
    offset += (options[choice_count].text_length + (unsigned char)2);
    choice_count++;
    }
  if (offset < W->SizeInColumns)
    start_offset = ((W->SizeInColumns - offset) / (unsigned char)2);
  else
    start_offset = 0;

  PopupWindow = W;
  PopupChoiceCount = choice_count;
  if (PopupChoiceCount)
    {
    PopupChoiceRow = W->Row + W->SizeInRows - (UCHAR) 1;
    PopupChoiceOptions = options;
    }

  for (i=0; i<choice_count; i++)
    {
    options[i].column_offset += start_offset;
    display_choice(W, options, i, REVERSE_COLOR);
    }

  if ((default_choice < 0) || (default_choice > (choice_count - 1)))
    default_choice = 0;
  current_choice = default_choice;

  do
    {
    display_choice(W, options, current_choice, HIGHLIGHT_COLOR);

    key = get_key_input();

    if (key & KEYS_HIGH_BIT)
      {
      switch (key)
        {
        case KEY_RIGHT:
        case KEY_RIGHT_FAR:
        case KEY_TAB:
          if (current_choice < (choice_count - 1))
            {
            display_choice(W, options, current_choice, REVERSE_COLOR);
            current_choice++;
            }
        break;

        case KEY_LEFT:
        case KEY_LEFT_FAR:
        case KEY_BACK_TAB:
          if (current_choice > 0)
            {
            display_choice(W, options, current_choice, REVERSE_COLOR);
            current_choice--;
            }
        break;

        case KEY_ENTER:
          done = TRUE;
        break;

        case KEY_ESCAPE:
          // escape will return a -1
          current_choice = -1;    // bail out
          done = TRUE;
        break;
        }
      }
    else
      {
      key = (char)toupper(key);

      for (i=0; i<choice_count; i++)
        {
        if (key == (unsigned char)toupper(options[i].text[0]))
          {
          current_choice = i;
          done = TRUE;
          break;
          }
        }
      }
    }
  while (! done);

  PopupChoiceOptions = NULL;
  PopupChoiceCount = 0;

  return(current_choice);
}

// -----------------------------------------------------------------------
void message_pause(void)
{
   unsigned char key;

  do
    {
    key = get_key_input();
    }
  while ((key != KEY_ENTER) && (key != KEY_ESCAPE));
}

// -----------------------------------------------------------------------
char ** start_new_string_set(int * elements)
{
  char ** new_string_set;
  int i;

  if ((new_string_set = malloc(sizeof(char *) *STRING_SET_GRANULES)) != NULL)
    {
    *elements = STRING_SET_GRANULES;
    for(i = 0; i < * elements; i ++)
      new_string_set[ i ] = NULL;
    }
  else
    *elements = 0;

  return(new_string_set);
}

// -----------------------------------------------------------------------
static char ** extend_string_set(char ** current_set, int * elements)
{
  char ** extended_string_set;

  if ((extended_string_set =
    malloc(sizeof(char *) * (*elements + STRING_SET_GRANULES))) != NULL)
    {
    int i;

    memcpy(extended_string_set, current_set, (*elements * sizeof(char *)));
    free(current_set);

    for(i = * elements; i < * elements + STRING_SET_GRANULES; i ++)
      extended_string_set[ i ] = NULL;

    *elements += STRING_SET_GRANULES;
    return(extended_string_set);
    }
  else
    return(current_set);
}

// -----------------------------------------------------------------------
static char ** add_string_to_string_set(char ** current_set, char * string,
                                        int * index, int * elements)
{
  int      string_length = (strlen(string) + 1);
  char **  working_set = current_set;
  char *   new_string;

  if((* index) >= ((* elements) - 1)) // leave a last NULL pointer
    {
    int size_before = *elements;

    working_set = extend_string_set(current_set, elements);
    if (*elements == size_before)
      return(current_set);
    }
  if ((new_string = malloc(sizeof(char) * string_length)) != NULL)
    {
    strcpy(new_string, string);
    working_set[ (*index)++ ] = new_string;
    }
  return(working_set);
}

// -----------------------------------------------------------------------
static void release_string_set(WINDOW * windowPtr)
{
  char ** current_set = windowPtr->TextStringSet;
  int i;

  for (i = 0; i < windowPtr->TextStringSetCount; i ++)
    free(current_set[ i ]);

  free(current_set);
}

// -----------------------------------------------------------------------
BOOLEAN put_up_message_window(char ** message_text, UCHAR coldex, WINDOW ** W)
{
  return put_up_title_message_window(NULL, message_text, NULL,
                                     (UCHAR)screen_rows,
                                     coldex, FALSE, W);
}

// Erase the screen area of a message window if there is one and return
// a NULL pointer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
WINDOW * release_message_window(WINDOW * MessageWindow)
{
  if(MessageWindow)
    {
    CRECT OldClipRect;
    CRECT ScreenArea;

    ScreenArea.ll.x = 0;
    ScreenArea.ll.y = 0;
    ScreenArea.ur.x = screen.LastVDCXY.x;
    ScreenArea.ur.y = screen.LastVDCXY.y;

    CInqClipRectangle(screen_handle, &OldClipRect);
    CSetClipRectangle(screen_handle, ScreenArea);

    close_window(MessageWindow);

    destroy_transient_window(MessageWindow);

    CSetClipRectangle(screen_handle, OldClipRect);
    }

  return 0;
}

// -----------------------------------------------------------------------
BOOLEAN message_window(char ** message_text, unsigned char color_set_index)
{
   return title_message_window(NULL, message_text, NULL, NULL, 0,
     (UCHAR)screen_rows, color_set_index, FALSE, NULL);
}

// -----------------------------------------------------------------------
int choice_window(char ** message_text, char ** choice_text,
   int default_choice, unsigned char color_set_index)
{
   WINDOW * ChoiceWindow;
   int      choice = default_choice;
   CRECT OldClipRect;
   CRECT ScreenArea;

   ScreenArea.ll.x = 0;
   ScreenArea.ll.y = 0;
   ScreenArea.ur.x = screen.LastVDCXY.x;
   ScreenArea.ur.y = screen.LastVDCXY.y;

   CInqClipRectangle(screen_handle, &OldClipRect);
   CSetClipRectangle(screen_handle, ScreenArea);

   ChoiceWindow =
   define_transient_window(0, 0, (UCHAR)screen_rows, (UCHAR)screen_cols, 0);

   if (ChoiceWindow != NULL)
   {
      attach_strings_to_window(ChoiceWindow, message_text);
      autosize_window(ChoiceWindow);
      autocenter_window(ChoiceWindow);

      open_window(ChoiceWindow, color_set_index);

      strings_to_win(ChoiceWindow, 1,
      0, ChoiceWindow->TextStringSetCount, NULL);

      choice =
      offer_choices_in_window(ChoiceWindow, choice_text, default_choice);

      close_window(ChoiceWindow);

      destroy_transient_window(ChoiceWindow);
   }
   CSetClipRectangle(screen_handle, OldClipRect);
   return(choice);
}

// -----------------------------------------------------------------------
int yes_no_choice_window(char ** message_text, int default_choice,
   unsigned char color_set_index)
{
   return(choice_window(message_text, yes_no_choice_text, default_choice,
   color_set_index));
}

// -----------------------------------------------------------------------
static void manage_window(WINDOW * W, UCHAR row_offset,
                          SHORT row_count,    PUSHORT attribute_set,
                          PUCHAR ItemRowCount,USHORT ItemCount,
                          BOOLEAN Pointer,    CharAction ActOnChar)
{
  SHORT TopIndex = 0;
  SHORT OffsetIndex = 0;
  UCHAR key;
  UCHAR centered_column = ((W->SizeInColumns / (char)2) - (char)6);

  BOOLEAN  done = FALSE;
  BOOLEAN  movement = TRUE;
  BOOLEAN  movement_permitted;
  USHORT   ItemIndex = 0;
  SHORT    Count;

  PopupWindow = W;
  PopupChoiceCount = 0;
  PopupChoiceOptions = NULL;

  if (ItemRowCount == NULL)
    {                          // set up to refer to bottom of window
    ItemCount = W->TextStringSetCount;
    ItemIndex = row_count - 1;
    if (ItemIndex >= ItemCount)
      ItemIndex = ItemCount;
    }

  movement_permitted = (W->TextStringSetCount > row_count);
  do
    {
    if (movement)
      {
      movement = FALSE;

      if (Pointer)
        {
        // put arrow into first two spaces of indexed string
        W->TextStringSet[TopIndex + OffsetIndex][0] = '=';
        W->TextStringSet[TopIndex + OffsetIndex][1] = '>';
        }
      strings_to_win(W, row_offset, TopIndex,
        row_count, attribute_set);

      if (Pointer)
        {
        // replace spaces in first two spaces of indexed string
        W->TextStringSet[TopIndex + OffsetIndex][0] = ' ';
        W->TextStringSet[TopIndex + OffsetIndex][1] = ' ';
        }
      if (movement_permitted)
        {
        if (TopIndex == (W->TextStringSetCount - row_count))
          {
          txt_to_win(W, " End  ", W->SizeInRows,
            centered_column, REVERSE_COLOR);
          }
        else
          {
          txt_to_win(W, " More ", W->SizeInRows,
            centered_column, REVERSE_COLOR);
          }
        }
      }

    key = get_key_input();

    switch (key)
      {
      case KEY_UP:
        if (ItemIndex > 0)
          {
          if (ItemRowCount != NULL)
            {
            ItemIndex--;
            if (OffsetIndex == 0)
              TopIndex -= ItemRowCount[ItemIndex];
            else
              OffsetIndex -= ItemRowCount[ItemIndex];
            }
          else
            {
            if (TopIndex != 0)
              {
              TopIndex--;
              ItemIndex = TopIndex + (row_count - 1);
              if (ItemIndex >= ItemCount)
                ItemIndex = ItemCount;
              }
            }
          movement = TRUE;
          }
      break;
      case KEY_DOWN:
        if (ItemIndex < (ItemCount-1))
          {
          if (ItemRowCount != NULL)
            {
            OffsetIndex += ItemRowCount[ItemIndex];

            ItemIndex++;
            // make sure full item shows in window
            while ((OffsetIndex + ItemRowCount[ItemIndex]) >
              row_count)
              {
              TopIndex++;
              OffsetIndex--;
              }
            }
          else
            {
            TopIndex++;
            ItemIndex = TopIndex + (row_count - 1);
            if (ItemIndex >= ItemCount)
              ItemIndex = ItemCount;
            }
          movement = TRUE;
          }
      break;

      case KEY_PG_UP:
        if (movement_permitted)
          {
          if (ItemRowCount != NULL)
            {
            // need to get to the start of item closest to the 
            // page limit without skipping over any lines
            Count = 0;
            while ((Count < row_count) && (ItemIndex > 0))
              {
              ItemIndex--;
              Count += ItemRowCount[ItemIndex];
              }
            // back up to show the full item
            if (Count > row_count)
              {
              Count -= ItemRowCount[ItemIndex];
              ItemIndex++;
              }

            OffsetIndex -= Count;
            while (OffsetIndex < 0)
              {
              TopIndex--;
              OffsetIndex++;
              }
            }
          else
            {
            TopIndex -= row_count;
            if (TopIndex < 0)
              TopIndex = 0;
            ItemIndex = TopIndex + row_count - 1;
            if (ItemIndex >= ItemCount)
              ItemIndex = ItemCount;
            }
          movement = TRUE;
          }
      break;
      case KEY_PG_DN:
        if (movement_permitted && (ItemIndex < (ItemCount - 1)))
          {
          if (ItemRowCount != NULL)
            {
            // need to get to the start of item closest to the page 
            // limit without skipping over any lines
            Count = 0;
            while ((Count < row_count) &&
                   ((TopIndex + Count) < (W->TextStringSetCount -
              ItemRowCount[ItemIndex])))
              {
              Count += ItemRowCount[ItemIndex];
              ItemIndex++;
              }
            if (Count > row_count)    // back up to show the full item
              {
              ItemIndex--;
              Count -= ItemRowCount[ItemIndex];
              }

            OffsetIndex += Count;
            while ((OffsetIndex + ItemRowCount[ItemIndex])> row_count)
              {
              TopIndex++;
              OffsetIndex--;
              }
            }
          else
            {
            TopIndex += row_count;
            if (TopIndex > (W->TextStringSetCount - row_count))
              TopIndex = (W->TextStringSetCount - row_count);
            ItemIndex = TopIndex + (row_count - 1);
            if (ItemIndex >= ItemCount)
              ItemIndex = ItemCount;
              }
            movement = TRUE;
            }
      break;

      case KEY_HOME:
        if (TopIndex != 0)
          {
          TopIndex = 0;
          OffsetIndex = 0;
          ItemIndex = row_count - 1;
          if (ItemIndex >= ItemCount)
            ItemIndex = ItemCount;
          movement = TRUE;
          }
      break;
      case KEY_END:
        if (movement_permitted)
          {
          if (ItemRowCount != NULL)
            {
            // need to get o the start of item closest to the page limit
            // without skipping over any lines
            Count = 0;
            while (ItemIndex < (ItemCount-1))
              {
              Count += ItemRowCount[ItemIndex];
              ItemIndex++;
              }
            OffsetIndex += Count;
            while ((OffsetIndex + ItemRowCount[ItemIndex])> row_count)
              {
              TopIndex++;
              OffsetIndex--;
              }
            }
          else
            {
            TopIndex = (W->TextStringSetCount - row_count);
            ItemIndex = TopIndex + (row_count - 1);
            if (ItemIndex >= ItemCount)
              ItemIndex = ItemCount;
            }
          movement = TRUE;
          }
      break;

      default:
        if (ActOnChar != NULL)
          {
          switch ((*ActOnChar)(W, row_offset, row_count,
            attribute_set, ItemRowCount, ItemCount, ItemIndex,
            Pointer, key))
            {
            case KEY_EXCEPTION:
              done = TRUE;
            break;
            case 1:
              movement = TRUE;
            break;
            case 0:
          break;
          }
        }
      else                         // compatibility with earlier action
        if ((key == KEY_ESCAPE) || (key == KEY_ENTER || key == SPACE))
        done = TRUE;
      }
    }
  while (! done);
}

// -----------------------------------------------------------------------
void title_into_window(WINDOW * W, CHAR *title, UCHAR color_index)
{
  UCHAR          column_offset;
  char           buffer[85];
  int            line_width;

  line_width = (int) (W->SizeInColumns - 1);

  // center title in window

  column_offset = (UCHAR) ((line_width - strlen(title)) / 2);
  sprintf(buffer, "%s", title);

  txt_to_win(W, buffer, 0, column_offset, color_index);
}

// -----------------------------------------------------------------------
static BOOLEAN put_up_title_message_window(PCHAR title_text,
                                            PCHAR * message_text,
                                            PUSHORT attribute_set,
                                            UCHAR MaxRows,
                                            UCHAR color_set_index,
                                            BOOLEAN Dynamic,
                                            WINDOW * *MessageWindow)
{
  *MessageWindow =
    define_transient_window(0, 0, MaxRows, (UCHAR)screen_cols, 0);

  if (*MessageWindow != NULL)
    {
    attach_strings_to_window(*MessageWindow, message_text);
    autosize_window(*MessageWindow);
    autocenter_window(*MessageWindow);

    open_window(*MessageWindow, color_set_index);

    if (title_text != NULL)
      title_into_window(*MessageWindow, title_text, HIGHLIGHT_COLOR);

    if (! Dynamic)    // will redraw in manage_dynamic_indexed_window
      strings_to_win(*MessageWindow, 1,
        0, (*MessageWindow)->SizeInRows - 2, attribute_set);

    return TRUE;
    }
  return FALSE;
}

// -----------------------------------------------------------------------
BOOLEAN title_message_window(PCHAR title_text, PCHAR *message_text,
                             PUSHORT attribute_set, PUCHAR ItemRowCount,
                             USHORT ItemCount, UCHAR MaxRows,
                             UCHAR color_set_index, BOOLEAN Pointer,
                             CharAction ActOnChar)
{
  WINDOW *    MessageWindow;
  CRECT OldClipRect;
  BOOLEAN ReturnVal = FALSE;
  CRECT ScreenArea;

  ScreenArea.ll.x = 0;
  ScreenArea.ll.y = 0;
  ScreenArea.ur.x = screen.LastVDCXY.x;
  ScreenArea.ur.y = screen.LastVDCXY.y;

  CInqClipRectangle(screen_handle, &OldClipRect);
  CSetClipRectangle(screen_handle, ScreenArea);

  if (put_up_title_message_window(title_text, message_text,
    attribute_set, MaxRows, color_set_index, TRUE, &MessageWindow))
    {
    manage_window(MessageWindow, 1,
      (int) (MessageWindow->SizeInRows - 2),
      attribute_set, ItemRowCount, ItemCount, Pointer,
      ActOnChar);

    MessageWindow = release_message_window(MessageWindow);
    ReturnVal = TRUE;
    CSetClipRectangle(screen_handle, OldClipRect);
    }
  else
    CSetClipRectangle(screen_handle, OldClipRect);

  return ReturnVal;
}

// allocate a window for PopupWindow. return TRUE if alloc OK.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN popupWindowBegin(void)
{
  PopupWindow = malloc(sizeof(WINDOW));

  if(! PopupWindow)
    return FALSE;

  return TRUE;
}

// initialize PopupWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void popupWindowSetup(unsigned char row,     unsigned char column,
                       unsigned char numRows, unsigned char numColumns)
{
  PopupWindow->Row = row;
  PopupWindow->Column = column;
  PopupWindow->SizeInRows = numRows;
  PopupWindow->SizeInColumns = numColumns;
}

// deallocate PopupWindow
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void popupWindowEnd(void)
{
  free(PopupWindow);
  PopupWindow = NULL;
}

BOOLEAN get_message_from_file(WINDOW * WindowPtr, const char * file_spec,
                              int message_index, va_list insert_args)
{
  FILE *   fileptr;
  BOOLEAN  message_found = FALSE;
  char **  message_string_array;
  int      string_array_elements;
  int      string_count = 0;
  int      string_length;

  message_string_array = start_new_string_set(&string_array_elements);
  if (string_array_elements == 0)
    {
    attach_strings_to_window( WindowPtr, NULL ) ;
    return(FALSE);
    }

  if( ( fileptr = fopen( file_spec, "r" ) ) != NULL)
    {
    char   file_line[128];
    char * marker;

    while ( fgets(file_line, 128, fileptr) != NULL)
      {
      if ((marker = strstr(file_line, "@@@")) != NULL)
        {
        if (message_found)
          break;
        else
          {
          if (atoi(&marker[3]) == message_index)
            message_found = TRUE;
          }
        }
      else if (message_found)
        {
        string_length = strlen(file_line);
        if (file_line[ (string_length - 1) ] == '\n')
          file_line[ (string_length - 1) ] = (char) 0; /* zap \n at end */

        if (strchr(file_line, '%') != NULL)
          {
          char format_string[128];

          vsprintf(format_string, file_line, insert_args);

          message_string_array =
            add_string_to_string_set(message_string_array,
            format_string, &string_count, &string_array_elements);
          }
        else
          {
          message_string_array =
            add_string_to_string_set(message_string_array,
            file_line, &string_count, &string_array_elements);
          }
        }
      }
    fclose(fileptr);
    }

  if (message_found && (string_count > 0))
    attach_strings_to_window( WindowPtr, message_string_array ) ;
  else
    attach_strings_to_window( WindowPtr, NULL ) ;

  return message_found ;
}

/*************************************************************************/
  
BOOLEAN va_file_message_window(const char * file_spec, int message_index,
                               unsigned char max_rows,
                               unsigned char color_set_index,
                               va_list insert_args)
{
  WINDOW * FileWindow;
  BOOLEAN  file_success = FALSE;

  if ((max_rows < 4) || (max_rows > (UCHAR)screen_rows))
    max_rows = (UCHAR)screen_rows;

  FileWindow =
    define_transient_window(0, 0, max_rows, (UCHAR)screen_cols, (char)0);

  if (FileWindow != NULL)
    {
    if (message_index == 0)
      {
      char *   problem_message[2];
      char *   text = "No Message Available";

      problem_message[0] = text;
      problem_message[1] = NULL;

      attach_strings_to_window(FileWindow, problem_message);
      }
    else if (get_message_from_file(FileWindow, file_spec, message_index,
      insert_args))
      {
      file_success = TRUE;
      }
    else
      {
      char *   problem_message[2];
      char     text[40];

      sprintf(text, "Message Number %d", message_index);
      problem_message[0] = text;
      problem_message[1] = NULL;

      attach_strings_to_window(FileWindow, problem_message);
      }

    autosize_window(FileWindow);
    autocenter_window(FileWindow);

    open_window(FileWindow, color_set_index);

    manage_dynamic_window(FileWindow, 1, -2 ) ;

    close_window(FileWindow);

    if (file_success)
      release_string_set( FileWindow ) ;

    destroy_transient_window(FileWindow);
    return(TRUE);
    }
  else
    return(FALSE);
}

BOOLEAN file_message_window(const char * file_spec, int message_index,
                            UCHAR max_rows, UCHAR color_set_index, ...)
{
  va_list insert_args;

  va_start(insert_args, color_set_index);
  return (va_file_message_window(file_spec, message_index, max_rows,
                                 color_set_index, insert_args));
}
