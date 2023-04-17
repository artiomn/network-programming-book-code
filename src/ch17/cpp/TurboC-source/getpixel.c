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

  Filename:	getpixel.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/25/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

unsigned
getpixel (int x, int y)
{
  gint i, maxi, ClosestS, s;
  XColor Color;
  unsigned RetVal = 0;
  XImage *Image;
  // Convert to viewport coordinates.
  x += TcViewLeft;
  y += TcViewTop;
  // Now do the interrogating.
  XLockDisplay (TcDisplay);
  Image = XGetImage (TcDisplay, TcPixmaps[TcVisualPage], x, y, 1, 1, 
  		     ~0L, XYPixmap);
  if (Image != NULL)
    {
      Color.pixel = XGetPixel (Image, 0, 0);
      XDestroyImage (Image);  
      // Look up the rgb components of the raw pixel we've read back.
      XQueryColor (TcDisplay, TcColormap, &Color);
      XUnlockDisplay (TcDisplay);
      // Find the closest-match color in our current palette.  Since the
      // colors in our palette are generally so far apart from each other,
      // we can save a lot of execution time by using a taxi-cab metric
      // rather than a Euclidean metric.
      ClosestS = 1000000;
      maxi = getmaxcolor ();
      for (i = 0; i <= maxi; i++)
        {
	  s = abs (Color.red - TcColors[i].r) + abs (Color.green - TcColors[i].g)
	      + abs (Color.blue - TcColors[i].b);
	  if (s < ClosestS)
	    {
	      RetVal = i;
	      ClosestS = s;
	    } 
	  if (s == 0)
	    break;   
	}
    }		     
  else  
    XUnlockDisplay (TcDisplay);
  return (RetVal);  
}

#endif // WITH_X
