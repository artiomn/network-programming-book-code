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

  Filename:	GraphicsTest.c
  Purpose:	A test for graphics.h functions.
  Mod history:	05/01/02 RSB	Created.
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "../graphics.h"

static char pat[8] = { 0x88, 0x88, 0x44, 0x44, 0x22, 0x22, 0x11, 0x11 };

static int poly[] = { 100, 120, 300, 100, 400, 200, 200, 300, 150, 250, 100, 120 };

gint
main (void)
{
  void *Buf;
  int v = 0;
  int Thick = NORM_WIDTH, Style = SOLID_LINE;
  int x = 50, y = 50;
  int GraphicsDriver, GraphicsMode;
  int i, j, k;
  struct viewporttype Viewport;
  TurboTrap ();
  GraphicsDriver = DETECT;
  initgraph (&GraphicsDriver, &GraphicsMode, "\\tc");
  for (i = 0; i < 16; i++)
    {
      for (j = 0; j < 50; j++)
        putpixel (j, 4 * i, i);
      k = getpixel (0, 4 * i);
      if (i != k)
        printf ("Color %d was read back as %d.\n", i, k);
      for (j += 10; j < 110; j++)
        putpixel (j, 4 * i, k);	
    }
  for (i = 0; i < 8; i++)
    {
      char s[33];
      for (j = 0; j < 32; j++)
        s[j] = j + 32 * i;
      s[32] = 0;	
      setcolor (13);  
      settextstyle (DEFAULT_FONT, HORIZ_DIR, 1); 
      outtextxy (10, 70 + i * 10, s);  
      setcolor (14);  
      settextstyle (DEFAULT_FONT, HORIZ_DIR, 2); 
      outtextxy (10, 150 + i * 20, s);  
      setcolor (15);  
      settextstyle (DEFAULT_FONT, HORIZ_DIR, 3); 
      outtextxy (10, 310 + i * 30, s);  
    }  
  setcolor (12);  
  settextjustify (RIGHT_TEXT, TOP_TEXT);
  settextstyle (DEFAULT_FONT, VERT_DIR, 3); 
  outtextxy (50, 50, "Hello, I am text.");  
  getch ();
  closegraph ();
  GraphicsDriver = VGA;
  GraphicsMode = VGAMED;
  initgraph (&GraphicsDriver, &GraphicsMode, "\\tc");

  setviewport (50, 50, getmaxx () - 50, getmaxy () - 50, 1);
  getviewsettings (&Viewport);

  setactivepage (0);
  setcolor (7);
  rectangle (50, 50, Viewport.right - 100, Viewport.bottom - 100);
  for (i = 0; i < 16; i++)
    {
      setcolor (i);
      line (0, 0 + 6 * i, Viewport.right - Viewport.left,
	    Viewport.bottom - Viewport.top - 6 * i);
      line (0, Viewport.bottom - Viewport.top - 6 * i,
	    Viewport.right - Viewport.left, 0 + 6 * i);
    }
  setrgbpalette (0, 128, 192, 255);
  setcolor (0);
  line (0, (Viewport.bottom - Viewport.top) / 2,
	(Viewport.right - Viewport.left),
	(Viewport.bottom - Viewport.top) / 2);

  setviewport (0, 0, getmaxx (), getmaxy (), 1);

  setactivepage (1);
  line (10, 10, getmaxx () - 10, 10);
  line (getmaxx () - 10, 10, getmaxx () - 10, getmaxy () - 10);
  line (getmaxx () - 10, getmaxy () - 10, 10, getmaxy () - 10);
  line (10, getmaxy () - 10, 10, 10);
  for (i = 0; i < 16; i++)
    for (j = 0; j < 9; j++)
      for (k = 0; k < 9; k++)
        putpixel (20 + j + i * 10, 20 + k, i);
  setcolor (15);	
  setlinestyle (SOLID_LINE, 0, NORM_WIDTH);
  line (20, 40, 100, 40);	
  setlinestyle (DOTTED_LINE, 0, NORM_WIDTH);
  line (20, 50, 100, 50);	
  setlinestyle (CENTER_LINE, 0, NORM_WIDTH);
  line (20, 60, 100, 60);	
  setlinestyle (DASHED_LINE, 0, NORM_WIDTH);
  line (20, 70, 100, 70);	
  setlinestyle (USERBIT_LINE, 0x3c3c, NORM_WIDTH);
  line (20, 80, 100, 80);	
  setlinestyle (SOLID_LINE, 0, THICK_WIDTH);
  line (20, 90, 100, 90);	
  setlinestyle (DOTTED_LINE, 0, THICK_WIDTH);
  line (20, 100, 100, 100);	
  setlinestyle (CENTER_LINE, 0, THICK_WIDTH);
  line (20, 110, 100, 110);	
  setlinestyle (DASHED_LINE, 0, THICK_WIDTH);
  line (20, 120, 100, 120);	
  setlinestyle (USERBIT_LINE, 0x3c3c, THICK_WIDTH);
  line (20, 130, 100, 130);	
  setcolor (0);
  arc (getmaxx () / 4, getmaxy () / 2, 135, 225, getmaxy () / 6);
  arc ((3 * getmaxx ()) / 4, getmaxy () / 2, 45, -45, getmaxy () / 6);
  circle (getmaxx () / 2, getmaxy () / 2, getmaxy () / 2 - 10);
  setfillstyle (XHATCH_FILL, 13);
  setcolor (11);
  sector (getmaxx () / 2, getmaxy () / 2, 45, 315, getmaxy () / 2 - 10,
	       getmaxy () / 4);
  //ellipse (getmaxx () / 2, getmaxy () / 2, 45, 315, getmaxy () / 2 - 10,
  //	       getmaxy () / 4);
  setcolor (0);	       
  setrgbpalette (0, 192, 192, 129);
  setfillstyle (EMPTY_FILL, 0);
  pieslice (getmaxx () / 2, getmaxy () / 2, 0, 45, getmaxy () / 6);
  
  drawpoly (6, poly);

  setactivepage (0); 
  
  setcolor (LIGHTRED);
  setlinestyle (CENTER_LINE, 0, THICK_WIDTH);
  
  setfillstyle (XHATCH_FILL, 13);
  fillpoly (6, poly);
  line (0, 0, getmaxx (), getmaxy ());
  //drawpoly (6, poly);

  setcolor (0);
  setlinestyle (SOLID_LINE, 0, NORM_WIDTH);

  i = imagesize (x, y, x + 20, y + 20);
  Buf = malloc (i);
	    
  while (1)
    { 
      line (x, y - 10, x, y + 10);
      line (x - 10, y, x + 10, y);
      i = toupper (getch ());
      line (x, y - 10, x, y + 10);
      line (x - 10, y, x + 10, y);
      switch (i)
	{
	case 27:
	  goto Done;
	case 'B':
	  setrgbpalette (0, 0, 0, 0);
	  for (i = 0; i <= USER_FILL; i++)
	    {
	      if (i == USER_FILL)
	        setfillpattern (pat, i);
	      else 
	        setfillstyle (i, i);
	      bar (0, 25 * i, 200, 25 * (i + 1) - 3);		
	    }
	  setcolor (15);
	  setactivepage (1);
	  bar3d (20, 20, 320, 70, 6, 0);
	  bar3d (20, 120, 320, 170, 6, 1);
	  bar3d (20, 220, 320, 270, 0, 0);
	  setcolor (0);  
	  setrgbpalette (0, 192, 192, 129);  
	  setactivepage (0);
	  break;  
	case '0':
	  setvisualpage (0);
	  break;
	case '1':
	  setvisualpage (1);
	  break;
	case 'X':
	  setwritemode (XOR_PUT);
	  break;
	case 'C':
	  setwritemode (COPY_PUT);
	  break;
	case 'N':
	  y -= 10;
	  break;
	case 'S':
	  y += 10;
	  break;
	case 'R':
	  {
	    getimage (x, y, x + 20, y + 20, Buf);
	    putimage (x + 50, y, Buf, COPY_PUT);
	  }
	  break;
	case 'W':
	  x -= 10;
	  break;
	case 'E':
	  x += 10;
	  break;    
	case 'T':
	  Thick ^= 2;
	  setlinestyle (Style, 0, Thick);
	  break;
	case 'L':
	  Style++;
	  if (Style > USERBIT_LINE)
	    Style = 0;
	  setlinestyle (Style, 0, Thick);
	  break;  
	case 'K':
	  cleardevice ();
	  break;
	case 'Q':
	  clearviewport ();
	  break;
	case 'V':
	  v = !v;
	  if (v)
	    setviewport (200, 200, 300, 300, 1);
	  else
	    setviewport (0, 0, getmaxx (), getmaxy (), 0);
	  break;        
	default:
	  printf (" %02X ", i);
	  fflush (stdout);
	  break;
	}
    }
Done:
  restorecrtmode ();
  printf ("Hello, I am fine, how are you?\n");
  getchar ();
  setgraphmode (VGAHI);
  line (0, 0, getmaxx (), getmaxy ());
  getch ();
  closegraph ();
  printf ("And again ...\n");
  getchar ();
  return (0);
}
