
#define More	   0
#define TimetoExit 1

#define BOX	   0
#define POLY	   1
#define TEXT	   2

#define RASTER_X Displayout.LastXY.x
#define RASTER_Y Displayout.LastXY.y
#define BUFFER_SIZE 64000
#define MAX_FONTS Displayout.TextFonts

#define MIN_X 0
#define MIN_Y 0
#define MAX_X Displayout.LastVDCXY.x
#define MAX_Y Displayout.LastVDCXY.y
#define THREEQUARTERX ((Displayout.LastVDCXY.x / 4 ) * 3 )
#define THREEQUARTERY ((Displayout.LastVDCXY.y / 4 ) * 3 )
#define HALF_X Displayout.LastVDCXY.x / 2
#define HALF_Y Displayout.LastVDCXY.y / 2
#define THIRD_X Displayout.LastVDCXY.x / 3
#define THIRD_Y Displayout.LastVDCXY.y / 3
#define QUARTERX (Displayout.LastVDCXY.x / 4)
#define QUARTERY (Displayout.LastVDCXY.y / 4)
#define EIGHTHX (Displayout.LastVDCXY.x / 8)
#define EIGHTHY (Displayout.LastVDCXY.y / 8)

#define HOLLOW	     0
#define SOLID	     1

#define NOTPRESSED   0
#define PRESSED      1

#define DONOTCARE    0
#define CONTINUOUS   0
#define MINHGT	     0
#define NOERROR      0
#define JUSTONECHAR  1
#define MINLIMIT     1
#define JUSTTWOCHARS 2
#define TWOPNTS      2
#define SQUAREPNTS   5
#define MARKSIZE     6
#define LIMITLTYPE   7
#define WIDTHLIMIT   8
#define TENMAX	    10
#define ONELINE     80
#define MAXHGT	 32767
#define HADERROR    -1

// Color Definitions
#define BLACK	   0
#define WHITE	   1
#define RED	   2
#define GREEN	   3
#define BLUE	   4
#define YELLOW	   5
#define CYAN	   6
#define MAGENTA    7
#define DKGREY	   8
#define LTGREY	   9
#define LTRED	  10
#define LTGREEN   11
#define LTBLUE	  12
#define BROWN	  13
#define LTCYAN	  14
#define LTMAGENTA 15

// Font Definitions
#define HWFONT	   1

// Hatch Fill Styles
#define NARROWSLASH 1
#define MEDIUMSLASH 2
#define WIDESLASH   3
#define NARROWCROSS 4
#define MEDIUMCROSS 5
#define WIDECROSS   6

// Pattern Fill Styles
#define NSLASHPAT      1
#define MSLASHPAT      2
#define WSLASHPAT      3
#define NCROSSPAT      4
#define MCROSSPAT      5
#define WCROSSPAT      6
#define SINVERDOT      7
#define NVERTSTRIPE    8
#define INVERSLASH     9
#define WVERTSTRIPE   10
#define BRICKPAT      11
#define WIDEDOT       12
#define CIRCLESPAT    13
#define WEAVE	      14
#define FLOWERPAT     15
#define ANGLEWALLP    16
#define MRANDOMDOT    17
#define SRANDOMDOT    18
#define TRANDOMDOT    19
#define NARROWLINE    20
#define MEDIUMLINE    21
#define SQUARES       22
#define ANGLEBRICK    23
#define THREEDOTPAT   24
#define MILKBOTTLES   25
#define FISHSCALES    26
#define PARALLELOGRAM 27
#define LIGHTEDBEADS  28
#define CHECKERBOARD  29
#define WRANDOMDOT    30
#define FATLINESLASH  31
#define VERTICALDASH  32
#define VERTICALBRICK 33

#define OPAQUE	    0
#define TRANSPARENT 1
