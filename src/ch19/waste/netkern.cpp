/*
WASTE - netkern.cpp (Common network runtime code)
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
#include "netq.hpp"
#include "netkern.hpp"

#ifdef _DEFINE_SRV
	#include "resourcesrv.hpp"
#else
	#include "resource.hpp"
#endif
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	#include "childwnd.hpp"
#endif
#include "m_search.hpp"
#include "m_ping.hpp"
#include "m_lcaps.hpp"
#include "srchwnd.hpp"

C_ItemList<C_Connection> g_new_net;

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	W_ListView g_lvnetcons;
	HWND g_netstatus_wnd;
#endif

#define SYNC_SIZE 16
static char g_con_str[SYNC_SIZE+1]="MUGWHUMPJISMSYN3";

static void ListenToSocket()
{
	if (g_listen) {
		C_Connection *c=g_listen->get_connect();
		if (c) {
			unsigned int addr=c->get_remote();
			if (!allowIP(addr)) {
				struct in_addr in;
				in.s_addr=addr;
				char *ad=inet_ntoa(in);
				log_printf(ds_Warning,"Denied connection from %s due to IP list!",ad);
				delete c;
			}
			else if (g_new_net.GetSize()>=HARD_NEW_CONNECTION_LIMIT) {
				struct in_addr in;
				in.s_addr=addr;
				char *ad=inet_ntoa(in);
				log_printf(ds_Error,"Denied connection from %s due to heavy load (Dos-Attack?)!",ad);
				delete c;
			}
			else {
				c->send_bytes(g_con_str,SYNC_SIZE);
				#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
					struct in_addr in;
					in.s_addr=addr;
					char *a=inet_ntoa(in);
					char str2[512];
					sprintf(str2,"%s (incoming)",a?a:"?");
					g_lvnetcons.InsertItem(0,"Authenticating",(int)c);
					g_lvnetcons.SetItemText(0,1,str2);
					g_lvnetcons.SetItemText(0,2,"100");
				#endif
				g_new_net.Add(c);
				#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
					PostMessage(g_netstatus_wnd,WM_USER_TITLEUPDATE,0,0);
				#endif
			};
		};
	}
}

void NetKern_ConnectToHostIfOK(unsigned long ip, unsigned short port)
{
	struct in_addr in;
	in.s_addr=ip;
	char *t=inet_ntoa(in);
	if (!t || !*t) return;
	char text[512];
	safe_strncpy(text,t,sizeof(text));

	if (!g_config->ReadInt(CONFIG_directxfers_connect,CONFIG_directxfers_connect_DEFAULT)) return;

	if (g_mql) {
		int n;
		for (n=0;n<g_mql->GetNumQueues();n++) {
			C_MessageQueue *cm=g_mql->GetQueue(n);
			C_Connection *cc=cm->get_con();
			if (cc) {
				if ((cc->get_remote()==ip)&&(cc->get_remote_port()==port)) return;
			};
		};
	};

	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		int netqpos=-1;
		int i,l;
		l=g_lvnetcons.GetCount();
		for (i = 0; i < l; i ++) {
			char text2[256];
			g_lvnetcons.GetText(i,1,text2,256);
			int len=strlen(text);
			if (!strnicmp(text2,text,len) &&
				(text2[len] == 0 ||
				text2[len] == ':' ||
				text2[len] == ' ')
				)
			{
				if (g_lvnetcons.GetParam(i)) return;
				else if (netqpos<0) netqpos=i;
			};
		};
	#endif

	if (is_accessable_addr(ip) && allowIP(ip)) {
		C_Connection *newcon=new C_Connection(text,port,g_dns);
		newcon->send_bytes(g_con_str,SYNC_SIZE);
		g_new_net.Add(newcon);

		#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
			if (netqpos<0) {
				char buf[1024];
				sprintf(buf,"%s:%d",text,(int)(unsigned short)port);
				g_lvnetcons.InsertItem(0,"Connecting",(int)newcon);
				g_lvnetcons.SetItemText(0,1,buf);
				g_lvnetcons.SetItemText(0,2,"90");
			}
			else {
				g_lvnetcons.SetItemParam(netqpos,(int)newcon);
				g_lvnetcons.SetItemText(netqpos,0,"Connecting");
			};
			PostMessage(g_netstatus_wnd,WM_USER_TITLEUPDATE,0,0);
		#endif//WIN32
	};
}

static void SendCaps(C_MessageQueue *mq)
{
	if (mq) { //send caps
		C_MessageLocalCaps l;
		int a=MAX_CONNECTION_SENDSIZE;
		if (g_conspeed<64)a=512; //modems only let sendahead be 512 or so.
		else if (g_conspeed<384)a=2048;
		else if (g_conspeed<1600)a=4096;
		else if (g_conspeed<20000)a=8192;
		l.add_cap(MLC_BANDWIDTH,a); //tell it our max bufsize
		l.add_cap(MLC_SATURATION,!!(g_throttle_flag&16));
		unsigned long rip=mq->get_con()->get_remote();
		if (rip) {
			l.add_cap(MLC_REMOTEIP, (int)rip);
		};
		T_Message msg={0,};
		msg.data=l.Make();
		if (msg.data) {
			msg.message_type=MESSAGE_LOCAL_CAPS;
			msg.message_length=msg.data->GetLength();
			msg.message_ttl=1;
			CreateID128(&msg.message_guid);

			msg.message_prio=C_MessageQueueList::GetMessagePriority(msg.message_type);
			C_MessageQueue::calc_md5(&msg,msg.message_md5);

			msg.data->Lock();
			mq->send_message(&msg);
			msg.data->Unlock();
		};
	};
}

void RebroadcastCaps(C_MessageQueueList *mql) //sends a local message to every host connected to
{
	int x;
	for (x = 0; x < mql->GetNumQueues(); x ++) SendCaps(mql->GetQueue(x));
}

static void HandleNewOutCons()
{
	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		static unsigned int next_runitem;

		int a=GetTickCount()-next_runitem;
		if (!next_runitem || a>=0) {
			if (!g_mql->GetNumQueues()) { //invalidate cache if no queues =)
				int ci;
				for (ci = 0; ci < SEARCHCACHE_NUMITEMS; ci ++) {
					if (g_searchcache[ci]->search_id_time)
						if (g_searchcache[ci]->numcons || !g_searchcache[ci]->searchreplies.GetSize()) g_searchcache[ci]->search_id_time=0;
				};
			};

			if (g_keepup) {
				if (g_mql->GetNumQueues()+g_new_net.GetSize() < g_keepup) {
					int hn=g_lvnetcons.GetCount();
					int x;
					int max_pos=-1;
					int max_rat=0;

					char text[512];
					unsigned short port=0;

					for (x = 0; x < hn; x ++) {
						if (g_lvnetcons.GetParam(x)) continue;

						char rat[32];
						g_lvnetcons.GetText(x,2,rat,sizeof(rat));
						int thisrat=rat[0] ? atoi(rat) : -1;
						if (max_pos < 0 || thisrat > max_rat) {
							g_lvnetcons.GetText(x,1,text,sizeof(text));

							char *p=text;
							while (*p != ':' && *p) p++;
							if (*p) {
								*p++=0;
								port=(unsigned short)(atoi(p)&0xffff);
							}
							else {
								port=CONFIG_port_DEFAULT;
							};
							int i;
							for (i = 0; i < hn; i ++) {
								if (g_lvnetcons.GetParam(i) && i != x) {
									char text2[256];
									g_lvnetcons.GetText(i,1,text2,256);
									if (!strnicmp(text2,text,strlen(text)) &&
										(text2[strlen(text)] == 0 ||
										text2[strlen(text)] == ':' ||
										text2[strlen(text)] == ' ')
										)
									{
										break;
									};
								};
							};
							unsigned long ip;
							if (i == hn && (ip=inet_addr(text)) != INADDR_NONE && is_accessable_addr(ip) && allowIP(ip)) {
								max_rat=thisrat;
								max_pos=x;
							};
						};
					}; //for
					if (max_pos >= 0) {
						char rat[32];
						g_lvnetcons.GetText(max_pos,2,rat,sizeof(rat));
						g_lvnetcons.DeleteItem(max_pos);
						int irat=atoi(rat);
						if (irat>0) {
							AddConnection(text,port,irat);
						};
					};
				};
			};
			next_runitem=GetTickCount()+NEXTITEM_NEW_OUT_CONNECTION_DELAY;
		};
	#endif// defined(_WIN32)&&(!defined(_DEFINE_SRV))
	int x;
	for (x = 0; x < g_new_net.GetSize(); x ++) {
		C_Connection *cc=g_new_net.Get(x);
		int s=cc->run(-1,-1);
		if (s == C_Connection::STATE_DIENOW) {
			#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
				int a=g_lvnetcons.FindItemByParam((int)cc);
				if (a >= 0) {
					g_lvnetcons.DeleteItem(a);
				};
			#endif
			struct in_addr in;
			in.s_addr=cc->get_remote();
			char *ad=inet_ntoa(in);
			log_printf(ds_Warning,"Could not connect to host: host %s not in access list!",ad);
			delete cc;
			g_new_net.Del(x--);
			#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
				PostMessage(g_netstatus_wnd,WM_USER_TITLEUPDATE,0,0);
			#endif
		}
		else if (s == C_Connection::STATE_CLOSED || s == C_Connection::STATE_ERROR) {
			#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
				unsigned short port=cc->get_remote_port();
				int irat=0;
				int a=g_lvnetcons.FindItemByParam((int)g_new_net.Get(x));
				if (a >= 0) {
					char rat[32];
					g_lvnetcons.GetText(a,2,rat,sizeof(rat));
					if (rat[0]) irat=atoi(rat);
					g_lvnetcons.DeleteItem(a);
					if (port) {
						int maxrat=irat;
						if (g_mql->GetNumQueues()) {
							irat-=8;
						}
						else {
							int i,n=g_lvnetcons.GetCount();
							for (i = 0; i < n; i ++) {
								if (i != a) {
									char t[32];
									if (!g_lvnetcons.GetParam(i)) {
										g_lvnetcons.GetText(i,2,t,sizeof(t));
										int thisrat=atoi(t);
										if (maxrat < thisrat) maxrat=thisrat;
									};
								};
							};
							irat=maxrat-1;
						};

						if (irat < 0) irat=0;
						add_to_netq(cc->get_remote(),port,irat,1);
					};
				};
			#endif
			struct in_addr in;
			in.s_addr=cc->get_remote();
			char *ad=inet_ntoa(in);
			log_printf(ds_Warning,"Could not connect to host: failed connect to %s!", ad);
			delete cc;
			g_new_net.Del(x--);
			#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
				PostMessage(g_netstatus_wnd,WM_USER_TITLEUPDATE,0,0);
			#endif
		}
		else if (cc->recv_bytes_available()>=(SYNC_SIZE+cc->get_RandomCrapLen())) {
			char b[SYNC_SIZE];
			cc->recv_bytes(b,SYNC_SIZE);
			if (!cc->PopRandomCrap()) {
				#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
					int a=g_lvnetcons.FindItemByParam((int)cc);
					if (a >= 0) {
						g_lvnetcons.DeleteItem(a);
					};
				#endif
				log_printf(ds_Error,"Cannot pop random crap. Bad shit happened!");
				delete cc;
				g_new_net.Del(x--);
				#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
					PostMessage(g_netstatus_wnd,WM_USER_TITLEUPDATE,0,0);
				#endif
			}
			else if (memcmp(b,g_con_str,SYNC_SIZE)) {
				#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
					int a=g_lvnetcons.FindItemByParam((int)cc);
					if (a >= 0) {
						g_lvnetcons.DeleteItem(a);
					};
				#endif
				char descstr_l[16+SHA_OUTSIZE*2+32];
				MakeUserStringFromHash(cc->get_remote_pkey_hash(), descstr_l, NULL);
				struct in_addr in;
				in.s_addr=cc->get_remote();
				char *ad=inet_ntoa(in);
				log_printf(ds_Warning,"Could not connect to host: failed auth by %s (%s)! Wrong network password?",ad,descstr_l);
				delete cc;
				g_new_net.Del(x--);
				#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
					PostMessage(g_netstatus_wnd,WM_USER_TITLEUPDATE,0,0);
				#endif
			}
			else {
				char descstr_l[16+SHA_OUTSIZE*2+32];
				char descstr_s[16+SHA_OUTSIZE*2+32];
				MakeUserStringFromHash(cc->get_remote_pkey_hash(),descstr_l,descstr_s);
				struct in_addr in;
				in.s_addr=cc->get_remote();
				char *ad=inet_ntoa(in);
				bool allowconnection=true;

				if (cc->get_remote_port()==0) {
					int in1=g_config->ReadInt(CONFIG_limitInCons,CONFIG_limitInCons_DEFAULT);
					in1=max(0,in1);
					int in2=g_config->ReadInt(CONFIG_limitInConsPHost,CONFIG_limitInConsPHost_DEFAULT);
					in2=max(0,in2);

					int numqueues=g_mql->GetNumQueues();
					int numincons=0,numsamecons=0,n;
					for (n=0;n<numqueues;n++) {
						C_MessageQueue *cm=g_mql->GetQueue(n);
						C_Connection *con=cm->get_con();
						if (con->get_remote_port()==0) {
							numincons++;
							if (!memcmp(cc->get_remote_pkey_hash(),con->get_remote_pkey_hash(),sizeof(SHA_OUTSIZE))) {
								numsamecons++;
							};
						};
					};

					dbg_printf(ds_Debug,"Limiter: Host %s as %s is already connected %i times of %i total incoming connections! LimitIn=%i LimitInPerhost=%i",ad,descstr_l,numsamecons,numincons,in1,in2);

					if ((in2>0 && numsamecons>=in2) ||
						(in1>0 && numincons>=in1))
					{
						if (in2>0 && numsamecons>=in2) {
							log_printf(ds_Warning,"Limiter: Host %s as %s is already connected more times than limit permits!",ad,descstr_l);
						};
						if (in1>0 && numincons>=in1) {
							log_printf(ds_Warning,"Limiter: Host %s as %s cannot connect, because incoming connection limit is reached!",ad,descstr_l);
						};
						#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
							int a=g_lvnetcons.FindItemByParam((int)cc);
							if (a >= 0) {
								g_lvnetcons.DeleteItem(a);
							};
						#endif
						delete cc;
						g_new_net.Del(x--);
						allowconnection=false;
						#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
							PostMessage(g_netstatus_wnd,WM_USER_TITLEUPDATE,0,0);
						#endif
					}
				};

				if (allowconnection) {
					#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
						int a=g_lvnetcons.FindItemByParam((int)cc);
						if (a >= 0) {
							char newconstr[512];
							struct in_addr in;
							in.s_addr=cc->get_remote();
							char *ad=inet_ntoa(in);
							if (!ad) ad="?";
							int port=cc->get_remote_port();
							if (!port) sprintf(newconstr,"%s (incoming)",ad);
							else sprintf(newconstr,"%s:%d",ad,(int)(unsigned short)port);
							g_lvnetcons.SetItemText(a,0,"OK");
							g_lvnetcons.SetItemText(a,1,newconstr);
							if (port) g_lvnetcons.SetItemText(a,2,"100");
							if (g_extrainf==1) g_lvnetcons.SetItemText(a,3,descstr_l);
							else g_lvnetcons.SetItemText(a,3,descstr_s);
						};
					#endif

					log_printf(ds_Console,"Connected to remote host %s as %s", ad, descstr_l);
					C_MessageQueue *newq=new C_MessageQueue(cc);

					#if 0
						#ifndef _DEBUG
							#error remove 1
						#endif
						dbg_printf(ds_Debug,"recv=%i send=%i",cc->recv_bytes_available(),cc->send_bytes_in_queue());
					#endif

					SendCaps(newq);
					g_mql->AddMessageQueue(newq);
					DoPing(newq);
					g_new_net.Del(x--);
					#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
						PostMessage(g_netstatus_wnd,WM_USER_TITLEUPDATE,0,0);
					#endif
				};
			};
		};
	};
}

void NetKern_Run()
{
	ListenToSocket();
	HandleNewOutCons();
}

void AddConnection(char *str, unsigned short port, int rating)
{
	C_Connection *newcon;
	newcon=new C_Connection(str,port,g_dns);
	newcon->send_bytes(g_con_str,SYNC_SIZE);
	g_new_net.Add(newcon);
	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		char str2[512];
		sprintf(str2,"%s:%d",str,(int)(unsigned short)port);
		g_lvnetcons.InsertItem(0,"Connecting",(int)newcon);
		g_lvnetcons.SetItemText(0,1,str2);
		sprintf(str2,"%d",rating);
		g_lvnetcons.SetItemText(0,2,str2);
	#endif
}

void DoPing(C_MessageQueue *mq)
{
	if (!mq && !g_mql->GetNumQueues()) return;

	int sendping=0;

	C_MessagePing rep;
	if (g_route_traffic&&
		g_port&&
		g_listen&&
		!g_listen->is_error()&&
		g_config->ReadInt(CONFIG_advertise_listen,CONFIG_advertise_listen_DEFAULT)
		)
	{
		if (g_forceip_dynip_mode==0) {
			if (mq) {
				rep.m_ip=mq->get_con()->get_interface();
			}
			else {
				rep.m_ip=g_mql->GetQueue(0)->get_con()->get_interface();
			};
			sendping++;
		}
		else if (g_forceip_dynip_mode==1) {
			if (g_forceip_dynip_addr!=INADDR_NONE) {
				rep.m_ip=g_forceip_dynip_addr;
			};
			sendping++;
		}
		else if (g_forceip_dynip_mode==2) {
			if (g_forceip_dynip_addr!=INADDR_NONE) {
				rep.m_ip=g_forceip_dynip_addr;
				sendping++;
			};
		};
		rep.m_port=g_listen->port();
	};

	if (g_regnick[0]) {
		if (g_regnick[0] != '.') sendping++;
		safe_strncpy(rep.m_nick,g_regnick,sizeof(rep.m_nick));
	};

	if (sendping) {
		T_Message msg={0,};
		msg.data=rep.Make();
		if (msg.data) {
			msg.message_type=MESSAGE_PING;
			msg.message_length=msg.data->GetLength();
			if (g_last_pingid_used && mq) {
				msg.message_guid=g_last_pingid;
				msg.message_ttl=G_DEFAULT_TTL;
				msg.data->Lock();
				msg.message_prio=C_MessageQueueList::GetMessagePriority(msg.message_type);
				C_MessageQueue::calc_md5(&msg,msg.message_md5);
				mq->send_message(&msg);
				msg.data->Unlock();
			}
			else {
				g_mql->send(&msg);
				g_last_pingid=msg.message_guid;
				g_last_pingid_used=1;
			};
		};
	};
	if (mq) { //if we're bringin up a new connection, do a "search"
		T_Message msg={0,};
		msg.data=new C_SHBuf(0);
		msg.message_type=MESSAGE_SEARCH_USERLIST;
		msg.message_length=0;
		if (time(NULL)-g_last_scanid_used > SCANID_REFRESH_DELAY) {
			g_mql->send(&msg);
			g_last_scanid_used=time(NULL);
			g_last_scanid=msg.message_guid;
		}
		else if (mq) {
			msg.message_guid=g_last_scanid;
			msg.message_ttl=G_DEFAULT_TTL;
			msg.data->Lock();
			msg.message_prio=C_MessageQueueList::GetMessagePriority(msg.message_type);
			C_MessageQueue::calc_md5(&msg,msg.message_md5);
			mq->send_message(&msg);
			msg.data->Unlock();
		};
	};
}

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	BOOL WINAPI Net_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		static dlgSizeInfo sizeinf={"net",90,140,440,330};
		static ChildWndResizeItem rlist[]={
			{IDC_ADDCONNECTION,0x1010},
			{IDC_NETCONS,0x0011},
			{IDC_NEWCONNECTION,0x0010},
		};
		switch (uMsg)
		{
		case WM_GETMINMAXINFO:
			{
				LPMINMAXINFO m=(LPMINMAXINFO)lParam;
				if (m) {
					m->ptMinTrackSize.x=245;
					m->ptMinTrackSize.y=180;
				};
				return 0;
			};
		case WM_CLOSE:
			{
				CheckMenuItem(GetMenu(g_mainwnd),
					ID_VIEW_NETWORKCONNECTIONS,MF_UNCHECKED|MF_BYCOMMAND);
				ShowWindow(hwndDlg,SW_HIDE);
				g_config->WriteInt(CONFIG_net_vis,0);
				return 0;
			};
		case WM_SHOWWINDOW:
			{
				static int m_hack;
				if (!m_hack) {
					m_hack=1;
					int m=g_config->ReadInt(CONFIG_net_maximized,CONFIG_net_maximized_DEFAULT);
					handleDialogSizeMsgs(hwndDlg,WM_INITDIALOG,0,0,&sizeinf);
					if (m) ShowWindow(hwndDlg,SW_SHOWMAXIMIZED);
				};
				return 0;
			};
		case WM_MOVE:
			{
				handleDialogSizeMsgs(hwndDlg,uMsg,wParam,lParam,&sizeinf);
				return 0;
			};
		case WM_SIZE:
			{
				if (wParam != SIZE_MINIMIZED) {
					childresize_resize(hwndDlg,rlist,sizeof(rlist)/sizeof(rlist[0]));
					handleDialogSizeMsgs(hwndDlg,uMsg,wParam,lParam,&sizeinf);
				};
				return 0;
			};
		case WM_TIMER:
			{
				if (IsWindowVisible(hwndDlg)) {
					int l=g_lvnetcons.GetCount();
					int x;
					for (x = 0; x < l; x ++) {
						C_Connection *thiscon = (C_Connection *)g_lvnetcons.GetParam(x);
						if (thiscon && thiscon->was_ever_connected()) {
							char str[128];
							int sbps, rbps;
							thiscon->get_last_bps(&sbps,&rbps);

							sprintf(str,"%d.%dk/s out, %d.%dk/s in",
								sbps/1024,((sbps*10)/1024)%10,
								rbps/1024,((rbps*10)/1024)%10);
							if (g_extrainf) {
								for (int n=g_mql->GetNumQueues()-1; n >= 0; n --) {
									C_MessageQueue *q=g_mql->GetQueue(n);
									if (thiscon == q->get_con()) {
										sprintf(str+strlen(str), " (%d,%d,%d)",
											q->get_stat_send(),
											q->get_stat_recv(),
											q->get_stat_drop());
										break;
									};
								};
							};

							g_lvnetcons.SetItemText(x,0,str);
						};
					};
				};
				return 0;
			};
		case WM_INITDIALOG:
			{
				CreateTooltip(GetDlgItem(hwndDlg,IDC_REMOVECATCH),"Disconnect and remove selected host(s)");
				CreateTooltip(GetDlgItem(hwndDlg,IDC_CONNECTCATCH),"Connect to selected host(s)");
				CreateTooltip(GetDlgItem(hwndDlg,IDC_REMOVECONNECTION),"Disconnect from selected host(s)");
				CreateTooltip(GetDlgItem(hwndDlg,IDC_ADDCONNECTION),"Connect to host specified in text box");

				// TODO: check where this button's gone
				//SendDlgItemMessage(hwndDlg,IDC_CLEAR,BM_SETIMAGE,IMAGE_ICON,
				//(LPARAM)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_CLEARALL),IMAGE_ICON,16,16,0));
				SendDlgItemMessage(hwndDlg,IDC_REMOVECATCH,BM_SETIMAGE,IMAGE_ICON,
					(LPARAM)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_DISCONNECT),IMAGE_ICON,16,16,0));
				SendDlgItemMessage(hwndDlg,IDC_CONNECTCATCH,BM_SETIMAGE,IMAGE_ICON,
					(LPARAM)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_CONNECT),IMAGE_ICON,16,16,0));

				SendDlgItemMessage(hwndDlg,IDC_REMOVECONNECTION,BM_SETIMAGE,IMAGE_ICON,
					(LPARAM)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_STOPDL),IMAGE_ICON,16,16,0));
				SendDlgItemMessage(hwndDlg,IDC_ADDCONNECTION,BM_SETIMAGE,IMAGE_ICON,
					(LPARAM)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_CONNECT),IMAGE_ICON,16,16,0));

				SetTimer(hwndDlg,1,250,NULL);
				g_lvnetcons.setwnd(GetDlgItem(hwndDlg,IDC_NETCONS));
				g_lvnetcons.AddCol("Status",g_config->ReadInt(CONFIG_net_col3,CONFIG_net_col3_DEFAULT));
				g_lvnetcons.AddCol("Host",g_config->ReadInt(CONFIG_net_col1,CONFIG_net_col1_DEFAULT));
				g_lvnetcons.AddCol("Rating",g_config->ReadInt(CONFIG_net_col2,CONFIG_net_col2_DEFAULT));
				g_lvnetcons.AddCol("User (key)",g_config->ReadInt(CONFIG_net_col4,CONFIG_net_col4_DEFAULT));
				g_mql->SetStatusListView(&g_lvnetcons);
				SetDlgItemInt(hwndDlg,IDC_NUMCONUP,g_keepup,0);
				childresize_init(hwndDlg,rlist,sizeof(rlist)/sizeof(rlist[0]));
				#if defined(SAVE_CB_HOSTS)&&(SAVE_CB_HOSTS)
					{
						int x;
						int p=g_config->ReadInt(CONFIG_concb_pos,CONFIG_concb_pos_DEFAULT);
						for (x = 0 ; x < MAX_CBHOSTS; x ++) {
							char buf[123];
							sprintf(buf,"concb_%d",(p+x)%MAX_CBHOSTS);
							const char *o=g_config->ReadString(buf,"");
							if (*o) SendDlgItemMessage(hwndDlg,IDC_NEWCONNECTION,CB_INSERTSTRING,0,(LPARAM)o);
						};
					};
				#endif
			};
		case WM_USER_TITLEUPDATE:
			{
				char buf[1024];
				strcpy(buf,"Network Status");
				int numcons=g_mql->GetNumQueues();
				int numneg=g_new_net.GetSize();
				int a=0;

				if (numcons) {
					sprintf(buf+strlen(buf),"%s%d connection%s up",a++?", ":" (",numcons,numcons==1?"":"s");
				};
				if (numneg) {
					sprintf(buf+strlen(buf),"%s%d connecting",a++?", ":" (",numneg);
				};

				if (a) strcat(buf,")");
				else strcat(buf," (network down)");

				strcat(buf," - " APP_NAME);
				SetWndTitle(hwndDlg,buf);
				return 0;
			};
		case WM_DESTROY:
			{
				g_config->WriteInt(CONFIG_net_col3,g_lvnetcons.GetColumnWidth(0));
				g_config->WriteInt(CONFIG_net_col1,g_lvnetcons.GetColumnWidth(1));
				g_config->WriteInt(CONFIG_net_col2,g_lvnetcons.GetColumnWidth(2));
				g_config->WriteInt(CONFIG_net_col4,g_lvnetcons.GetColumnWidth(3));
				return 0;
			};
		case WM_NOTIFY:
			{
				LPNMHDR l=(LPNMHDR)lParam;
				if (l->idFrom==IDC_NETCONS) {
					if (l->code == NM_DBLCLK) {
						SendMessage(hwndDlg,WM_COMMAND,IDC_CONNECTCATCH,0);
					};
					int isSel=!!ListView_GetSelectedCount(l->hwndFrom);
					int isCon=0;
					int isDiscon=0;
					if (isSel) {
						int a,n=g_lvnetcons.GetCount();
						for (a = 0; a < n; a ++) {
							if (g_lvnetcons.GetSelected(a)) {
								if (g_lvnetcons.GetParam(a)) isCon++;
								else isDiscon++;
							};
							if (isCon && isDiscon) break;
						};
					};
					EnableWindow(GetDlgItem(hwndDlg,IDC_REMOVECONNECTION),isCon);
					EnableWindow(GetDlgItem(hwndDlg,IDC_CONNECTCATCH),isDiscon);
					EnableWindow(GetDlgItem(hwndDlg,IDC_REMOVECATCH),isSel);
				};
				return 0;
			};
		case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
				case IDC_NEWCONNECTION:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE) {
							EnableWindow(GetDlgItem(hwndDlg,IDC_ADDCONNECTION),1);
						}
						else {
							char buf[32];
							GetDlgItemText(hwndDlg,IDC_NEWCONNECTION,buf,sizeof(buf));
							EnableWindow(GetDlgItem(hwndDlg,IDC_ADDCONNECTION),!!buf[0]);
						};
						return 0;
					};
				case IDC_NUMCONUP:
					{
						int r;
						BOOL b;
						r=GetDlgItemInt(hwndDlg,IDC_NUMCONUP,&b,0);
						if (b) {
							g_keepup=r;
							g_config->WriteInt(CONFIG_keepupnet,r);
						};
						return 0;
					};
				case IDC_ADDCONNECTION:
					{
						char text[512];
						unsigned short port=CONFIG_port_DEFAULT;
						char *p=text;
						GetDlgItemText(hwndDlg,IDC_NEWCONNECTION,text,511);
						if (strlen(text)>0)
						{
							if (SendDlgItemMessage(hwndDlg,IDC_NEWCONNECTION,CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)text) == CB_ERR) {
								if (SendDlgItemMessage(hwndDlg,IDC_NEWCONNECTION,CB_GETCOUNT,0,0)>MAX_CBHOSTS) {
									SendDlgItemMessage(hwndDlg,IDC_NEWCONNECTION,CB_DELETESTRING,SendDlgItemMessage(hwndDlg,IDC_NEWCONNECTION,CB_GETCOUNT,0,0)-1,0);
								};
								SendDlgItemMessage(hwndDlg,IDC_NEWCONNECTION,CB_INSERTSTRING,0,(LPARAM)text);
								#if defined(SAVE_CB_HOSTS)&&(SAVE_CB_HOSTS)
									int p=g_config->ReadInt(CONFIG_concb_pos,CONFIG_concb_pos_DEFAULT);
									char buf[123];
									sprintf(buf,"concb_%d",p);
									g_config->WriteString(buf,text);
									p++;
									g_config->WriteInt(CONFIG_concb_pos,p%MAX_CBHOSTS);
								#endif
							}
							while (*p && *p != ':') p++;
							if (*p==':') {
								*p++=0;
								port=(unsigned short)(atoi(p)&0xffff);
							};
							if (port) {
								AddConnection(text,port,1);
							};
						};
						return 0;
					};
				case IDC_REMOVECONNECTION:
					{
						int n=g_lvnetcons.GetCount();
						int x;
						for (x = 0; x < n; x++) {
							if (g_lvnetcons.GetSelected(x)) {
								C_Connection *t= (C_Connection*)g_lvnetcons.GetParam(x);
								if (t) t->close(1);
								//g_lvnetcons.DeleteItem(x--);
								//n--;
							};
						};
						break;
				};
				case IDC_REMOVECATCH:
					{
						int n=g_lvnetcons.GetCount();
						int x;
						for (x = 0; x < n; x++) {
							if (g_lvnetcons.GetSelected(x)) {
								C_Connection *t= (C_Connection*)g_lvnetcons.GetParam(x);
								if (t) t->close(1);
								g_lvnetcons.DeleteItem(x--);
								n--;
							};
						};
						break;
					};
				case IDC_CONNECTCATCH:
					{
						int n=g_lvnetcons.GetCount();
						int x;
						for (x = 0; x < n; x++) {
							if (g_lvnetcons.GetSelected(x)) {
								if (!g_lvnetcons.GetParam(x)) {
									char text[512];
									unsigned short port;
									char rat[32];
									g_lvnetcons.GetText(x,1,text,sizeof(text));
									g_lvnetcons.GetText(x,2,rat,sizeof(rat));

									char *p=text;
									while (*p != ':' && *p) p++;
									if (*p) {
										*p++=0;
										port=(unsigned short)(atoi(p)&0xffff);
									}
									else port=CONFIG_port_DEFAULT;

									g_lvnetcons.DeleteItem(x--);
									AddConnection(text,port,atoi(rat));
								};
							};
						};
						break;
					};
				};
				return 0;
			};
		};
		return 0;
	}

#endif//WIN32
