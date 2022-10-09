/*
WASTE - mqueue.hpp (Class that handles sending messages over a connection)
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

#ifndef _C_MQUEUE_H_
#define _C_MQUEUE_H_

#include "util.hpp"
#include "connection.hpp"
#include "shbuf.hpp"

#define G_DEFAULT_TTL (96)

#define G_MAX_TTL (G_DEFAULT_TTL)

//message types.
//lowest bit=routed, second lowest bit = local (unless low bit set)

#define MESSAGE_LOCAL_CAPS          (10)
#define MESSAGE_UPLOAD              (16)
#define MESSAGE_CHAT                (32)
#define MESSAGE_CHAT_REPLY          (32|1)
#define MESSAGE_KEYDIST             (36)
#define MESSAGE_KEYDIST_REPLY       (36|1)
#define MESSAGE_PING                (40)
#define MESSAGE_SEARCH              (64)
#define MESSAGE_SEARCH_REPLY        (64|1)
#define MESSAGE_SEARCH_USERLIST     (68)
#define MESSAGE_FILE_REQUEST        (72)
#define MESSAGE_FILE_REQUEST_REPLY  (72|1)
#define MESSAGE_LOCAL_SATURATE      (254)
//we have room for another few billion types of messages, phew.

#define MESSAGE_TYPE_ROUTED(x) ((x)&1)
#define MESSAGE_TYPE_BCAST(x) (!((x)&3))
#define MESSAGE_TYPE_LOCAL(x) (((x)&3)==2)

#define MESSAGE_MAX_PAYLOAD_ROUTE 32768
#define MESSAGE_MAX_PAYLOAD_BCAST 2048

//priorities for the messages
#define MSGPRIO_LOCAL_CAPS 4 //very high
#define MSGPRIO_UPLOAD 8 //pretty high priority.
#define MSGPRIO_CHAT 24
#define MSGPRIO_CHAT_REPLY 36
#define MSGPRIO_KEYDIST_REPLY 40
#define MSGPRIO_KEYDIST 44
#define MSGPRIO_PING 54
#define MSGPRIO_SEARCH 64
#define MSGPRIO_SEARCH_USERLIST 68
#define MSGPRIO_SEARCH_REPLY 80
#define MSGPRIO_FILE_REQUEST 96
#define MSGPRIO_FILE_REQUEST_REPLY 110
#define MSGPRIO_LOCAL_SATURATE 255 //lowest possible priority

struct T_Message //the header is now 40 bytes per packet. this is kinda a lot, heh. oh well.
{
	//data sent over network
	unsigned char message_md5[16];

	//crc applied to the following
	unsigned char message_prio;
	unsigned int message_type;
	int message_length; //actually encoded as two bytes
	unsigned char message_ttl;

	T_GUID message_guid;

	//etc
	C_SHBuf *data;
};

enum _eROUTETAB{
	ROUTETAB_GENERAL,
	ROUTETAB_PING,
	ROUTETAB_FILE,
	ROUTETAB_NUM
};

class C_MessageQueue
{
public:
	C_MessageQueue(C_Connection *con, int maxmsg=256, int maxroute=8192);
	~C_MessageQueue();
	static void calc_md5(T_Message *msg, unsigned char buf[16]);
	int send_message(T_Message *msg); //return 1 on error (full)
	int recv_message(T_Message *msg); //return 0 if message filled
	C_Connection *get_con() { return m_con; }

	void run(int isrecv, int maxbytesend);

	void add_route(T_GUID *id, unsigned int msgtype);
	int is_route(T_GUID *id, unsigned int msgtype);

	int get_stat_recv() { return m_stat_recv; }
	int get_stat_send() { return m_stat_send; }
	int get_stat_drop() { return m_stat_drop; }

	int getlen() { return m_msg_used; }
	int getmaxlen() {  return m_msg_len; }

protected:
	T_Message *get(int x);

	void saturate(int satsize);

	void insert(int pos, T_Message *msg);
	void removefirst();

	int msg2tab(unsigned int msg);

	int find_route(T_GUID *id, int whichtab); //returns index of where route should go (or is)

	C_Connection *m_con;

protected:
	//list
	T_Message *m_msg;
	int m_msg_len;
	int m_msg_used;
	T_GUID *m_routes[ROUTETAB_NUM];
	T_GUID *m_routes_order[ROUTETAB_NUM];
	int m_num_routes[ROUTETAB_NUM];
	int m_route_opos[ROUTETAB_NUM];

	int m_msg_bsent;
	T_Message m_newmsg;
	int m_newmsg_pos;

	int m_stat_recv;
	int m_stat_send;
	int m_stat_drop;

};

#endif //_C_MQUEUE_H_
