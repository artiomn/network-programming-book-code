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

  Filename:	dir.h
  Purpose:	A header file for TurboC. 
  Mod history:	03/02/02 RSB	Created.
  		03/19/02 RSB	Added stuff for findfirst/findnext.
*/

#ifndef _INCLUDED_DIR_H
#define _INCLUDED_DIR_H

#include <sys/cdefs.h>
#include <glob.h>
#include "TurboC.h"

//----------------------------------------------------------------------------
// Constants.

// Maximum length of a filename.
#ifndef FILENAME_MAX
#define FILENAME_MAX 1024
#endif

//----------------------------------------------------------------------------
// Data types.

struct ffblk
{
  char ff_reserved[21];		// Not needed, obviously.
  char ff_attrib;
  int ff_ftime;
  int ff_fdate;
  long ff_fsize;
  char ff_name[FILENAME_MAX + 1];	// Size 13 in Turbo C 2.x.
  // Stuff not in Turbo, used for the emulation.
  glob_t *GlobPtr;
  gint OnFile;
  int DesiredAttributes;
};

//----------------------------------------------------------------------------
// Function prototypes.

__BEGIN_DECLS
#ifndef __MINGW32__
#define mkdir(filename) mkdir(filename,0755)
#endif
int findfirst (const char *pathname, struct ffblk *fblk, int attrib);
int findnext (struct ffblk *fblk);
void findlast (struct ffblk *fblk);

__END_DECLS
//--------------------------------------------------------------------------
// Variables.
#endif // _INCLUDED_DIR_H
