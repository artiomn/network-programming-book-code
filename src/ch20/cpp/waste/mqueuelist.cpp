/*
WASTE - mqueuelist.cpp (Class that handles routing messages through queues)
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
#include "main.hpp"
#include "mqueuelist.hpp"
#include "util.hpp"
#include "netkern.hpp"

C_MessageQueueList::C_MessageQueueList(MessageQueueCallback gm, unsigned char maxttl)
{
	m_got_message=gm;
	m_queues=new C_ItemList<C_MessageQueue>;
	m_local_route=new C_MessageQueue(NULL,1);
	m_max_ttl=maxttl;
	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		m_lv=NULL;
	#endif
	m_stat_route_errors=0;
	m_run_rr=0;
}

C_MessageQueueList::~C_MessageQueueList()
{
	if (m_queues) {
		while (m_queues->GetSize()>0) {
			delete m_queues->Get(0);
			m_queues->Del(0);
		};
		delete m_queues;
	};
	if (m_local_route) delete m_local_route;
}

int C_MessageQueueList::AddMessageQueue(C_MessageQueue *q)
{
	m_queues->Add(q);
	if (m_queues->GetSize()==1) {
		update_forceip_dns_resolution();
	};
	return 0;
}

unsigned char C_MessageQueueList::GetMessagePriority(int type)
{
	switch (type)
	{
#define POO(x) case MESSAGE_##x: return MSGPRIO_##x;
		POO(LOCAL_CAPS);
		POO(UPLOAD);
		POO(CHAT);
		POO(CHAT_REPLY);
		POO(KEYDIST);
		POO(KEYDIST_REPLY);
		POO(PING);
		POO(SEARCH);
		POO(SEARCH_REPLY);
		POO(SEARCH_USERLIST);
		POO(FILE_REQUEST);
		POO(FILE_REQUEST_REPLY);
		POO(LOCAL_SATURATE);
#undef POO
	};
	return 128; //default so-so priority
}

int C_MessageQueueList::send(T_Message *msg)
{
	msg->message_prio = GetMessagePriority(msg->message_type);
	msg->message_ttl=G_DEFAULT_TTL;
	msg->data->Lock();

	if (MESSAGE_TYPE_ROUTED(msg->message_type)) { //routed
		int a=find_route(&msg->message_guid, msg->message_type);
		if (a >= 0) {
			C_MessageQueue::calc_md5(msg,msg->message_md5);
			C_MessageQueue *cm=m_queues->Get(a);
			cm->send_message(msg);
		}
		else if (a == -1) {
			log_printf(ds_Warning,"queuelist::send(): warning: tried to send routed message to self!");
		}
		else {
			m_stat_route_errors++;
			log_printf(ds_Warning,"queuelist::send(): no route found for message of type %d!",msg->message_type);
		};
	}
	else if (MESSAGE_TYPE_LOCAL(msg->message_type)) {
		log_printf(ds_Warning,"queuelist::send(): warning: tried to send local message to list!");
	}
	else { //broadcast
		CreateID128(&msg->message_guid);
		C_MessageQueue::calc_md5(msg,msg->message_md5);
		m_local_route->add_route(&msg->message_guid, msg->message_type);
		int o;
		for (o = 0; o < m_queues->GetSize(); o ++) {
			C_MessageQueue *cm=m_queues->Get(o);
			cm->send_message(msg);
		};
	};
	msg->data->Unlock();
	return 0;
}

void C_MessageQueueList::run(int doRouting)
{
	int runcnt=0;
	int numqueues=m_queues->GetSize();
	int total_sendbw=0,total_recvbw=0;
	int sbps, rbps;

	while (runcnt < numqueues) {
		C_MessageQueue *cm=m_queues->Get(runcnt);
		cm->get_con()->calc_bps(&sbps,&rbps);
		total_sendbw+=sbps;
		total_recvbw+=rbps;
		runcnt++;
	};
	runcnt=0;

	while (runcnt < numqueues) {
		int thisq = (runcnt + m_run_rr)%numqueues;
		C_MessageQueue *cm=m_queues->Get(thisq);

		C_Connection *con=cm->get_con();

		con->get_last_bps(&sbps,&rbps);

		if (g_throttle_flag&4) rbps=total_recvbw;
		if (g_throttle_flag&8) sbps=total_sendbw;

		if (!(g_throttle_flag&2) || sbps < g_throttle_send*1024) sbps=-1;
		else sbps=0;

		if (!(g_throttle_flag&1) || rbps < g_throttle_recv*1024) rbps=-1;
		else rbps=0;

		if (sbps!=0) {
			cm->run(0,sbps);
		};
		int s=con->run(sbps,rbps);

		for (;;) {
			T_Message msg={0,};
			cm->run(1,-1);
			if (cm->recv_message(&msg)) break;

			if (MESSAGE_TYPE_ROUTED(msg.message_type)) {
				int r=find_route(&msg.message_guid,msg.message_type);
				if (r >= 0 && r != thisq) {
					if (msg.message_type != MESSAGE_KEYDIST_REPLY || (g_keydist_flags&4)) {
						msg.message_ttl--;
						if (msg.message_ttl>0) {
							C_MessageQueue *cm2=m_queues->Get(r);
							cm2->send_message(&msg);
						};
					};
				}
				else if (r==-1) {//this one was meant for us!
					m_got_message(&msg,this,cm->get_con());
				}
				else {
					m_stat_route_errors++;
					log_printf(ds_Informational,"queuelist::run(): no route found for relaying message!");
				};
			}
			else if (MESSAGE_TYPE_LOCAL(msg.message_type)) {
				m_got_message(&msg,this,cm->get_con());
			}
			else { //broadcast
				int r=find_route(&msg.message_guid,msg.message_type);
				if (r < -1) {
					int o;
					cm->add_route(&msg.message_guid,msg.message_type);
					if (msg.message_ttl>1 && doRouting && (msg.message_type != MESSAGE_KEYDIST || (g_keydist_flags&4))) {
						if (msg.message_ttl>m_max_ttl) msg.message_ttl=m_max_ttl;
						msg.message_ttl--;
						for (o = 0; o < numqueues; o ++) {
							if (o != thisq) {
								C_MessageQueue *cm2=m_queues->Get(o);
								cm2->send_message(&msg);
							};
						};
					};
					m_got_message(&msg,this,cm->get_con());
				};
			};
			msg.data->Unlock();
		};

		if (s == C_Connection::STATE_ERROR || s == C_Connection::STATE_CLOSED) {
			#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
				if (m_lv) {
					int i=m_lv->FindItemByParam((int)cm->get_con());
					if (i != -1) {
						char text[512];
						m_lv->GetText(i,1,text,sizeof(text));
						if (!strstr(text,":")) m_lv->DeleteItem(i);
						else {
							m_lv->SetItemParam(i,0);
							m_lv->SetItemText(i,0,"Disconnected");
							if (!cm->get_stat_recv() || !cm->get_stat_send()) {
								m_lv->GetText(i,2,text,sizeof(text));
								int a=atoi(text)-10;
								if (a < 0)a=0;
								sprintf(text,"%d",a);
								m_lv->SetItemText(i,2,text);
							};
						};
					};
				};
			#endif
			delete cm;
			m_queues->Del(thisq);
			numqueues--;
			if (numqueues==0) {
				if (g_forceip_dynip_mode==2) {
					log_printf(ds_Informational,"All queues closed! Resetting dynip!!!");
					g_forceip_dynip_addr=INADDR_NONE;
				};
			};
			#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
				PostMessage(g_netstatus_wnd,WM_USER_TITLEUPDATE,0,0);
			#endif
		}
		else {
			runcnt++;
		};
	};
	m_run_rr++;
}

int C_MessageQueueList::find_route(T_GUID *id, unsigned int msgtype)
{
	int r;
	if (m_local_route->is_route(id,msgtype)) return -1;
	for (r = 0; r < m_queues->GetSize(); r ++) {
		C_MessageQueue *cm=m_queues->Get(r);
		if (cm->is_route(id,msgtype)) return r;
	};
	return -2;
}
