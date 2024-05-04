/*
WASTE - m_search.cpp (Search/browse message builder/parser class)
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
#include "mqueue.hpp"
#include "m_search.hpp"

C_MessageSearchReply::~C_MessageSearchReply()
{
	free(m_replies);
}

C_MessageSearchReply::C_MessageSearchReply()
{
	m_size=5+16;
	m_conspeed=0;
	m_numitems=0;
	m_replies=NULL;
	memset(&m_guid,0,sizeof(m_guid));
}

C_MessageSearchReply::C_MessageSearchReply(C_SHBuf *in)
{
	m_size=0;
	m_conspeed=0;
	m_numitems=0;
	m_replies=NULL;
	unsigned char *data=(unsigned char*)in->Get();
	int l=in->GetLength();
	if (l < 5+16+12) return;
	int nitems = data[0]; data++;
	m_conspeed=DataUInt4(data);data+=4;
	l-=5;
	if (!nitems) {
		log_printf(ds_Warning,"searchreply: invalid message (0 items)!");
		return;
	};
	int x;
	m_replies=(ReplyEntry *)malloc(sizeof(ReplyEntry)*nitems);
	for (x = 0; x < nitems; x ++) {
		if (l<16) {
			log_printf(ds_Warning,"searchreply: invalid message (length error)!");
			return;
		};
		m_replies[x].id=DataUInt4(data); data+=4; l-=4;
		m_replies[x].length_bytes_low=DataUInt4(data); data+=4; l-=4;
		m_replies[x].length_bytes_high=DataUInt4(data); data+=4; l-=4;
		m_replies[x].file_time=DataUInt4(data); data+=4; l-=4;

		int t;
		for (t=0; t < min(l,SEARCHREPLY_MAX_FILESIZE) && data[t]; t ++);
		if (t >= min(l,SEARCHREPLY_MAX_FILESIZE)) {
			log_printf(ds_Warning,"searchreply: invalid message (filename error)!");
			return;
		};
		memcpy(m_replies[x].name,data,t+1);
		l-=t+1;
		data+=t+1;

		for (t=0; t < min(l,SEARCHREPLY_MAX_METASIZE) && data[t]; t ++);
		if (t >= min(l,SEARCHREPLY_MAX_METASIZE)) {
			log_printf(ds_Warning,"searchreply: invalid message (meta error)!");
			return;
		};
		memcpy(m_replies[x].metadata,data,t+1);
		l-=t+1;
		data+=t+1;
	};

	if (l < 16) {
		log_printf(ds_Warning,"searchreply: invalid message (no guid)!",x);
		return;
	};
	memcpy(&m_guid,data,16); data+=16; l-=16;
	m_numitems=nitems;
}

void C_MessageSearchReply::clear_items()
{
	m_numitems=0;
	m_size=5+16;
}

int C_MessageSearchReply::would_fit(char *name, char *metadata)
{
	if (m_numitems >= 255) return 0;
	int l1=strlen(name);
	int l2=metadata?strlen(metadata):0;
	if (l1>SEARCHREPLY_MAX_FILESIZE-1) l1=SEARCHREPLY_MAX_FILESIZE-1;
	if (l2>SEARCHREPLY_MAX_METASIZE-1) l2=SEARCHREPLY_MAX_METASIZE-1;

	return (m_size + 16+l1+1+l2+1 <= MESSAGE_MAX_PAYLOAD_ROUTE);
}

void C_MessageSearchReply::add_item(int id, char *name, char *metadata, int length_bytes_low, int length_bytes_high, int file_time)
{
#if 0
	if (!would_fit(name,metadata)) {
		log_printf(ds_Informational,"searchreply::add_item() called but !would_fit()");
		return;
	};
#endif

	dbg_printf(ds_Debug,"C_MessageSearchReply::add_item: %d,%s,%s",id,name,metadata);
	if (!m_replies) {
		m_replies=(ReplyEntry*)malloc(sizeof(ReplyEntry)*255);
	};
	m_replies[m_numitems].id=id;
	m_replies[m_numitems].length_bytes_low=length_bytes_low;
	m_replies[m_numitems].length_bytes_high=length_bytes_high;
	m_replies[m_numitems].file_time=file_time;

	int l1=strlen(name);
	int l2=metadata?strlen(metadata):0;
	if (l1>SEARCHREPLY_MAX_FILESIZE-1) l1=SEARCHREPLY_MAX_FILESIZE-1;
	if (l2>SEARCHREPLY_MAX_METASIZE-1) l2=SEARCHREPLY_MAX_METASIZE-1;

	memcpy(m_replies[m_numitems].name,name,l1);
	m_replies[m_numitems].name[l1]=0;
	if (l2) memcpy(m_replies[m_numitems].metadata,metadata,l2);
	m_replies[m_numitems].metadata[l2]=0;

	m_size+=16+l1+1+l2+1;
	m_numitems++;
}

int C_MessageSearchReply::find_item(char *name, char *meta, int maxitems)
{
	int x;
	if (maxitems < 0 || maxitems > m_numitems) maxitems=m_numitems;
	if (m_replies) for (x = 0; x < maxitems; x ++) {
		if (!strcmp(m_replies[x].metadata,meta) && !stricmp(m_replies[x].name,name)) return x;
	};
	return -1;
}

void C_MessageSearchReply::delete_item(int index)
{
	if (m_replies && index >= 0 && index < m_numitems) {
		if (index != --m_numitems) memcpy(m_replies+index,m_replies+index+1,(m_numitems-index)*sizeof(m_replies[0]));
	};
}

int C_MessageSearchReply::get_item(int index, int *id, char *name, char *metadata, int *length_bytes_low, int *length_bytes_high, int *file_time)
{
	if (index<0 || index>=m_numitems) return 1;
	if (id) *id=m_replies[index].id;
	if (length_bytes_low) *length_bytes_low=m_replies[index].length_bytes_low;
	if (length_bytes_high) *length_bytes_high=m_replies[index].length_bytes_high;
	if (file_time) *file_time=m_replies[index].file_time;
	if (name) strcpy(name,m_replies[index].name);
	if (metadata) strcpy(metadata,m_replies[index].metadata);
	return 0;
}

C_SHBuf *C_MessageSearchReply::Make()
{
	if (!m_numitems) return NULL;
	if (m_numitems > 255) {
		log_printf(ds_Informational,"searchreply::make(): numitems=%d",m_numitems);
		m_numitems=255;
	};

	C_SHBuf *buf=new C_SHBuf(m_size);

	unsigned char *data=(unsigned char *)buf->Get();
	IntData1(data,(char)m_numitems);data++;
	IntData4(data,m_conspeed);data+=4;

	int x;
	for (x = 0; x < m_numitems; x ++) {
		IntData4(data,m_replies[x].id);data+=4;
		IntData4(data,m_replies[x].length_bytes_low);data+=4;
		IntData4(data,m_replies[x].length_bytes_high);data+=4;
		IntData4(data,m_replies[x].file_time);data+=4;

		int l1=strlen(m_replies[x].name)+1;
		memcpy((char*)data,m_replies[x].name,l1);
		data+=l1;
		l1=strlen(m_replies[x].metadata)+1;
		memcpy((char*)data,m_replies[x].metadata,l1);
		data+=l1;
	};
	memcpy(data,&m_guid,16); data+=16;
	return buf;
}

C_MessageSearchRequest::~C_MessageSearchRequest()
{
}

C_MessageSearchRequest::C_MessageSearchRequest()
{
	m_min_conspeed=0;
	m_searchstring[0]=0;
}

C_MessageSearchRequest::C_MessageSearchRequest(C_SHBuf *in)
{
	m_min_conspeed=0;
	m_searchstring[0]=0;
	unsigned char *data=(unsigned char*)in->Get();
	int l=in->GetLength();
	if (l < 2) return;
	m_min_conspeed=DataUInt2(data);data+=2;l-=2;
	int t;
	for (t=0; t < min(l,256) && data[t]; t ++);
	if (t == min(l,256)) return;
	memcpy(m_searchstring,data,t+1);
	data+=t+1;
	l-=t+1;
}

void C_MessageSearchRequest::set_searchstring(char *str)
{
	safe_strncpy(m_searchstring,str,sizeof(m_searchstring));
}

C_SHBuf *C_MessageSearchRequest::Make()
{
	if (strlen(m_searchstring) < 1) return NULL;
	C_SHBuf *buf=new C_SHBuf(2+strlen(m_searchstring)+1);
	unsigned char *data=(unsigned char *)buf->Get();
	UIntData2(data,(unsigned short)min(m_min_conspeed,0xffff));data+=2;
	memcpy((char*)data,m_searchstring,strlen(m_searchstring)+1);
	return buf;
}
