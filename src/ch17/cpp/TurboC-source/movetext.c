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

  Filename:	movetext.c
  Purpose:	A Turbo C conio function.
  Mod history:	01/31/02 RSB	Created, as a stub.
  		03/17/02 RSB	Added actual code.
		03/23/02 RSB	Fixed the refresh of the current window
				at the end of the process.
*/

#include "conio.h"

int
movetext (int left, int top, int right, int bottom, int destleft, int desttop)
{
  int ReturnValue = 0;
  int sminrow, smincol, dminrow, dmincol, height_1, width_1;
  WINDOW *TempWin = NULL;
  if (!ConioInitialized)
    textmode (LASTMODE);
  if (left > right || top > bottom)
    goto Error;
  if (left < 1 || right > CurrentAttributes.screenwidth)
    goto Error;
  if (top < 1 || bottom > CurrentAttributes.screenheight)
    goto Error;
  if (left == destleft && top == desttop)
    goto Okay;
  sminrow = top - 1;
  smincol = left - 1;
  dminrow = desttop - 1;
  dmincol = destleft - 1;
  height_1 = bottom - top;
  width_1 = right - left;
  if (CurrentWindow != stdscr)
    {
      copywin (CurrentWindow, stdscr, 0, 0, CurrentAttributes.wintop - 1,
	       CurrentAttributes.winleft - 1,
	       CurrentAttributes.winbottom - 1,
	       CurrentAttributes.winright - 1, 0);
      refresh ();
    }
  // Create temporary storage for the data.  I'd copy it in-place, 
  // if I was sure that overlapping areas would work.  
  TempWin = newpad (height_1 + 1, width_1 + 1);
  if (TempWin == NULL)
    goto Error;
  if (OK != copywin (stdscr, TempWin, sminrow, smincol, 0, 0,
		     height_1, width_1, 0))
    goto Error;
  if (OK != copywin (TempWin, stdscr, 0, 0, dminrow, dmincol,
		     dminrow + height_1, dmincol + width_1, 0))
    goto Error;
  refresh ();
  // Update the current window, also.
  if (CurrentWindow != stdscr)
    {
      copywin (stdscr, CurrentWindow, CurrentAttributes.wintop - 1,
	       CurrentAttributes.winleft - 1, 0, 0,
	       CurrentAttributes.winbottom - CurrentAttributes.wintop,
	       CurrentAttributes.winright - CurrentAttributes.winleft, 0);
      wrefresh (CurrentWindow);
    }
Okay:
  ReturnValue = 1;
Error:
  if (NULL != TempWin)
    delwin (TempWin);
  return (ReturnValue);
}
