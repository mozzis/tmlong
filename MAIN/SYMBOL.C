#include <math.h>
#include <cgibind.h>
#include <string.h>

#include "primtype.h"
#include "constant.h"
#include "device.h"

void symbol(CXY *TextPt, char * s, float angle, float cell_width,
            float cell_height, CCOLOR color)
{
  CGTEXTREPR TextInfo;
  CDVHANDLE handle = deviceHandle();
  
  /* symbol prints string at location x,y with angle angle (in radians) */
  
  CInqGTextRepr(handle, &TextInfo);

  /* GSS Text Angle is tenths of a degree */
  /* use deg = (rad * 360) / 2pi */

  TextInfo.Angle = (int) (angle * 1800.0F / PI);
//  TextInfo.CellSize.x = (int)cell_width;
//  TextInfo.CellSize.y = (int)cell_height;
  TextInfo.Color = color;

  CSetGTextRepr(handle, &TextInfo);
   
  CGText(handle, *TextPt, (UCHAR *)s);
}

static void StrBox(char * text, float angle, float cell_width,
                   float cell_height, CXY * LLeftPt, CXY * LRightPt,
                   CXY * URightPt, CXY * ULeftPt, USHORT * length )
{
  /* angle (in radians), calculate sines and cosines needed for rotation */
  float c  = (float) cos(angle) ;
  float si = (float) sin(angle) ;
  
  *length = (USHORT)(cell_width * (float)strlen(text));
  
  /* lower left corner will always be at 0.0 */
  LLeftPt->x = 0;
  LLeftPt->y = 0;
  
  /* adjust for string rotation */
  LRightPt->x = (int) (c * *length);
  LRightPt->y = (int) (si * *length);
  
  /* adjust for character rotation */
  URightPt->x = LRightPt->x - (int) (si * cell_height);
  URightPt->y = LRightPt->y + (int) (c * cell_height);
  
  ULeftPt->x = URightPt->x - LRightPt->x;
  ULeftPt->y = URightPt->y - LRightPt->y;
}

/* -----------------------------------------------------------------------
/
/  function:   Resets alignment_pt to get the proper alignment if drawn
/              with current text characteristics.
/
/  requires:   (FLOAT) cell_width - width of character cell
/              (FLOAT) cell_height - height of character cell
/              (SHORT) xalign - the X alignment indicator ALIGN_LEFT |
/                               ALIGN_RIGHT | ALIGN_CENTER
/              (SHORT) yalign - the Y alignment indicator ALIGN_TOP |
/                               ALIGN_BOTTOM | ALIGN_CENTER
/              (CHAR *) string - the null ended string to be written
/              (CXY *) alignment_pt - Input. Relative point to align on
/                                     Output. Lower left coordinate of
/                                     string in order to get proper
/                                     alignment
/              (FLOAT) angle - writing angle in radians
/
/  returns:    (int) FALSE
/
/  side effects: none
/
/ ----------------------------------------------------------------------- */
SHORT AlignText( FLOAT cell_width, FLOAT cell_height,
                 SHORT xalign, SHORT yalign, CHAR * string,
                 CXY * alignment_pt, FLOAT angle )
{
  USHORT string_length;
  CXY LLeftPt, LRightPt, URightPt, ULeftPt;

  StrBox( string, angle, cell_width, cell_height,
    &LLeftPt, &LRightPt, &URightPt, &ULeftPt, &string_length);

  switch( xalign )
    {
    case CTX_Left:
      /* stays the same */
    break;
    case CTX_Center:
      alignment_pt->x -= LRightPt.x / 2;
      alignment_pt->y -= LRightPt.y / 2;
    break;
    case CTX_Right:
      alignment_pt->x -= LRightPt.x;
      alignment_pt->y -= LRightPt.y;
    break;
    }

  switch( yalign )
    {
    case CTX_Center:
      alignment_pt->x -= ULeftPt.x / 2;
      alignment_pt->y -= ULeftPt.y / 2;
    break;
    case CTX_Top:
      alignment_pt->x -= ULeftPt.x;
      alignment_pt->y -= ULeftPt.y;
    break;
    case CTX_Bottom:
      /* nothing to do here */
    break;
    }

  return FALSE;
}


