/*
WASTE - srvmain.cpp (non-GUI client main entry point and a lot of code :)
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

#include "rsa/md5.hpp"
#include "m_upload.hpp"
#include "m_chat.hpp"
#include "m_search.hpp"
#include "m_ping.hpp"
#include "m_file.hpp"
#include "m_keydist.hpp"
#include "m_lcaps.hpp"
#include "xferwnd.hpp"
#include "netkern.hpp"
#include "xfers.hpp"
#include "d_chat.hpp"
#include "license.hpp"

//T_GUID g_search_id;
//unsigned int g_search_id_time;

int g_exit=0;

static void chat_AutoReply(C_MessageChat &chat)
{
	T_Message msg;
	C_MessageChat req;
	req.set_chatstring("[autoreply] I am but a dumb server");
	req.set_dest(chat.get_src());
	req.set_src(g_regnick);
	msg.data=req.Make();
	msg.message_type=MESSAGE_CHAT;
	if (msg.data) {
		msg.message_length=msg.data->GetLength();
		g_mql->send(&msg);
	};
}

//Server version
int chat_HandleMsg(T_Message *message)
{
	C_MessageChat chat(message->data);
	if (chat_handle_whois(chat)) return 0;
	if (chat_IsForMe(chat)) {
		log_printf(ds_Console,"privmsg(%s): %s",chat.get_src(),chat.get_chatstring());
		chat_AutoReply(chat);
		return 1;
	};
	return 0;
};

static void sighandler(int sig)
{
	if (sig == SIGHUP) {
		log_printf(ds_Console,"SIGHUP, rereading config, rereading db");
		UnifiedReadConfig(true);
		g_TriggerDbRefresh=true;
		fflush(_logfile);
	};
	#ifndef _WIN32
		if (sig == SIGUSR1) {
			log_printf(ds_Console,"SIGUSR1");
			fflush(_logfile);
		};
		if (sig == SIGPIPE) log_printf(ds_Informational,"got SIGPIPE!");
	#endif
	if (sig == SIGINT) {
		g_exit=1;
		log_printf(ds_Console,"got SIGINT, setting global exit flag!");
		fflush(_logfile);
	};
}

#ifdef _WIN32
static BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
		{
			sighandler(SIGHUP);
			return true;
		};
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		{
			raise(SIGINT);
			return true;
		};
	};
	return false;
}
#endif

static void installsighandler()
{
	#ifndef _WIN32
		signal(SIGHUP,sighandler);
		signal(SIGUSR1,sighandler);
		signal(SIGPIPE,sighandler);
	#endif
	signal(SIGINT,sighandler);
	#ifdef _WIN32
		SetConsoleCtrlHandler(ConsoleCtrlHandler,true);
		HANDLE hs=GetStdHandle(STD_INPUT_HANDLE);
		DWORD mode;
		if (GetConsoleMode(hs,&mode)) {
			mode|=ENABLE_PROCESSED_INPUT;
			SetConsoleMode(hs,mode);
		};
	#endif
}

int main(int argc, char **argv)
{
	SetProgramDirectory(argv[0]);

	g_log_level=ds_Console;
	_logfile=stderr;

	{
		bool dohelp=false;
		if (argc<2) {
			dohelp=true;
		}
		else {
			if (!strcmp(argv[1],"-i")) {
				log_printf(ds_Console,"Interactive!");
			}
			else if (!strcmp(argv[1],"-L")) {
				char *szLI2;
				szLI2=(char*)malloc(sK0[3]);
				safe_strncpy(szLI2,(char*)sK1[3],sK0[3]);
				dpi(szLI2,4);
				RelpaceCr(szLI2);
				fprintf(_logfile,"\n\n%s\n",szLI2);
				memset(szLI2,0,sK0[3]);free(szLI2);
				return 1;
			}
			else if (!strcmp(argv[1],"-d")) {
				if (argc==3) {
					log_printf(ds_Console,"Forking DAEMON!");
					log_UpdatePath(argv[2],true);
					#ifndef _WIN32
						daemon(1,0);
					#endif
					log_printf(ds_Console,"DAEMON!");
				}
				else {
					dohelp=true;
				};
			}
			else {
				dohelp=true;
			};
		};
		if (dohelp) {
			char *szCR2;
			szCR2=(char*)malloc(sK0[1]);
			safe_strncpy(szCR2,(char*)sK1[1],sK0[1]);
			dpi(szCR2,2);
			RelpaceCr(szCR2);

			log_printf(ds_Console,
				"%s\n"
				"%s\n"
				"\n"
				"Usage: wastesrv <-i> | -d <logfile>\n"
				"\t -L print license\n"
				"\t -i interactive mode\n"
				"\t -d daemon mode (on linux this will put wastesrv in the background)\n"
				"\n"
				"\twastesrv.ini must to be present on Windows\n"
				"\tthe config is default.pr0 to default.pr4\n",
				g_nameverstr,
				szCR2
				);
			memset(szCR2,0,sK0[1]);free(szCR2);
			return 1;
		};
	};

	installsighandler();

	log_printf(ds_Console,"%s starting up...",g_nameverstr);

	MYSRAND();
	if (!g_exit) //emergency break!
	{
		strcat(g_config_prefix,"default");

		#ifdef _WIN32
			WSADATA wsaData;
			if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
				memset(&g_key,0,sizeof(g_key));
				MessageBox(NULL,"Error initializing Winsock\n",APP_NAME " Error",0);
				return 1;
			}
		#endif

		UnifiedReadConfig();

		InitialLoadDb();

		PrepareDownloadDirectory();

		if (!g_key.bits) {
			reloadKey(
				g_config->ReadInt(CONFIG_storepass,CONFIG_storepass_DEFAULT)?
				g_config->ReadString(CONFIG_keypass,CONFIG_keypass_DEFAULT):
				NULL
				);
		};

		InitializeNetworkparts();

		// run loop

		while (!g_exit)
		{
			DoMainLoop();
			Sleep(33);
		};

		// exit
		log_printf(ds_Console,"cleaning up");

		SaveDbToDisk();

		delete g_listen;
		delete g_dns;
		if (g_newdatabase != g_database) delete g_newdatabase;
		delete g_database;
	};

	int x;
	for (x = 0; x < g_recvs.GetSize(); x ++) delete g_recvs.Get(x);
	for (x = 0; x < g_sends.GetSize(); x ++) delete g_sends.Get(x);
	for (x = 0; x < g_new_net.GetSize(); x ++) delete g_new_net.Get(x);
	for (x = 0; x < g_uploads.GetSize(); x ++) free(g_uploads.Get(x));
	if (g_aclist) free(g_aclist);
	KillPkList();

	delete g_mql;

	delete g_config;
	memset(&g_key,0,sizeof(g_key));

	#ifdef _WIN32
		WSACleanup();
	#endif
	log_UpdatePath(NULL);
	return 0;
}
