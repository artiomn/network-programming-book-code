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

  Filename:	Test.cpp
  Purpose:	A test program for TurboC.  It's not really a test 
  		program so much as something where I look at whatever
		feature I'm currently adding.  It's not as if you can 
		run this program and come back with a warm-fuzzy about
		the operation of libTurboC.  Perhaps I'll create something
		like that later.
  Mod history:	01/31/02 RSB	Created.
  		03/16/02 RSB	Added mode for checking special keys.
		... ongoing.
		05/19/02 RSB	Added some stuff for testing clock/delay.
*/

#ifdef TEST_TURBOC

#include "conio.h"
#include "fnkeys.h"
#include "dir.h"
#include "dos.h"
#include "io.h"

#ifdef __libTurboC__
gint
#else
void
#endif
main (void)
{
  guint milliseconds;
  int Handle;
  struct ftime ft;
  struct ffblk fblk;
  uint16_t Buffer[20][40];
  int i, j;
  gint k;
  char s[11], *ss;
  short sRed, sGreen, sBlue, sFore, sBack;
  for (;;)
    {
      printf ("-----\nEnter time: ");
      scanf ("%u", &milliseconds);
      if (milliseconds == 0)
        break;
      printf ("%u %u\n", clock (), CLK_TCK);
      delay (milliseconds);
      printf ("%u %u\n", clock (), CLK_TCK);	
    }
  textmode (C80X50);
#if 0
  Handle = open ("Test", O_RDONLY);
  getftime (Handle, &ft);
  cprintf ("%02d/%02d/%04d %02d:%02d:%02d ",
	   ft.ft_month, ft.ft_day, ft.ft_year + 1980,
	   ft.ft_hour, ft.ft_min, ft.ft_tsec * 2);
  close (Handle);
  getch ();
#endif // 0
#if 0
  window (1, 1, 10, 5);
  _setcursortype (_NOCURSOR);
  for (i = 0; i != 'X';)
    i = getche ();
  _wscroll = 0;
  window (1, 6, 10, 10);
  _setcursortype (_NORMALCURSOR);
  for (i = 0; i != 'X';)
    i = getche ();
  _wscroll = 1;
  window (1, 1, 80, 50);
  _setcursortype (_SOLIDCURSOR);
  cputs ("Up to 8 chars: ");
  s[0] = 9;
  cgets (s);
  cprintf ("\r\n%d chars, \"%s\"\r\n", s[1], &s[2]);
  ss = getpass ("Password: ");
  cprintf ("\r\n\"%s\"\r\n", ss);
  getch ();
#endif // 0
#if 0
  cprintf ("Hidden\r\n");
  for (i = findfirst ("*", &fblk, FA_HIDDEN); i == 0; i = findnext (&fblk))
    cprintf ("%ld bytes, %02X, %02d/%02d/%04d %02d:%02d:%02d, %s\r\n",
	     fblk.ff_fsize, fblk.ff_attrib,
	     ((fblk.ff_fdate & 0x1e0) >> 5),
	     (fblk.ff_fdate & 0x1f),
	     ((fblk.ff_fdate & 0xfe00) >> 9) + 1980,
	     (fblk.ff_ftime & 0xf800) >> 11,
	     (fblk.ff_ftime & 0x7e0) >> 5,
	     (fblk.ff_ftime & 0x1f) * 2, fblk.ff_name);
  getch ();
  cprintf ("Hidden directories\r\n");
  for (i = findfirst ("*", &fblk, FA_HIDDEN | FA_DIREC); i == 0;
       i = findnext (&fblk))
    cprintf ("%ld bytes, %02X, %02d/%02d/%04d %02d:%02d:%02d, %s\r\n",
	     fblk.ff_fsize, fblk.ff_attrib, ((fblk.ff_fdate & 0x1e0) >> 5),
	     (fblk.ff_fdate & 0x1f), ((fblk.ff_fdate & 0xfe00) >> 9) + 1980,
	     (fblk.ff_ftime & 0xf800) >> 11, (fblk.ff_ftime & 0x7e0) >> 5,
	     (fblk.ff_ftime & 0x1f) * 2, fblk.ff_name);
  getch ();
  cprintf ("Directories\r\n");
  for (i = findfirst ("*", &fblk, FA_DIREC); i == 0; i = findnext (&fblk))
    cprintf ("%ld bytes, %02X, %02d/%02d/%04d %02d:%02d:%02d, %s\r\n",
	     fblk.ff_fsize, fblk.ff_attrib,
	     ((fblk.ff_fdate & 0x1e0) >> 5),
	     (fblk.ff_fdate & 0x1f),
	     ((fblk.ff_fdate & 0xfe00) >> 9) + 1980,
	     (fblk.ff_ftime & 0xf800) >> 11,
	     (fblk.ff_ftime & 0x7e0) >> 5,
	     (fblk.ff_ftime & 0x1f) * 2, fblk.ff_name);
  getch ();
  cprintf ("Regular\r\n");
  for (i = findfirst ("*", &fblk, 0); i == 0; i = findnext (&fblk))
    cprintf ("%ld bytes, %02X, %02d/%02d/%04d %02d:%02d:%02d, %s\r\n",
	     fblk.ff_fsize, fblk.ff_attrib,
	     ((fblk.ff_fdate & 0x1e0) >> 5),
	     (fblk.ff_fdate & 0x1f),
	     ((fblk.ff_fdate & 0xfe00) >> 9) + 1980,
	     (fblk.ff_ftime & 0xf800) >> 11,
	     (fblk.ff_ftime & 0x7e0) >> 5,
	     (fblk.ff_ftime & 0x1f) * 2, fblk.ff_name);
  getch ();
#endif // 0
#if 0
  k = ACS_ULCORNER;
  mvwaddch (stdscr, 0, 0, k);
  addch (ACS_HLINE);
  addch (ACS_TTEE);
  addch (ACS_HLINE);
  addch (ACS_URCORNER);
  //addch ('\r');
  addch ('\n');
  addch (ACS_VLINE);
  addch (' ');
  addch (ACS_VLINE);
  addch (' ');
  addch (ACS_VLINE);
  //addch ('\r');
  addch ('\n');
  addch (ACS_LTEE);
  addch (ACS_HLINE);
  addch (ACS_PLUS);
  addch (ACS_HLINE);
  addch (ACS_RTEE);
  //addch ('\r');
  addch ('\n');
  addch (ACS_VLINE);
  addch (' ');
  addch (ACS_VLINE);
  addch (' ');
  addch (ACS_VLINE);
  //addch ('\r');
  addch ('\n');
  addch (ACS_LLCORNER);
  addch (ACS_HLINE);
  addch (ACS_BTEE);
  addch (ACS_HLINE);
  addch (ACS_LRCORNER);
  addch ('\n');
  addch (ACS_S1);
  addch (ACS_S3);
  addch (ACS_S7);
  addch (ACS_S9);
  addch (ACS_DIAMOND);
  addch (ACS_CKBOARD);
  addch (ACS_DEGREE);
  addch (ACS_PLMINUS);
  addch (ACS_BULLET);
  addch (ACS_LARROW);
  addch (ACS_HLINE);
  addch (' ');
  addch (ACS_HLINE);
  addch (ACS_RARROW);
  addch ('\n');
  addch (ACS_UARROW);
  addch (ACS_VLINE);
  addch ('\n');
  addch (ACS_VLINE);
  addch (ACS_DARROW);
  addch ('\n');
  addch (ACS_LANTERN);
  addch (ACS_BLOCK);
  addch (ACS_LEQUAL);
  addch (ACS_GEQUAL);
  addch (ACS_PI);
  addch (ACS_NEQUAL);
  addch (ACS_STERLING);

  getch ();
#endif // 0
  for (i = 0; i < 50; i++)
    cprintf ("My name is Sam, and this is line #%d.\r\n", i);
  textcolor (LIGHTGREEN);
  gotoxy (10, 10);
  cputs ("Sam I am");
  window (20, 20, 50, 40);
  for (i = 0; i < 21; i++)
    cprintf ("%d:  Hello, chum!\r\n", i);
  window (40, 1, 80, 30);
  textbackground (GREEN);
  clrscr ();
  for (j = 0; j < 16; j++)
    {
      textbackground (j);
      for (i = 0; i < 16; i++)
	{
	  textcolor (i);
	  clreol ();
	  cprintf ("%d,%d: Hello, you evil scumbag from hell!\r\n", j, i);
	}
      //getchar ();  
    }
  window (1, 1, 80, 43);
  textcolor (BLACK);
  textbackground (LIGHTGRAY);
  movetext (5, 4, 25, 15, 10, 28);
  gettext (20, 2, 59, 21, Buffer);
#if 0
  for (i = 0; i < 20; i++)
    {
      gotoxy (1, i + 22);
      for (j = 0; j < 40; j++)
	{
	  textattr (Buffer[i][j] >> 8);
	  putch (Buffer[i][j] & 0xff);
	}
      putch (' ');
    }
#else
  puttext (1, 22, 40, 41, Buffer);
#endif
  gotoxy (5, 15);
  delline ();
  delline ();
  gotoxy (5, 17);
  insline ();
  do
    i = getchNcurses ();
  while (i < ' ' || i >= 127);
  if (i == 'N')
    {
      for (i = 0; i != 'X';)
	{
	  i = getchNcurses ();
	  if (i != ERR)
	    cprintf ("%08X  ", i);
	}
    }
  else
    {
      for (i = 0; i != 'X';)
	{
	  i = getch ();
	  cprintf ("%02X ", i);
	  if (i == 0)
	    {
	      i = getch ();
	      cprintf ("%02X ", i);
	      switch (i)
		{
		case A_A:
		  cprintf ("Alt-A");
		  break;
		case A_B:
		  cprintf ("Alt-B");
		  break;
		case A_C:
		  cprintf ("Alt-C");
		  break;
		case A_D:
		  cprintf ("Alt-D");
		  break;
		case A_E:
		  cprintf ("Alt-E");
		  break;
		case A_F:
		  cprintf ("Alt-F");
		  break;
		case A_G:
		  cprintf ("Alt-G");
		  break;
		case A_H:
		  cprintf ("Alt-H");
		  break;
		case A_I:
		  cprintf ("Alt-I");
		  break;
		case A_J:
		  cprintf ("Alt-J");
		  break;
		case A_K:
		  cprintf ("Alt-K");
		  break;
		case A_L:
		  cprintf ("Alt-L");
		  break;
		case A_M:
		  cprintf ("Alt-M");
		  break;
		case A_N:
		  cprintf ("Alt-N");
		  break;
		case A_O:
		  cprintf ("Alt-O");
		  break;
		case A_P:
		  cprintf ("Alt-P");
		  break;
		case A_Q:
		  cprintf ("Alt-Q");
		  break;
		case A_R:
		  cprintf ("Alt-R");
		  break;
		case A_S:
		  cprintf ("Alt-S");
		  break;
		case A_T:
		  cprintf ("Alt-T");
		  break;
		case A_U:
		  cprintf ("Alt-U");
		  break;
		case A_V:
		  cprintf ("Alt-V");
		  break;
		case A_W:
		  cprintf ("Alt-W");
		  break;
		case A_X:
		  cprintf ("Alt-X");
		  break;
		case A_Y:
		  cprintf ("Alt-Y");
		  break;
		case A_Z:
		  cprintf ("Alt-Z");
		  break;
		case N_F1:
		  cprintf ("F1");
		  break;
		case N_F2:
		  cprintf ("F2");
		  break;
		case N_F3:
		  cprintf ("F3");
		  break;
		case N_F4:
		  cprintf ("F4");
		  break;
		case N_F5:
		  cprintf ("F5");
		  break;
		case N_F6:
		  cprintf ("F6");
		  break;
		case N_F7:
		  cprintf ("F7");
		  break;
		case N_F8:
		  cprintf ("F8");
		  break;
		case N_F9:
		  cprintf ("F9");
		  break;
		case N_F10:
		  cprintf ("F10");
		  break;
		case N_HOME:
		  cprintf ("home");
		  break;
		case N_UP:
		  cprintf ("up");
		  break;
		case N_PGUP:
		  cprintf ("PgUp");
		  break;
		case N_PADMINUS:
		  cprintf ("Pad-");
		  break;
		case N_LEFT:
		  cprintf ("left");
		  break;
		case N_RIGHT:
		  cprintf ("right");
		  break;
		case N_PADPLUS:
		  cprintf ("Pad+");
		  break;
		case N_END:
		  cprintf ("end");
		  break;
		case N_DOWN:
		  cprintf ("down");
		  break;
		case N_PGDN:
		  cprintf ("PgDn");
		  break;
		case N_INS:
		  cprintf ("ins");
		  break;
		case N_DEL:
		  cprintf ("del");
		  break;
		case S_F1:
		  cprintf ("Shift-F1");
		  break;
		case S_F2:
		  cprintf ("Shift-F2");
		  break;
		case S_F3:
		  cprintf ("Shift-F3");
		  break;
		case S_F4:
		  cprintf ("Shift-F4");
		  break;
		case S_F5:
		  cprintf ("Shift-F5");
		  break;
		case S_F6:
		  cprintf ("Shift-F6");
		  break;
		case S_F7:
		  cprintf ("Shift-F7");
		  break;
		case S_F8:
		  cprintf ("Shift-F8");
		  break;
		case S_F9:
		  cprintf ("Shift-F9");
		  break;
		case S_F10:
		  cprintf ("Shift-F10");
		  break;
		case C_F1:
		  cprintf ("Ctrl-F1");
		  break;
		case C_F2:
		  cprintf ("Ctrl-F2");
		  break;
		case C_F3:
		  cprintf ("Ctrl-F3");
		  break;
		case C_F4:
		  cprintf ("Ctrl-F4");
		  break;
		case C_F5:
		  cprintf ("Ctrl-F5");
		  break;
		case C_F6:
		  cprintf ("Ctrl-F6");
		  break;
		case C_F7:
		  cprintf ("Ctrl-F7");
		  break;
		case C_F8:
		  cprintf ("Ctrl-F8");
		  break;
		case C_F9:
		  cprintf ("Ctrl-F9");
		  break;
		case C_F10:
		  cprintf ("Ctrl-F10");
		  break;
		case A_F1:
		  cprintf ("Alt-F1");
		  break;
		case A_F2:
		  cprintf ("Alt-F2");
		  break;
		case A_F3:
		  cprintf ("Alt-F3");
		  break;
		case A_F4:
		  cprintf ("Alt-F4");
		  break;
		case A_F5:
		  cprintf ("Alt-F5");
		  break;
		case A_F6:
		  cprintf ("Alt-F6");
		  break;
		case A_F7:
		  cprintf ("Alt-F7");
		  break;
		case A_F8:
		  cprintf ("Alt-F8");
		  break;
		case A_F9:
		  cprintf ("Alt-F9");
		  break;
		case A_F10:
		  cprintf ("Alt-F10");
		  break;
		case C_LEFT:
		  cprintf ("Ctrl-left");
		  break;
		case C_RIGHT:
		  cprintf ("Ctrl-right");
		  break;
		case C_END:
		  cprintf ("Ctrl-end");
		  break;
		case C_PGDN:
		  cprintf ("Ctrl-PgDn");
		  break;
		case C_HOME:
		  cprintf ("Ctrl-home");
		  break;
		case A_0:
		  cprintf ("Alt-0");
		  break;
		case A_1:
		  cprintf ("Alt-1");
		  break;
		case A_2:
		  cprintf ("Alt-2");
		  break;
		case A_3:
		  cprintf ("Alt-3");
		  break;
		case A_4:
		  cprintf ("Alt-4");
		  break;
		case A_5:
		  cprintf ("Alt-5");
		  break;
		case A_6:
		  cprintf ("Alt-6");
		  break;
		case A_7:
		  cprintf ("Alt-7");
		  break;
		case A_8:
		  cprintf ("Alt-8");
		  break;
		case A_9:
		  cprintf ("Alt-9");
		  break;
		case A_MINUS:
		  cprintf ("Alt-minus");
		  break;
		case A_EQUALS:
		  cprintf ("Alt-equals");
		  break;
		case C_PGUP:
		  cprintf ("Ctrl-PgUp");
		  break;
		}
	    }
	  else
	    {
	      if (i > ' ' && i <= 126)
		cprintf ("%c", i);
	      else
		cprintf ("unprintable");
	    }
	  cprintf ("\r\n");
	}
    }
#ifdef __libTurboC__
  textmode (EXITMODE);
  return (0);
#endif
}

#endif /* TEST_TURBOC */
