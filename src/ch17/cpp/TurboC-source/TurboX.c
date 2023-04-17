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

  Filename:	TurboX.c
  Purpose:	This is a callback function which is called when the
  		user tries to manually shutdown the program.
  Mod history:	05/21/02 RSB	Created.

  The point of this is handling the situation in which the user manually
  tries to shutdown the program by closing its graphics window.  We can't
  do much about it if the text-mode window is closed manually, but in the
  case of the graphics window we can allow the programmer to decide whether
  or not to terminate, and whatever graceful shutdown measures are needed.
  
  We do all this by means of a callback function, TurboX, which is called
  when the program has the urge to shutdown.  This file contains the
  default version of TurboX, but the programmer can replaced it with 
  his/her own version.
  
  ... but, it works imperfectly at best.  X doesn't seem to generate the
  same messages on all systems, and there are other difficulties.  *Sigh!*
*/

#include "TurboC.h"

//-----------------------------------------------------------------------
// The input parameter, Forced, is interpreted as follows:
//      0       The pending shutdown is due to the user manually trying
//              to close the graphics window.
//      1       The pending shutdown is due a SIGTERM or SIGKILL signal
//              having been intercepted.
// The return value is interpreted as follows:
//      0       Try to keep executing after TurboX finishes.
//      1       Shutdown the program after TurboX finishes. 
// In some cases, TurboX may be called with a parameter of 0, and then
// immediately called again with a parameter of 1.

int
TurboX (int Forced)
{
  return (0);
}
