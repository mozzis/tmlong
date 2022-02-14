/* colors.h */

#ifndef COLORS_INCLUDED
#define COLORS_INCLUDED

enum colordex 
  {REGULAR_COLOR=0, REVERSE_COLOR, HIGHLIGHT_COLOR, SHADED_COLOR, MAX_COLOR};

enum color_hues {
  CBLACK = 0,    /*  0 */
  CBRT_WHITE,    /*  1 */           
  CGREEN,        /*  2 */
  CRED,          /*  3 */
  CCYAN,         /*  4 */
  CPURPLE,       /*  5 */
  CBROWN,        /*  6 */
  CWHITE,        /*  7 */           
  CBRT_BLUE,     /*  8 */
  CBRT_GREEN,    /*  9 */
  CBRT_RED,      /* 10 */
  CBRT_CYAN,     /* 11 */
  CBRT_PURPLE,   /* 12 */
  CBRT_YELLOW,   /* 13 */
  CBLUE,         /* 14 */             
  CBRT_ORANGE,   /* 15 */
};

enum { COLORS_DEFAULT=0, COLORS_MESSAGE, COLORS_ERROR, COLORS_MENU } ;

typedef struct colorset_CI_pair {
   enum color_hues foreground;
   enum color_hues background;
} COLOR_PAIR;

struct colorset {
   COLOR_PAIR     regular;
   COLOR_PAIR     reverse;
   COLOR_PAIR     highlight;
   COLOR_PAIR     shaded;
   /* and others as needed */
} ;

typedef struct colorset COLOR_SET;

#endif
