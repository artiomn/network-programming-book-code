#include <conio.h>
#include <fnkeys.h>

gint				/* changed from void for *nix */
main (void)
{
  int c, i;
  textmode (C80);
  clrscr ();
  cprintf ("\r\n");
  for (c = 0; c <= 255; c++)
    {
      if (0 == (c % 8))
	cprintf ("\r\n  ");
      if (c == 8 || c == 7 || c == 10 || c == 9 || c == 13)
	i = '.';
      else
	i = c;
      if (i == 0)
	{
	  // Have to handle this case separately, since the %c in the cprintf
	  // used for the other branches terminates the string.  This is a bug
	  // in my handling of cprintf, and doesn't occur in actual Turbo C.
	  cprintf ("%02X ", c);
	  putch (i);
	  cprintf ("    ");
	}
      else if (0 == (c & 8))
	cprintf ("%02X %c    ", c, i);
      else
	cprintf ("%02X  %c   ", c, i);
      if (c == 31)
	c = 127;
    }
  getch ();
#if 0
  i = 1;
  while (1)
    {
      c = getch ();
      if (c)
	break;
      c = getch ();
      if (c != N_RESIZE)
	break;
      gotoxy (1, 1);
      cprintf ("Resize #%d, LINES=%d, COLS=%d  ", i++, LINES, COLS);
    }
#endif // 0
  textmode (EXITMODE);		/* added for *nix */
  return (0);			/* added for *nix */
}

//---------------------------------------------------------------------------
// Callback function for a user screen-resize.

void
ConioResizeCallback (void)
{
  static int i = 1;
  gotoxy (1, 1);
  cprintf ("Resize #%d, LINES=%d, COLS=%d", i++, LINES, COLS);
  clreol ();
}
