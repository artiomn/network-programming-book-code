/*
WASTE - mqueuelist.hpp (Class that handles routing messages through queues)
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

#ifndef _C_MQUEUELIST_H_
#define _C_MQUEUELIST_H_

#include "mqueue.hpp"
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
#include "listview.hpp"
#endif
#include "itemlist.hpp"

class C_MessageQueueList;

typedef void (*MessageQueueCallback)(T_Message *message, C_MessageQueueList *_this, C_Connection *cn);

class C_MessageQueueList
{
public:
	C_MessageQueueList(MessageQueueCallback gm, unsigned char maxttl=DEFAULT_TTL);
	~C_MessageQueueList();

	int AddMessageQueue(C_MessageQueue *q);
	int send(T_Message *msg);
	int GetNumQueues() { return m_queues->GetSize(); }
	C_MessageQueue *GetQueue(int x) { return m_queues->Get(x); }

	void run(int doRouting);

	void reset_route_error_count() { m_stat_route_errors=0; }
	int get_route_error_count() { return m_stat_route_errors; }

	unsigned char get_max_ttl() { return m_max_ttl; }
	void set_max_ttl(unsigned char a) { m_max_ttl=a; }

	int find_route(T_GUID *id, unsigned int msgtype); //-1 = local, -2 = none, >=0 = queue

	static unsigned char GetMessagePriority(int type);

	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		void SetStatusListView(W_ListView *lv) {m_lv=lv;}
	#endif

protected:
	C_ItemList<C_MessageQueue> *m_queues;
	C_MessageQueue *m_local_route;

	unsigned char m_max_ttl;
	int m_stat_route_errors;
	int m_run_rr;

	MessageQueueCallback m_got_message;

	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		W_ListView *m_lv;
	#endif
};

#endif //_C_MQUEUELIST_H_
