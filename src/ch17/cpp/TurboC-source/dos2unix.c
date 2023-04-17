/*
  TurboC, a library for porting Borland Turbo C to GNU gcc.
  The entire contents of this source file is 
  Copyright (c) Igor Bujna 1999-2001.
 
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
  Contact Igor Bujna <igor.bujna@maxi-tip.cz>

  Filename:	dos2unix.c
  Purpose:	Various TurboC library functions contributed by 
  		Igor Bujna.
  Mod history:	05/19/02 RSB	Integrated Igor's contributions to the
  				existing TurboC library code.
		12/16/02 RSB	getdate changed to getdateTurbo, in order to
				coexist with the getdate in newer system
				libraries.
				
  This file contains a lot of miscellaneous functions, appropriate to 
  a variety of headers (io.h, dos.h, bios.h), but I (RSB) felt it best and 
  least confusing to keep them together as Igor	delivered them to me.
  The associated header file, on the other hand, has been split up and joined
  with io.h, dos.h, etc.  Otherwise, minimal editing has been performed --
  for example, to get the integer types to correspond to TurboC integer types.
  The month and day names have been commented out not because they're Czech,
  but because they don't seem to be used by the code.  Similarly, the 
  sequence_STAR_34X function has been removed because I'm not entirely sure
  what it's for.  (I mean, I'm sure it translates the control codes for
  some specific type of printer -- perhaps one of the Micronics line --
  but I'm not sure how to document it or whether it's of any general use.)			
*/

#ifdef __linux__
// The following apparent constants are macros containing casts that will
// fail if TurboC's automatic integer-type translation is used.
#include <sys/ioctl.h>
#include <linux/ppdev.h>
int pprstatus = PPRSTATUS;
int pprelease = PPRELEASE;
int ppclaim = PPCLAIM;
int ppwdata = PPWDATA;
int ppfcontrol = PPFCONTROL;
#endif // __linux__
#include "io.h"
#include "dos.h"
#include "bios.h"

#ifndef ESC_KEY
#define ESC_KEY		27
#define TAB_KEY		9
#define CTRL_X		24
#define RETURN		13
#define BACKSPACE	8
#endif

#ifndef N_C
#define N_C	'\0'		/* Definice NULL-teho znaku */
#endif

#ifndef TRUE
#define FALSE	0
#define TRUE  !FALSE
#endif

/*
 * Date and Time function
 */
struct date date_null = { 0, 0, 0 };
struct time time_null = { 0, 0, 0, 0 };

#if 0
char *month[] =
  { "---", "Led", "Uno", "Bre", "Dub", "Kve", "Cer", "Cec", "Srp", "Zar",
  "Rij", "Lis", "Pr",
};
char *month_l[] =
  { "---", "Leden", "Unor", "Brezen", "Duben", "Kveten", "Cerven",
  "Cervenec", "Srpen", "Zari", "Rijen", "Listopad", "Prosinec",
};

char *day[] = { "Ned", "Pon", "Ute", "Str", "Ctv", "Pat", "Sob", };
char *day_l[] =
  { "Nedele", "Pondeli", "Utery", "Streda", "Ctvrtek", "Patek", "Sobota", };
#endif // 0

time_t
dostounix (struct date *d, struct time *t)
{
  struct tm utm, *ptm;
  time_t the_time = time (0);

  ptm = localtime (&the_time);
  memset (&utm, N_C, sizeof (struct tm));
  memcpy ((void *) &utm, (void *) ptm, sizeof (struct tm));

  utm.tm_sec = t->ti_sec;
  utm.tm_min = t->ti_min;
  utm.tm_hour = t->ti_hour;
  utm.tm_mday = d->da_day;
  utm.tm_mon = d->da_mon - 1;
  utm.tm_year = d->da_year - 1900;

  return (mktime (&utm));
}

void
unixtodos (time_t the_time, struct date *d, struct time *t)
{
  struct tm utm, *ptm;

  memset (&utm, N_C, sizeof (struct tm));
  ptm = localtime (&the_time);
  memcpy ((void *) &utm, (void *) ptm, sizeof (struct tm));

  t->ti_sec = utm.tm_sec;
  t->ti_hund = 0;
  t->ti_min = utm.tm_min;
  t->ti_hour = utm.tm_hour;
  d->da_day = utm.tm_mday;
  d->da_mon = utm.tm_mon + 1;
  d->da_year = utm.tm_year + 1900;
}

void
unixdos_time (time_t the_time, struct time *t)
{
  struct date d;

  memset (&d, N_C, sizeof (struct date));
  unixtodos (the_time, &d, t);
}

void
unixdos_date (time_t the_time, struct date *d)
{
  struct time t;

  memset (&t, N_C, sizeof (struct time));
  unixtodos (the_time, d, &t);
}

void
getdateTurbo (struct date *d)
{
  unixdos_date (time (0), d);
}

void
gettime_d (struct time *t)
{
  unixdos_time (time (0), t);
}

/*
 * Print function (Biosprint ...)
 * Value of return is status of printer
 * cmd
 */
#ifdef __linux__
int
open_printer (int port)
{
  int fd, mode = IEEE1284_MODE_COMPAT;
  char pom[50];

  switch (port)
    {
    case LPT1:
      sprintf (pom, "/dev/parport0");
      break;
    case LPT2:
      sprintf (pom, "/dev/parport1");
      break;
    case LPT3:
      sprintf (pom, "/dev/parport2");
      break;
    case TO_FILE:
      strcpy (pom, PRINT_FILE);
      break;
    }

  fd = open (pom, O_RDWR);
  if (fd != -1 && ioctl (fd, PPCLAIM))
    {
      close (fd);
      fd = -1;
    }

  if (ioctl (fd, PPNEGOT, &mode))
    {
      close (fd);
      fd = -1;
    }

  return (fd);
}


ssize_t
write_printer (int fd, const char *buf)
{
  ssize_t wrote = 0;
  int len = strlen (buf);

//write(fd, buf, sizeof(char) + strlen(buf));

  while (wrote < len)
    {
      guchar status, data;
      guchar mask = (PARPORT_STATUS_ERROR | PARPORT_STATUS_BUSY);
      guchar val = (PARPORT_STATUS_ERROR | PARPORT_STATUS_BUSY);
      struct ppdev_frob_struct frob;
      struct timespec ts;

      /* Wait for printer to be ready */
      for (;;)
	{
	  ioctl (fd, pprstatus, &status);

	  if ((status & mask) == val)
	    break;

	  ioctl (fd, pprelease);
	  /* Delay for a bit */
	  ts.tv_sec = 0;
	  ts.tv_nsec = 1;
//    nanosleep (&ts, NULL);
	  ioctl (fd, ppclaim);
	}

      /* Set the data lines */
      data = (guchar) buf[wrote];
      ioctl (fd, ppwdata, &data);

      /* Delay for a bit */
      ts.tv_sec = 0;
      ts.tv_nsec = 1000;
//    nanosleep (&ts, NULL);

      /* Pulse strobe */
//    if(data == '\n')
      frob.mask = PARPORT_CONTROL_STROBE;
      frob.val = PARPORT_CONTROL_STROBE;
      ioctl (fd, ppfcontrol, &frob);
//    nanosleep (&ts, NULL);

      /* End the pulse */
      frob.val = 0;
      ioctl (fd, ppfcontrol, &frob);
//    nanosleep (&ts, NULL);

      wrote++;
    }
  return wrote;
}
#endif

int
biosprint (int cmd, int byte, int port)
{
  int ret = 144;

#ifdef __linux__
  int fd;
  char pom[10];
  guchar status;

  switch (cmd)
    {
    case 1:
      break;
    case 2:
      fd = open_printer (port);
      ioctl (fd, pprstatus, &status);
      ret = (int) status;
      close_printer (fd);
      ret = 144;
      break;
    default:
      fd = open_printer (port);
      sprintf (pom, "%c%c", byte, N_C);
      write_printer (fd, pom);
//      write (fd, &byte, sizeof(int) );
      close_printer (fd);
/*	  f = fopen (PRINT_FILE, "a+t");
	  fprintf (f, "%c", byte);
	  fclose (f);
*/ break;
    }
#endif /* __linux__    */

#ifdef __CYGWIN__
  FILE *f;

  if (cmd != 0)
    return (ret);
  f = fopen (PRINT_FILE, "a+t");
  fprintf (f, "%c", byte);
  fclose (f);

#endif /* __CYGWIN__   */
  return (ret);
}

void
flush_print (char *buf, int port)
{
  guint i;
  FILE *f;

  if (port == TO_FILE)
    {
      f = fopen (PRINT_FILE, "a+t");
      fprintf (f, "%s", buf);
      fclose (f);
    }
  else
    {
      for (i = 0; i < strlen (buf); i++)
	biosprint (0, buf[i], port);
    }
}

void
print_file (const char *file, int port, int lowlevel_print)
{
/*  int ch;*/
  FILE *f;
  char *text, pom[1024 + 1];

  if (lowlevel_print == TRUE)
    {

      if ((f = fopen (file, "rt")) == NULL)
	return;

      if ((text = (char *) malloc (20 + 1)) == NULL)
	return;

/*  while ((ch = fgetc (f)) != EOF)
	biosprint (0, ch, port);*/
      while (fgets (text, 20, f) != NULL)
	flush_print (text, port);

      free (text);
      fclose (f);
    }
  else
    {
#ifdef __DJGPP__
      sprintf (pom, "type %s > lpt%1.1i%c", file, port, N_C);
#endif
#ifdef __CYGWIN__
      sprintf (pom, "cat %s > lpt%1.1i &%c", file, port, N_C);
#endif
#ifdef __linux__
#endif
      system (pom);
    }
}

#if 0

char *
seqence_STAR_34X (int typ)
{
  static char put[15];

  memset (put, N_C, sizeof (put));

  switch (typ)
    {
    case NEXT_LINE:		/*Dalsi radek */
      sprintf (put, "\n%c", N_C);
      break;
    case CUTTING:		/*Odstrihnuti papiru */
      sprintf (put, "%s\n\n\n\n\n%cd0%c", seqence_STAR_34X (LINE_1_8_INCH),
	       ESC_KEY, N_C);
      break;
    case FONT_7_9:		/*Font 7x9 (half dot)  | Male pismo */
      sprintf (put, "%cM%c", ESC_KEY, N_C);
      break;
    case FONT_5_9_2P:		/*Font 5x9 (2 pulse +1 dot) | Stredni pismo */
      sprintf (put, "%cP%c", ESC_KEY, N_C);
      break;
    case FONT_5_9_3P:		/*Font 5x9 (3 pulse +1 dot) | Velke pismo */
      sprintf (put, "%c:%c", ESC_KEY, N_C);
      break;
    case EXPANDED_MODE:	/* Select Expanded mode-rozsireny (2x vic nez normal) */
      /*sprintf (put, "%c%c", 14, N_C);  Stary typ */
      sprintf (put, "%cW1%c", ESC_KEY, N_C);
      break;
    case CANCEL_EXPANDED_MODE:	/* CANCEL Expanded mode-rozsireny */
      /*sprintf (put, "%c%c", 20, N_C);      Stary typ */
      sprintf (put, "%cW0%c", ESC_KEY, N_C);
      break;
    case EMPHASIZED_MODE:	/* Select emphasized mode (zduraznene) */
      sprintf (put, "%cE%c", ESC_KEY, N_C);
      break;
    case CANCEL_EMPHASIZED_MODE:	/* Cancel emphasized mode */
      sprintf (put, "%cF%c", ESC_KEY, N_C);
      break;
    case UNDERLINE_MODE:	/* Select underline mode (podtrzene) */
      sprintf (put, "%c-1%c", ESC_KEY, N_C);
      break;
    case CANCEL_UNDERLINE_MODE:	/* Cancel underline mode */
      sprintf (put, "%c-0%c", ESC_KEY, N_C);
      break;
    case UPPERLINE_MODE:	/* Select upperline mode (horni index) */
      sprintf (put, "%c_1%c", ESC_KEY, N_C);
      break;
    case CANCEL_UPPERLINE_MODE:	/* Cancel upperline mode */
      sprintf (put, "%c_0%c", ESC_KEY, N_C);
      break;
    case HIGHLIGHT_MODE:	/* Select highlight mode (zvyraznene) */
      sprintf (put, "%c4%c", ESC_KEY, N_C);
      break;
    case CANCEL_HIGHLIGHT_MODE:	/* Cancel highlight mode */
      sprintf (put, "%c5%c", ESC_KEY, N_C);
      break;
    case INVERTED_MODE:	/* Select inverted mode (obraceny) */
      sprintf (put, "%c%c", 15, N_C);
      break;
    case CANCEL_INVERTED_MODE:	/* Cancel inverted mode */
      sprintf (put, "%c%c", 18, N_C);
      break;
    case IBM_CHARSET_1:	/* Select IBM charset mode #1 */
      sprintf (put, "%c7%c", ESC_KEY, N_C);
      break;
    case IBM_CHARSET_2:	/* Select IBM charset mode #2 */
      sprintf (put, "%c6%c", ESC_KEY, N_C);
      break;
    case SOUND:		/* Sound - buzzer (zapipani) */
      sprintf (put, "%c%c", 30, N_C);
      break;
    case INITIALIZE_PRINTER_CLEAN:	/* Cancel print data in buffer & initialize
					   printer */
      sprintf (put, "%c%c", 24, N_C);
      break;
    case DESELECT_MODE:	/* Set deselect mode (zruseni voleb) */
      sprintf (put, "%c%c", 19, N_C);
      break;
    case SELECT_MODE:		/* Set select mode (zruseni voleb) */
      sprintf (put, "%c%c", 17, N_C);
      break;
    case UNIDIRECTIONAL_MODE:	/* Select uni-directional mode (jednosmerny tisk) */
      sprintf (put, "%cU1%c", ESC_KEY, N_C);
      break;
    case BIDIRECTIONAL_MODE:	/* Select bi-directional mode (obousmerny tisk) */
      sprintf (put, "%cU0%c", ESC_KEY, N_C);
      break;
    case INITIALIZE_PRINTER:	/* Initialize printer */
      sprintf (put, "%c@%c", ESC_KEY, N_C);
      break;
    case LINE_1_8_INCH:	/* Velikost mezery mezi radky je 1/8 palce (0.167mm) */
      sprintf (put, "%cz1%c", ESC_KEY, N_C);
      break;
    case LINE_1_6_INCH:	/* Velikost mezery mezi radky je 1/6 palce (0.125mm) */
      sprintf (put, "%c0%c", ESC_KEY, N_C);
      break;
    }

  return (put);
}

#endif // 0
