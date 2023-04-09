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

  Filename:	rectangle.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/01/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
rectangle (int x1, int y1, int x2, int y2)
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
  // Now do the actual drawing.
  XLockDisplay (TcDisplay);
  XDrawRectangle (TcDisplay, TcPixmaps[TcActivePage], TcGc, x1, y1, i, j);
  if (TcActivePage == TcVisualPage)
    {
      XDrawRectangle (TcDisplay, TcWindow, TcGc, x1, y1, i, j);
      XSync (TcDisplay, False);
    }
  XUnlockDisplay (TcDisplay);
}

#endif // WITH_X
