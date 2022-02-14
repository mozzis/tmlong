/* -----------------------------------------------------------------------
/
/  userin.c
/
/ ----------------------------------------------------------------------- */
  
#include <dos.h>
#include <conio.h>
#include <bios.h>
  

#include "primtype.h"
#include "keys.h"
#include "userin.h"

#define  BIOS_KEYBOARD_INT    0x16
#define  GET_KEY_AND_SCAN        0
  
#define SC_F1               59
#define SC_F2               60
#define SC_F3               61
#define SC_F4               62
#define SC_F5               63
#define SC_F6               64
#define SC_F7               65
#define SC_F8               66
#define SC_F9               67
#define SC_F10              68
#define SC_LEFT_ARROW       75
#define SC_RIGHT_ARROW      77
#define SC_UP_ARROW         72
#define SC_DOWN_ARROW       80
#define SC_HOME             71
#define SC_END              79
#define SC_PG_UP            73
#define SC_PG_DN            81
#define SC_BS               14
#define SC_DEL              83
#define SC_INS              82
#define SC_ESC               1
#define SC_KEYPAD_MINUS     74
#define SC_KEYPAD_PLUS      78
#define SC_CTRL_RTARR      116
#define SC_CTRL_LFARR      115
#define SC_CTRL_PGDN       118
#define SC_CTRL_PGUP       132
#define SC_CTRL_HOME       119
#define SC_CTRL_END        117
#define SC_CTRL_PRTSC      114
#define SC_ENTER            28
#define SC_TAB              15   /* ASCII = 9, shift-tab ASCII = 0 */
#define SC_
  
  
#define SC_SHIFT_F1         84
#define SC_SHIFT_F2         85
#define SC_SHIFT_F3         86
#define SC_SHIFT_F4         87
#define SC_SHIFT_F5         88
#define SC_SHIFT_F6         89
#define SC_SHIFT_F7         90
#define SC_SHIFT_F8         91
#define SC_SHIFT_F9         92
#define SC_SHIFT_F10        93
  
#define SC_CTRL_F1          94
#define SC_CTRL_F2          95
#define SC_CTRL_F3          96
#define SC_CTRL_F4          97
#define SC_CTRL_F5          98
#define SC_CTRL_F6          99
#define SC_CTRL_F7         100
#define SC_CTRL_F8         101
#define SC_CTRL_F9         102
#define SC_CTRL_F10        103
  
#define SC_ALT_F1          104
#define SC_ALT_F2          105
#define SC_ALT_F3          106
#define SC_ALT_F4          107
#define SC_ALT_F5          108
#define SC_ALT_F6          109
#define SC_ALT_F7          110
#define SC_ALT_F8          111
#define SC_ALT_F9          112
#define SC_ALT_F10         113
  
  
  
#define SC_A                30
#define SC_B                48
#define SC_C                46
#define SC_D                32
#define SC_E                18
#define SC_F                33
#define SC_G                34
#define SC_H                35
#define SC_I                23
#define SC_J                36
#define SC_K                37
#define SC_L                38
#define SC_M                50
#define SC_N                49
#define SC_O                24
#define SC_P                25
#define SC_Q                16
#define SC_R                19
#define SC_S                31
#define SC_T                20
#define SC_U                22
#define SC_V                47
#define SC_W                17
#define SC_X                45
#define SC_Y                21
#define SC_Z                44
  
  
#pragma pack(1)
  
struct key_map_pair {
   unsigned char     input_key;
   unsigned char     key_code_out;
};
  
static struct key_map_pair  AsciiPrefilterKeyMap[] = {
  
  { SC_ENTER, KEY_ENTER },
  { SC_ESC, KEY_ESCAPE },
  { SC_TAB, KEY_TAB },
  { SC_UP_ARROW, KEY_UP },
  { SC_DOWN_ARROW, KEY_DOWN },
  { SC_LEFT_ARROW, KEY_LEFT },
  { SC_RIGHT_ARROW, KEY_RIGHT },
  { SC_HOME, KEY_HOME },
  { SC_END, KEY_END },
  { SC_PG_UP, KEY_PG_UP },
  { SC_PG_DN, KEY_PG_DN },
  { SC_INS, KEY_INSERT },
  { SC_DEL, KEY_DELETE },
  { SC_KEYPAD_MINUS, KEY_MINUS },
  { SC_KEYPAD_PLUS, KEY_PLUS },
  { '\0', '\0' }                
};
  
  
static struct key_map_pair  NoAsciiKeyMap[] = {
  
  { SC_TAB, KEY_BACK_TAB },
  { SC_UP_ARROW, KEY_UP },
  { SC_DOWN_ARROW, KEY_DOWN },
  { SC_CTRL_RTARR, KEY_RIGHT_FAR },
  { SC_CTRL_LFARR, KEY_LEFT_FAR },
  { SC_LEFT_ARROW, KEY_LEFT },
  { SC_RIGHT_ARROW, KEY_RIGHT },
  { SC_HOME, KEY_HOME },
  { SC_END, KEY_END },
  { SC_CTRL_HOME, KEY_HOME_FAR },
  { SC_CTRL_END, KEY_END_FAR },
  { SC_PG_UP, KEY_PG_UP },
  { SC_PG_DN, KEY_PG_DN },
  { SC_CTRL_PGUP, KEY_PG_UP_FAR },
  { SC_CTRL_PGDN, KEY_PG_DN_FAR },
  { SC_INS, KEY_INSERT },
  { SC_DEL, KEY_DELETE },
  { SC_F1, KEY_F1 },
  { SC_F2, KEY_F2 },
  { SC_F3, KEY_F3 },
  { SC_F4, KEY_F4 },
  { SC_F5, KEY_F5 },
  { SC_F6, KEY_F6 },
  { SC_F7, KEY_F7 },
  { SC_F8, KEY_F8 },
  { SC_F9, KEY_F9 },
  { SC_F10, KEY_F10 },
  { SC_SHIFT_F1, KEY_F11 },
  { SC_SHIFT_F2, KEY_F12 },
  { SC_SHIFT_F3, KEY_F13 },
  { SC_SHIFT_F4, KEY_F14 },
  { SC_SHIFT_F5, KEY_F15 },
  { SC_SHIFT_F6, KEY_F16 },
  { SC_SHIFT_F7, KEY_F17 },
  { SC_SHIFT_F8, KEY_F18 },
  { SC_SHIFT_F9, KEY_F19 },
  { SC_SHIFT_F10, KEY_F20 },
  { SC_CTRL_F1, KEY_F21 },
  { SC_CTRL_F2, KEY_F22 },
  { SC_CTRL_F3, KEY_F23 },
  { SC_CTRL_F4, KEY_F24 },
  { SC_CTRL_F5, KEY_F25 },
  { SC_CTRL_F6, KEY_F26 },
  { SC_CTRL_F7, KEY_F27 },
  { SC_CTRL_F8, KEY_F28 },
  { SC_CTRL_F9, KEY_F29 },
  { SC_CTRL_F10, KEY_F30 },
  { SC_ALT_F1, KEY_F31 },
  { SC_ALT_F2, KEY_F32 },
  { SC_ALT_F3, KEY_F33 },
  { SC_ALT_F4, KEY_F34 },
  { SC_ALT_F5, KEY_F35 },
  { SC_ALT_F6, KEY_F36 },
  { SC_ALT_F7, KEY_F37 },
  { SC_ALT_F8, KEY_F38 },
  { SC_ALT_F9, KEY_F39 },
  { SC_ALT_F10, KEY_F40 },
  { SC_A, KEY_ALT_A },
  { SC_B, KEY_ALT_B },
  { SC_C, KEY_ALT_C },
  { SC_D, KEY_ALT_D },
  { SC_E, KEY_ALT_E },
  { SC_F, KEY_ALT_F },
  { SC_G, KEY_ALT_G },
  { SC_H, KEY_ALT_H },
  { SC_I, KEY_ALT_I },
  { SC_J, KEY_ALT_J },
  { SC_K, KEY_ALT_K },
  { SC_L, KEY_ALT_L },
  { SC_M, KEY_ALT_M },
  { SC_N, KEY_ALT_N },
  { SC_O, KEY_ALT_O },
  { SC_P, KEY_ALT_P },
  { SC_Q, KEY_ALT_Q },
  { SC_R, KEY_ALT_R },
  { SC_S, KEY_ALT_S },
  { SC_T, KEY_ALT_T },
  { SC_U, KEY_ALT_U },
  { SC_V, KEY_ALT_V },
  { SC_W, KEY_ALT_W },
  { SC_X, KEY_ALT_X },
  { SC_Y, KEY_ALT_Y },
  { SC_Z, KEY_ALT_Z },
  { '\0', '\0' }          
};
  
#pragma pack()
  
  
/* -----------------------------------------------------------------------
/
/  int raw_keyboard_input()
/
/  function:   calls the BIOS keyboard functions interrupt to grab
/              a ASCII/scan_code combination from the keyboard input
/              ring buffer.  This is used whenever you are looking
/              for keys to be struck that may not yield an ASCII value.
/  requires:   (void)
/  returns:    (int) - scan_code in the high order byte, ASCII in the
/              low order byte.
/  side effects:
/
/ ----------------------------------------------------------------------- */
  
int raw_keyboard_input(void)
{
#ifdef __TURBOC__
    return bioskey(0);
#else
    return _bios_keybrd(_KEYBRD_READ);
#endif
}
  
unsigned char get_key_input()
{
  int            raw_key;
  unsigned char  key;
  unsigned char  scan_code;
  unsigned char  translated_key;

  while ( ! kbhit() )
    {
    /* idle tasks go here */
    ;
    }

  raw_key = raw_keyboard_input();
  scan_code = (unsigned char) ((raw_key >> 8) & 255);
  key = (unsigned char) (raw_key & 255);

  if (scan_code == SC_BS)
    {
    if (key == ASCII_BACKSPACE)
      translated_key = KEY_BACKSPACE;
    else                /* control-backspace case */
      translated_key = KEY_DELETE_FAR;
    }
  else
    {
    struct key_map_pair *   KeyMap;

    translated_key = key;

    if (key == (unsigned char) 0)
      KeyMap = NoAsciiKeyMap;
    else
      KeyMap = AsciiPrefilterKeyMap;

    while (KeyMap->input_key != (unsigned char) 0)
      {
      if (scan_code == KeyMap->input_key)
        {
        translated_key = KeyMap->key_code_out;
        break;
        }
      KeyMap++;
      }
    }
  return (translated_key);
}
  
/* -----------------------------------------------------------------------
/
/
/
/  function:   int DumpKeyBuffer( unsigned ScanKey )
/  requires:   ScanKey - the scan code and key value (as returned from
/                         raw_keyboard_input ) which is to be deleted
/                         from the front of the buffer.  Deletes all
/                         characters if set to 0;
/  returns:    (int) scancode and key value for the first different
/                    character or 0 if no keys left in buffer
/  side effects:
/
/                 added 12/8/89 DAI
/ ----------------------------------------------------------------------- */
  
int DumpKeyBuffer(unsigned ScanKey)
{
  int Key;

  do
    {
    /* check for key ready */
      
#ifdef __TURBOC__ 
    Key = bioskey(_KEYBRD_READY);
#else
    Key = _bios_keybrd(_KEYBRD_READ);
#endif

    if (ScanKey)           /* if don't want to erase all keys */
      {  /* if Key doesn't match return the value of the next key */
      if ( Key != (SHORT)((USHORT) ScanKey ))
        return Key;
      }
    if (Key)  /* pull the next key out of the buffer */
      Key = raw_keyboard_input();
    }
  while ( Key );

  return Key;
}

// return TRUE iff ESCAPE key has been pressed
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOOLEAN keyboardEscape( void )
{
  return kbhit() && getch() == ESCAPE ;
}

