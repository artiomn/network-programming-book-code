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

  Filename:	setviewport.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/08/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
setviewport (int left, int top, int right, int bottom, int clip)
{
  XRectangle Rectangle;
  if (left < 0 || right > TcXresolution - 1 || left >= right ||
      top < 0 || bottom > TcYresolution - 1 || top >= bottom)
    {
      TcGraphResult = grError;
      return;
    }
  TcViewLeft = left;
  TcViewTop = top;
  TcViewRight = right;
  TcViewBottom = bottom;
  TcViewMaxx = right - left;
  TcViewMaxy = bottom - top;
  TcViewClip = clip;
  Tcx = Tcy = 0;
  // Account for clipping.  Fortunately, this can be done entirely
  // within X, by setting the clipping region within the graphics
  // context.
  if (clip)
    {
      Rectangle.x = left;
      Rectangle.y = top;
      Rectangle.width = TcViewMaxx + 1;
      Rectangle.height = TcViewMaxy + 1;
      XSetClipRectangles (TcDisplay, TcGc, 0, 0, &Rectangle, 1, Unsorted);
    }
  else
    XSetClipMask (TcDisplay, TcGc, None);
}

#endif // WITH_X
