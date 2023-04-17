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

  Filename:	random.c
  Purpose:	Replaces the GNU gcc random function with a Borland random
  		function. 
  Mod history:	03/02/02 RSB	Created.
  		03/23/02 RSB	Added the __APPLE__ option and removed a
				stray #elif that was supposed to be an
				#else.
*/

#include <stdlib.h>
#include <time.h>
#if defined(linux)
#include <stdint.h>
#elif defined(__FreeBSD__) || defined(__APPLE__)
#include <inttypes.h>
#else
#error Need header file for integer datatypes.
#endif
//#include "TurboC.h"                   Must *not* include this.

//------------------------------------------------------------------------
// The Turbo C function is supposed to return a random number between
// 0 and num-1.  This function doesn't *exactly* do that since the 
// distribution is skewed somewhat by the fact that num probably does
// not exactly divide RAND-MAX+1.  However, since it's likely RAND_MAX
// is much much greather than num, I'm willing to ignore it rather than 
// correct for it.

int16_t
randomTurbo (int16_t num)
{
  return (random () % num);
}

void
randomize (void)
{
  long t;
  srand (time (&t));
}
