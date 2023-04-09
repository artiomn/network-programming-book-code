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

  Filename:	ctype.c
  Purpose:	Wrappers for ctype.h macros. 
  Mod history:	03/02/02 RSB	Created.
*/

#include <ctype.h>

int
isupperTurbo (int c)
{
  return (isupper (c));
}

int
islowerTurbo (int c)
{
  return (islower (c));
}

int
isalphaTurbo (int c)
{
  return (isalpha (c));
}

int
isdigitTurbo (int c)
{
  return (isdigit (c));
}

int
isxdigitTurbo (int c)
{
  return (isxdigit (c));
}

int
isspaceTurbo (int c)
{
  return (isspace (c));
}

int
isprintTurbo (int c)
{
  return (isprint (c));
}

int
isgraphTurbo (int c)
{
  return (isgraph (c));
}

int
iscntrlTurbo (int c)
{
  return (iscntrl (c));
}

int
ispunctTurbo (int c)
{
  return (ispunct (c));
}

int
isalnumTurbo (int c)
{
  return (isalnum (c));
}
