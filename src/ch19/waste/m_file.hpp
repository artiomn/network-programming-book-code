/*
WASTE - m_file.hpp (File transfer message builder/parser class)
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

#ifndef _C_MFILE_H_
#define _C_MFILE_H_

#include "shbuf.hpp"
#include "util.hpp"

#define FILE_CHUNKSIZE 4096
#define FILE_MAX_CHUNKS_PER_REQ 64

class C_FileSendRequest
{
public:
	C_FileSendRequest();
	C_FileSendRequest(C_SHBuf *in);
	~C_FileSendRequest();

	C_SHBuf *Make();

	//host guid
	T_GUID *get_guid() { return &m_guid; }
	void set_guid(T_GUID *guid) { m_guid=*guid; }

	//prev search guid if any
	T_GUID *get_prev_guid() { return &m_prev_guid; }
	void set_prev_guid(T_GUID *guid) { m_prev_guid=*guid; }

	//abort
	void set_abort(int abort) { m_abort=abort; }
	int is_abort() { return m_abort; }

	void set_idx(int idx) { m_idx=idx; }
	int get_idx() { return m_idx; }

	void set_fn_hash(unsigned char fnhash[SHA_OUTSIZE]);
	void get_fn_hash(unsigned char fnhash[SHA_OUTSIZE]);

	void set_dc_ipport(unsigned long ip, unsigned short port)
	{
		m_ip=ip;
		m_port=port;
	};
	void get_dc_ipport(unsigned long *ip, unsigned short *port)
	{
		*port=m_port;
		*ip=m_ip;
	};

	void set_nick(const char *nick);
	const char *get_nick() { return m_nick; };

	bool get_bIsInitialRequest() { return bIsInitialRequest; };

	void clear_need_chunks()
	{
		m_need_chunks_used=0;
		m_need_chunks_indexused=0;
	};
	void add_need_chunk(unsigned int idx);

	int get_chunks_needed() { return m_need_chunks_used; }
	unsigned int get_need_chunk(int idx) //only valid on the decompress side (otherwise returns RLE crap)
	{
		return m_need_chunks_offs[idx];
	};

protected:
	int m_abort;
	T_GUID m_guid,m_prev_guid;
	int m_idx;
	unsigned long m_ip;
	unsigned short m_port;
	unsigned char m_fnhash[SHA_OUTSIZE];
	char m_nick[32];
	bool bIsInitialRequest;

	//on write side, this stores some nifty RLE stuff.
	//on the read side, it is decompressed on the fly into m_need_chunks_offs and m_need_chunks_len is not used
	unsigned int m_need_chunks_offs[FILE_MAX_CHUNKS_PER_REQ];
	char m_need_chunks_len[FILE_MAX_CHUNKS_PER_REQ];

	int m_need_chunks_used;
	int m_need_chunks_indexused;
};

class C_FileSendReply
{
public:
	C_FileSendReply();
	C_FileSendReply(C_SHBuf *in);
	~C_FileSendReply();

	C_SHBuf *Make();

	void set_error(int err) { m_error=err; }
	int get_error() { return m_error; }

	void set_index(unsigned int index) { m_index=index; }
	int get_index() { return m_index; } //-1 if header

	//header only fields
	void get_hash(unsigned char hash[SHA_OUTSIZE]) { memcpy(hash,m_hash,SHA_OUTSIZE); }
	void set_hash(unsigned char hash[SHA_OUTSIZE]) { memcpy(m_hash,hash,SHA_OUTSIZE); }
	void get_file_len(unsigned int *low, unsigned int *high) { *low = m_file_len_low; *high=m_file_len_high; }
	void set_file_len(unsigned int file_len_low, unsigned int file_len_high);

	void set_file_dates(unsigned int create_date, unsigned int mod_date) { m_create_date=create_date; m_mod_date = mod_date; }
	void get_file_dates(unsigned int *create_date, unsigned int *mod_date) { *create_date=m_create_date; *mod_date=m_mod_date; }

	void set_chunkcount(int cnt) { m_chunkcnt=cnt; }
	int get_chunkcount() { return m_chunkcnt; }

	void set_dc_ipport(unsigned long ip, unsigned short port)
	{
		m_ip=ip;
		m_port=port;
	};

	void get_dc_ipport(unsigned long *ip, unsigned short *port)
	{
		*port=m_port;
		*ip=m_ip;
	};

	void set_nick(const char *nick);
	const char *get_nick() { return m_nick; };

	//data only fields
	void set_data(unsigned char *buf, int len);
	unsigned char *get_data() { return m_data; }
	int get_data_len() { return m_data_len; }

protected:
	unsigned int m_index;

	int m_error;

	//header only
	unsigned char m_hash[SHA_OUTSIZE];
	unsigned int m_file_len_low, m_file_len_high;
	unsigned int m_create_date;
	unsigned int m_mod_date;
	unsigned int m_chunkcnt;
	unsigned long m_ip;
	unsigned short m_port;
	char m_nick[32]; //ADDED Md5Chap - let's do real things ;)

	//data only
	unsigned char m_data[FILE_CHUNKSIZE];
	int m_data_len;

};

#endif//_C_MFILE_H_
