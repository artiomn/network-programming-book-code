/*
WASTE - sharedmain.cpp (unified main file for gui and server)
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

---- some things that would be good to do eventually:

permissions per key:
C=chat
S=search/browse out
s=search/browsable
X=file transfer
K=key distribution
U=uploads
P=pings/ips/etc
?=unknown (all others)
have the connection keep track of the flags and key hash, then have the
prefs update all connections' flags.

improve ip list sorting (use a sorted listview?)

plugin interface
ipv6 support
Ephemeral Diffie-Hellman for session key exchange?

---- bits about how the negotiation works:

RNG: based on RSAREF's, but extended (uses 32 byte state, with 16 bytes counter, and 16 bytes of system entropy
constantly mixed in -- this takes from when messages arrive, mouse movement, timing information on when connections
come up, etc).

On connection:

Md5Chap -> This protocol has slightly changed due to security reasons.
	have a direct look at the code. It should be obvious.

1)
A sends B 16 random bytes (bfhpkA)
A sends B blowFish(bfhpkA,20 byte SHA-1 of public key + 4 pad bytes)
B sends A 16 random bytes (bfhpkB)
B sends A blowFish(bfhpkB,20 byte SHA-1 of public key + 4 pad bytes)

2)
if A knows B's public key, and B knows A public key, continue.

A sends B: RSA(pubkey_B,sKey1 + bfhpkB), sKey1 is 64 random bytes. (+ = concat)
B sends A: RSA(pubKey_A,sKey2 + bfhpkA), sKey2 is 64 random bytes. (+ = concat)

Check to make sure bfhpkA and bfhpkB are correct, and to make sure sKey1^sKey2 is nonzero,
with the last 8 bytes of sKey1 differing from the last 8 bytes of sKey2.

3)
each side initializes blowfish using the first 56 bytes sKey1^sKey2 as key.
A uses the last 8 bytes of sKey2 as recv CBC IV.
B uses the last 8 bytes of sKey1 as recv CBC IV.

4)
each side sends a 16 byte signature, blowfished.
each side verifies 16 byte signature.
message communication begins.
further messages are verified with a MD5 to detect tampering.

*/

#include "stdafx.hpp"

#include "main.hpp"
#include "rsa/md5.hpp"
#include "childwnd.hpp"
#include "m_chat.hpp"
#include "m_search.hpp"
#include "m_ping.hpp"
#include "m_lcaps.hpp"
#include "d_chat.hpp"
#include "prefs.hpp"
#include "xferwnd.hpp"
#include "netq.hpp"
#include "netkern.hpp"
#include "srchwnd.hpp"

#ifdef _DEFINE_SRV
	#include "resourcesrv.hpp"
#else
	#include "resource.hpp"
#endif

#include "build/build.hpp"

#ifdef _DEBUG
	#define VERSION "v" __VER_SFULL __VER_SCUST2 " debug" " (build " __VER_SBUILD ")"
#else
	#define VERSION "v" __VER_SFULL __VER_SCUST2 " (build " __VER_SBUILD ")"
#endif

#ifdef _DEFINE_SRV
	const char g_nameverstr[]=APP_NAME " Server " VERSION;
#else
	const char g_nameverstr[]=APP_NAME " " VERSION;
#endif

C_FileDB *g_database;
C_FileDB *g_newdatabase;
C_AsyncDNS *g_dns;
C_Listen *g_listen;
C_MessageQueueList *g_mql;
C_Config *g_config;

unsigned char g_networkhash[SHA_OUTSIZE];
int g_use_networkhash;
int g_networkhash_PSK;
CBlowfish g_networkhash_PSK_fish;

char g_config_dir[1024];
char g_config_prefix[1024];

#ifdef _WIN32
	char g_config_mainini[1024];
#endif

char g_profile_name[128];
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	int g_extrainf;
#endif
int g_keepup;
int g_conspeed,g_route_traffic;
int g_log_level;
int g_log_flush_auto;
int g_max_simul_dl;
unsigned int g_max_simul_dl_host;
int g_use_accesslist;
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	int g_appendprofiletitles;
#endif
int g_do_autorefresh;
bool g_TriggerDbRefresh;
int g_accept_downloads;
unsigned short g_port;
int g_chat_timestamp;
int g_keydist_flags;

//ADDED Md5Chap DynIp Support
// g_forceip_dynip_mode:
// 0=none/take interface ip
// 1=force
// 2=dynamic resolve
int g_forceip_dynip_mode;
unsigned long g_forceip_dynip_addr;
char g_forceip_name[256];

FILE* _logfile=0;

const char g_def_extlist[]=CONFIG_EXTENSIONS_LIST_DEFAULT;

R_RSA_PRIVATE_KEY g_key;
unsigned char g_pubkeyhash[SHA_OUTSIZE];

char g_regnick[32];

char g_filedlg_ulpath[256];

int g_throttle_flag, g_throttle_send, g_throttle_recv;

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	int g_search_showfull;
#endif

int g_scanloadhack;
char g_scan_status_buf[128];
time_t g_next_refreshtime;

time_t g_last_pingtime;
time_t g_last_bcastkeytime;
T_GUID g_last_scanid;
int g_last_scanid_used;
T_GUID g_last_pingid;
int g_last_pingid_used;
T_GUID g_client_id;
char g_client_id_str[33];

C_ItemList<C_UploadRequest> uploadPrompts;
C_ItemList<C_KeydistRequest> keydistPrompts;

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	HMENU g_context_menus;
	HWND g_mainwnd;
	HICON g_hSmallIcon;
	HINSTANCE g_hInst;
	int g_hidewnd_state;
	char g_performs[4096];  // TODO: check for bo
	int g_search_showfullbytes;
#endif

void main_BroadcastPublicKey(T_Message *src)
{
	C_KeydistRequest rep;
	rep.set_nick(g_regnick);
	rep.set_flags((g_port && g_listen && !g_listen->is_error())?M_KEYDIST_FLAG_LISTEN:0);
	R_RSA_PUBLIC_KEY k;
	k.bits=g_key.bits;
	memcpy(k.exponent,g_key.publicExponent,MAX_RSA_MODULUS_LEN);
	memcpy(k.modulus,g_key.modulus,MAX_RSA_MODULUS_LEN);
	rep.set_key(&k);

	T_Message msg={0,};
	msg.data=rep.Make();
	if (msg.data) {
		if (src) {
			msg.message_guid=src->message_guid;
			msg.message_type=MESSAGE_KEYDIST_REPLY;
		}
		else {
			msg.message_type=MESSAGE_KEYDIST;
		};
		msg.message_length=msg.data->GetLength();
		g_mql->send(&msg);
	};
}

void main_handleKeyDist(C_KeydistRequest *kdr, int pending)
{
	if (!kdr->get_key()->bits) return;
	if (findPublicKeyFromKey(kdr->get_key())) return;

	PKitem *p=(PKitem *)malloc(sizeof(PKitem));

	SHAify m;
	safe_strncpy(p->name,kdr->get_nick(),sizeof(p->name));
	p->pk = *kdr->get_key();
	m.add((unsigned char *)p->pk.modulus,MAX_RSA_MODULUS_LEN);
	m.add((unsigned char *)p->pk.exponent,MAX_RSA_MODULUS_LEN);
	m.final(p->hash);

	if (pending) {
		g_pklist_pending.Add(p);
	}
	else {
		g_pklist.Add(p);
	};

	savePKList();
}

void update_set_port()
{
	delete g_listen;
	g_listen=NULL;

	g_port=0;
	if (g_route_traffic && g_config->ReadInt(CONFIG_listen,CONFIG_listen_DEFAULT)) {
		g_port=(unsigned short)(g_config->ReadInt(CONFIG_port,CONFIG_port_DEFAULT)&0xffff);
	};
	if (g_port) {
		log_printf(ds_Informational,"[main] creating listen object on %d",g_port);
		g_listen = new C_Listen(g_port);
	};
}

void InitialLoadDb()
{
	if (g_config->ReadInt(CONFIG_db_save,CONFIG_db_save_DEFAULT)) {
		g_database=new C_FileDB();
		if (g_config->ReadInt(CONFIG_use_extlist,CONFIG_use_extlist_DEFAULT))
			g_database->UpdateExtList(g_config->ReadString(CONFIG_extlist,g_def_extlist));
		else
			g_database->UpdateExtList("*");
		char dbfile[1024+8];
		sprintf(dbfile,"%s.pr2",g_config_prefix);
		g_database->readIn(dbfile);
		g_scanloadhack=1;
	};
	if (g_config->ReadInt(CONFIG_scanonstartup,CONFIG_scanonstartup_DEFAULT)&&
		((g_config->ReadInt(CONFIG_downloadflags,CONFIG_downloadflags_DEFAULT)&1)!=0)
		)
	{
		doDatabaseRescan();
	};
}

void DoDbAutoRefresh(int &scanworking)
{
	if ((time(NULL) > g_next_refreshtime && g_do_autorefresh)||g_TriggerDbRefresh) {
		g_TriggerDbRefresh=false;
		if (!g_newdatabase && !scanworking) {
				if (g_next_refreshtime) {
					g_next_refreshtime=0;
					log_printf(ds_Informational,"DB: initiating autorescan of files");
					g_newdatabase=new C_FileDB();
					if (g_config->ReadInt(CONFIG_use_extlist,CONFIG_use_extlist_DEFAULT)) {
						g_newdatabase->UpdateExtList(g_config->ReadString(CONFIG_extlist,g_def_extlist));
					}
					else {
						g_newdatabase->UpdateExtList("*");
					};
					g_newdatabase->Scan(g_config->ReadString(CONFIG_databasepath,CONFIG_databasepath_DEFAULT));
					if (!g_database->GetNumFiles()) {
						log_printf(ds_Informational,"DB: making scanning db live");
						delete g_database;
						g_database=g_newdatabase;
					};
				}
				else {
					g_next_refreshtime = time(NULL)+60*g_config->ReadInt(CONFIG_refreshint,DEFAULT_DB_REFRESH_DELAY);
				};
			};
		};
}

static void DoLoopedScanDb(int &scanworking)
{
	if (g_scanloadhack) {
		g_scanloadhack=0;
		if (g_database) {
			sprintf(g_scan_status_buf,"%d files (cached)",g_database->GetNumFiles());
			log_printf(ds_Informational,"DB: %s",g_scan_status_buf);
		};
	};
	if (g_newdatabase) {
		int r=g_newdatabase->DoScan(15,
			(g_newdatabase != g_database && g_database->GetNumFiles()) ? g_database : NULL
			);
		if (r != -1) {
			scanworking=1;
			sprintf(g_scan_status_buf,"Scanning %d",r);
		}
		else {
			sprintf(g_scan_status_buf,"Scanned %d",g_newdatabase->GetNumFiles());
			log_printf(ds_Informational,"DB: %s",g_scan_status_buf);
			if (g_newdatabase != g_database) {
				log_printf(ds_Informational,"DB: replacing old database with temp database");
				delete g_database;
				g_database=g_newdatabase;
			}
			else {
				log_printf(ds_Informational,"DB: removing temp database reference, scanning done");
			};
			g_newdatabase=0;
		};
	};
}

void DoMainLoop()
{
	int scanworking=0;
	DoLoopedScanDb(scanworking);

	int lastqueues=g_mql->GetNumQueues();

	int x;
	NetKern_Run();
	int n=1;
	if (g_conspeed>=64) n++;
	if (g_conspeed>=384) n++;
	if (g_conspeed>=1600) n+=2; //fast connections
	if (g_conspeed>=20000) n+=3; //really fast connections

	for (x = 0; x < n; x ++) {
		Xfer_Run();
		g_mql->run(g_route_traffic);
	};

	int newl=g_mql->GetNumQueues();
	if (lastqueues != newl) {
		MYSRANDUPDATE((unsigned char *)&lastqueues,sizeof(lastqueues));
		#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
			char text[32];
			sprintf(text,":%d",newl);
			SetDlgItemText(g_mainwnd,IDC_NETSTATUS,text);
		#endif
	};

	DoDbAutoRefresh(scanworking);

	if (time(NULL) > g_last_pingtime && newl) { // send ping every PING_DELAY s
		g_last_pingtime = time(NULL)+PING_DELAY;
		DoPing(NULL);

		//flush config and netq
		g_config->Flush();
		#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
			SaveNetQ();
		#endif
	};
	if (time(NULL) > g_last_bcastkeytime && newl) {
		g_last_bcastkeytime = time(NULL)+KEY_BROADCAST_DELAY; //every KEY_BROADCAST_DELAY seconds
		if (g_config->ReadInt(CONFIG_bcastkey,CONFIG_bcastkey_DEFAULT)) {
			main_BroadcastPublicKey();
		};
	};
}

void SaveDbToDisk()
{
	if (g_config->ReadInt(CONFIG_db_save,CONFIG_db_save_DEFAULT) && g_database && g_database != g_newdatabase) {
		char dbfile[1024+8];
		sprintf(dbfile,"%s.pr2",g_config_prefix);
		g_database->writeOut(dbfile);
	};
}

void InitReadClientid()
{
	safe_strncpy(g_client_id_str,g_config->ReadString(CONFIG_clientid128,""),sizeof(g_client_id_str));
	if (MakeID128FromStr(g_client_id_str,&g_client_id)) {
		CreateID128(&g_client_id);
		MakeID128Str(&g_client_id,g_client_id_str);
		g_config->WriteString(CONFIG_clientid128,g_client_id_str);
	};
}

void doDatabaseRescan()
{
	log_printf(ds_Informational,"DB: reinitializing database");
	if (g_database && g_database != g_newdatabase && g_newdatabase) {
		log_printf(ds_Informational,"DB: deleting temp db");
		delete g_newdatabase;
	};
	g_newdatabase=new C_FileDB();
	if (g_config->ReadInt(CONFIG_use_extlist,CONFIG_use_extlist_DEFAULT)) {
		g_newdatabase->UpdateExtList(g_config->ReadString(CONFIG_extlist,g_def_extlist));
	}
	else {
		g_newdatabase->UpdateExtList("*");
	};

	g_newdatabase->Scan(g_config->ReadString(CONFIG_databasepath,CONFIG_databasepath_DEFAULT));
	if (!g_database || !g_database->GetNumFiles()) {
		if (g_database) {
			log_printf(ds_Informational,"DB: making scanning db live");
			delete g_database;
		};
		g_database=g_newdatabase;
	};
}

void main_handleUpload(char *guidstr, char *fnstr, C_UploadRequest *t)
{
	#ifndef _DEFINE_SRV
		int willq=Xfer_WillQ(fnstr,guidstr);
		int p=g_lvrecvq.InsertItem(g_lvrecvq.GetCount(),fnstr,0);
	#endif
	char sizebuf[64];
	strcpy(sizebuf,"?");
	int fs_l,fs_h;
	t->get_fsize(&fs_l,&fs_h);
	if (fs_l != -1 || fs_h != -1) {
		FormatSizeStr64(sizebuf,fs_l,fs_h);
	};
	log_printf(ds_Informational,"Receiving file: %s(%s) from %s",fnstr,sizebuf,t->get_nick());

	#ifndef _DEFINE_SRV
		g_lvrecvq.SetItemText(p,1,sizebuf);
		g_lvrecvq.SetItemText(p,2,guidstr);
		g_files_in_download_queue++;
		if (g_config->ReadInt(CONFIG_aorecv,CONFIG_aorecv_DEFAULT)) {
			HWND h=GetForegroundWindow();
			SendMessage(g_mainwnd,WM_COMMAND,ID_VIEW_TRANSFERS,0);

			if (g_config->ReadInt(CONFIG_aorecv_btf,CONFIG_aorecv_btf_DEFAULT)) SetForegroundWindow(g_xferwnd);
			else SetForegroundWindow(h);

			XferDlg_SetSel(willq);
		};
		RecvQ_UpdateStatusText();
	#else
		const char *path=g_config->ReadString(CONFIG_downloadpath,"");
		if (path&&path[0]) g_recvs.Add(new XferRecv(g_mql,guidstr,sizebuf,fnstr,path));
	#endif
}

void main_MsgCallback(T_Message *message, C_MessageQueueList *_this, C_Connection *cn)
{
	switch(message->message_type)
	{
	case MESSAGE_LOCAL_SATURATE:
		{
			//dbg_printf(ds_Debug,"got a %d byte saturation message",40+message->message_length);
			break;
		}
	case MESSAGE_LOCAL_CAPS:
		{
			C_MessageLocalCaps mlc(message->data);
			int x;
			for (x = 0; x < mlc.get_numcaps(); x ++) {
				int n,v;
				mlc.get_cap(x,&n,&v);
				switch (n)
				{
				case MLC_REMOTEIP:
					{
						if ((g_forceip_dynip_mode==2) && cn && (cn->get_remote_port())) {
							if (!cn->get_has_sent_remoteip()) {
								unsigned long rip;
								cn->set_has_sent_remoteip();
								rip=(unsigned long)v;
								if ((rip!=0)&&(rip!=INADDR_NONE)&&(!IPv4IsLoopback(rip))&&(!IPv4IsPrivateNet(rip))) {
									if (g_forceip_dynip_addr!=rip) {
										char ad2[64];
										struct in_addr in;
										in.s_addr=rip;
										char *ad=inet_ntoa(in);
										safe_strncpy(ad2,ad,64);
										in.s_addr=cn->get_remote();
										ad=inet_ntoa(in);
										log_printf(ds_Informational,"received external ip %s from %s",ad2,ad);
									};
									g_forceip_dynip_addr=rip;
								};
							};
						};
						break;
					};
				case MLC_SATURATION:
					{
						log_printf(ds_Informational,"got request that saturation be %d on this link",v);
						if (cn) cn->set_saturatemode(v);
						// if ((get_saturatemode()&1) && (g_throttle_flag&32)) do outbound saturation
						break;
					};
				case MLC_BANDWIDTH:
					{
						log_printf(ds_Informational,"got request that max sendbuf size be %d on this link",v);
						if (cn) cn->set_max_sendsize(v);
						break;
					};
				};
			};
			break;
		};
	case MESSAGE_KEYDIST_REPLY:
	case MESSAGE_KEYDIST:
		{
			C_KeydistRequest *r=new C_KeydistRequest(message->data);
			if ((r->get_flags() & M_KEYDIST_FLAG_LISTEN) || (g_port && g_listen && !g_listen->is_error()) && r->get_key()->bits) {
				if (g_keydist_flags&1) { //add without prompt
					main_handleKeyDist(r,0);
					delete r;
				}
				else if (g_keydist_flags&2) { //add with prompt
					keydistPrompts.Add(r);
				}
				else { //add to pending
					main_handleKeyDist(r,1);
					delete r;
				};

				if (message->message_type != MESSAGE_KEYDIST_REPLY) {
					main_BroadcastPublicKey(message);
				};
			}
			else {
				delete r;
			};
			break;
		};
	case MESSAGE_PING:
		{
			MYSRANDUPDATE((unsigned char *)&message->message_guid,16);
			C_MessagePing rep(message->data);

			#if (defined(_WIN32)&&(!defined(_DEFINE_SRV)))
				unsigned int a=(unsigned int)(cn->get_interface());// Intellisense bug here
				if (rep.m_port && rep.m_ip && (a != rep.m_ip)) {
					add_to_netq(rep.m_ip,rep.m_port,90,0);
				};
			#endif
			if (rep.m_nick[0] && rep.m_nick[0] != '#' && rep.m_nick[0] != '&' &&
				rep.m_nick[0] != '.')
			{
				main_onGotNick(rep.m_nick,0);
			};
			break;
		};
	case MESSAGE_SEARCH_USERLIST:
		{
			if (g_regnick[0] && g_regnick[0] != '.') {
				C_MessageSearchReply repl;
				repl.set_conspeed(g_conspeed);
				repl.set_guid(&g_client_id);
				repl.add_item(-1,g_regnick,"Node",g_database->GetNumFiles(),g_database->GetNumMB(),g_database->GetLatestTime());
				T_Message msg={0,};
				msg.message_guid=message->message_guid;
				msg.data=repl.Make();
				if (msg.data) {
					msg.message_type=MESSAGE_SEARCH_REPLY;
					msg.message_length=msg.data->GetLength();
					_this->send(&msg);
				};
				break;
			};
		};
	case MESSAGE_SEARCH:
		{
			if ((g_accept_downloads&1) && g_database->GetNumFiles()>0) {
				C_MessageSearchRequest req(message->data);
				char *ss=req.get_searchstring();
				if (
					(ss[0] == '/' && (g_accept_downloads&4)) || (ss[0] && ss[0] != '/' && (g_accept_downloads&2))
					)
				{
					C_MessageSearchReply repl;
					repl.set_conspeed(g_conspeed);
					repl.set_guid(&g_client_id);
					g_database->Search(ss,&repl,_this,message,main_MsgCallback);
				};
				break;
			};
		};
	case MESSAGE_SEARCH_REPLY:
		{
			if (g_last_scanid_used && !memcmp(&g_last_scanid,&message->message_guid,16)) {
				C_MessageSearchReply repl(message->data);
				if (repl.get_numitems()==1) {
					char name[1024],metadata[256];
					if (!repl.get_item(0,NULL,name,metadata,NULL,NULL,NULL))
						main_onGotNick(name,0);
				};
			}
			else {
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
				Search_AddReply(message);
#endif
			};
			break;
		};
	case MESSAGE_FILE_REQUEST:
		{
			C_FileSendRequest *r = new C_FileSendRequest(message->data);
			if (!memcmp(r->get_guid(),&g_client_id,sizeof(g_client_id))) {
				int n=g_sends.GetSize();
				int x;
				for (x = 0; x < n; x ++) {
					XferSend *xs=g_sends.Get(x);
					if (!memcmp(r->get_prev_guid(),xs->get_guid(),16)) {
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
						if (r->is_abort()==2) {
							int a=xs->GetIdx()-UPLOAD_BASE_IDX;
							if (a >= 0 && a < g_uploads.GetSize()) {
								char *p=g_uploads.Get(a);
								if (p) {
									int lvidx;
									while ((lvidx=g_lvsend.FindItemByParam((int)p)) >= 0) {
										g_lvsend.DeleteItem(lvidx);
									};
									free(p);
									g_uploads.Set(a,NULL);
								};
							};
						};
#endif
						xs->set_guid(&message->message_guid);
						xs->onGotMsg(r);
						break;
					};
				};
				if (x == n && (r->is_abort()==3)) { //file exists
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
					int idx=r->get_idx();
					if (idx >= UPLOAD_BASE_IDX) {
						idx-=UPLOAD_BASE_IDX;
						int lvidx=g_lvsend.FindItemByParam((int)g_uploads.Get(idx));
						if (lvidx >= 0) {
							g_lvsend.SetItemParam(lvidx,0);
							g_lvsend.SetItemText(lvidx,3,"File already uploaded.");
						};
					};
#endif
				}
				else if (x == n && !r->is_abort()) { //new file request
#if 0
#ifndef _DEBUG
#error todo
#endif
					dbg_printf(ds_Debug,"XXX: new request on idx=%05i",r->get_idx());
#endif
					if (!g_config->ReadInt(CONFIG_limit_uls,CONFIG_limit_uls_DEFAULT)||
						(n < g_config->ReadInt(CONFIG_ul_limit,CONFIG_ul_limit_DEFAULT)))
					{
						char fn[2048];
						fn[0]=0;
						int idx=r->get_idx();
						if (idx >= 0) {
							if (idx < UPLOAD_BASE_IDX) {
								if (!(g_accept_downloads&1)) fn[0]=0;
								else g_database->GetFile(idx,fn,NULL,NULL,NULL);
							}
							else {
								idx-=UPLOAD_BASE_IDX;
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
								if (idx<g_uploads.GetSize() && g_uploads.Get(idx))
#else
								if (idx<g_uploads.GetSize())
#endif
								{
									safe_strncpy(fn,g_uploads.Get(idx),sizeof(fn));
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
									int lvidx;
									if ((lvidx=g_lvsend.FindItemByParam((int)g_uploads.Get(idx))) >= 0) {
										g_lvsend.DeleteItem(lvidx);
									};
#endif
								};
							};
						};
						if (fn[0]) {
#if 0
#ifndef _DEBUG
#error todo
#endif
							dbg_printf(ds_Debug,"XXX: accept request on idx=%05i",r->get_idx());
#endif
							XferSend *a=new XferSend(_this,&message->message_guid,r,fn);
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
							//remove any timed out files of a->GetName() from r->get_nick()
							n=g_lvsend.GetCount();
							for (x = 0; x < n; x ++) {
								if (g_lvsend.GetParam(x)) continue;
								char buf[1024];
								g_lvsend.GetText(x,3,buf,sizeof(buf));
								if (strncmp(buf,"Timed out",9)) continue;
								g_lvsend.GetText(x,0,buf,sizeof(buf));
								if (strcmp(buf,a->GetName())) continue;
								g_lvsend.GetText(x,1,buf,sizeof(buf));
								if (strcmp(buf,r->get_nick())) continue;
								g_lvsend.DeleteItem(x);
								x--;
								n--;
							};
#endif

							char *err=a->GetError();
							if (err) {
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
								if (!g_config->ReadInt(CONFIG_send_autoclear,CONFIG_send_autoclear_DEFAULT)) {
									g_lvsend.InsertItem(0,a->GetName(),0);
									char buf[32];
									int fs_l,fs_h;
									a->GetSize((unsigned int *)&fs_l,(unsigned int *)&fs_h);
									FormatSizeStr64(buf,fs_l,fs_h);
									g_lvsend.SetItemText(0,1,r->get_nick());
									g_lvsend.SetItemText(0,2,buf);
									g_lvsend.SetItemText(0,3,err);
								};
#endif
								delete a;
							}
							else {
								g_sends.Add(a);
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
								g_lvsend.InsertItem(0,a->GetName(),(int)a);
								char buf[32];
								int fs_l,fs_h;
								a->GetSize((unsigned int *)&fs_l,(unsigned int *)&fs_h);
								FormatSizeStr64(buf,fs_l,fs_h);
								g_lvsend.SetItemText(0,1,r->get_nick());
								g_lvsend.SetItemText(0,2,buf);
								g_lvsend.SetItemText(0,3,"Sending");
#endif
							};
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
							PostMessage(g_xferwnd,WM_USER_TITLEUPDATE,0,0);
#endif
						}
						else {
							// TODO: check if ok for server
							T_Message msg={0,};
							C_FileSendReply reply;
							reply.set_error(1);
							msg.data=reply.Make();
							if (msg.data) {
								msg.message_type=MESSAGE_FILE_REQUEST_REPLY;
								msg.message_length=msg.data->GetLength();
								msg.message_guid=message->message_guid;
								g_mql->send(&msg);
							};
						};
					};
				};
			};
			delete r;
			break;
		};
	case MESSAGE_FILE_REQUEST_REPLY:
		{
			int n=g_recvs.GetSize();
			int x;
			for (x = 0; x < n; x ++) {
				XferRecv *xr=g_recvs.Get(x);
				if (!memcmp(xr->get_guid(),&message->message_guid,16)) {
					xr->onGotMsg(new C_FileSendReply(message->data));
					break;
				};
			};
			break;
		};
	case MESSAGE_CHAT_REPLY:
		{
			#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
				chat_HandleMsg(message);
			#endif
			break;
		}
	case MESSAGE_CHAT:
		{
			if (chat_HandleMsg(message) && g_regnick[0] && g_regnick[0]!='.') {
				//send reply
				C_MessageChatReply rep;
				rep.setnick(g_regnick);

				T_Message msg={0,};
				msg.message_guid=message->message_guid;
				msg.data=rep.Make();
				if (msg.data) {
					msg.message_type=MESSAGE_CHAT_REPLY;
					msg.message_length=msg.data->GetLength();
					_this->send(&msg);
				};
			};
			break;
		};
	case MESSAGE_UPLOAD:
		{
			int upflag=g_config->ReadInt(CONFIG_accept_uploads,CONFIG_accept_uploads_DEFAULT);
			if (upflag&1) {
				C_UploadRequest *r = new C_UploadRequest(message->data);
				if (!stricmp(r->get_dest(),g_client_id_str) || (g_regnick[0] && !stricmp(r->get_dest(),g_regnick))) {
					char *fn=r->get_fn();
					// TODO: check with this better?
					//int weirdfn=!strncmp(fn,"..",2) || strstr(fn,":") || strstr(fn,"\\..") || strstr(fn,"/..") || strstr(fn,"..\\") || strstr(fn,"../") || fn[0]=='\\' || fn[0] == '/';
					int weirdfn=!strncmp(fn,"..",2) || strstr(fn,":") || strstr(fn,"..\\") || strstr(fn,"../") || fn[0]=='\\' || fn[0] == '/';
					if (!(upflag & 4) || weirdfn) {
						char *p=fn;
						while (*p) p++;
						while (p >= fn && *p != '/' && *p != '\\') p--;
						p++;
						if (p != fn || !weirdfn) fn=p;
						else fn="";
					};

					if (fn[0]) {
						if (!(upflag & 2)) {
							char str[64];
							MakeID128Str(r->get_guid(),str);
							sprintf(str+strlen(str),":%d",r->get_idx());
							main_handleUpload(str,fn,r);
						}
						else {
							char *t=strdup(fn);
							r->set_fn(t);
							free(t);
							uploadPrompts.Add(r);
							break;
						};
					}
					else {
						log_printf(ds_Warning,"got upload request that was invalid");
					};
				};
				delete r;
			};
			break;
		};
	default:
		{
			dbg_printf(ds_Debug,"unknown message received : %d",message->message_type);
			break;
		};
	};
}

void main_onGotChannel(const char *cnl)
{
	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		HWND htree=GetDlgItem(g_mainwnd,IDC_CHATROOMS);
		int added=0;
		HTREEITEM h = TreeView_GetChild(htree,TVI_ROOT);
		while (h) {
			char text[512];
			TVITEM i;
			i.mask=TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT;
			i.cchTextMax=sizeof(text);
			i.pszText=text;
			i.hItem=h;
			TreeView_GetItem(htree,&i);
			if (cnl) {
				if (!stricmp(text,cnl)) {
					i.pszText=(char*)cnl;
					i.lParam=time(NULL);
					TreeView_SetItem(htree,&i);
					added=1;
				};
			};

			if (time(NULL) - (unsigned int)i.lParam >  CHANNEL_STARVE_DELAY) {
				h=TreeView_GetNextSibling(htree,h);
				chatroom_item *p=L_Chatroom;
				while(p!=NULL) {
					if (!stricmp(p->channel,text)) break;
					p=p->next;
				};
				if (!p) TreeView_DeleteItem(htree,i.hItem);
			}
			else h=TreeView_GetNextSibling(htree,h);
		};
		if (!added && cnl) {
			TVINSERTSTRUCT i;
			i.hParent=TVI_ROOT;
			i.hInsertAfter=TVI_SORT;
			i.item.mask=TVIF_PARAM|TVIF_TEXT;
			i.item.pszText=(char*)cnl;
			i.item.lParam=time(NULL);
			TreeView_InsertItem(htree,&i);
		};
	#endif
}

void main_onGotNick(const char *nick, int del)
{
	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		if (nick) {
			if (!stricmp(nick,g_regnick)) return;
			KillTimer(g_mainwnd,5);
			SetTimer(g_mainwnd,5,5000,0);
		};
		HWND htree=GetDlgItem(g_mainwnd,IDC_USERS);
		HTREEITEM h=TreeView_GetChild(htree,TVI_ROOT);
		while (h) {
			char text[512];
			TVITEM i;
			i.mask=TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT;
			text[0]=0;
			i.cchTextMax=sizeof(text);
			i.pszText=text;
			i.hItem=h;
			TreeView_GetItem(htree,&i);
			if (nick) {
				if (!stricmp(nick,i.pszText)) {
					if (del) {
						TreeView_DeleteItem(htree,h);
						return;
					};
					i.mask=TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT;
					i.pszText=(LPSTR)nick;
					i.lParam=time(NULL);
					TreeView_SetItem(htree,&i);
					break;
				};
				h=TreeView_GetNextSibling(htree,h);
			}
			else {
				h=TreeView_GetNextSibling(htree,h);
				if (time(NULL)-i.lParam > NICK_STARVE_DELAY) {
					TreeView_DeleteItem(htree,i.hItem);
				};
			};
		};
		if (!h && nick) {
			TVINSERTSTRUCT i;
			i.hParent=TVI_ROOT;
			i.hInsertAfter=TVI_SORT;
			i.item.mask=TVIF_PARAM|TVIF_TEXT;
			i.item.pszText=(LPSTR)nick;
			i.item.lParam=time(NULL);
			TreeView_InsertItem(htree,&i);
		};
	#endif
}

void InitNeworkHash()
{
	const char *buf=g_config->ReadString(CONFIG_networkname,CONFIG_networkname_DEFAULT);
	if (buf[0]) {
		SHAify m;
		m.add((unsigned char *)buf,strlen(buf));
		m.final(g_networkhash);
		m.reset();
		g_use_networkhash=1;
		g_networkhash_PSK=g_config->ReadInt(CONFIG_USE_PSK,CONFIG_USE_PSK_DEFAULT);
		g_networkhash_PSK_fish.Init(g_networkhash,sizeof(g_networkhash));
	}
	else {
		memset(g_networkhash,0,sizeof(g_networkhash));
		g_use_networkhash=0;
		g_networkhash_PSK=0;
		g_networkhash_PSK_fish.Final();
	};
}

void PrepareDownloadDirectory()
{
	if (!g_config->ReadString(CONFIG_downloadpath,"")[0]) {
		char tmp[1024];
		safe_strncpy(tmp,g_config_dir,sizeof(tmp));
		strcat(tmp,"downloads");
		CreateDirectory(tmp,NULL);
		g_config->WriteString(CONFIG_downloadpath,tmp);
	};
}

void UnifiedReadConfig(bool isreload)
{
	char tmp[1024+8];
	sprintf(tmp,"%s.pr0",g_config_prefix);
	log_printf(ds_Console,"loading config from %s",tmp);
	if (g_config) delete g_config;
	g_config = new C_Config(tmp);

	g_log_level=g_config->ReadInt(CONFIG_LOGLEVEL,CONFIG_LOGLEVEL_DEFAULT);
	g_log_flush_auto=g_config->ReadInt(CONFIG_LOG_FLUSH_AUTO,CONFIG_LOG_FLUSH_AUTO_DEFAULT);

	KillPkList();
	loadPKList();

	g_forceip_dynip_mode=g_config->ReadInt(CONFIG_forceip_dynip_mode,CONFIG_forceip_dynip_mode_DEFAULT);
	const char* iptmp = g_config->ReadString(CONFIG_forceip_name,CONFIG_forceip_name_DEFAULT);
	safe_strncpy(g_forceip_name, iptmp, 256);
	g_forceip_dynip_addr=INADDR_NONE;
	update_forceip_dns_resolution();

	g_conspeed=g_config->ReadInt(CONFIG_conspeed,DEFAULT_CONSPEED);
	g_route_traffic=g_config->ReadInt(CONFIG_route,(g_conspeed>MIN_ROUTE_SPEED));
	g_keepup=g_config->ReadInt(CONFIG_keepupnet,CONFIG_keepupnet_DEFAULT);
	g_use_accesslist=g_config->ReadInt(CONFIG_ac_use,CONFIG_ac_use_DEFAULT);
	g_accept_downloads=g_config->ReadInt(CONFIG_downloadflags,CONFIG_downloadflags_DEFAULT);
	g_do_autorefresh=g_config->ReadInt(CONFIG_dorefresh,CONFIG_dorefresh_DEFAULT);
	g_max_simul_dl=g_config->ReadInt(CONFIG_recv_maxdl,CONFIG_recv_maxdl_DEFAULT);
	g_max_simul_dl_host=g_config->ReadInt(CONFIG_recv_maxdl_host,CONFIG_recv_maxdl_host_DEFAULT);
	g_keydist_flags=g_config->ReadInt(CONFIG_keydistflags,CONFIG_keydistflags_DEFAULT);
	g_throttle_flag=g_config->ReadInt(CONFIG_throttleflag,CONFIG_throttleflag_DEFAULT);
	g_throttle_send=g_config->ReadInt(CONFIG_throttlesend,CONFIG_throttlesend_DEFAULT);
	g_throttle_recv=g_config->ReadInt(CONFIG_throttlerecv,CONFIG_throttlerecv_DEFAULT);

	if (!isreload) {
		#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
			const char* performstmp = g_config->ReadString(CONFIG_performs,CONFIG_performs_DEFAULT);
			safe_strncpy(g_performs, performstmp, sizeof(g_performs));
		#endif
	};

	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		g_appendprofiletitles=g_config->ReadInt(CONFIG_appendpt,CONFIG_appendpt_DEFAULT);
		g_extrainf=g_config->ReadInt(CONFIG_extrainf,CONFIG_extrainf_DEFAULT);
		g_search_showfull=g_config->ReadInt(CONFIG_search_showfull,CONFIG_search_showfull_DEFAULT);
		g_search_showfullbytes=g_config->ReadInt(CONFIG_search_showfullb,CONFIG_search_showfullb_DEFAULT);
	#endif

	g_chat_timestamp=g_config->ReadInt(CONFIG_chat_timestamp,CONFIG_chat_timestamp_DEFAULT);

	safe_strncpy(g_regnick,g_config->ReadString(CONFIG_nick,""),sizeof(g_regnick));
	if (g_regnick[0] == '#' || g_regnick[0] == '&') g_regnick[0]=0;

	InitReadClientid();
	InitNeworkHash();
	updateACList(NULL);
}

void SetProgramDirectory(const char *progpath)
{
	char *p;
	progpath;
	#ifdef _WIN32
		{
			GetModuleFileName(NULL,g_config_dir,sizeof(g_config_dir));
		};
	#else
		{
			safe_strncpy(g_config_dir,progpath,sizeof(g_config_dir));
		};
	#endif

	#ifdef _WIN32
		safe_strncpy(g_config_mainini,g_config_dir,sizeof(g_config_mainini));
	#endif

	p=g_config_dir+strlen(g_config_dir);
	while (p >= g_config_dir && *p != DIRCHAR) p--;
	*++p=0;

	#ifdef _WIN32
		p=g_config_mainini+strlen(g_config_mainini);
		while (p >= g_config_mainini && *p != '.' && *p != DIRCHAR) p--;
		*++p=0;
		strcat(g_config_mainini,"ini");
	#endif

	safe_strncpy(g_config_prefix,g_config_dir,sizeof(g_config_prefix));
}

void InitializeNetworkparts()
{
	g_dns=new C_AsyncDNS;
	g_mql = new C_MessageQueueList(main_MsgCallback,DEFAULT_TTL);
	update_set_port();
}
