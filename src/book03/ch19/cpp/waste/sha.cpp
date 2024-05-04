/*
WASTE - sha.cpp (SHA-1 implementation class)
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

#include "stdafx.hpp"

#include "platform.hpp"
#include "sha.hpp"

///sha

SHAify::SHAify()
{
	reset();
}

void SHAify::reset()
{
	lenW = 0;
	size[0] = size[1] = 0;
	H[0] = 0x67452301L;
	H[1] = 0xefcdab89L;
	H[2] = 0x98badcfeL;
	H[3] = 0x10325476L;
	H[4] = 0xc3d2e1f0L;
	memset(W,0,sizeof(W));
}

#define SHA_ROTL(X,n) (((X) << (n)) | ((X) >> (32-(n))))
#define SHUFFLE() E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP

void SHAify::add(unsigned char *data, unsigned int datalen)
{
	unsigned int i;
	for (i = 0; i < datalen; i++) {
		W[lenW / 4] <<= 8;
		W[lenW / 4] |= (unsigned long)data[i];
		if (!(++lenW & 63)) {
			unsigned int t;

			unsigned long A = H[0];
			unsigned long B = H[1];
			unsigned long C = H[2];
			unsigned long D = H[3];
			unsigned long E = H[4];

			for (t = 16; t < 80; t++) W[t] = SHA_ROTL(W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16], 1);

			for (t = 0; t < 20; t++) {
				unsigned long TEMP = SHA_ROTL(A,5) + E + W[t] + 0x5a827999L + (((C^D)&B)^D);
				SHUFFLE();
			};
			for (; t < 40; t++) {
				unsigned long TEMP = SHA_ROTL(A,5) + E + W[t] + 0x6ed9eba1L + (B^C^D);
				SHUFFLE();
			};
			for (; t < 60; t++) {
				unsigned long TEMP = SHA_ROTL(A,5) + E + W[t] + 0x8f1bbcdcL + ((B&C)|(D&(B|C)));
				SHUFFLE();
			};
			for (; t < 80; t++) {
				unsigned long TEMP = SHA_ROTL(A,5) + E + W[t] + 0xca62c1d6L + (B^C^D);
				SHUFFLE();
			};

			H[0] += A;
			H[1] += B;
			H[2] += C;
			H[3] += D;
			H[4] += E;

			lenW = 0;
		};
		size[0] += 8;
		if (size[0] < 8) size[1]++;
	};
}

void SHAify::final(unsigned char *out)
{
	unsigned char pad0x80 = 0x80;
	unsigned char pad0x00 = 0x00;
	unsigned char padlen[8];
	unsigned int i;
	padlen[0] = (unsigned char)((size[1] >> 24) & 0xff);
	padlen[1] = (unsigned char)((size[1] >> 16) & 0xff);
	padlen[2] = (unsigned char)((size[1] >> 8) & 0xff);
	padlen[3] = (unsigned char)((size[1]) & 0xff);
	padlen[4] = (unsigned char)((size[0] >> 24) & 0xff);
	padlen[5] = (unsigned char)((size[0] >> 16) & 0xff);
	padlen[6] = (unsigned char)((size[0] >> 8) & 0xff);
	padlen[7] = (unsigned char)((size[0]) & 0xff);
	add(&pad0x80, 1);
	while (lenW != 56) add(&pad0x00, 1);
	add(padlen, 8);
	for (i = 0; i < 20; i++) {
		out[i] = (unsigned char)(H[i / 4] >> 24);
		H[i / 4] <<= 8;
	};
	reset();
}
