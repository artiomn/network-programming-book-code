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

  Filename:	clearXXXX.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/13/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
cleardevice (void)
{
  int Left, Top, Right, Bottom, Clip;
  Left = TcViewLeft;
  Top = TcViewTop;
  Right = TcViewRight;
  Bottom = TcViewBottom;
  Clip = TcViewClip;
  setviewport (0, 0, TcXresolution - 1, TcYresolution - 1, 0);
  clearviewport ();
  setviewport (Left, Top, Right, Bottom, Clip);
}

//----------------------------------------------------------------------------

void
clearviewport (void)
{
  int Mode;
  Mode = TcWritemode;
  setwritemode (COPY_PUT);
  XSetForeground (TcDisplay, TcGc, TcXbackground.pixel);
  // Now do the actual drawing.
  XLockDisplay (TcDisplay);
  XFillRectangle (TcDisplay, TcPixmaps[TcActivePage], TcGc, TcViewLeft,
		  TcViewTop, TcViewRight - TcViewLeft + 1,
		  TcViewBottom - TcViewTop + 1);
  if (TcActivePage == TcVisualPage)
    {
      XFillRectangle (TcDisplay, TcWindow, TcGc, TcViewLeft,
		      TcViewTop, TcViewRight - TcViewLeft + 1,
		      TcViewBottom - TcViewTop + 1);
      XSync (TcDisplay, False);
    }
  XUnlockDisplay (TcDisplay);
  XSetForeground (TcDisplay, TcGc, TcXforeground.pixel);
  setwritemode (Mode);
}

#endif // WITH_X
