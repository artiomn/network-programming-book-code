/*
WASTE - sha.hpp (SHA-1 implementation class)
Based on public domain code.
Copyright (C) 2003 Nullsoft, Inc.
Copyright (C) 2004 WASTE Development Team

WASTE is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

WASTE  is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with WASTE; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _SHA_H_
#define _SHA_H_

#define SHA_OUTSIZE 20

//sha

class SHAify {

public:
	SHAify();
	void add(unsigned char *data, unsigned int datalen);
	void final(unsigned char *out);
	void reset();

private:

	unsigned long H[5];
	unsigned long W[80];
	unsigned int lenW;
	unsigned long size[2];
};

#endif//_SHA_H_
