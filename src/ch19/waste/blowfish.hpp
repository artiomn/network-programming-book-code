/*
WASTE - blowfish.hpp (Blowfish cipher)
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

/*
Current class and templates written by Md5Chap
*/

#ifndef _BLOWFISH_H
#define _BLOWFISH_H

struct _BLOWFISH_CTX
{
	unsigned long P[16+2];
	unsigned long S[4*256];
};

class CBlowfish
{
	public:
		enum _eIV
		{
			IV_BOTH,
			IV_ENC,
			IV_DEC
		};
		CBlowfish();
		CBlowfish(void* key, unsigned int len);
		~CBlowfish();
		void Init(void* key, unsigned int len);
		void SetIV(_eIV which, unsigned long IV[2]);
		void SetIV(_eIV which, unsigned long left, unsigned long right);
		void GetIV(_eIV which, unsigned long IV[2]);
		void GetIV(_eIV which, unsigned long &left, unsigned long &right);
		void Final();
		void EncryptECB(void *data, unsigned int len);
		void DecryptECB(void *data, unsigned int len);
		void EncryptCBC(void *data, unsigned int len);
		void DecryptCBC(void *data, unsigned int len);
		void EncryptPCBC(void *data, unsigned int len);
		void DecryptPCBC(void *data, unsigned int len);
	private:
		bool m_bInited;
		unsigned long m_IV_Enc[2];
		unsigned long m_IV_Dec[2];
		_BLOWFISH_CTX m_ctx;
};

#endif//_BLOWFISH_H
