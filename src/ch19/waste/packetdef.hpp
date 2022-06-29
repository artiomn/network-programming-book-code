/*
WASTE - xferwnd.hpp (Packet definitions)
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

#ifndef _PACKETDEF_H_
#define _PACKETDEF_H_

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)
#pragma pack(push,1)
#endif

union _InitialPacket1
{
	unsigned char raw[CONNECTION_BFPUBKEY+CONNECTION_PKEYHASHSIZE];
	struct
	{
		unsigned char sRand[CONNECTION_BFPUBKEY];
		unsigned char sPubkeyHash[CONNECTION_PKEYHASHSIZE];
	};
};

union _InitialPacket2
{
	unsigned char raw[CONNECTION_PADSKEYSIZE];
	struct
	{
		unsigned char sPubCrypted[CONNECTION_PADSKEYSIZE];//will contain _Keyinfo
	};
};

union _InitialPacket2_PSK
{
	unsigned char raw[CONNECTION_PADSKEYSIZE+CONNECTION_BFPUBKEY];
	struct
	{
		unsigned char sPubCrypted[CONNECTION_PADSKEYSIZE];//will contain _Keyinfo
		unsigned char sRand[CONNECTION_BFPUBKEY]; //remote srand for quick mac
	};
};


union _Keyinfo //should not interfere with packing
{
	unsigned char raw[CONNECTION_KEYSIZE+8+CONNECTION_BFPUBKEY];
	struct
	{
		unsigned char sKey[CONNECTION_KEYSIZE];
		unsigned long sIV[2];
		unsigned char sRand[CONNECTION_BFPUBKEY];//This is used as MAC
	};
};

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif

#endif//_PACKETDEF_H_

