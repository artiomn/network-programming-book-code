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

  Filename:	drawpoly.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/21/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

#if 0

// This version of the function works perfectly fine, but it can't be 
// generalized to fillpoly.  Having written fillpoly, however, trivially
// adapting it makes a more efficient version (the second on) of drawpoly.

void
drawpoly (int numpoints, int *polypoints)
{
  int i;
  for (i = 1; i < numpoints; i++)
    line (polypoints[2 * i - 2], polypoints[2 * i - 1],
	  polypoints[2 * i], polypoints[2 * i + 1]);
}

#else // 0

void
drawpoly (int numpoints, int *polypoints)
{
  int i;
  XPoint *Points;
  Points = malloc (numpoints * sizeof (XPoint));
  if (Points == NULL)
    {
      TcGraphResult = grNoScanMem;
      return;
    }
  // Convert to viewport coordinates.
  for (i = 0; i < numpoints; i++)
    {
      Points[i].x = TcViewLeft + *polypoints++;
      Points[i].y = TcViewTop + *polypoints++;
    }
  // Now do the actual drawing.
  XLockDisplay (TcDisplay);
  XDrawLines (TcDisplay, TcPixmaps[TcActivePage], TcGc, Points, numpoints,
	      CoordModeOrigin);
  if (TcActivePage == TcVisualPage)
    {
      XDrawLines (TcDisplay, TcWindow, TcGc, Points, numpoints,
		  CoordModeOrigin);
      XSync (TcDisplay, False);
    }
  XUnlockDisplay (TcDisplay);
  free (Points);
}

#endif // 0

//----------------------------------------------------------------------------

void
fillpoly (int numpoints, int *polypoints)
{
  int i;
  XPoint *Points;
  Points = malloc (numpoints * sizeof (XPoint));
  if (Points == NULL)
    {
      TcGraphResult = grNoScanMem;
      return;
    }
  // Convert to viewport coordinates.
  for (i = 0; i < numpoints; i++)
    {
      Points[i].x = TcViewLeft + *polypoints++;
      Points[i].y = TcViewTop + *polypoints++;
    }
  // Now do the actual drawing.
  XLockDisplay (TcDisplay);
  XSetFillStyle (TcDisplay, TcGc, FillTiled);
  XSetTile (TcDisplay, TcGc, TcTile);
  XFillPolygon (TcDisplay, TcPixmaps[TcActivePage], TcGc, Points,
		numpoints, Complex, CoordModeOrigin);
  XDrawLines (TcDisplay, TcPixmaps[TcActivePage], TcGc, Points,
	      numpoints, CoordModeOrigin);
  if (TcActivePage == TcVisualPage)
    {
      XFillPolygon (TcDisplay, TcWindow, TcGc, Points,
		    numpoints, Complex, CoordModeOrigin);
      XDrawLines (TcDisplay, TcWindow, TcGc, Points,
		  numpoints, CoordModeOrigin);
      XSync (TcDisplay, False);
    }
  XSetFillStyle (TcDisplay, TcGc, FillSolid);
  XUnlockDisplay (TcDisplay);
  free (Points);
}

#endif // WITH_X
