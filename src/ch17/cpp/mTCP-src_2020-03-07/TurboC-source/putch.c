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

  Filename:	putch.c
  Purpose:	A Turbo C conio function.
  Mod history:	01/31/02 RSB	Created.
  		03/16/02 RSB	Added various special characters -- mainly
			        for line-drawing.  Because no double-line
				characters are defined, I arbitrarily make
				them the same as the single-line characters.
		03/23/02 RSB	Added _wscroll.				
*/

#include "conio.h"

//------------------------------------------------------------------------------------

int
putch (int c)
{
  if (!ConioInitialized)
    textmode (LASTMODE);
  if (c == '\n')
    {
      if (CurrentAttributes.cury + CurrentAttributes.wintop ==
	  CurrentAttributes.winbottom + 1)
	{
	  if (_wscroll)
	    scroll (CurrentWindow);
	  wclrtoeol (CurrentWindow);
	}
      else
	wmove (CurrentWindow, CurrentAttributes.cury,
	       CurrentAttributes.curx - 1);
    }
  else
    {
      extern gint TranslatedChar[256];
      if (c < 0 || c > 255)
	return (EOF);
      if (_wscroll == 0)
	{
	  scrollok (CurrentWindow, FALSE);
	  mvwaddch (CurrentWindow, CurrentAttributes.cury - 1,
		    CurrentAttributes.curx - 1, TranslatedChar[c]);
	  scrollok (CurrentWindow, TRUE);
	}
      else
	mvwaddch (CurrentWindow, CurrentAttributes.cury - 1,
		  CurrentAttributes.curx - 1, TranslatedChar[c]);
    }
  if (ConioRefreshOkay)
    wrefresh (CurrentWindow);
  getyx (CurrentWindow, CurrentAttributes.cury, CurrentAttributes.curx);
  CurrentAttributes.curx++;
  CurrentAttributes.cury++;
  return (c);
}
