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

  Filename:	puttext.c
  Purpose:	A Turbo C conio function.
  Mod history:	01/31/02 RSB	Created, as a stub.
  		03/17/02 RSB	Added actual code.
		03/23/02 RSB	Fixed the refresh of the current window
				at the end of the process.
		06/02/02 RSB	Accessing the _line array turned out
				not to be as portable as I'd hoped.
				Now replaced by the mvwaddchstr
				function instead.
*/

#include "conio.h"

//------------------------------------------------------------------------

int
puttext (int left, int top, int right, int bottom, void *source)
{
  chtype ch;
  int32_t TranslateTurboColor (int newattr);
  uint8_t *TurboData;
  int32_t NcursesAttr;
  int Row, Col, i, j;
  gint Char;
  int ReturnValue = 0;
  int sminrow, smincol, height_1, width_1;
  WINDOW *TempWin = NULL;
  if (!ConioInitialized)
    textmode (LASTMODE);
  if (left > right || top > bottom)
    goto Error;
  if (left < 1 || right > CurrentAttributes.screenwidth)
    goto Error;
  if (top < 1 || bottom > CurrentAttributes.screenheight)
    goto Error;
  sminrow = top - 1;
  smincol = left - 1;
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
  // Create temporary storage for the data. We can't directly use the
  // buffer provided by the calling code, because it's not the right
  // size.  
  TempWin = newpad (height_1 + 1, width_1 + 1);
  if (TempWin == NULL)
    goto Error;
  // Now we have all of the data, but we must translate it from the 
  // form provided by ncurses to the form needed in Turbo C.  
  TurboData = (int8_t *) source;
  for (Row = top, i = 0; Row <= bottom; Row++, i++)
    for (Col = left, j = 0; Col <= right; Col++, j++)
      {
	extern gint TranslatedChar[256];
	// What Turbo C character/color/attribute is at this position?
	Char = TranslatedChar[*TurboData++];
	NcursesAttr = TranslateTurboColor (*TurboData++);
	//TempWin->_line[i].text[j] = Char | NcursesAttr;
	ch = Char | NcursesAttr;
	mvwaddchnstr (TempWin, i, j, &ch, 1);
      }
  // Now write actual data to the screen.    
  if (OK != copywin (TempWin, stdscr, 0, 0, sminrow, smincol,
		     sminrow + height_1, smincol + width_1, 0))
    goto Error;
  refresh ();
  // Also, update the current window.
  if (CurrentWindow != stdscr)
    {
      copywin (stdscr, CurrentWindow, CurrentAttributes.wintop - 1,
	       CurrentAttributes.winleft - 1, 0, 0,
	       CurrentAttributes.winbottom - CurrentAttributes.wintop,
	       CurrentAttributes.winright - CurrentAttributes.winleft, 0);
      wrefresh (CurrentWindow);
    }
  ReturnValue = 1;
Error:
  if (NULL != TempWin)
    delwin (TempWin);
  return (ReturnValue);
}
