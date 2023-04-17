# TurboC, a library for porting Borland Turbo C to GNU gcc.
# Copyright 2002,2004,2010 Ronald S. Burkey
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 
# Contact Ron Burkey at info@sandroid.org.
#
# Filename:	Makefile
# Purpose:	TurboC is a library that's helpful in converting Borland
#		Turbo C DOS programs to GNU gcc console programs.
# Mod history:	01/27/02 RSB	Created.
#		03/17/02 RSB	Added TurboCChars.
#		03/18/02 RSB	Changed Test.c to Test.cpp, 
#				and fixed -I for Test and TurboCChars
#				so that they can be built prior to
#				putting the headers in with the 
#				system headers.
#		03/19/02 RSB	Now builds both a library with 
#				translated integer types (libTurboC.a)
#				and one without (libTurboCu.a).
#		03/23/02 RSB	gcc replaced by cc to allow potential
#				compilation on Mac OS X.
#		06/02/02 RSB	Made some changes for compiling in BSD.
#		12/16/02 RSB	Now compile the Test.cpp specifically with 
#				c++ rather than cc, because (apparently, from
#				internet posts) there's a bug in gcc 3.x
#				which causes the wrong compiler name to be
#				passed, with an error message about 
#				"__gxx_personality_v0" being missing.  
#				I *hope* this doesn't break other unices.
#		02/14/04 RSB	Moved hex2h.c to Programs, and added it to
#				the utilities.
#		02/26/04 RSB	Added TestCscanf. 
#		08/28/10 RSB	Minor changes to the default targets.
#
# The compiler, $(CC), is always supposed to be gcc.  The name "gcc" works
# in Linux or FreeBSD, while the name "cc" works in Linux, FreeBSD, *and*
# Mac OS X.  

# Use this for Linux.
THREADLIB:=-lpthread
# Use this for FreeBSD.
#THREADLIB:=-lc_r

# Use this to disable graphics.h when X-window system unavailable.
#COPTIONS:=-O0 -g
# Use this when the X-window system is available.
COPTIONS:=-O0 -g -DWITH_X -I/usr/X11R6/include

CC:=cc
CPP:=c++
CSOURCE:=$(wildcard *.c)
CHEADERS:=$(wildcard *.h)
COBJ:=$(CSOURCE:.c=.o)
COBJU:=$(CSOURCE:.c=.ou)

.PHONY: Default
Default: libTurboC.a libTurboCu.a TestGraphics Test TurboCChars GraphicsTest \
	hex2h TestCscanf

.PHONY: all
all:	Default

TurboCChars: Programs/TurboCChars.c libTurboCu.a
	$(CC) $(COPTIONS) -DDoNotFixIntegers \
		-o $@ -I. $< -L. -lTurboCu -lncurses $(THREADLIB)
	strip $@	

TestGraphics: Programs/TestGraphics.c libTurboC.a
	$(CC) $(COPTIONS) \
		-o $@ -I. $< -L. -L/usr/X11R6/lib \
		-lTurboC -lncurses -lX11 $(THREADLIB)

TestCscanf: Programs/TestCscanf.c libTurboC.a
	$(CC) $(COPTIONS) \
		-o $@ -I. $< -L. -L/usr/X11R6/lib \
		-lTurboC -lncurses -lX11 $(THREADLIB)

GraphicsTest: Programs/GraphicsTest.c libTurboC.a
	$(CC) $(COPTIONS) \
		-o $@ -I. $< -L. -L/usr/X11R6/lib \
		-lTurboC -lncurses -lX11 -lm $(THREADLIB)

hex2h:	Programs/hex2h.c
	$(CC) -O3 -o$@ $<

Test:	Programs/Test.cpp libTurboC.a
	$(CPP) $(COPTIONS) -DTEST_TURBOC \
		-o $@ -I. $< -L. -lTurboC -lncurses $(THREADLIB)
	strip $@	

libTurboC.a: $(COBJ)
	ar r $@ $(COBJ)
	ranlib $@
	
libTurboCu.a: $(COBJU)
	ar r $@ $(COBJU)
	ranlib $@
	
clean:
	-rm *.a *.o *.ou Test TurboCChars TestGraphics GraphicsTest \
		hex2h TestCscanf
	
initgraph2.o: initgraph2.c
	$(CC) $(COPTIONS) -Wall -c -funsigned-char \
		`gtk-config --cflags` -o $@ $<	
	
initgraph2.ou: initgraph2.c
	$(CC) $(COPTIONS) -Wall -c -funsigned-char \
		`gtk-config --cflags` \
		-DDoNotFixIntegers -o $@ $<	
	
%.o:	%.c ${CHEADERS} Makefile
	$(CC) $(COPTIONS) -Wall -c -funsigned-char -o $@ $<

%.ou:	%.c ${CHEADERS} Makefile
	$(CC) $(COPTIONS) -DDoNotFixIntegers -Wall -c \
		-funsigned-char -o $@ $<
		
