/* keymenu.h */

#ifndef KEYMEMU_INCLUDED
#define KEYMEMU_INCLUDED

#define     MENU_IS_SUBMENU            0x01
#define     MENU_                      0x02
#define     MENUITEM_CALLS_SUBMENU     0x01
#define     MENUITEM_CALLS_FUNCTION    0x02
#define     MENUITEM_INACTIVE          0x04
#define     MENUITEM_                  0x08
#define     MENUITEM_CALLS_FORM        0x10
#define     MENUITEM_CALLS_FORMTABLE   0x20

#pragma  pack(1)

typedef struct menudef
{
   unsigned char        Control;
   char                 ColorSetIndex;
   char                 ItemCount;
   struct menuitemdef * ItemList;
} KMENU;

/* structure to define a Menu Item */

typedef struct menuitemdef
{
   char *            Text;
   char              SelectCharOffset;
   char              Column;
   char              TextLen;
   void *            SubMenu;               
   int               (*Action)(void);
   unsigned char     Control;
} KMENUITEM;

/* structure to define a Menu */

typedef struct menu_context
{
   char                    Row;
   char                    Column;
   char                    SizeInRows;
   char                    ItemIndex;
   KMENU *                 ActiveMENU;
   COLOR_SET *             MenuColorSet;
   struct menu_context *   PriorContext;
} KMENUCONTEXT;

#pragma  pack()

extern KMENUITEM FKeyItems[];
extern KMENU     FKey;

int ShowFKeys(void);
int RunFKeyForm(void);

#endif
