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

  Filename:	hex2h.h
  Purpose:	A small utility for transforming the "hex" files
  		exported by xmbdfed to a format useful in a
		C initializer.
  Mod history:	06/08/02 RSB	Created.
  
  Input from stdin and output to stdout.  
*/

#include <stdio.h>

int
main (void)
{
  int Count = 0, CharNum, i;
  char s[257], JustData[257], *ss;
  s[sizeof (s) - 1] = 0;
  while (NULL != fgets (s, sizeof (s) - 1, stdin))
    {
      if (2 != sscanf (s, "%x:%s", &CharNum, JustData))
        {
	BadInputLine:
	  printf ("#error Bad input line.\n");
	  return (1);
	}
      if (CharNum > 255)
        break;	
      for (; Count < CharNum; Count++)
        printf ("  { 0 },\n");
      for (ss = JustData; *ss; )
        {
	  s[0] = *ss++;
	  s[1] = *ss++;
	  s[2] = 0;
	  if (1 != sscanf (s, "%x", &i))
	    goto BadInputLine;
	  if (ss == &JustData[2])
	    printf ("  { ");
	  else
	    printf (", ");    
	  printf ("0x%02X", i); 
	}
      if (Count == 255)
        printf (" }\n");
      else 
        printf (" },\n");		
      if (ss == JustData)
        goto BadInputLine;		
      Count++;	
    }
  if (Count < 256)
    printf ("  { 0 }\n");  
  return (0);
}

