// FILE : cwindows.h

#ifndef CWINDOWS_INCLUDED
#define CWINDOWS_INCLUDED

#ifndef __STDARG_H
#include <stdarg.h>
#endif

extern char * BusyWorking[];
extern char * BusyWorkingEsc[];
extern char * DataFileOverwritePrompt[];

typedef struct wind WINDOW;

void autocenter_window(WINDOW * WindowPtr);
WINDOW * define_transient_window(UCHAR Row, UCHAR Column, UCHAR SzRow,
                                 UCHAR SzCol, USHORT Attrib);
WINDOW * destroy_transient_window(WINDOW * WindowPtr);
void ReleaseAllWindows(void);
void open_window(WINDOW * WindowPtr, UCHAR color_set_index);
void close_window(WINDOW * WindowPtr);
unsigned char get_window_attribute(WINDOW * W, UCHAR color_index);
void message_pause(void);
BOOLEAN put_up_message_window(char ** message, UCHAR coldex, WINDOW ** W);
WINDOW * release_message_window(WINDOW * MessageWindow);
BOOLEAN message_window(char ** message_text, UCHAR color_set_index);
int choice_window(char ** msg, char ** choice_text, int deflt, UCHAR coldex);
int yes_no_choice_window(char **message_text, int deflt, UCHAR coldex);
void manage_dynamic_window(WINDOW * W, UCHAR row_offset, int delta_row_count);
void title_into_window(WINDOW * W, CHAR *title, UCHAR color_index);
void txt_to_win(WINDOW * W, char * string, UCHAR row_offset,
                UCHAR column_offset, UCHAR color_index);
BOOLEAN GetWinRowCol(WINDOW * W, int *Row, int *Col);

// functions to manage the popup window PopupWindow

BOOLEAN popupWindowBegin(void);
void popupWindowSetup(UCHAR row,UCHAR column,UCHAR numRows, UCHAR numColumns);
void popupWindowEnd(void);

BOOLEAN file_message_window(const char * file_spec, int message_index,
                            unsigned char max_rows, 
                            unsigned char color_set_index, ...);

BOOLEAN va_file_message_window(const char * file_spec, int message_index,
                               unsigned char max_rows,
                               unsigned char color_set_index,
                               va_list insert_args);


#endif  //  CWINDOWS_INCLUDED

