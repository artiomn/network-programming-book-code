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

  Filename:	detectgraph.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	04/29/02 RSB	Created.
  		05/22/02 RSB	Auto-select maximum screen resolution.
*/

#ifdef WITH_X

#include "graphics.h"

// We make the graphics screen this (integer) percentage of the full
// screen size, or less.
#define SHRINK_PERCENT 90

//----------------------------------------------------------------------------

void
detectgraph (int *graphdriver, int *graphmode)
{
  gint ScreenWidth, ScreenHeight;
  ScreenWidth = 800;
  ScreenHeight = 600;
  if (TcGraphicsInitialized)
    {
      ScreenWidth = XDisplayWidth (TcDisplay, TcScreen);
      ScreenHeight = XDisplayHeight (TcDisplay, TcScreen);
    }
  else
    {
      TcDisplay = XOpenDisplay (NULL);
      if (TcDisplay != NULL)
	{
	  TcScreen = DefaultScreen (TcDisplay);
	  ScreenWidth = XDisplayWidth (TcDisplay, TcScreen);
	  ScreenHeight = XDisplayHeight (TcDisplay, TcScreen);
	  XCloseDisplay (TcDisplay);
	}
    }
  ScreenWidth = (SHRINK_PERCENT * ScreenWidth + 50) / 100;
  ScreenHeight = (SHRINK_PERCENT * ScreenHeight + 50) / 100;
  // Now find the "best" mode that fits.  Of course, what's "best" depends
  // on your criteria ...
  if (ScreenWidth >= 1024 && ScreenHeight >= 768)
    {
      *graphdriver = IBM8514;
      *graphmode = IBM8514HI;
    }
  else if (ScreenWidth >= 640 && ScreenHeight >= 480)
    {
      *graphdriver = IBM8514;
      *graphmode = IBM8514LO;
    }
  else if (ScreenWidth >= 640 && ScreenHeight >= 350)
    {
      *graphdriver = VGA;
      *graphmode = VGAMED;
    }
  else if (ScreenWidth >= 640 && ScreenHeight >= 200)
    {
      *graphdriver = EGA;
      *graphmode = EGALO;
    }
  else
    {
      *graphdriver = CGA;
      *graphmode = CGAC0;
    }
}

#endif // WITH_X
