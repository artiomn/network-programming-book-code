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

  Filename:	sector.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/27/02 RSB	Split off from arc.c.
*/

#ifdef WITH_X

#include <math.h>
#include "graphics.h"

//----------------------------------------------------------------------------

void
sector (int x, int y, int stangle, int endangle, int xradius, int yradius)
{
  int StartX, StartY, EndX, EndY, CenterX, CenterY, xdiam, ydiam;
  struct linesettingstype Settings;
  getlinesettings (&Settings);
  if (Settings.linestyle != SOLID_LINE || Settings.thickness != NORM_WIDTH)
    setlinestyle (SOLID_LINE, 0, TcLinestyle.thickness);
  // Convert to viewport coordinates.
  CenterX = x + TcViewLeft;
  CenterY = y + TcViewTop;
  x += TcViewLeft - xradius;
  y += TcViewTop - yradius;
  xdiam = xradius * 2;
  ydiam = yradius * 2;
  // Now do the actual drawing.
  XLockDisplay (TcDisplay);
  XSetFillStyle (TcDisplay, TcGc, FillTiled);
  XSetTile (TcDisplay, TcGc, TcTile);
  XFillArc (TcDisplay, TcPixmaps[TcActivePage], TcGc, x, y, xdiam, ydiam,
	    stangle * 64, (endangle - stangle) * 64);
  XDrawArc (TcDisplay, TcPixmaps[TcActivePage], TcGc, x, y, xdiam, ydiam,
	    stangle * 64, (endangle - stangle) * 64);
  if (stangle != endangle)
    {
      StartX = CenterX + xradius * cos (stangle * RADIANS_PER_DEGREE);
      StartY = CenterY - yradius * sin (stangle * RADIANS_PER_DEGREE);
      EndX = CenterX + xradius * cos (endangle * RADIANS_PER_DEGREE);
      EndY = CenterY - yradius * sin (endangle * RADIANS_PER_DEGREE);
      XDrawLine (TcDisplay, TcPixmaps[TcActivePage], TcGc, CenterX, CenterY, StartX, StartY);
      XDrawLine (TcDisplay, TcPixmaps[TcActivePage], TcGc, CenterX, CenterY, EndX, EndY);
    }	    
  if (TcActivePage == TcVisualPage)
    {
      XFillArc (TcDisplay, TcWindow, TcGc, x, y, xdiam, ydiam,
		stangle * 64, (endangle - stangle) * 64);
      XDrawArc (TcDisplay, TcWindow, TcGc, x, y, xdiam, ydiam,
		stangle * 64, (endangle - stangle) * 64);
      if (stangle != endangle)
	{
	  XDrawLine (TcDisplay, TcWindow, TcGc, CenterX, CenterY, StartX, StartY);
	  XDrawLine (TcDisplay, TcWindow, TcGc, CenterX, CenterY, EndX, EndY);
	}	    
     XSync (TcDisplay, False);
    }
  XSetFillStyle (TcDisplay, TcGc, FillSolid);
  XUnlockDisplay (TcDisplay);
  if (Settings.linestyle != SOLID_LINE || Settings.thickness != NORM_WIDTH)
    setlinestyle (Settings.linestyle, Settings.upattern, Settings.thickness);
}

#endif // WITH_X
