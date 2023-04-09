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

  Filename:	getimage.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/23/02 RSB	Created.
  		05/24/02 RSB	Completely rethought and replaced the
				means for associating Pixmap handles
				with image buffers.  In particular, the
				freeimage function has been dropped.
  
  
			WARNING!  WARNING!  WARNING!
			----------------------------
  
  It's very important to understand that though these functions work 
  according to Borland's documentation, it's very likely that they will
  not actually work as the original programmer intended.  I suspect that
  when used in most program they may result in segfaults, lost connections 
  with the X-server, and whatnot.
  
  The problem is that Borland's docs don't specify the format of the data
  in the graphics buffer used by these functions (except to note that the
  first two words contain the width and height).  But to have performed 
  most *useful* operations -- such as saving or loading an image to/from
  a file in a standard graphics format -- the programmer would have had
  to actually manipulate the data within the graphics buffer, and would
  therefore have hacked the format.  This was actually pretty easy to do,
  since it was generally the same as the formats supported in hardware
  by the graphics controller, and so it's pretty likely a lot of guys
  would have done it.
  
  In the case of the TurboC library, on the other hand, there is 
  NO RELATIONSHIP WHATEVER between the graphics buffer and those old
  hardware-dependent formats.  In fact, the image buffer does not even
  contain graphical data at all; rather, it merely contains the handle
  of a Pixmap structure maintain by the X-window system elsewhere.
  Thus, it is not only impossible to manipulate the image buffer, it is 
  even impossible to save the buffer as a file and to reload it later.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

extern unsigned
imagesize (int left, int top, int right, int bottom)
{
  return (sizeof (struct TcImageBuffer));
}

//----------------------------------------------------------------------------

extern void
getimage (int left, int top, int right, int bottom, void *bitmap)
{
#define Buffer ((struct TcImageBuffer *) bitmap)
  Pixmap Handle;
  int OldWritemode;
  int Left, Top, Right, Bottom, Clip;
  Buffer->Width = right - left + 1;
  Buffer->Height = bottom - top + 1;
  Handle = XCreatePixmap (TcDisplay,
			  DefaultRootWindow (TcDisplay),
			  Buffer->Width, Buffer->Height,
			  DefaultDepth (TcDisplay, TcScreen));
  AssociatePixmap (bitmap, Handle);
  Buffer->Handle = Handle;
  OldWritemode = TcWritemode;
  if (OldWritemode != COPY_PUT)
    setwritemode (COPY_PUT);
  if (left > right)
    {
      Left = left;
      left = right;
      right = Left;
    }
  if (top > bottom)
    {
      Top = top;
      top = bottom;
      bottom = Top;
    }
  Left = TcViewLeft;
  Top = TcViewTop;
  Right = TcViewRight;
  Bottom = TcViewBottom;
  Clip = TcViewClip;
  setviewport (0, 0, TcXresolution - 1, TcYresolution - 1, 0);
  // Now do the actual drawing.
  XLockDisplay (TcDisplay);
  XCopyArea (TcDisplay, TcWindow, Buffer->Handle,
	     TcGc, left, top, Buffer->Width, Buffer->Height, 0, 0);
  XSync (TcDisplay, False);
  XUnlockDisplay (TcDisplay);
  setviewport (Left, Top, Right, Bottom, Clip);
  if (OldWritemode != TcWritemode)
    setwritemode (OldWritemode);
#undef Buffer
}

//----------------------------------------------------------------------------

extern void
putimage (int left, int top, void *bitmap, int op)
{
#define Buffer ((struct TcImageBuffer *) bitmap)
  int OldWritemode;
  int Left, Top, Right, Bottom, Clip;
  if (bitmap == NULL)
    return;
  OldWritemode = TcWritemode;
  if (OldWritemode != op)
    setwritemode (op);
  Left = TcViewLeft;
  Top = TcViewTop;
  Right = TcViewRight;
  Bottom = TcViewBottom;
  Clip = TcViewClip;
  setviewport (0, 0, TcXresolution - 1, TcYresolution - 1, 0);
  // Now do the actual drawing.
  XLockDisplay (TcDisplay);
  XCopyArea (TcDisplay, Buffer->Handle, TcPixmaps[TcVisualPage],
	     TcGc, 0, 0, Buffer->Width, Buffer->Height, left, top);
  XCopyArea (TcDisplay, Buffer->Handle, TcWindow,
	     TcGc, 0, 0, Buffer->Width, Buffer->Height, left, top);
  XSync (TcDisplay, False);
  XUnlockDisplay (TcDisplay);
  setviewport (Left, Top, Right, Bottom, Clip);
  if (OldWritemode != TcWritemode)
    setwritemode (OldWritemode);
#undef Buffer
}

#endif // WITH_X
