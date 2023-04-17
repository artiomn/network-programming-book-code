/*
  02/26/04 RSB --- this is a test of the cscanf function that
  Justin Godden has sent me.
*/

#include <conio.h>
#include <fnkeys.h>


#include <stdarg.h>
#include "conio.h"

static char StringBuffer[1024];

int cscanf (const char *format, ...)
{
  int i=0;
  va_list ap;
  if (!ConioInitialized)
    textmode (LASTMODE);
  echo();
  if (wgetstr(CurrentWindow, StringBuffer)==ERR)
return (-1);
  va_start(ap, format);
  i=vsscanf(StringBuffer,format,ap);
  va_end (ap);
  noecho();
  return(i);
}


gint				/* changed from void for *nix */
main (void)
{
  int c, i, j, k;
  textmode (C80);
  window (2, 3, 40, 15);
  clrscr ();
  gotoxy (5, 6);
  cprintf ("> ");
  cscanf ("%hd%hd%hd", &i, &j, &k);
  gotoxy (5, 8);
  cprintf ("%d, %d, %d", i, j, k);
  getch ();
  textmode (EXITMODE);		/* added for *nix */
  return (0);			/* added for *nix */
}

