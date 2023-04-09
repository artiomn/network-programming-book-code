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

  Filename:	kbhit.c
  Purpose:	A Turbo C conio function.
  Mod history:	03/02/02 RSB	Created.
  		05/09/02 RSB	Added a secondary FIFO for getting
				keyboard input from the graphics window.
*/

#include <pthread.h>
#include "graphics.h"
#include "conio.h"

//-----------------------------------------------------------------------
// Secondary FIFO, for keyboard stuff from graphics window.

// Add to graphics keyboard buffer, returning 0 on success.
int
TcAddKeybuf (char c)
{
  int RetVal = 1, i;
  pthread_mutex_lock (&TcMutex);
  if (TcKeybufSize < TC_KEYBUF_SIZE)
    {
      RetVal = 0;
      i = TcKeybufStart + TcKeybufSize;
      if (i >= TC_KEYBUF_SIZE)
	i -= TC_KEYBUF_SIZE;
      TcKeybuf[i] = c;
      TcKeybufSize++;
    }
  pthread_mutex_unlock (&TcMutex);
  return (RetVal);
}

// Extract a character from the keyboard buffer, returning 0 on success.
int
TcExtractKeybuf (char *c)
{
  int RetVal = 1;
  pthread_mutex_lock (&TcMutex);
  if (TcKeybufSize > 0)
    {
      RetVal = 0;
      *c = TcKeybuf[TcKeybufStart];
      TcKeybufSize--;
      TcKeybufStart++;
      if (TcKeybufStart >= TC_KEYBUF_SIZE)
	TcKeybufStart = 0;
    }
  pthread_mutex_unlock (&TcMutex);
  return (RetVal);
}

//-----------------------------------------------------------------------
int
kbhit (void)
{
  if (TcGraphicsInitialized)
    {
      if (TcKeybufSize > 0)
	return (1);
    }
  if (ConioInitialized)
    {
      int KeyCode;
      KeyCode = getchNcurses ();
      if (KeyCode != ERR)
	{
	  ungetchNcurses (KeyCode);
	  return (1);
	}
    }
  return (0);
}
