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

  Filename:	initgraph.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	04/29/02 RSB	Created.
  		05/06/02 RSB	... lotsa continuing work.
		05/10/02 RSB	... more of the same.
		05/21/02 RSB	Added fixes for manual closing of
				graphics window.
		05/22/02 RSB	Now uses detectgraph.		
*/

#ifdef WITH_X

#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
static pthread_t ExposerThread;
static int ThreadsInitialized = 0;
static void *ExposerFunction (void *);

#include "graphics.h"
#include "conio.h"

//----------------------------------------------------------------------------

extern void
setgraphmode (int mode)
{
  int graphdriver;
  graphdriver = TcDriver;
  initgraph (&graphdriver, &mode, "");
}

//----------------------------------------------------------------------------
// Borland Turbo C relies on loading a "graphics driver" ("*.BGI" files,
// which are like a plug-in) at runtime.  The pathtodriver variable is 
// supposed to specify the directory in which these driver files are located.  
// The TurboC library, on the other hand, doesn't rely on the "driver" concept, 
// and so the pathtodriver variables and graphdriver are unused.
// The TurboC library will allow any graphics mode (graphmode) to be applied
// to any "driver".

extern void
initgraph (int *graphdriver, int *graphmode, char *pathtodriver)
{
  XSizeHints *SizeHints;
  int i, Xres, Yres, Pcnt, Pal;
  gulong White, Black;
  Atom wm_delete_window;
  if (!ThreadsInitialized)
    {
      XInitThreads ();
      ThreadsInitialized = 1;
    }
  if (TcGraphicsInitialized)
    closegraph ();
  // An alternate display name might allow the program to be run remotely.  
  // Since the conio.h functionality of the TurboC library (implemented 
  // in ncurses) is accessible remotely, it would be consistent to implement 
  // this.  If/when I figure this out, I'll fix it up.
  if (*graphdriver == 0)
    detectgraph (graphdriver, graphmode);
  TcDisplay = XOpenDisplay (NULL);
  if (TcDisplay == NULL)
    {
      TcGraphResult = *graphdriver = grNotDetected;
      return;
    }
  TcScreen = DefaultScreen (TcDisplay);
  switch ((*graphmode) % TCX)
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
      XCloseDisplay (TcDisplay);
      TcGraphResult = *graphdriver = grInvalidMode;
      return;
    }
  switch ((*graphmode / TCX) % TCY)
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
      XCloseDisplay (TcDisplay);
      TcGraphResult = *graphdriver = grInvalidMode;
      return;
    }
  switch ((*graphmode / TCXY) % TCG)
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
      XCloseDisplay (TcDisplay);
      TcGraphResult = *graphdriver = grInvalidMode;
      return;
    }
  White = WhitePixel (TcDisplay, TcScreen);
  Black = BlackPixel (TcDisplay, TcScreen);
  SizeHints = XAllocSizeHints ();
  SizeHints->flags = PMinSize | PMaxSize | PBaseSize;
  SizeHints->min_width = Xres;
  SizeHints->max_width = Xres;
  SizeHints->base_width = Xres;
  SizeHints->min_height = Yres;
  SizeHints->max_height = Yres;
  SizeHints->base_height = Yres;
  TcWindow = XCreateSimpleWindow (TcDisplay,
				  DefaultRootWindow (TcDisplay),
				  0, 0, Xres, Yres, 0, White, Black);
  // Beg the window manager not to allow the window to be resized.                                
  XSetNormalHints (TcDisplay, TcWindow, SizeHints);
  // Tell the window manager we don't want the graphics window 
  // arbitrarily closed on us -- we want a signal instead.
  wm_delete_window = XInternAtom (TcDisplay, "WM_DELETE_WINDOW", False);
  XSetWMProtocols (TcDisplay, TcWindow, &wm_delete_window, 1);
  TcGc = XCreateGC (TcDisplay, TcWindow, 0, 0);
  XSetArcMode (TcDisplay, TcGc, ArcPieSlice);
  XSetForeground (TcDisplay, TcGc, Black);
  for (i = 0; i < Pcnt; i++)
    {
      TcPixmaps[i] = XCreatePixmap (TcDisplay,
				    DefaultRootWindow (TcDisplay),
				    Xres, Yres,
				    DefaultDepth (TcDisplay, TcScreen));
      XFillRectangle (TcDisplay, TcPixmaps[i], TcGc, 0, 0, Xres, Yres);
    }
  XFree (SizeHints);
  // Save what minimal info we need about the palette.
  Pal = (*graphmode / TCXYG) % TCP;
  // Set up the default colors in the palette.  
  if (TcDefaultColors (Pal))
    {
      for (i = 0; i < Pcnt; i++)
	XFreePixmap (TcDisplay, TcPixmaps[i]);
      XDestroyWindow (TcDisplay, TcWindow);
      XFreeGC (TcDisplay, TcGc);
      XCloseDisplay (TcDisplay);
      TcGraphResult = *graphdriver = grInvalidMode;
      return;
    }
  TcColormap = DefaultColormap (TcDisplay, TcScreen);
  XMapWindow (TcDisplay, TcWindow);
  XSync (TcDisplay, False);
  TcDriver = *graphdriver;
  TcCurrentGraphMode = *graphmode;
  TcXresolution = Xres;
  TcYresolution = Yres;
  TcPageCount = Pcnt;
  TcVisualPage = TcActivePage = 0;
  TcPaletteNum = Pal;
  TcBackgroundColor = 0;
  TcTile = XCreatePixmap (TcDisplay,
			  DefaultRootWindow (TcDisplay),
			  8, 8, DefaultDepth (TcDisplay, TcScreen));
  TcGraphDefaults ();			  
  TcGraphicsInitialized = 1;
  if (ThreadsInitialized == 1)
    {
      ThreadsInitialized = 2;
      if (pthread_create (&ExposerThread, NULL, ExposerFunction, NULL))
	{
	  TcGraphResult = *graphdriver = grNoLoadMem;
	  return;
	}
    }
}

//-----------------------------------------------------------------

int volatile TcShutdownGraphics = 0;

static void *
ExposerFunction (void *DummyArg)
{
  KeySym Key;
  char Text[32];
  XEvent Event;
  int i, j;
  XSelectInput (TcDisplay, TcWindow,
		ExposureMask | KeyPressMask | SubstructureNotifyMask);
  while (1)
    {
      // When the main thread wants to turn off graphics mode, it 
      // sets TcShutdownGraphics to 1 and waits until it turns
      // back to 0 before proceeding.
      if (TcShutdownGraphics)
	{
	  ThreadsInitialized = 0;
	  TcShutdownGraphics = 0;
	  TcGraphicsInitialized = 0;
	  return (NULL);
	}
      XLockDisplay (TcDisplay);
      if (XCheckWindowEvent
	  (TcDisplay, TcWindow, ExposureMask | KeyPressMask |
	   SubstructureNotifyMask, &Event))
	switch (Event.type)
	  {
	  case Expose:
	    // Come here after something moves in front of our window
	    // so that we need to redraw it.
	    XCopyArea (TcDisplay, TcPixmaps[TcVisualPage], TcWindow,
		       TcGc, Event.xexpose.x, Event.xexpose.y,
		       Event.xexpose.width, Event.xexpose.height,
		       Event.xexpose.x, Event.xexpose.y);
	    break;
	  case KeyPress:
	    // This needs to be fixed up some to accomodate things
	    // like arrows and function keys, but it works okay for 
	    // regular keys as-is.
	    i = XLookupString (&Event.xkey, Text, sizeof (Text), &Key, NULL);
	    for (j = 0; j < i; j++)
	      TcAddKeybuf (Text[j]);
	    break;
	  case NoExpose:
	    break;
	  case ClientMessage:
	    // Come here if the user tries to manually close the graphics
	    // window.  We allow the programmer to decide what to do
	    // about it.  The callback function TurboX can be provided
	    // by the programmer, but we also provide a default version that
	    // just returns 1.  A return value of 0 means to continue 
	    // executing, while a non-zero value means to close down the 
	    // program.
	    if (TurboX (0))
	      {
		XUnlockDisplay (TcDisplay);
		//closegraph ();
		//kill (0, SIGTERM);
		ThreadsInitialized = 0;
		TcGraphicsInitialized = 0;
		//return (NULL);
		exit (2);
	      }
	    break;
	  default:
	    break;
	  }
      XUnlockDisplay (TcDisplay);
    }
}

#endif // WITH_X
