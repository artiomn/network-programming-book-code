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

  Filename:	findfirst.c
  Purpose:	Functions for TurboC. 
  Mod history:	03/19/02 RSB	Created.
  
  It's necessary to use globbing to immitate findfirst/findnext, but it's
  not possible to do it perfectly.  The problem is that each findfirst
  has to create a new glob_t structure, but there is no function equivalent
  to globfree that can free the data structure when we're finished with it.
  We do two tricks to get around this.  Firstly, if findnext reaches the
  end if the filelist, the glob_t structure is freed.  However, this will
  not necessarily happen in every program.  So additionally, we keep a sort
  of pool of glob_t structures.  However, it's still possible for the pool
  to get used up if the program keeps using findfirst but never gets to the
  the ends of the filelists.  So, finally, we provide the equivalent
  function for globfree, calling it "filelast", for those rare cases where
  the source code has to be modified.
  
  Also, the various file attributes cannot be mimicked perfectly.  Here's
  the way I do it:
  
    FA_DIREC	As you'd expect -- directory.
    FA_HIDDEN	Filename begins with a dot.
    FA_SYSTEM	The user does not have read access.
    FA_RDONLY	The user does not have write access.
    FA_LABEL	not supported.
    FA_ARCH	not supported.
*/

#include <stdlib.h>
#include <sys/stat.h>
#include <glob.h>
#include "dir.h"
#include "dos.h"

#define MAX_GLOBS_OPEN 32
static glob_t *GlobsOpen[MAX_GLOBS_OPEN];

//----------------------------------------------------------------------------

int
findfirst (const char *pathname, struct ffblk *fblk, int attrib)
{
  int i;
  gint Flag;
  for (i = 0; i < MAX_GLOBS_OPEN; i++)
    if (GlobsOpen[i] == NULL)
      break;
  if (i == MAX_GLOBS_OPEN)
    return (1);
  fblk->OnFile = 0;
  fblk->GlobPtr = GlobsOpen[i] = calloc (1, sizeof (glob_t));
  if (fblk->GlobPtr == NULL)
    return (1);
  fblk->DesiredAttributes =
    attrib & (FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_DIREC);
  fblk->GlobPtr->gl_offs = 0;
  Flag = GLOB_NOSORT;
#ifdef GLOB_PERIOD
  if (0 != (FA_HIDDEN & attrib))
    Flag |= GLOB_PERIOD;
#endif
  //if (0 != (FA_DIREC & attrib))
  //  Flag |= GLOB_ONLYDIR;
  if (0 != glob (pathname, Flag, NULL, fblk->GlobPtr))
    {
      findlast (fblk);
      return (1);
    }
  return (findnext (fblk));
}

//---------------------------------------------------------------------------

int
findnext (struct ffblk *fblk)
{
  struct tm *TimeBuf;
  struct stat StatBuf;
  int ReturnValue = 1, attr;
  if (fblk == NULL || fblk->GlobPtr == NULL)
    return (1);
  for (; fblk->OnFile < fblk->GlobPtr->gl_pathc; fblk->OnFile++)
    {
      strcpy (fblk->ff_name, fblk->GlobPtr->gl_pathv[fblk->OnFile]);
      if (0 != stat (fblk->ff_name, &StatBuf))
	continue;
      attr = 0;
      if (0 == (S_IWUSR & StatBuf.st_mode))
	attr |= FA_RDONLY;
      if (0 != (S_IFDIR & StatBuf.st_mode))
	attr |= FA_DIREC;
      if (0 == (S_IRUSR & StatBuf.st_mode))
	attr |= FA_SYSTEM;
      if (*fblk->ff_name == '.')
	attr |= FA_HIDDEN;
      if (0 != (attr & ~fblk->DesiredAttributes))
	continue;
      fblk->ff_attrib = attr;
      fblk->ff_fsize = StatBuf.st_size;
      TimeBuf = localtime (&StatBuf.st_mtime);
      fblk->ff_fdate = (TimeBuf->tm_year - 80) << 9;
      fblk->ff_fdate += (TimeBuf->tm_mon + 1) << 5;
      fblk->ff_fdate += TimeBuf->tm_mday;
      fblk->ff_ftime = TimeBuf->tm_hour << 11;
      fblk->ff_ftime += TimeBuf->tm_min << 5;
      fblk->ff_ftime += TimeBuf->tm_sec / 2;
      ReturnValue = 0;
      fblk->OnFile++;
      break;
    }
  // All done with this record.  Close out the glob, if appropriate.
  if (fblk->OnFile >= fblk->GlobPtr->gl_pathc)
    findlast (fblk);
  return (ReturnValue);
}

//---------------------------------------------------------------------------

void
findlast (struct ffblk *fblk)
{
  int i;
  if (fblk != NULL && fblk->GlobPtr != NULL)
    {
      for (i = 0; i < MAX_GLOBS_OPEN; i++)
	if (GlobsOpen[i] == fblk->GlobPtr)
	  {
	    GlobsOpen[i] = NULL;
	    break;
	  }
      globfree (fblk->GlobPtr);
      fblk->GlobPtr = NULL;
    }
}
