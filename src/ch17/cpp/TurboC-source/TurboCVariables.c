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

  Filename:	TurboCVariables.c
  Purpose:	TurboC library global variables.
  Mod history:	05/10/02 RSB	Created.

  The global variables are all defined in header files, along with the 
  constant TURBOC_VARIABLES_C which is used either to set them as extern
  or not.  The only file in which they aren't extern is this one.
*/

#define TURBOC_VARIABLES_C

// All of the TurboC library header files follow.  Not all of them have
// global variables defined within them, but just in case ...
#include "TurboC.h"
#include "fnkeys.h"
#include "alloc.h"
#include "dir.h"
#include "dos.h"
#include "io.h"
#ifdef WITH_X
#include "graphics.h"
#endif
// conio.h has to be last in the list.
#include "conio.h"
