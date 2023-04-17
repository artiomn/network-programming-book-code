/*
  TurboC, a library for porting Borland Turbo C to GNU gcc.
  Copyright 2002 Ronald S. Burkey
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
  Contact Ron Burkey at info@sandroid.org.

  Filename:	outtextxy.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	06/08/02 RSB	Created.
*/

#include "graphics.h"

#ifdef WITH_X

#include "Console-8.h"

//----------------------------------------------------------------------------

void
outtextxy (int x, int y, char * textstring)
{
  XPoint Points[64], DumPoints[64];
  int NumPoints;
  int CharX, CharY, DeltaCharX, DeltaCharY;
  int RowX, RowY, DeltaRowX, DeltaRowY, DeltaX, DeltaY;
  int OffX, OffY;
  int i, j, Mask;
  // We need to convert x,y to where it is the upper-left corner of the
  // first character of the string (if viewed comfortably for reading).
  if (TcTextDirection == HORIZ_DIR)
    {
      switch (TcTextHoriz)
        {
	case LEFT_TEXT:
	  break;
	case CENTER_TEXT:
	  x -= textwidth (textstring) / 2;
	  break;
	case RIGHT_TEXT:
	  x -= textwidth (textstring);
	  break;
	}
      switch (TcTextVert)
        {
	case BOTTOM_TEXT:
	  y -= textheight (textstring);
	  break;
	case CENTER_TEXT:
	  y -= textheight (textstring) / 2;
	  break;
	case TOP_TEXT:
	  break;
	}	
    }  
  else				// VERT_DIR
    {
      switch (TcTextHoriz)
        {
	case LEFT_TEXT:
	  // This does not actually work in true Turbo C 2.x.
	  // The text ends up right-justified in my tests.  
	  // I'm not sure if I should duplicate Borland's bug or not ....
	  // No, I guess I'll make it work the way it's documented,
	  // since Borland may have fixed it in later versions.
	  break;
	case CENTER_TEXT:
	  x -= textheight (textstring) / 2;
	  break;
	case RIGHT_TEXT:
	  x -= textheight (textstring);
	  break;
	}
      switch (TcTextVert)
        {
	case BOTTOM_TEXT:
	  break;
	case CENTER_TEXT:
	  y += textwidth (textstring) / 2;
	  break;
	case TOP_TEXT:
	  y += textwidth (textstring);
	  break;
	}	
    }
  // Convert to viewport coordinates.
  CharX = x + TcViewLeft;
  CharY = y + TcViewTop;
  XLockDisplay (TcDisplay);
  
  // The bitmapped font ...
  if (TcTextFont == DEFAULT_FONT)
    {
      if (TcTextDirection == HORIZ_DIR)
	{
	  DeltaCharX = 8 * TcTextCharsize;
	  DeltaCharY = 0;
	  DeltaX = TcTextCharsize;
	  DeltaY = 0;
	  DeltaRowX = 0;
	  DeltaRowY = TcTextCharsize;
	}  
      else
	{
	  DeltaCharY = -8 * TcTextCharsize;
	  DeltaCharX = 0;
	  DeltaY = -TcTextCharsize;
	  DeltaX = 0;
	  DeltaRowY = 0;
	  DeltaRowX = TcTextCharsize;
	}
	for (; *textstring; textstring++)
	  {
	    // Here's the basic idea.  For each character, we set up the
	    // Points array to contain all of the visible pixels in the char.
	    // This entire set of pixels is drawn at once with XDrawPoints.
	    // If the chracter size has been magnified, so that each 
	    // "pixel" is really an NxN block of pixels, we modify the
	    // coordinates in Points NxN times and use XDrawPoints NxN times.
	    // First, though, there is the initial setup of Points.
	    NumPoints = 0;
	    i = (guchar) *textstring;
	    RowX = CharX;
	    RowY = CharY;
	    for (j = 0; j < 8; j++)
	      {
		x = RowX;
		y = RowY;
		for (Mask = TcConsole8Font[i][j] ; Mask; Mask = Mask << 1)
		  {
		    if (0 != (Mask & 128))
		      {
			// Note that this CAN'T overflow the array.
			Points[NumPoints].x = x;
			Points[NumPoints].y = y;
			NumPoints++;
		      }
		    x += DeltaX;
		    y += DeltaY;  
		  }
		RowX += DeltaRowX;
		RowY += DeltaRowY;  
	      }
	    //  Now, draw the points
	    if (TcTextCharsize == 1)
	      {
		XDrawPoints (TcDisplay, TcPixmaps[TcActivePage], TcGc, Points, NumPoints, CoordModeOrigin);
		if (TcActivePage == TcVisualPage)
		  XDrawPoints (TcDisplay, TcWindow, TcGc, Points, NumPoints, CoordModeOrigin);
	      }
	    else 
	      {  
		for (x = 0; x < TcTextCharsize; x++)
		  {
		    for (y = 0; y < TcTextCharsize; y++)
		      {
			if (TcTextDirection == HORIZ_DIR)
			  {
			    OffX = x;
			    OffY = y;
			  }
			else
			  {
			    OffX = y;
			    OffY = -x;
			  }  
			for (i = 0; i < NumPoints; i++)
			  {
			    DumPoints[i].x = Points[i].x + OffX;
			    DumPoints[i].y = Points[i].y + OffY;
			  }  
			XDrawPoints (TcDisplay, TcPixmaps[TcActivePage], TcGc, DumPoints, NumPoints, CoordModeOrigin);
			if (TcActivePage == TcVisualPage)
			  XDrawPoints (TcDisplay, TcWindow, TcGc, DumPoints, NumPoints, CoordModeOrigin);
		      }   
		  }
	      }
	    CharX += DeltaCharX;
	    CharY += DeltaCharY;  
	  }
      }
  if (TcActivePage == TcVisualPage)
    XSync (TcDisplay, False);
  XUnlockDisplay (TcDisplay);
}

#endif // WITH_X

//-------------------------------------------------------------------------

void
outtext (char *textstring)
{
  outtextxy (Tcx, Tcy, textstring);
  if (TcTextDirection == HORIZ_DIR && TcTextHoriz == LEFT_TEXT)
    Tcx += textwidth (textstring);
}


