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

  Filename:	bar.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/20/02 RSB	Created.
  		05/21/02 RSB	Was mistakenly partially using 
				XFillRectangle and partially XDrawRectangle.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
bar (int x1, int y1, int x2, int y2)
{
  int i, j;
  if (x1 > x2)
    {
      i = x1;
      x1 = x2;
      x2 = i;
    }
  if (y1 > y2)
    {
      i = y1;
      y1 = y2;
      y2 = i;
    }
  i = x2 - x1 + 1;
  j = y2 - y1 + 1;
  // Convert to viewport coordinates.
  x1 += TcViewLeft;
  y1 += TcViewTop;
  XLockDisplay (TcDisplay);
  XSetFillStyle (TcDisplay, TcGc, FillTiled);
  XSetTile (TcDisplay, TcGc, TcTile);
  // Now do the actual drawing.
  XFillRectangle (TcDisplay, TcPixmaps[TcActivePage], TcGc, x1, y1, i, j);
  if (TcActivePage == TcVisualPage)
    {
      XFillRectangle (TcDisplay, TcWindow, TcGc, x1, y1, i, j);
      XSync (TcDisplay, False);
    }
  XSetFillStyle (TcDisplay, TcGc, FillSolid);
  XUnlockDisplay (TcDisplay);
}

//----------------------------------------------------------------------------

void
bar3d (int x1, int y1, int x2, int y2, int depth, int topflag)
{
  int dx, dy;
  int i1, i2, j1, j2;
  // Order the coordinates correctly.
  if (x1 > x2)
    {
      i1 = x1;
      x1 = x2;
      x2 = i1;
    }
  if (y1 > y2)
    {
      j1 = y1;
      y1 = y2;
      y2 = j1;
    }
  // Draw the innards of the bar.
  bar (x1, y1, x2, y2);
  // Put an outline around the bar.
  line (x1, y1, x2, y1);
  line (x2, y1, x2, y2);
  line (x2, y2, x1, y2);
  line (x1, y2, x1, y1);
  // Calculate the x,y offsets associated with the depth.  I assume
  // a 60-degree angle.
  if (depth <= 0)
    return;
  dy = (depth + 1) / 2;		// dy = depth * sin(30)
  dx = (depth * 866 + 500) / 1000;	// dx = depth * cos(30);
  // Draw the right-hand side.
  i1 = x1 + dx;
  j1 = y1 - dy;
  i2 = x2 + dx;
  j2 = y2 - dy;
  line (x2, y2, i2, j2);
  line (i2, j2, i2, j1);
  // Draw the top.
  if (topflag)
    {
      line (x2, y1, i2, j1);
      line (x1, y1, i1, j1);
      line (i1, j1, i2, j1);
    }
}


#endif // WITH_X
