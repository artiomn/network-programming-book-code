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

  Filename:	getmodename.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/24/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

extern char *
getmodename (int mode_number)
{
  static char ModeName[41];
  int Xres, Yres, Pcnt;
  switch (mode_number % TCX)
    {
    case TC_X_320:
      Xres = 320;
      break;
    case TC_X_640:
      Xres = 640;
      break;
    case TC_X_720:
      Xres = 720;
      break;
    case TC_X_800:
      Xres = 800;
      break;
    case TC_X_1024:
      Xres = 1024;
      break;
    case TC_X_1280:
      Xres = 1280;
      break;
    default:
      strcpy (ModeName, "Invalid graphics mode");
      return (ModeName);
    }
  switch ((mode_number / TCX) % TCY)
    {
    case TC_Y_200:
      Yres = 200;
      break;
    case TC_Y_348:
      Yres = 348;
      break;
    case TC_Y_350:
      Yres = 350;
      break;
    case TC_Y_400:
      Yres = 400;
      break;
    case TC_Y_480:
      Yres = 480;
      break;
    case TC_Y_600:
      Yres = 600;
      break;
    case TC_Y_768:
      Yres = 768;
      break;
    case TC_Y_1024:
      Yres = 1024;
      break;
    default:
      strcpy (ModeName, "Invalid graphics mode");
      return (ModeName);
    }
  switch ((mode_number / TCXY) % TCG)
    {
    case TC_PG_1:
      Pcnt = 1;
      break;
    case TC_PG_2:
      Pcnt = 2;
      break;
    case TC_PG_4:
      Pcnt = 4;
      break;
    default:
      strcpy (ModeName, "Invalid graphics mode");
      return (ModeName);
    }
  sprintf (ModeName, "%dx%d %d-page ", Xres, Yres, Pcnt);
  // Set up the default colors in the palette.
  switch ((mode_number / TCXYG) % TCP)
    {
    case TC_PAL_2:
      strcat (ModeName, "2-color");
      break;
    case TC_PAL_C0:
      strcat (ModeName, "palette-1");
      break;
    case TC_PAL_C1:
      strcat (ModeName, "palette-2");
      break;
    case TC_PAL_C2:
      strcat (ModeName, "palette-3");
      break;
    case TC_PAL_C3:
      strcat (ModeName, "palette-4");
      break;
    case TC_PAL_4:
      strcat (ModeName, "4-color");
      break;
    case TC_PAL_16:
      strcat (ModeName, "16-color");
      break;
    case TC_PAL_256:
      strcat (ModeName, "256-color");
      break;
    default:
      strcpy (ModeName, "Invalid grapics mode");
      break;
    }
  return (ModeName);
}

#endif // WITH_X
