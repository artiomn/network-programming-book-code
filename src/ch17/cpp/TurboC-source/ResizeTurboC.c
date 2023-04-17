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

  Filename:	ResizeTurboC.c
  Purpose:	A function to physically resize the text console.
  Mod history:	04/18/02 RSB	Created.
		04/19/02 RSB	Allowed TIOCGWINSZ to be a macro.  It isn't
				in Linux, but is in FreeBSD.  Also, finally
				licked (I think) the problem of getting
				the screen redrawn properly.
												
*/

//#include <termios.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>

// If TIOCGWINSZ is a macro containing a cast (as it is in FreeBSD, but not
// in Linux), we can't allow it to be directly used in the code after the
// integer types have been redefined.
static unsigned long tiocgwinsz = TIOCGWINSZ;

#include "conio.h"

extern char BypassResizeXterm;

//-----------------------------------------------------------------------
// Resize the physical terminal.

void
RawResizeTurboC (gint Rows, gint Columns)
{
  // This is really lame, but ....  The printf statement logically
  // (and actually) accomplishes the resizing of an xterm
  // terminal.  None of the other stuff should be necessary.  
  // However, the resizing takes a certain amount of time, for 
  // reasons unclear to me.  Perhaps it's the passage of time, 
  // perhaps it's a buffer getting filled up ... I don't know.
  // The problem is that stuff may get written to screen positions
  // that don't yet exist, because the sizing has not yet completed,
  // and this can cause a segfault.  Anyhow, the loop is an attempt 
  // to kill time, or fill up the buffer, or whatever.  It aborts
  // if it finds that the size has actually reached the correct
  // value, so there's not too much harm in it except for remote
  // logins (in which case the remote terminal size doesn't seem
  // to be determined correctly).  We make a feeble attempt to 
  // determine if the login is remote, so that we can shorten the
  // loop in that case.
  // ... Later:  I don't think there's actually any time penalty
  // for the big loop.  There's some kind of intermittent timing
  // problem with the screen resize when running remotely, but
  // it's not the loop.
  int i, MaxCount;
  //char *display;
  MaxCount = 16384;
  // Possibly detect a remote login.
  //display = getenv ("DISPLAY");
  //if (display == NULL || atoi (display) > 0)
  //  MaxCount = 1024;
  for (i = 0; i < MaxCount; i++)
    {
      struct winsize winsz;
      printf ("\033[8;%d;%dt", Rows, Columns);
      fflush (stdout);
      ioctl (0, tiocgwinsz, &winsz);
      if (Columns == winsz.ws_col && Rows == winsz.ws_row)
	break;
    }
}

//-------------------------------------------------------------------
// This function is activated under various conditions, of which the
// most interesting is if the console is resized.

void
ResizeTurboC (gint Code)
{
  if (BypassResizeXterm)
    {
      // What this code does is essentially just to *accept* the fact
      // that the screen has resized, to adjust the variables that
      // track this as best it can, and to call ConioResizeCallback
      // to allow the user program to redraw the screen (or whatever)
      // as best it can.  This code was written before I figured out
      // how to physically resize the text console effectively, but 
      // it's still a reasonable approach.
      struct winsize winsz;
      ioctl (0, tiocgwinsz, &winsz);
      resizeterm (winsz.ws_row, winsz.ws_col);
      CurrentAttributes.screenwidth = COLS;
      CurrentAttributes.screenheight = LINES;
      ConioResizeCallback ();
    }
  else
    {
      // What this code does is to undo the manual resizing which the
      // user has performed, and to restore the physical terminal
      // size to what textmode has set it at.  Unfortunately, this 
      // only works if the ANSI sequence for resizing the screen works.
      // Fortunately, this works in xterm.
      struct winsize winsz;
      ioctl (0, tiocgwinsz, &winsz);
      if (CurrentAttributes.screenwidth != winsz.ws_col ||
	  CurrentAttributes.screenheight != winsz.ws_row)
	{
	  RawResizeTurboC (CurrentAttributes.screenheight,
			   CurrentAttributes.screenwidth);
	  // The following line forces a complete redraw of the screen
	  // (using the image ncurses has buffered).               
	  wrefresh (curscr);
	}
    }
}
