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

  Filename:	gettext.c
  Purpose:	A Turbo C conio function.
  Mod history:	01/31/02 RSB	Created, as a stub.
  		03/17/02 RSB	Began adding actual code.
		06/02/02 RSB	The method of reading back the _line
				array turned out not to be as 
				portable as I hoped.  Now replaced
				by the mvwinchstr function.
		11/02/03 RSB	gettext renamed to gettextTurboC to
				avoid conflicts with the internationalization
				library's gettext function.
		
  What makes this apparently trivial function tricky is that the format
  of the data required by Turbo C (as documented in the Reference Guide)
  and that provided by ncurses (as deduced by looking at the ncurses
  lib_pad.c source code) are not the same.  Each of them provides an
  integer for each character cell, and both arrange the data as rows
  and columns, but character values, colors, and other attributes must be
  translated.
*/

#include "conio.h"

extern const int32_t ForegroundColors[16];
extern const int32_t ForegroundFlags[16];
extern const int32_t BackgroundColors[8];
extern const int32_t BackgroundFlags[16];

//------------------------------------------------------------------------

int
gettextTurboC (int left, int top, int right, int bottom, void *dest)
{
  uint8_t *TurboData;
  uint16_t dFore, dBack;
  int Row, Col, i, j, k;
  chtype ch;
  gint Char, Color, Flag;
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
  if (OK != copywin (stdscr, TempWin, sminrow, smincol, 0, 0,
		     height_1, width_1, 0))
    goto Error;
  // Now we have all of the data, but we must translate it from the 
  // form provided by ncurses to the form needed in Turbo C.  
  TurboData = (int8_t *) dest;
  for (Row = top, i = 0; Row <= bottom; Row++, i++)
    for (Col = left, j = 0; Col <= right; Col++, j++)
      {
	extern gint TranslatedChar[256];
	// What ncurses character/color/attribute is at this position?
	// The following line worked every time I tried it up to
	// SuSE 8.0, at which point it wouldn't compile any longer.
	// So, I've replaced it.
	//ch = TempWin->_line[i].text[j];
	mvwinchnstr (TempWin, i, j, &ch, 1);
	// Pick off the character part, and "untranslate" it back to 
	// Turbo C form.
	Char = ch & A_CHARTEXT;
	if (Char < 0x20 || Char >= 0x7f)
	  {
	    for (k = 0; k < 256; k++)
	      {
		if (Char == TranslatedChar[k])
		  {
		    Char = k;
		    break;
		  }
		if (k == 0x1f)
		  k = 0x7e;
	      }
	    if (k == 256)
	      Char = ' ';
	  }
	// Now translate the colors and attibutes. The ncurses colors
	// have to be referenced back to the Turbo C colors.
	Color = PAIR_NUMBER (ch & A_COLOR);
	pair_content (Color, &dFore, &dBack);
	if (0 != (ch & A_BOLD))
	  Flag = A_BOLD;
	else
	  Flag = A_NORMAL;
	for (k = 0; k < 16; k++)
	  if (dFore == ForegroundColors[k] && Flag == ForegroundFlags[k])
	    {
	      dFore = k;
	      break;
	    }
	if (k == 16)
	  dFore = 15;
	if (0 != (ch & A_BLINK))
	  Flag = A_BLINK;
	else
	  Flag = A_NORMAL;
	for (k = 0; k < 16; k++)
	  if (dBack == BackgroundColors[k & 7] && Flag == BackgroundFlags[k])
	    {
	      dBack = k;
	      break;
	    }
	if (k == 16)
	  dBack = 0;
	// Well, we've got it.  Buffer it.
	*TurboData++ = Char;
	*TurboData++ = (dBack << 4) | dFore;
      }
  ReturnValue = 1;
Error:
  if (NULL != TempWin)
    delwin (TempWin);
  return (ReturnValue);
}
