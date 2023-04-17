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

  Filename:	getpass.c
  Purpose:	A Turbo C conio function.
  Mod history:	03/21/02 RSB	Created.
*/

#include "conio.h"

char *
getpass (const char *prompt)
{
  extern char CgetsEchoOff;
  static char str[11];
  str[0] = 9;
  CgetsEchoOff = 1;
  cputs (prompt);
  cgets (str);
  CgetsEchoOff = 0;
  str[2 + (int) str[1]] = 0;
  return (&str[2]);
}
