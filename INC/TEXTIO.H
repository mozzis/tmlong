/* textio.h */
/* part of admsr program */
/* provide RequestString and Outchar functions */
/* al a Basic's INPUT and LOCATE : PRINT stuff */

void cursor_out(int x, int y);
void outchar(int x, int y, char * Outstring);
void fmt_outchar(int x, int y, const char * FmtStr, ...);
void out_cur_char(int x, int y, char * Outstring);
void erase_line(int x, int y, int len);
void eeol(int x, int y, char * Outstring);
int RequestInput(int x, int y, char * Instring, int maxchar);
void ScreenSetup(char * driverpath);
void EndScreen(void);
