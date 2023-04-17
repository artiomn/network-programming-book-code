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

  Filename:	cgets.c
  Purpose:	A Turbo C conio function.
  Mod history:	03/04/02 RSB	Created.
  		03/21/02 RSB	Added CgetsEchoOff so that cgets can
				be used to implement getpass.  Also,
				fixed an unreported bug in which 
				cgets will abort after reaching the 
				maximum string length.
*/

#include "conio.h"

// This global variable exists to allow the cgets code (which echoes
// to the screen) to be used for getpass (which does not).  0 is the
// normal state, and non-zero is for getpass.
char CgetsEchoOff = 0;

char *
cgets (char *str)
{
  int Len, Max, ch, x, y;
  Max = (guchar) str[0];
  Len = 0;
  while (!CgetsEchoOff || Len < Max - 1)
    {
      ch = getch ();
      if (ch == '\r' || ch == '\n')
	{
	  str[2 + Len] = 0;
	  break;
	}
      if (ch == '\b')
	{
	  if (Len > 0)
	    {
	      if (!CgetsEchoOff)
		{
		  x = wherex ();
		  y = wherey ();
		  x--;
		  if (x < 1)
		    {
		      struct text_info ti;
		      gettextinfo (&ti);
		      x = ti.screenwidth;
		      y--;
		    }
		  gotoxy (x, y);
		  putch (' ');
		  gotoxy (x, y);
		}
	      Len--;
	    }
	}
      else if (ch >= ' ' && ch < 127)
	{
	  if (Len < Max - 1)
	    {
	      str[2 + Len++] = ch;
	      if (!CgetsEchoOff)
		putch (ch);
	    }
	}
    }
  str[1] = Len;
  return (str);
}
