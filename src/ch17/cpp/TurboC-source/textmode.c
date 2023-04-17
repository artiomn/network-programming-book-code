/*
  TurboC, a library for porting Borland Turbo C to GNU gcc.
  Copyright 2002,2004 Ronald S. Burkey
 
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

  Filename:	textmode.c
  Purpose:	conio.h is emulated using ncurses.  textmode is used to
  		initialize ncurses.  It is not optional as it is in 
		Turbo C.  Also, it must be called at the end of the program,
		as textmode(EXITMODE).
  Mod history:	01/27/02 RSB	Created.
  		03/02/02 RSB	Reordered header files.
		03/16/02 RSB	Now can automatically resize the physical 
				console if the xterm is running.
		03/17/02 RSB	Added initialization of the display-character
				translation table.
		03/24/02 RSB	Oops!  This had broken tabs.
		03/31/02 RSB	Added signal handling for SIGKILL and
				SIGTERM.
		04/18/02 RSB	The ResizeTurboC function was moved
				into its own file.
		02/17/04 RSB	Jim Huang has reported a problem (on some
				unspecified version of *nix) in which the
				ACS macros may get messed up with TurboC's
				automatic integer-type redefinitions.  
				Hopefully fixed here.
*/

//#include <termios.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "conio.h"

char BypassResizeXterm = 0;
static uint32_t LastMode = C80;
#define NUM_VIDEO_MODES 65
static struct
{
  int Columns;
  int Rows;
}
VideoModes[NUM_VIDEO_MODES] =
{
  {
  40, 25}
  ,				// BW40
  {
  40, 25}
  ,				// C40
  {
  80, 25}
  ,				// BW80
  {
  80, 25}
  ,				// C80
  {
  80, 25}
  ,				// not used (4).
  {
  80, 25}
  ,				// not used (5);
  {
  80, 25}
  ,				// not used (6);
  {
  80, 25}
  ,				// MONO
  {
  40, 14}
  ,				// C40X14, C40X21, etc.
  {
  40, 21}
  ,
  {
  40, 28}
  ,
  {
  40, 43}
  ,
  {
  40, 50}
  ,
  {
  40, 60}
  ,
  {
  80, 14}
  ,
  {
  80, 21}
  ,
  {
  80, 28}
  ,
  {
  80, 43}
  ,
  {
  80, 50}
  ,
  {
  80, 60}
  ,
  {
  40, 14}
  ,
  {
  40, 21}
  ,
  {
  40, 28}
  ,
  {
  40, 43}
  ,
  {
  40, 50}
  ,
  {
  40, 60}
  ,
  {
  80, 14}
  ,
  {
  80, 21}
  ,
  {
  80, 28}
  ,
  {
  80, 43}
  ,
  {
  80, 50}
  ,
  {
  80, 60}
  ,
  {
  80, 14}
  ,
  {
  80, 21}
  ,
  {
  80, 28}
  ,
  {
  80, 43}
  ,
  {
  80, 50}
  ,
  {
  80, 60}
  ,
  {
  80, 25}
  ,				// not used (38-63)
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 25}
  ,
  {
  80, 50}			// C4350 (64)
};

//------------------------------------------------------------------------------------
// Translation table for the displayed characters.  Change DefaultChar prior to the 
// first use of any conio function, or else it won't take effect.

gint DefaultChar = '.';
gint TranslatedChar[256];

// We initialize the translation table via a function rather than through a 
// static intializer because ACS_xxxxx aren't really constants.
static void
InitializeTranslatedChar (void)
{
  int i;
  // Hit most of the characters first with some simple defaults.
  for (i = 0; i < 0x20; i++)
    TranslatedChar[i] = DefaultChar;
  for (; i < 0x7f; i++)
    TranslatedChar[i] = i;
  for (; i <= 0xff; i++)
    TranslatedChar[i] = DefaultChar;
  // Now fix the ones that are different from the simple defaults.
  // Many of these are not exact matches, but they're the best that can be 
  // done under the circumstances.  At least, it should be possible to 
  // make a decent line-drawing, except that all double-lines have been
  // replaced by single-lines.
#ifndef DoNotFixIntegers
#undef short
#undef int
#undef unsigned
#undef long
#endif
  TranslatedChar[(int) '\t'] = '\t';
  TranslatedChar[(int) '\r'] = '\r';
  TranslatedChar[(int) '\n'] = '\n';
  TranslatedChar[(int) 0x10] = ACS_RARROW;
  TranslatedChar[(int) 0x11] = ACS_LARROW;
  TranslatedChar[(int) 0x18] = ACS_UARROW;
  TranslatedChar[(int) 0x19] = ACS_DARROW;
  TranslatedChar[(int) 0x1a] = ACS_RARROW;
  TranslatedChar[(int) 0x1b] = ACS_LARROW;
  TranslatedChar[(int) 0x1e] = ACS_UARROW;
  TranslatedChar[(int) 0x1f] = ACS_DARROW;
  TranslatedChar[(int) 0x04] = ACS_DIAMOND;
  TranslatedChar[(int) 0x9c] = ACS_STERLING;
  TranslatedChar[(int) 0xb0] = ACS_CKBOARD;
  TranslatedChar[(int) 0xb1] = ACS_CKBOARD;
  TranslatedChar[(int) 0xb2] = ACS_CKBOARD;
  TranslatedChar[(int) 0xb3] = ACS_VLINE;
  TranslatedChar[(int) 0xb4] = ACS_RTEE;
  TranslatedChar[(int) 0xb5] = ACS_RTEE;
  TranslatedChar[(int) 0xb6] = ACS_RTEE;
  TranslatedChar[(int) 0xb7] = ACS_URCORNER;
  TranslatedChar[(int) 0xb8] = ACS_URCORNER;
  TranslatedChar[(int) 0xb9] = ACS_RTEE;
  TranslatedChar[(int) 0xba] = ACS_VLINE;
  TranslatedChar[(int) 0xbb] = ACS_URCORNER;
  TranslatedChar[(int) 0xbc] = ACS_LRCORNER;
  TranslatedChar[(int) 0xbd] = ACS_LRCORNER;
  TranslatedChar[(int) 0xbe] = ACS_LRCORNER;
  TranslatedChar[(int) 0xbf] = ACS_URCORNER;
  TranslatedChar[(int) 0xc0] = ACS_LLCORNER;
  TranslatedChar[(int) 0xc1] = ACS_BTEE;
  TranslatedChar[(int) 0xc2] = ACS_TTEE;
  TranslatedChar[(int) 0xc3] = ACS_LTEE;
  TranslatedChar[(int) 0xc4] = ACS_HLINE;
  TranslatedChar[(int) 0xc5] = ACS_PLUS;
  TranslatedChar[(int) 0xc6] = ACS_LTEE;
  TranslatedChar[(int) 0xc7] = ACS_LTEE;
  TranslatedChar[(int) 0xc8] = ACS_LLCORNER;
  TranslatedChar[(int) 0xc9] = ACS_ULCORNER;
  TranslatedChar[(int) 0xca] = ACS_BTEE;
  TranslatedChar[(int) 0xcb] = ACS_TTEE;
  TranslatedChar[(int) 0xcc] = ACS_LTEE;
  TranslatedChar[(int) 0xcd] = ACS_HLINE;
  TranslatedChar[(int) 0xce] = ACS_PLUS;
  TranslatedChar[(int) 0xcf] = ACS_BTEE;
  TranslatedChar[(int) 0xd0] = ACS_BTEE;
  TranslatedChar[(int) 0xd1] = ACS_TTEE;
  TranslatedChar[(int) 0xd2] = ACS_TTEE;
  TranslatedChar[(int) 0xd3] = ACS_LLCORNER;
  TranslatedChar[(int) 0xd4] = ACS_LLCORNER;
  TranslatedChar[(int) 0xd5] = ACS_ULCORNER;
  TranslatedChar[(int) 0xd6] = ACS_ULCORNER;
  TranslatedChar[(int) 0xd7] = ACS_PLUS;
  TranslatedChar[(int) 0xd8] = ACS_PLUS;
  TranslatedChar[(int) 0xd9] = ACS_LRCORNER;
  TranslatedChar[(int) 0xda] = ACS_ULCORNER;
  TranslatedChar[(int) 0xdb] = ACS_CKBOARD;
  TranslatedChar[(int) 0xe3] = ACS_PI;
  TranslatedChar[(int) 0xf1] = ACS_PLMINUS;
  TranslatedChar[(int) 0xf2] = ACS_GEQUAL;
  TranslatedChar[(int) 0xf3] = ACS_LEQUAL;
  TranslatedChar[(int) 0xf8] = ACS_DEGREE;
  TranslatedChar[(int) 0xf9] = ACS_BULLET;
#ifndef DoNotFixIntegers
#define short int16_t
#define int int16_t
#define unsigned uint16_t
#define long int32_t
#endif
};

//----------------------------------------------------------------------------

void
textmode (int newmode)
{
  int32_t Rows, Columns;
  // Parse the current and desired modes.
  if (ConioInitialized)
    {
      if (CurrentWindow != NULL)
	{
	  if (CurrentWindow != stdscr)
	    delwin (CurrentWindow);
	  CurrentWindow = NULL;
	}
      endwin ();
      // Reset the terminal type.
      //system ("reset");
      ConioInitialized = 0;
    }
  if (newmode == EXITMODE)
    {
      return;
    }
  if (newmode == LASTMODE)
    newmode = LastMode;
  if (0xffff == (newmode & 0xffff))
    {
      Rows = 0xff & (newmode >> 24);
      Columns = 0xff & (newmode >> 16);
    }
  else
    {
      if (newmode < 0 || newmode >= NUM_VIDEO_MODES)
	newmode = C80;
      Rows = VideoModes[newmode].Rows;
      Columns = VideoModes[newmode].Columns;
    }

#ifdef OBSOLETE_TERM_RESIZE
  if (NULL == getenv ("TERM") || 0 != strcasecmp ("xterm", getenv ("TERM")))
    BypassResizeXterm = 1;
  if (BypassResizeXterm == 0)
    {
      char s[32];
      // The following resizes the physical console, but it only works on
      // xterm (not on KDE Konsole, for example).  If xterm is not being 
      // run it causes a fairly chunky delay until it times out.
      sprintf (s, "xtermset -geometry %dx%d", Columns, Rows);
      system (s);
    }
#else // OBSOLETE_TERM_RESIZE
  RawResizeTurboC (Rows, Columns);
#endif // OBSOLETE_TERM_RESIZE

  // ncurses initialization.
  //sprintf (s, "LINES=%d", Rows);
  //putenv (s);
  //sprintf (s, "COLUMNS=%d", Columns);
  //putenv (s);
  //use_env (TRUE);
  //system ("tset pcmw");
  initscr ();
  start_color ();
  //resizeterm (Rows, Columns);
  nodelay (stdscr, TRUE);
  scrollok (stdscr, TRUE);
  // cbreak (); 
  raw ();
  noecho ();
  nonl ();
  intrflush (stdscr, FALSE);
  keypad (stdscr, TRUE);
  wresize (stdscr, Rows, Columns);
  // Initialize the translation table.
  if (TranslatedChar[(int) ' '] == 0)
    InitializeTranslatedChar ();

  // conio initialization.
  CurrentAttributes.currmode = LastMode = newmode;
  CurrentAttributes.winleft = CurrentAttributes.curx = 1;
  CurrentAttributes.winright = CurrentAttributes.screenwidth = Columns;
  CurrentAttributes.wintop = CurrentAttributes.cury = 1;
  CurrentAttributes.winbottom = CurrentAttributes.screenheight = Rows;
  CurrentAttributes.attribute = CurrentAttributes.normattr = WHITE;
  clear ();

  CurrentWindow = stdscr;
  ColorPairsUsed = 1;
  ConioInitialized = 1;
  window (CurrentAttributes.winleft, CurrentAttributes.wintop,
  	  CurrentAttributes.winbottom, CurrentAttributes.winright);
  TurboTrap ();
#ifdef SIGWINCH
  signal (SIGWINCH, ResizeTurboC);
#endif // SIGWINCH
}
