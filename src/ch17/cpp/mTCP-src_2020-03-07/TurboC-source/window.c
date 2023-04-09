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

  Filename:	window.c
  Purpose:	A Turbo C conio function.
  Mod history:	01/31/02 RSB	Created.
*/

#include "conio.h"

void
window (int left, int top, int right, int bottom)
{
  if (!ConioInitialized)
    textmode (LASTMODE);
  if (left >= right || top >= bottom)
    return;
  if (left < 1 || right > CurrentAttributes.screenwidth)
    return;
  if (top < 1 || bottom > CurrentAttributes.screenheight)
    return;
  if (CurrentWindow != stdscr)
    {
      wrefresh (CurrentWindow);
      copywin (CurrentWindow, stdscr, 0, 0, CurrentAttributes.wintop - 1,
	       CurrentAttributes.winleft - 1,
	       CurrentAttributes.winbottom - 1,
	       CurrentAttributes.winright - 1, 0);
      refresh ();
      delwin (CurrentWindow);
      refresh ();
      CurrentWindow = stdscr;
    }
  else
    refresh ();
  CurrentAttributes.winleft = left;
  CurrentAttributes.winright = right;
  CurrentAttributes.wintop = top;
  CurrentAttributes.winbottom = bottom;
  CurrentAttributes.curx = 1;
  CurrentAttributes.cury = 1;
  if (left > 1 || top > 1 || right < CurrentAttributes.screenwidth ||
      bottom < CurrentAttributes.screenheight)
    {
      CurrentWindow =
	newwin (bottom - top + 1, right - left + 1, top - 1, left - 1);
      copywin (stdscr, CurrentWindow, CurrentAttributes.wintop - 1,
	       CurrentAttributes.winleft - 1, 0, 0,
	       CurrentAttributes.winbottom - CurrentAttributes.wintop,
	       CurrentAttributes.winright - CurrentAttributes.winleft, 0);
    }
  wrefresh (CurrentWindow);
  scrollok (stdscr, FALSE);
  scrollok (CurrentWindow, TRUE);
}
