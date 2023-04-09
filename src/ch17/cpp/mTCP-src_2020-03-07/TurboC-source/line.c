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

  Filename:	line.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/01/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
line (int x1, int y1, int x2, int y2)
{
  // Convert to viewport coordinates.
  x1 += TcViewLeft;
  x2 += TcViewLeft;
  y1 += TcViewTop;
  y2 += TcViewTop;
  // Now do the actual drawing.
  XLockDisplay (TcDisplay);
  XDrawLine (TcDisplay, TcPixmaps[TcActivePage], TcGc, x1, y1, x2, y2);
  if (TcActivePage == TcVisualPage)
    {
      XDrawLine (TcDisplay, TcWindow, TcGc, x1, y1, x2, y2);
      XSync (TcDisplay, False);
    }
  XUnlockDisplay (TcDisplay);
}

//----------------------------------------------------------------------------

void
lineto (int x, int y)
{
  int x1, y1;
  x1 = Tcx;
  y1 = Tcy;
  Tcx = x;
  Tcy = y;
  line (x1, y1, x, y);
}

//----------------------------------------------------------------------------

void
linerel (int dx, int dy)
{
  lineto (Tcx + dx, Tcy + dy);
}

//----------------------------------------------------------------------------

void
moveto (int x, int y)
{
  Tcx = x;
  Tcy = y;
}

//----------------------------------------------------------------------------

void
moverel (int dx, int dy)
{
  Tcx += dx;
  Tcy += dy;
}

//----------------------------------------------------------------------------

int
getx (void)
{
  return (Tcx);
}

//----------------------------------------------------------------------------

int
gety (void)
{
  return (Tcy);
}

#endif // WITH_X
