/***************************************************************************
 * Function: Pause
 *
 * Description:  This module waits for user input so the displayed
 *		 Graphics doesn't disappear too fast to see.
 *
 ***************************************************************************/
#include <cgibind.h>

#define MAX_X Displayout.LastVDCXY.x

extern	  CDVHANDLE	  DisplayHandle;
extern	  CDVCAPABILITY   Displayout;

void pause(int Hold)
{
    CXY 	EchoXY;
    auto int	RetStrLength,
		Horiz,
		Vert;
    static char string[5];
    CCOLOR	Selected;

    EchoXY.x = MAX_X;
    EchoXY.y = 0;
    CSetGTextColor (DisplayHandle, (CCOLOR)1, (CCOLOR *)&Selected);
    CSetGTextAlign (DisplayHandle, CTX_Right, CTX_Bottom,
		    (CHORALIGN *)&Horiz, (CVERTALIGN *)&Vert);
    switch (Hold)
    {
	case 1:
	{
	    CGText (DisplayHandle, EchoXY, "Press return to exit.");
	    break;
	}
	case 0:
	default:
	{
	    CGText (DisplayHandle, EchoXY, "Continue?(y/n)");
	    break;
	}
    }
    CReqString (DisplayHandle, 1, COff, EchoXY, &RetStrLength, string);
}
