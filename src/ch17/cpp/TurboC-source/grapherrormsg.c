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

  Filename:	grapherrormsg.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/24/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

static const char *ErrorMessages[] = {
  "No error",
  "(BGI) graphics not installed (use initgraph)",
  "Graphics hardware not detected",
  "Device driver file not found",
  "Invalid device driver file",
  "Not enough memory to load driver",
  "Out of memory in scan fill",
  "Out of memory in flood fill",
  "Font file not found",
  "Not enough memory to load font",
  "Invalid graphics mode for selected driver",
  "Graphics error",
  "Graphics I/O error",
  "Invalid font file",
  "Invalid font number",
  "Invalid device number",
  "Unknown error",
  "Unknown error",
  "Invalid version of file"
};
#define NUM_MESSAGES (sizeof (ErrorMessages) / sizeof (ErrorMessages[0]))

//----------------------------------------------------------------------------

char *
grapherrormsg (int errorcode)
{
  static char ErrorMessage[61];
  if (errorcode < 0 || errorcode >= NUM_MESSAGES)
    strcpy (ErrorMessage, "Unknown error");
  else
    strcpy (ErrorMessage, ErrorMessages[errorcode]);
  return (ErrorMessage);
}

#endif // WITH_X
