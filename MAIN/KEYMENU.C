/* keymenu.c */

#include <string.h>
#include <stdio.h>
#include <cgibind.h>

#include "primtype.h"
#include "colors.h"
#include "keys.h"
#include "keymenu.h"
#include "device.h"
#include "cwindows.h"
#include "userin.h"
#include "plotmenu.h"
#include "timemenu.h"

extern int GenRealData(void); /* temp def for testing */

#define FKEY_ROW_SIZE         2

USHORT CurrentShiftMode = 0;        
static BOOLEAN Done = FALSE;
static int MenuPlot(void);
static int TryExit(void);

KMENUCONTEXT MenuFocus;

KMENUITEM FKeyItems[] = {
      
   {  "F1 Calibrate",                     // Text;
      0,                                  // SelectCharOffset;
      0,                                  // Column;
      0,                                  // TextLen;
      NULL,                               // SubMenu;               
      NULL,                               // (*Action)(unsigned int);
      MENUITEM_CALLS_FUNCTION},           // Control;
   {  "F2 Linearity",
      0,
      0,
      0,
      NULL,
      GenRealData,
      MENUITEM_CALLS_FUNCTION },
   {  "F3 Do Plot  ",
      0,
      0,
      0,
      NULL,
      MenuPlot,
      MENUITEM_CALLS_FUNCTION },
   {  "F4 Set Scales",
      0,
      0,
      0,
      NULL,
      DoPlotSetupMenu,
      MENUITEM_CALLS_FUNCTION },
   {  "F5 Run Time ",
      0,
      0,
      0,
      NULL,
      DoTimeMenu,
      MENUITEM_CALLS_FUNCTION },
   {  "F6 Exit",
      0,
      0,
      0,
      NULL,
      TryExit,
      MENUITEM_CALLS_FUNCTION },
};

KMENU FKey={0, COLORS_MENU, sizeof(FKeyItems) / sizeof(KMENUITEM), FKeyItems};

/* -----------------------------------------------------------------------
/
/  function:
/  requires:   (int) use_item - the index of the menuitem in the menu.
/  returns:    (void)
/  side effects:
/
/ ----------------------------------------------------------------------- */

void draw_menuitem(KMENUITEM * ThisItem, UCHAR attribute,
                   UCHAR select_attribute)
{
  char text_column;

  text_column = (MenuFocus.Column + ThisItem->Column);

  emit(SPACE, MenuFocus.Row, text_column++, attribute);

  display_string(ThisItem->Text, ThisItem->TextLen, MenuFocus.Row,
    text_column, attribute);

  if (! (ThisItem->Control & MENUITEM_INACTIVE))
    {
    // don't draw any highlighted char if it's select index is -1
    if (ThisItem->SelectCharOffset != (char)-1 )
      emit(ThisItem->Text[ThisItem->SelectCharOffset], MenuFocus.Row,
        (text_column + ThisItem->SelectCharOffset), select_attribute);
    }
  emit(SPACE, MenuFocus.Row, (text_column+ThisItem->TextLen), attribute);
}

// -----------------------------------------------------------------------

void shade_menuitem(char index)
{
  KMENUITEM * ThisItem = &MenuFocus.ActiveMENU->ItemList[index];
  unsigned char attribute;

  attribute = set_attributes(MenuFocus.MenuColorSet->shaded.foreground,
                             MenuFocus.MenuColorSet->shaded.background);

  draw_menuitem(ThisItem, attribute, attribute);
}

// -----------------------------------------------------------------------

void highlight_menuitem(char index)
{
  KMENUITEM * ThisItem = &MenuFocus.ActiveMENU->ItemList[index];
  unsigned char attribute;

  attribute = set_attributes(MenuFocus.MenuColorSet->reverse.foreground,
                             MenuFocus.MenuColorSet->reverse.background);

  draw_menuitem(ThisItem, attribute, attribute);
}

// -----------------------------------------------------------------------

void unhighlight_menuitem(char index)
{
  KMENUITEM *    ThisItem = &MenuFocus.ActiveMENU->ItemList[index];
  unsigned char attribute;
  unsigned char select_attribute;

  attribute = set_attributes(MenuFocus.MenuColorSet->regular.foreground,
                             MenuFocus.MenuColorSet->regular.background);

  select_attribute =
    set_attributes(MenuFocus.MenuColorSet->highlight.foreground,
                   MenuFocus.MenuColorSet->highlight.background);

  draw_menuitem(ThisItem, attribute, select_attribute);
}


SHORT ShowFKeys(void)
{
  SHORT       i;
  UCHAR       column_offset = 0, attribute;
  KMENUITEM *  ThisItem;
  CRECT       OldClipRect;
  CFILLREPR   FillRep;
  int         shadeItem;

  CInqFillRepr(screen_handle, &FillRep);

  CInqClipRectangle(screen_handle, &OldClipRect);
  setClipRectToFullScreen();

  MenuFocus.Row          = (CHAR)(screen_rows - FKEY_ROW_SIZE);
  MenuFocus.Column       = 0;
  MenuFocus.SizeInRows   = FKEY_ROW_SIZE ;
  MenuFocus.ItemIndex    = 0 ;
  MenuFocus.ActiveMENU   = &FKey;
  MenuFocus.MenuColorSet = &ColorSets[COLORS_MENU];
  MenuFocus.PriorContext = NULL ;

  attribute = 
    set_attributes(MenuFocus.MenuColorSet->regular.foreground,
                   MenuFocus.MenuColorSet->regular.background);

  erase_screen_area(MenuFocus.Row, MenuFocus.Column, MenuFocus.SizeInRows,
                    (UCHAR)(screen_cols/*+1*/), attribute, FALSE);

  for (i=0; i < MenuFocus.ActiveMENU->ItemCount; i++)
    {
    ThisItem = &MenuFocus.ActiveMENU->ItemList[i];

    if (ThisItem->Text != NULL)
      {
      if (ThisItem->TextLen == 0)
        ThisItem->TextLen = (char) strlen(ThisItem->Text);

      /* Wrap To next line */    
      if ((column_offset + ThisItem->TextLen) >= (UCHAR)screen_cols)
        {
        column_offset = 0;
        MenuFocus.Column = 0;
        MenuFocus.Row++;
        }

      ThisItem->Column = (unsigned char) column_offset;

      shadeItem = ThisItem->Control & MENUITEM_INACTIVE;

      if (shadeItem)
        shade_menuitem((unsigned char) i);
      else
        {
        SHORT j = 0;

        unhighlight_menuitem((unsigned char)i);

        /* highlight select char (Function key), */
        /* highlights to first space */
        do
          {
          emit(ThisItem->Text[j], MenuFocus.Row, column_offset + j + 1,
               set_attributes(MenuFocus.MenuColorSet->highlight.foreground,
                              MenuFocus.MenuColorSet->highlight.background));
          j++;
          }
        while (ThisItem->Text[j] != ' ');
        }
      }
    column_offset += strlen(ThisItem->Text) + 2; /* not Column Inc */
    }

  CSetClipRectangle(screen_handle, OldClipRect);
  CSetFillRepr(screen_handle, &FillRep);

  return FALSE;
}

char *PlotterChoices[] = { "Printer", "Plotter", "Screen", NULL };
char *OutputMessage[] =  { " ",
                           "Please choose the output device desired:",
                           " ",
                           NULL };
int MenuPlot(void)
{
  int choice = choice_window(OutputMessage, PlotterChoices, 1, COLORS_MESSAGE);
  if (choice != -1)
    {
    return (plotDataToDevice(PlotterChoices[choice]));
    }
  else
     return(0);
}

char *QuitMessage[] =  { " ",
                         "Ready to Exit and save data?",
                         " ",
                         NULL };
int TryExit(void)
{
  int choice = yes_no_choice_window(QuitMessage, 0, COLORS_MESSAGE);
  if (choice == 1)
    Done = TRUE;
  return(0);
}

int RunFKeyForm(void)
{
  int inkey;
  SHORT      ChoiceIndex;
  KMENUITEM * Choice;

  do
    {
    inkey = get_key_input();
    switch (inkey)
      {
      case KEY_F1:
      case KEY_F2:
      case KEY_F3:
      case KEY_F4:
      case KEY_F5:
      case KEY_F6:
      case KEY_F7:
        ChoiceIndex  = inkey - KEY_F1 ;
        Choice       = & (FKeyItems[ChoiceIndex]);
        if((!(Choice->Control & MENUITEM_INACTIVE))    && /* Active? */
           (Choice->Control & MENUITEM_CALLS_FUNCTION) && /* Right Type? */
           (Choice->Action))                              /* Has address? */
          {
          (*(Choice->Action))();
          }
      break;
      default:
        break;
      }
    }
  while (!Done);

  return(FALSE);
}
