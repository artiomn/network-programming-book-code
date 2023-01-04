/*
WASTE - m_file.cpp (File transfer message builder/parser class)
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
#include "m_file.hpp"
#include "blowfish.hpp"

//constructor for building
C_FileSendRequest::C_FileSendRequest()
{
	m_ip=0;
	m_port=0;
	m_need_chunks_used=0;
	m_need_chunks_indexused=0;
	memset(&m_prev_guid,0,sizeof(m_prev_guid));
	memset(&m_guid,0,sizeof(m_guid));
	memset(m_fnhash,0,sizeof(m_fnhash));
	memset(m_nick,0,sizeof(m_nick));
	bIsInitialRequest=false;
	m_idx=-1;
	m_abort=0;
}

C_FileSendRequest::C_FileSendRequest(C_SHBuf *in)
{
	m_ip=0;
	m_port=0;
	m_need_chunks_used=0;
	memset(&m_prev_guid,0,sizeof(m_prev_guid));
	memset(&m_guid,0,sizeof(m_guid));
	memset(m_fnhash,0,sizeof(m_fnhash));
	memset(m_nick,0,sizeof(m_nick));
	bIsInitialRequest=false;
	m_idx=-1;
	m_abort=0;

	unsigned char *data=(unsigned char *)in->Get();
	int datalen=in->GetLength();

	if (datalen < 16+16) {
		return;
	};
	memcpy(&m_guid,data,16); data+=16; datalen-=16;
	memcpy(&m_prev_guid,data,16); data+=16; datalen-=16;

	if (datalen < 4+SHA_OUTSIZE+(sizeof(m_nick)-1)+4+2) {
		if (datalen) {//m_abort>=2!
			m_abort=DataUInt1(data);data++;datalen--;
			if (m_abort<2) {
				dbg_printf(ds_Debug,"FileSendRequest parser received malformed m_abort(0x%x)",m_abort);
				m_abort=2;
			};
			m_idx=DataUInt4(data);data+=4;datalen-=4;
		}
		else {
			m_abort=1;
		}
		return;
	};

	m_idx=DataUInt4(data);data+=4;datalen-=4;

	memcpy(m_fnhash,data,SHA_OUTSIZE);data+=SHA_OUTSIZE;datalen-=SHA_OUTSIZE;
	memcpy(m_nick,data,sizeof(m_nick)-1);data+=sizeof(m_nick)-1;datalen-=sizeof(m_nick)-1;
	m_ip=  DataUInt4(data);data+=4;datalen-=4;
	m_port=DataUInt2(data);data+=2;datalen-=2;

	while (datalen>4 && m_need_chunks_used < FILE_MAX_CHUNKS_PER_REQ) {
		unsigned int offs=DataUInt4(data);  data+=4;datalen-=4;
		unsigned int len= DataUInt1(data)+1;data+=1;datalen-=1;
		unsigned int x;

		for (x = 0; (x < len) && (m_need_chunks_used < FILE_MAX_CHUNKS_PER_REQ); x++) {
			if (offs==0) bIsInitialRequest=true;
			m_need_chunks_offs[m_need_chunks_used++]=offs++; //rle decompress
		};
	};
}

C_SHBuf *C_FileSendRequest::Make()
{
	// TODO: this has changed fix docu
	//send guid (16 bytes)
	//send old guid (16 bytes)
	//if not abort=1:
	//one random byte
	//if not abort=2
	//send file index (4 bytes)
	//send filename hash (SHA_OUTSIZE bytes)
	//send nick (31 bytes, can be zeros)
	//send dc ip/port (6 bytes, can be 0)
	//if request table
	//send request table
	int x;
	int size=16+16;
	if (!m_abort) size+=4+SHA_OUTSIZE+(sizeof(m_nick)-1)+6+m_need_chunks_indexused*5;
	if (m_abort >= 2) {
		size++; //error
		size+=4; //idx (we need this!)
	};

	C_SHBuf *p=new C_SHBuf(size);
	unsigned char *data=(unsigned char *)p->Get();
	memcpy(data,&m_guid,16); data+=16;
	memcpy(data,&m_prev_guid,16); data+=16;

	if (m_abort==1) return p;
	if (m_abort >= 2) {
		UIntData1(data,(unsigned char)(m_abort&0xff));data++;
		IntData4(data,m_idx);data+=4;
	};

	if (m_abort) return p;

	IntData4(data,m_idx);data+=4;
	memcpy(data,m_fnhash,SHA_OUTSIZE); data+=SHA_OUTSIZE;
	memcpy(data,m_nick,sizeof(m_nick)-1);data+=sizeof(m_nick)-1; // 31 no null
	UIntData4(data,m_ip);data+=4;
	UIntData2(data,m_port);data+=2;

	dbg_printf(ds_Debug,"sending a request with %d pairs, for %d items",m_need_chunks_indexused,m_need_chunks_used);
	for (x = 0; x < m_need_chunks_indexused; x ++) {
		UIntData4(data,m_need_chunks_offs[x]);data+=4;
		IntData1(data,m_need_chunks_len[x]);data++;
	};
	return p;
}

void C_FileSendRequest::add_need_chunk(unsigned int idx)
{
	if (m_need_chunks_used >= FILE_MAX_CHUNKS_PER_REQ) return;

	//see if this one can be added on to the old run
	if (m_need_chunks_indexused > 0 && m_need_chunks_offs[m_need_chunks_indexused-1] + m_need_chunks_len[m_need_chunks_indexused-1] + 1 == idx) {
		m_need_chunks_len[m_need_chunks_indexused-1]++;
		dbg_printf(ds_Debug,"continuing run with %d",idx);
	}
	else {
		dbg_printf(ds_Debug,"new run at %d",idx);
		m_need_chunks_len[m_need_chunks_indexused]=0;
		m_need_chunks_offs[m_need_chunks_indexused++]=idx;
	};

	m_need_chunks_used++;
}

C_FileSendRequest::~C_FileSendRequest()
{
}

void C_FileSendRequest::set_fn_hash(unsigned char fnhash[SHA_OUTSIZE])
{
	memcpy(m_fnhash,fnhash,SHA_OUTSIZE);
}

void C_FileSendRequest::get_fn_hash(unsigned char fnhash[SHA_OUTSIZE])
{
	memcpy(fnhash,m_fnhash,SHA_OUTSIZE);
}

void C_FileSendRequest::set_nick(const char *nick)
{
	if (nick && nick[0]!='.') safe_strncpy(m_nick,nick,sizeof(m_nick));
}


C_FileSendReply::C_FileSendReply()
{
	m_error=0;

	m_index=0;

	m_file_len_low=m_file_len_high=0;
	m_create_date=m_mod_date=0;
	m_chunkcnt=0;
	m_ip=0;
	m_port=0;
	memset(m_nick,0,sizeof(m_nick));

	m_data_len=0;

}

C_FileSendReply::C_FileSendReply(C_SHBuf *in)
{
	m_error=0;

	m_index=0;
	m_ip=0;
	m_port=0;

	m_file_len_low=m_file_len_high=0;
	m_create_date=m_mod_date=0;
	m_chunkcnt=0;

	m_data_len=0;
	memset(m_nick,0,sizeof(m_nick));

	unsigned char *data=(unsigned char *)in->Get();
	unsigned int datalen=in->GetLength();

	if (datalen < 4) { m_error=datalen; return; }

	m_index=DataUInt4(data);data+=4;datalen-=4;

	if (m_index != (unsigned int)~0) { //woohoo we get data now
		m_data_len=datalen;
		if (m_data_len < 0 || m_data_len > FILE_CHUNKSIZE) {
			log_printf(ds_Warning,"filesendreply: data length out of range, %d",m_data_len);
			m_data_len=0;
		};
		if (m_data_len) {
			memcpy(m_data,data,m_data_len);
		};
	}
	else { //header parsing
		if (datalen < 4+4+4+4+4+SHA_OUTSIZE+4+2) {
			log_printf(ds_Warning,"filesendreply: got packet that is a header, with length of %d",datalen);
			return;
		};
		m_file_len_low =DataUInt4(data);data+=4;datalen-=4;
		m_file_len_high=DataUInt4(data);data+=4;datalen-=4;
		m_create_date  =DataUInt4(data);data+=4;datalen-=4;
		m_mod_date     =DataUInt4(data);data+=4;datalen-=4;
		m_chunkcnt     =DataUInt4(data);data+=4;datalen-=4;
		memcpy(m_hash,data,SHA_OUTSIZE);data+=SHA_OUTSIZE;datalen-=SHA_OUTSIZE;
		m_ip           =DataUInt4(data);data+=4;datalen-=4;
		m_port         =DataUInt2(data);data+=2;datalen-=2;
		if (datalen>=(sizeof(m_nick)-1)) { //let's get nick & maintain compat
			memcpy(m_nick,data,sizeof(m_nick)-1);
			m_nick[sizeof(m_nick)-1]=0;
			data+=sizeof(m_nick)-1;
			datalen-=sizeof(m_nick)-1;
		};
	};
}

C_FileSendReply::~C_FileSendReply()
{
}

C_SHBuf *C_FileSendReply::Make()
{
	//chunk index (-1 is header info)
	//if header:
	// file length 8 bytes
	// dates 8 bytes
	// count of max chunks coming 4 bytes
	// file md5 16 bytes
	//else
	// FILE_CHUNKSIZE bytes of data :)
	C_SHBuf *p;

	if (m_error) return new C_SHBuf(m_error>3?3:m_error);

	if (m_index!=(unsigned int)~0) {
		if (m_data_len > FILE_CHUNKSIZE) {
			log_printf(ds_Warning,"filesendreply::make() data length = %d",m_data_len);
			return new C_SHBuf(0);
		};

		p=new C_SHBuf(4+m_data_len);
		unsigned char *data=(unsigned char *)p->Get();
		IntData4(data,m_index);data+=4;
		memcpy(data,m_data,m_data_len);
	}
	else {
		p=new C_SHBuf(4+8+8+4+SHA_OUTSIZE+4+2+(sizeof(m_nick)-1));
		unsigned char *data=(unsigned char *)p->Get();
		IntData4(data,m_index);					data+=4;
		IntData4(data,m_file_len_low);			data+=4;
		IntData4(data,m_file_len_high);			data+=4;
		IntData4(data,m_create_date);			data+=4;
		IntData4(data,m_mod_date);				data+=4;
		IntData4(data,m_chunkcnt);				data+=4;
		memcpy(data,m_hash,SHA_OUTSIZE);		data+=SHA_OUTSIZE;
		IntData4(data,m_ip);					data+=4;
		IntData2(data,m_port);					data+=2;
		memcpy(data,m_nick,sizeof(m_nick)-1);	data+=sizeof(m_nick)-1;
	};
	return p;
}

void C_FileSendReply::set_file_len(unsigned int file_len_low, unsigned int file_len_high)
{
	m_file_len_low=file_len_low;
	m_file_len_high=file_len_high;
}

void C_FileSendReply::set_data(unsigned char *buf, int len)
{
	if (len < 0 || len > FILE_CHUNKSIZE) {
		log_printf(ds_Warning,"filesendreply::set_data() data length out of range, %d",len);
		return;
	};
	m_data_len=len;
	memcpy(m_data,buf,len);
}

void C_FileSendReply::set_nick(const char *nick)
{
	if (nick && nick[0]!='.') safe_strncpy(m_nick,nick,sizeof(m_nick));
}
