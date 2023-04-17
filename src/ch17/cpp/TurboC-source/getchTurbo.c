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

  Filename:	getchTurbo.c
  Purpose:	A Turbo C conio function getch.
  Mod history:	01/31/02 RSB	Created.
  		03/02/02 RSB	Added actual code.
		03/16/02 RSB	In the n2g array, changed the comments 
				from hex to octal, to make it easier'
				to match against ncurses.h.
		03/32/02 RSB	Added KEY_RESIZE/N_RESIZE.
		04/20/02 RSB	Found yet other function-key sequences
				for F1-F5,F10 when running remotely.
		05/09/02 RSB	Allowed for secondary keyboard buffer, for
				getting input from the graphics window.						
*/

#include <stdio.h>
#include "conio.h"
#include "fnkeys.h"

//-----------------------------------------------------------------------
// ncurses returns various special keys as codes in the range 0x100-0x1ff.
// Borland's getch function returns these as 2-code sequences:  
// 0, followed by some other code (from fnkeys.h).  The following table
// converts (where non-zero) from ncurses to the 2nd Borland code.

static const char n2g[] = {
  /* 0400 */ 0, 0, N_DOWN, N_UP, N_LEFT, N_RIGHT, 0, 0,
  /* 0410 */ 0, N_F1, N_F2, N_F3, N_F4, N_F5, N_F6, N_F7,
  /* 0420 */ N_F8, N_F9, N_F10, 0, 0, S_F1, S_F2, S_F3,
  /* 0430 */ S_F4, S_F5, S_F6, S_F7, S_F8, 0, 0, 0,
  /* 0440 */ 0, 0, 0, 0, 0, 0, 0, 0,
  /* 0450 */ 0, 0, 0, 0, 0, 0, 0, 0,
  /* 0460 */ 0, 0, 0, 0, 0, 0, 0, 0,
  /* 0470 */ 0, 0, 0, 0, 0, '=', '/', '*',
  /* 0500 */ N_PADMINUS, N_PADPLUS, 0, 0, 0, 0, 0, 0,
  /* 0510 */ 0, 0, N_DEL, N_INS, 0, 0, 0, 0,
  /* 0520 */ 0, 0, N_PGDN, N_PGUP, 0, 0, 0, '\r',
  /* 0530 */ 0, 0, 0, 0, 0, 0, 0, 0,
  /* 0540 */ 0, 0, 0, 0, 0, 0, 0, 0,
  /* 0550 */ 0, 0, N_HOME, 0, 0, 0, 0, 0,
  /* 0560 */ 0, 0, 0, 0, 0, 0, 0, 0,
  /* 0570 */ 0, 0, 0, 0, 0, 0, 0, '.',
  /* 0600 */ 0, N_END, 0, 0, 0, 0, 0, 0,
  /* 0610 */ 0, 0, 0, 0, 0, 0, 0, 0,
  /* 0620 */ 0, 0, 0, 0, 0, 0, 0, 0
};

// Alternate keys.

static const enum N_NAMES AltAlphabetic[] =
{
  A_A, A_B, A_C, A_D, A_E, A_F, A_G, A_H, A_I, A_J, A_K, A_L, A_M,
  A_N, A_O, A_P, A_Q, A_R, A_S, A_T, A_U, A_V, A_W, A_X, A_Y, A_Z
};

static const enum N_NAMES AltDigits[] =
{
  A_0, A_1, A_2, A_3, A_4, A_5, A_6, A_7, A_8, A_9
};

//-----------------------------------------------------------------------
// A buffer, for ungetching.

#define MAX_UNGETCH 128
static int NumUngetch = 0;
static char UngetchBuffer[MAX_UNGETCH];
FILE *KeyLog = NULL;

//----------------------------------------------------------------------
// Trap numeric keypad digits.
void
KeypadDigit (int *KeyCode)
{
  switch (*KeyCode)
    {
    case 0x188:
      *KeyCode = '0';
      break;
    case 0x182:
      *KeyCode = '1';
      break;
    case 0x18c:
      *KeyCode = '2';
      break;
    case 0x150:
      *KeyCode = '3';
      break;
    case 0x189:
      *KeyCode = '4';
      break;
    case 0x17a:
      *KeyCode = '5';
      break;
    case 0x192:
      *KeyCode = '6';
      break;
    case 0x187:
      *KeyCode = '7';
      break;
    case 0x18e:
      *KeyCode = '8';
      break;
    case 0x151:
      *KeyCode = '9';
      break;
    default:
      break;
    }
}

//-----------------------------------------------------------------------
int
getchTurbo (void)
{
#define MAX_ESCAPE_SEQUENCE 16
  char c;
  int KeyCode;
  int EscapeSequence[MAX_ESCAPE_SEQUENCE], NumEscapeSequence = 0;

  //if (!ConioInitialized)
  //  textmode (LASTMODE);

  // First take care of any ungetched keystrokes, or buffered multi-code 
  // keystrokes like up/down arrows.
  if (NumUngetch)
    return (UngetchBuffer[--NumUngetch]);
  // No buffered codes.  Wait for a real keystroke. 
  for (;;)
    {
      while (kbhit () == 0);
      if (TcExtractKeybuf (&c))
	KeyCode = getchNcurses ();
      else
	KeyCode = c;
      if (KeyLog != NULL)
	fwrite (&KeyCode, sizeof (KeyCode), 1, KeyLog);
      if (KeyCode == KEY_RESIZE)
	continue;
      KeypadDigit (&KeyCode);
      // Trap escape sequences. 
      if (KeyCode == '\x1b')
	{
	  NumEscapeSequence = 0;
	  while (kbhit () != 0 && NumEscapeSequence < MAX_ESCAPE_SEQUENCE)
	    {
	      if (TcExtractKeybuf (&c))
		KeyCode = getchNcurses ();
	      else
		KeyCode = c;
	      if (KeyLog != NULL)
		fwrite (&KeyCode, sizeof (KeyCode), 1, KeyLog);
	      EscapeSequence[NumEscapeSequence++] = KeyCode;
	      if (KeyCode == 0x7e)
		break;
	    }
	  // The interpretation of some of these escape sequences is
	  // more certain than others. Alt-anything seems to pretty
	  // consistently return Esc-anything.  On the other hand,
	  // things like Shift-Fn can be wildly different.  I'm simply 
	  // assuming that the behavior I see on my Linux iMax
	  // is correct and I'll try to work out the inconsistencies
	  // later.  All unrecognized escape sequences are discarded.
	  if (NumEscapeSequence == 1)
	    {
	      KeyCode = EscapeSequence[0];
	      if (KeyCode >= 'A' && KeyCode <= 'Z')
		{
		  ungetchTurbo (AltAlphabetic[KeyCode - 'A']);
		  return (0);
		}
	      if (KeyCode >= 'a' && KeyCode <= 'z')
		{
		  ungetchTurbo (AltAlphabetic[KeyCode - 'a']);
		  return (0);
		}
	      KeypadDigit (&KeyCode);
	      if (KeyCode >= '0' && KeyCode <= '9')
		{
		  ungetchTurbo (AltDigits[KeyCode - '0']);
		  return (0);
		}
	      if (KeyCode == '-' || KeyCode == 0x140)
		{
		  ungetchTurbo (A_MINUS);
		  return (0);
		}
	      if (KeyCode == '=')
		{
		  ungetchTurbo (A_EQUALS);
		  return (0);
		}
	      // Unknown escape code.  Discard it.
	      continue;
	    }

	  // Shifted-function-key escape sequences, as I've seen them
	  // in xterm.
	  if (NumEscapeSequence == 6 && EscapeSequence[0] == 0x5b
	      && EscapeSequence[3] == 0x3b && EscapeSequence[4] == 0x32
	      && EscapeSequence[5] == 0x7e)
	    {
	      if (EscapeSequence[1] == 0x31)
		{
		  if (EscapeSequence[2] == 0x31)
		    {
		      ungetchTurbo (S_F1);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x32)
		    {
		      ungetchTurbo (S_F2);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x33)
		    {
		      ungetchTurbo (S_F3);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x34)
		    {
		      ungetchTurbo (S_F4);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x35)
		    {
		      ungetchTurbo (S_F5);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x37)
		    {
		      ungetchTurbo (S_F6);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x38)
		    {
		      ungetchTurbo (S_F7);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x39)
		    {
		      ungetchTurbo (S_F8);
		      return (0);
		    }
		}
	      else if (EscapeSequence[1] == 0x32)
		{
		  if (EscapeSequence[2] == 0x30)
		    {
		      ungetchTurbo (S_F9);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x31)
		    {
		      ungetchTurbo (S_F10);
		      return (0);
		    }
		}
	    }

	  // Ctrl-PgUp Ctrl-PgDn, as I've seen them in xterm.
	  if (NumEscapeSequence == 5 && EscapeSequence[0] == 0x5b
	      && EscapeSequence[2] == 0x3b && EscapeSequence[3] == 0x35
	      && EscapeSequence[4] == 0x7e)
	    {
	      if (EscapeSequence[2] == 0x35)
		{
		  ungetchTurbo (C_PGUP);
		  return (0);
		}
	      if (EscapeSequence[2] == 0x36)
		{
		  ungetchTurbo (C_PGDN);
		  return (0);
		}
	    }

	  // Ctrl-arrows, as I've seen them in xterm.
	  if (NumEscapeSequence == 3 && EscapeSequence[0] == 0x4f
	      && EscapeSequence[1] == 0x35)
	    {
	      if (EscapeSequence[2] == 0x43)
		{
		  ungetchTurbo (C_RIGHT);
		  return (0);
		}
	      if (EscapeSequence[2] == 0x44)
		{
		  ungetchTurbo (C_LEFT);
		  return (0);
		}
	      if (EscapeSequence[2] == 0x46)
		{
		  ungetchTurbo (C_END);
		  return (0);
		}
	      if (EscapeSequence[2] == 0x48)
		{
		  ungetchTurbo (C_HOME);
		  return (0);
		}
	    }

	  // Some function keys, as I've seen them in KDE Konsole.
	  if (NumEscapeSequence == 2 && EscapeSequence[0] == 0x4f)
	    {
	      if (EscapeSequence[1] == 0x50)
		{
		  ungetchTurbo (N_F1);
		  return (0);
		}
	      if (EscapeSequence[1] == 0x51)
		{
		  ungetchTurbo (N_F2);
		  return (0);
		}
	      if (EscapeSequence[1] == 0x52)
		{
		  ungetchTurbo (N_F3);
		  return (0);
		}
	      if (EscapeSequence[1] == 0x53)
		{
		  ungetchTurbo (N_F4);
		  return (0);
		}
	    }

	  // Some function keys, as I've seen them running remotely.
	  if (NumEscapeSequence == 4 && EscapeSequence[0] == 0x5b
	      && EscapeSequence[3] == 0x7e)
	    {
	      if (EscapeSequence[1] == 0x31)
		{
		  if (EscapeSequence[2] == 0x31)
		    {
		      ungetchTurbo (N_F1);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x32)
		    {
		      ungetchTurbo (N_F2);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x33)
		    {
		      ungetchTurbo (N_F3);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x34)
		    {
		      ungetchTurbo (N_F4);
		      return (0);
		    }
		  if (EscapeSequence[2] == 0x35)
		    {
		      ungetchTurbo (N_F5);
		      return (0);
		    }
		}
	      else if (EscapeSequence[1] == 0x32)
		{
		  if (EscapeSequence[2] == 0x31)
		    {
		      ungetchTurbo (N_F10);
		      return (0);
		    }
		}
	    }

	  // Discard unknown escape sequences.
	  if (NumEscapeSequence > 0)
	    continue;
	}
      // Treat backspace key specially, because it doesn't follow the
      // pattern of being converted to a 2-code sequence.
      if (KeyCode == 0x107)
	KeyCode = C_H;
      // If it's a regular 7-bit ASCII keystroke, we can simply return it
      // right now.  Otherwise, we have to interpret it as a multi-code
      // keystroke and buffer some of it.
      if (KeyCode < 128)
	return (KeyCode);
      if (KeyCode >= 0x100)
	{
	  KeyCode -= 0x100;
	  if (KeyCode < sizeof (n2g) && n2g[KeyCode] != 0)
	    {
	      ungetchTurbo (n2g[KeyCode]);
	      return (0);
	    }
	}
    }
}

//----------------------------------------------------------------------
int
ungetchTurbo (int ch)
{
  if (NumUngetch >= MAX_UNGETCH)
    return (EOF);
  UngetchBuffer[NumUngetch++] = ch;
  return (ch);
}
