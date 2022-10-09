/*
WASTE - prefs.cpp (Preferences Dialogs)
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

#include "main.hpp"

#ifdef _DEFINE_SRV
#include "resourcesrv.hpp"
#else
#include "resource.hpp"
#endif

#include "sha.hpp"
#include "childwnd.hpp"
#include "d_chat.hpp"
#include "prefs.hpp"
#include "util.hpp"
#include "srchwnd.hpp"
#include "netq.hpp"
#include "netkern.hpp"
#include "license.hpp"

static COLORREF custcolors[16];

static int CALLBACK WINAPI BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData)
{
	if (uMsg==BFFM_INITIALIZED) {
		char buf[1024];
		if (lpData) GetWindowText((HWND)lpData,buf,sizeof(buf));
		SetWindowText(hwnd,"Choose directory");
		SendMessage(hwnd,BFFM_SETSELECTION,(WPARAM)1,(LPARAM)buf);
	};
	return 0;
}

static void getProfilePath(char *tmp)
{
	GetModuleFileName(g_hInst,tmp,1024);
	char *p=tmp;
	while (*p) p++;
	while (p >= tmp && *p != '\\') p--;
	p[1]=0;
}

/////////////////////////////////////prefs ////////////////////////////////
HWND prefs_hwnd,prefs_cur_wnd;
static int prefs_last_page;

static HTREEITEM lp_v=NULL;
static HTREEITEM _additem(HWND hwnd, HTREEITEM h, char *str, int children, int data)
{
	HTREEITEM h2;
	TV_INSERTSTRUCT is={h,TVI_LAST,{TVIF_PARAM|TVIF_TEXT|TVIF_CHILDREN,0,0,0,str,strlen(str),0,0,children?1:0,data}};
	h2=TreeView_InsertItem(hwnd,&is);
	if (prefs_last_page == data) lp_v=h2;
	return h2;
}

static BOOL CALLBACK Pref_FilesProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		//------------------------------------------------------------------------------
		if (g_config->ReadInt(CONFIG_aotransfer,CONFIG_aotransfer_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_DISPLAY_TRANSFER_ON_DL,BST_CHECKED);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_DL),0);
		};
		if (g_config->ReadInt(CONFIG_aotransfer_btf,CONFIG_aotransfer_btf_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_DL,BST_CHECKED);
		};
		//------------------------------------------------------------------------------
		if (g_config->ReadInt(CONFIG_aoupload,CONFIG_aoupload_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_DISPLAY_TRANSFER_ON_UL,BST_CHECKED);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_UL),0);
		};
		if (g_config->ReadInt(CONFIG_aoupload_btf,CONFIG_aoupload_btf_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_UL,BST_CHECKED);
		};
		//------------------------------------------------------------------------------
		if (g_config->ReadInt(CONFIG_aorecv,CONFIG_aorecv_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_DISPLAY_TRANSFER_ON_DL_BY_REMOTEINIT,BST_CHECKED);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_DL_BY_REMOTEINIT),0);
		};
		if (g_config->ReadInt(CONFIG_aorecv_btf,CONFIG_aorecv_btf_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_DL_BY_REMOTEINIT,BST_CHECKED);
		};
		//------------------------------------------------------------------------------
		if (g_config->ReadInt(CONFIG_DOWNLOAD_ONCE,CONFIG_DOWNLOAD_ONCE_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_DOWNLOAD_ONCE,BST_CHECKED);
		};
		//------------------------------------------------------------------------------
		if (g_config->ReadInt(CONFIG_nickonxfers,CONFIG_nickonxfers_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_NICK_ON_XFER,BST_CHECKED);
		};
		//------------------------------------------------------------------------------
		if (g_config->ReadInt(CONFIG_directxfers,CONFIG_directxfers_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_DIRECT_XFER,BST_CHECKED);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DIRECT_XFER_CONNECT),0);
		};
		if (g_config->ReadInt(CONFIG_directxfers_connect,CONFIG_directxfers_connect_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_DIRECT_XFER_CONNECT,BST_CHECKED);
		};
		//------------------------------------------------------------------------------
	};
	if (uMsg == WM_COMMAND) {
		int a;
		switch (LOWORD(wParam))
		{
		case IDC_CHECK_NICK_ON_XFER:
			{
				g_config->WriteInt(CONFIG_nickonxfers,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_NICK_ON_XFER));
				break;
			};
		case IDC_CHECK_DIRECT_XFER:
			{
				bool t=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_DIRECT_XFER);
				g_config->WriteInt(CONFIG_directxfers,t);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DIRECT_XFER_CONNECT),t);
				break;
			};
		case IDC_CHECK_DIRECT_XFER_CONNECT:
			{
				bool t=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_DIRECT_XFER_CONNECT);
				g_config->WriteInt(CONFIG_directxfers_connect,t);
				break;
			};
		case IDC_CHECK_DISPLAY_TRANSFER_ON_DL:
			{
				g_config->WriteInt(CONFIG_aotransfer,a=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_DISPLAY_TRANSFER_ON_DL));
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_DL),a);
				break;
			};
		case IDC_CHECK_FOCUS_TRANSFER_ON_DL:
			{
				g_config->WriteInt(CONFIG_aotransfer_btf,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_DL));
				break;
			};
		case IDC_CHECK_DISPLAY_TRANSFER_ON_UL:
			{
				g_config->WriteInt(CONFIG_aoupload,a=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_DISPLAY_TRANSFER_ON_UL));
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_UL),a);
				break;
			};
		case IDC_CHECK_FOCUS_TRANSFER_ON_UL:
			{
				g_config->WriteInt(CONFIG_aoupload_btf,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_UL));
				break;
			};
		case IDC_CHECK_DISPLAY_TRANSFER_ON_DL_BY_REMOTEINIT:
			{
				g_config->WriteInt(CONFIG_aorecv,a=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_DISPLAY_TRANSFER_ON_DL_BY_REMOTEINIT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_DL_BY_REMOTEINIT),a);
				break;
			};
		case IDC_CHECK_FOCUS_TRANSFER_ON_DL_BY_REMOTEINIT:
			{
				g_config->WriteInt(CONFIG_aorecv_btf,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_FOCUS_TRANSFER_ON_DL_BY_REMOTEINIT));
				break;
			};
		case IDC_DOWNLOAD_ONCE:
			{
				g_config->WriteInt(CONFIG_DOWNLOAD_ONCE,!!IsDlgButtonChecked(hwndDlg,IDC_DOWNLOAD_ONCE));
				break;
			};
		};
	};
	return 0;
}

static BOOL CALLBACK Pref_ThrottleProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		SetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_OUTBOUND,g_throttle_send,FALSE);
		SetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_INBOUND,g_throttle_recv,FALSE);
		if (g_throttle_flag&1)
			CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_INBOUND,BST_CHECKED);
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_INBOUND),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_RADIO_LIMIT_INBOUND_PER_CONN),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_RADIO_LIMIT_INBOUND_PER_TOTAL),0);
		};
		if (g_throttle_flag&2)
			CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_OUTBOUND,BST_CHECKED);
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_OUTBOUND),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_RADIO_LIMIT_OUTBOUND_PER_CONN),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_RADIO_LIMIT_OUTBOUND_PER_TOTAL),0);
		};
		CheckDlgButton(hwndDlg,(g_throttle_flag&4)?IDC_RADIO_LIMIT_INBOUND_PER_TOTAL:IDC_RADIO_LIMIT_INBOUND_PER_CONN,BST_CHECKED);
		CheckDlgButton(hwndDlg,(g_throttle_flag&8)?IDC_RADIO_LIMIT_OUTBOUND_PER_TOTAL:IDC_RADIO_LIMIT_OUTBOUND_PER_CONN,BST_CHECKED);
		if (g_throttle_flag&16) CheckDlgButton(hwndDlg,IDC_SAT_INC,BST_CHECKED);
		if (g_throttle_flag&32) CheckDlgButton(hwndDlg,IDC_SAT_OUT,BST_CHECKED);
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_CHECK_LIMIT_INBOUND:
		case IDC_CHECK_LIMIT_OUTBOUND:
		case IDC_RADIO_LIMIT_INBOUND_PER_CONN:
		case IDC_RADIO_LIMIT_INBOUND_PER_TOTAL:
		case IDC_RADIO_LIMIT_OUTBOUND_PER_CONN:
		case IDC_RADIO_LIMIT_OUTBOUND_PER_TOTAL:
		case IDC_SAT_INC:
		case IDC_SAT_OUT:
			{
				int otf=g_throttle_flag;
				g_throttle_flag=(IsDlgButtonChecked(hwndDlg,IDC_CHECK_LIMIT_INBOUND)?1:0)|
					(IsDlgButtonChecked(hwndDlg,IDC_CHECK_LIMIT_OUTBOUND)?2:0)|
					(IsDlgButtonChecked(hwndDlg,IDC_RADIO_LIMIT_INBOUND_PER_TOTAL)?4:0)|
					(IsDlgButtonChecked(hwndDlg,IDC_RADIO_LIMIT_OUTBOUND_PER_TOTAL)?8:0)|
					(IsDlgButtonChecked(hwndDlg,IDC_SAT_INC)?16:0)|
					(IsDlgButtonChecked(hwndDlg,IDC_SAT_OUT)?32:0);
				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_INBOUND),g_throttle_flag&1);
				EnableWindow(GetDlgItem(hwndDlg,IDC_RADIO_LIMIT_INBOUND_PER_CONN),g_throttle_flag&1);
				EnableWindow(GetDlgItem(hwndDlg,IDC_RADIO_LIMIT_INBOUND_PER_TOTAL),g_throttle_flag&1);
				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_OUTBOUND),(g_throttle_flag&2)>>1);
				EnableWindow(GetDlgItem(hwndDlg,IDC_RADIO_LIMIT_OUTBOUND_PER_CONN),(g_throttle_flag&2)>>1);
				EnableWindow(GetDlgItem(hwndDlg,IDC_RADIO_LIMIT_OUTBOUND_PER_TOTAL),(g_throttle_flag&2)>>1);
				if ((otf&(32+16))!=(g_throttle_flag&(32+16))) {
					//rebroadcast our new prefs
					RebroadcastCaps(g_mql);
				};
				g_config->WriteInt(CONFIG_throttleflag,g_throttle_flag);
				break;
			};
		case IDC_EDIT_LIMIT_OUTBOUND:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					BOOL t;
					int a=GetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_OUTBOUND,&t,FALSE);
					if (t) g_config->WriteInt(CONFIG_throttlesend,g_throttle_send=a);
				};
				break;
			};
		case IDC_EDIT_LIMIT_INBOUND:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					BOOL t;
					int a=GetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_INBOUND,&t,FALSE);
					if (t) g_config->WriteInt(CONFIG_throttlerecv,g_throttle_recv=a);
				};
				break;
			};
		};
	};
	return 0;
}

static BOOL CALLBACK Pref_ProfilesProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		SetDlgItemText(hwndDlg,IDC_EDIT_PROFILENAME,g_profile_name);
		if (g_appendprofiletitles) CheckDlgButton(hwndDlg,IDC_CHECK_APPEND_PRIFILE_NAME,BST_CHECKED);
		if (GetPrivateProfileInt("config","showprofiles",0,g_config_mainini)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_SHOW_PROFILEMAN_ON_START,BST_CHECKED);
		};

		if (g_log_level>0) {
			g_log_level=max(CONFIG_LOGLEVEL_MIN,min(CONFIG_LOGLEVEL_MAX,g_log_level));
			CheckDlgButton(hwndDlg,IDC_ENABLELOG,BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDITLOGLOCATION),1);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTONLOGLOCATION),1);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTONLOGFLUSH),1);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDITLOGLEVEL),1);
			EnableWindow(GetDlgItem(hwndDlg,IDC_ENABLEIMMEDIATEFLUSH),1);
			char szll[sizeof(int)*3+1];szll[0]=0;
			sprintf(szll,"%u",g_log_level);
			SetDlgItemText(hwndDlg,IDC_EDITLOGLEVEL,szll);
			if (g_log_flush_auto) {
				CheckDlgButton(hwndDlg,IDC_ENABLEIMMEDIATEFLUSH,BST_CHECKED);
			};
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDITLOGLOCATION),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTONLOGLOCATION),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTONLOGFLUSH),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDITLOGLEVEL),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_ENABLEIMMEDIATEFLUSH),0);
		};
		SendDlgItemMessage(hwndDlg,IDC_EDITLOGLEVEL,EM_LIMITTEXT,1,0);

		SetDlgItemText(hwndDlg,IDC_EDITLOGLOCATION,g_config->ReadString(CONFIG_logpath,""));
		SetDlgItemText(hwndDlg,IDC_STATUSLOGERROR,"Path is checked here");
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_START_PROFILE_MANAGER:
			{
				g_config->Flush();
				PROCESS_INFORMATION ProcInfo={0,};
				STARTUPINFO StartUp={0,};
				StartUp.cb=sizeof(StartUp);
				char tmp[1024];
				tmp[0]='\"';
				GetModuleFileName(g_hInst,tmp+1,sizeof(tmp)-32);
				strcat(tmp,"\" /profile=\"\"");
				CreateProcess(NULL,tmp,NULL,NULL,FALSE,0,NULL,NULL,&StartUp, &ProcInfo);
				break;
			};
		case IDC_CHECK_SHOW_PROFILEMAN_ON_START:
			{
				WritePrivateProfileString("config","showprofiles",IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOW_PROFILEMAN_ON_START)?"1":"0",g_config_mainini);
				break;
			};
		case IDC_CHECK_APPEND_PRIFILE_NAME:
			{
				g_config->WriteInt(CONFIG_appendpt,g_appendprofiletitles=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_APPEND_PRIFILE_NAME));
				SetWndTitle(GetParent(hwndDlg),APP_NAME " Preferences");
				SendMessage(g_mainwnd,WM_USER_TITLEUPDATE,0,0);
				break;
			};
		case IDC_ENABLELOG:
			{
				if (IsDlgButtonChecked(hwndDlg,IDC_ENABLELOG)) {
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDITLOGLOCATION),1);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTONLOGLOCATION),1);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTONLOGFLUSH),1);
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDITLOGLEVEL),1);
					EnableWindow(GetDlgItem(hwndDlg,IDC_ENABLEIMMEDIATEFLUSH),1);
					int ll=CONFIG_LOGLEVEL_DEFAULT;
					ll=max(CONFIG_LOGLEVEL_MIN,min(CONFIG_LOGLEVEL_MAX,ll));
					char szll[sizeof(int)*3+1];szll[0]=0;
					sprintf(szll,"%u",ll);
					SetDlgItemText(hwndDlg,IDC_EDITLOGLEVEL,szll);
					g_config->WriteInt(CONFIG_LOGLEVEL,ll);
					g_log_level=ll;
					if(!log_UpdatePath(g_config->ReadString(CONFIG_logpath,""))) {
						SetDlgItemText(hwndDlg,IDC_EDITLOGLOCATION,"");
						g_config->WriteString(CONFIG_logpath,"");
					};
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDITLOGLOCATION),0);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTONLOGLOCATION),0);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTONLOGFLUSH),0);
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDITLOGLEVEL),0);
					EnableWindow(GetDlgItem(hwndDlg,IDC_ENABLEIMMEDIATEFLUSH),0);
					SetDlgItemText(hwndDlg,IDC_EDITLOGLEVEL,"");
					g_config->WriteInt(CONFIG_LOGLEVEL,0);
					g_log_level=0;
					log_UpdatePath(NULL);
				};
				break;
			};
		case IDC_EDITLOGLOCATION:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS) {
					char name[1024];
					name[0]=0;
					GetDlgItemText(hwndDlg,IDC_EDITLOGLOCATION,name,sizeof(name));
					if (!log_UpdatePath(name)) {
						SetDlgItemText(hwndDlg,IDC_STATUSLOGERROR,"The directory or file is not accesible");
					}
					else {
						g_config->WriteString(CONFIG_logpath,name);
						SetDlgItemText(hwndDlg,IDC_STATUSLOGERROR,"The directory is accessible");
					};
				};
				break;
			};
		case IDC_EDITLOGLEVEL:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS) {
					char szll[sizeof(int)*3+1];
					int ll;
					szll[0]=0;
					GetDlgItemText(hwndDlg,IDC_EDITLOGLEVEL,szll,sizeof(szll));
					ll=atoi(szll);
					ll=max(CONFIG_LOGLEVEL_MIN,min(CONFIG_LOGLEVEL_MAX,ll));
					g_config->WriteInt(CONFIG_LOGLEVEL,ll);
					g_log_level=ll;
					sprintf(szll,"%u",ll);
					SetDlgItemText(hwndDlg,IDC_EDITLOGLEVEL,szll);
				};
				break;
			};
		case IDC_BUTTONLOGLOCATION:
			{
				BROWSEINFO bi={0};
				ITEMIDLIST *idlist;
				char name[1024];
				GetDlgItemText(hwndDlg,IDC_EDITLOGLOCATION,name,sizeof(name));
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = name;
				bi.lpfn=BrowseCallbackProc;
				bi.lParam=(LPARAM)GetDlgItem(hwndDlg,IDC_EDITLOGLOCATION);
				bi.lpszTitle = "Select a directory:";
				bi.ulFlags = BIF_RETURNONLYFSDIRS;
				idlist = SHBrowseForFolder( &bi );
				if (idlist) {
					SHGetPathFromIDList( idlist, name );
					IMalloc *m;
					SHGetMalloc(&m);
					m->Free(idlist);
					m->Release();
					SetDlgItemText(hwndDlg,IDC_EDITLOGLOCATION,name);
					SetFocus(GetDlgItem(hwndDlg,IDC_EDITLOGLOCATION));
				};
				break;
			};
		case IDC_BUTTONLOGFLUSH:
			{
				if (_logfile) fflush(_logfile);
				break;
			};
		case IDC_ENABLEIMMEDIATEFLUSH:
			{
				g_log_flush_auto=IsDlgButtonChecked(hwndDlg,IDC_ENABLEIMMEDIATEFLUSH)?1:0;
				g_config->WriteInt(CONFIG_LOG_FLUSH_AUTO,g_log_flush_auto);
				break;
			};
		};
	};
	return 0;
}

static BOOL CALLBACK Pref_DisplayProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG) {
		if (g_config->ReadInt(CONFIG_confirmquit,CONFIG_confirmquit_DEFAULT))
			CheckDlgButton(hwndDlg,IDC_CHECK_CONFIRM_EXIT,BST_CHECKED);

		if (g_config->ReadInt(CONFIG_systray,CONFIG_systray_DEFAULT))
			CheckDlgButton(hwndDlg,IDC_CHECK_DISPLAY_IN_TRAY,BST_CHECKED);

		if (g_config->ReadInt(CONFIG_toolwindow,CONFIG_toolwindow_DEFAULT))
			CheckDlgButton(hwndDlg,IDC_CHECK_DISPLAY_AS_TOOLWIN,BST_CHECKED);

		if (g_config->ReadInt(CONFIG_systray_hide,CONFIG_systray_hide_DEFAULT))
			CheckDlgButton(hwndDlg,IDC_CHECK_TRAY_AUTOHIDE,BST_CHECKED);

		if (g_config->ReadInt(CONFIG_srchcb_use,CONFIG_srchcb_use_DEFAULT))
			CheckDlgButton(hwndDlg,IDC_CHECK_SAVE_BROWSE_MRU,BST_CHECKED);

		if (g_search_showfull==1)
			CheckDlgButton(hwndDlg,IDC_RADIO_SRF,BST_CHECKED);
		else if (g_search_showfull==2)
			CheckDlgButton(hwndDlg,IDC_RADIO_SRP,BST_CHECKED);
		else
			CheckDlgButton(hwndDlg,IDC_RADIO_SRN,BST_CHECKED);

		if (!g_search_showfullbytes)
			CheckDlgButton(hwndDlg,IDC_SFB,BST_CHECKED);

		if (g_extrainf)
			CheckDlgButton(hwndDlg,IDC_CHECK_SHOW_DIAGNOSTIC_INFOS,BST_CHECKED);

		if (g_config->ReadInt(CONFIG_hideallonmin,CONFIG_hideallonmin_DEFAULT))
			CheckDlgButton(hwndDlg,IDC_CHECK_HIDE_ALL_ON_MIN,BST_CHECKED);

	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_USERLIST_BACKCOLOR:
		case IDC_BUTTON_USERLIST_TEXTCOLOR:
			{
				CHOOSECOLOR cs;
				cs.lStructSize = sizeof(cs);
				cs.hwndOwner = hwndDlg;
				cs.hInstance = 0;
				cs.rgbResult=g_config->ReadInt(LOWORD(wParam) == IDC_BUTTON_USERLIST_BACKCOLOR ? "mul_bgc" : "mul_color",LOWORD(wParam) == IDC_BUTTON_USERLIST_BACKCOLOR ? 0xffffff : 0);
				cs.lpCustColors = custcolors;
				cs.Flags = CC_RGBINIT|CC_FULLOPEN;
				if (ChooseColor(&cs)) {
					g_config->WriteInt(LOWORD(wParam) == IDC_BUTTON_USERLIST_BACKCOLOR ? "mul_bgc" : "mul_color",cs.rgbResult);
					SendMessage(g_mainwnd,WM_USER_TITLEUPDATE,0,0);
					InvalidateRect(hwndDlg,NULL,FALSE);
				};
				break;
			};
		case IDC_CHECK_SAVE_BROWSE_MRU:
			{
				g_config->WriteInt(CONFIG_srchcb_use,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_SAVE_BROWSE_MRU));
				break;
			};
		case IDC_CHECK_DISPLAY_IN_TRAY:
			{
				int s=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_DISPLAY_IN_TRAY);
				g_config->WriteInt(CONFIG_systray,s);
				if (!s != !systray_state) {
					if (s) systray_add(g_mainwnd,g_hSmallIcon);
					else systray_del(g_mainwnd);
				};
				break;
			};
		case IDC_CHECK_DISPLAY_AS_TOOLWIN:
			{
				int s=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_DISPLAY_AS_TOOLWIN);
				g_config->WriteInt(CONFIG_toolwindow,s);
				toolWindowSet(s);
				break;
			};
		case IDC_CHECK_TRAY_AUTOHIDE:
			{
				g_config->WriteInt(CONFIG_systray_hide,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_TRAY_AUTOHIDE));
				break;
			};
		case IDC_CHECK_HIDE_ALL_ON_MIN:
			{
				g_config->WriteInt(CONFIG_hideallonmin,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_HIDE_ALL_ON_MIN));
				break;
			};
		case IDC_CHECK_CONFIRM_EXIT:
			{
				g_config->WriteInt(CONFIG_confirmquit,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_CONFIRM_EXIT));
				break;
			};
		case IDC_SFB:
			{
				g_search_showfullbytes = !IsDlgButtonChecked(hwndDlg,IDC_SFB);
				g_config->WriteInt(CONFIG_search_showfullb,g_search_showfullbytes);
				Search_Resort();
				break;
			};
		case IDC_RADIO_SRF:
		case IDC_RADIO_SRP:
		case IDC_RADIO_SRN:
			{
				if (IsDlgButtonChecked(hwndDlg,IDC_RADIO_SRF)) g_search_showfull=1;
				else if (IsDlgButtonChecked(hwndDlg,IDC_RADIO_SRP)) g_search_showfull=2;
				else g_search_showfull=0;
				g_config->WriteInt(CONFIG_search_showfull,g_search_showfull);
				Search_Resort();
				break;
			};
		case IDC_CHECK_SHOW_DIAGNOSTIC_INFOS:
			{
				g_config->WriteInt(CONFIG_extrainf,g_extrainf=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOW_DIAGNOSTIC_INFOS));
				break;
			};
		};
	};
	if (uMsg == WM_DRAWITEM) {
		DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT *)lParam;
		if (di->CtlID == IDC_BUTTON_USERLIST_BACKCOLOR || di->CtlID == IDC_BUTTON_USERLIST_TEXTCOLOR) {
			int color=g_config->ReadInt(di->CtlID == IDC_BUTTON_USERLIST_BACKCOLOR ? "mul_bgc" : "mul_color",
				di->CtlID == IDC_BUTTON_USERLIST_BACKCOLOR ? 0xffffff : 0);
			HBRUSH hBrush,hOldBrush;
			LOGBRUSH lb={BS_SOLID,color,0};
			hBrush = CreateBrushIndirect(&lb);
			hOldBrush=(HBRUSH)SelectObject(di->hDC,hBrush);
			Rectangle(di->hDC,di->rcItem.left,di->rcItem.top,di->rcItem.right,di->rcItem.bottom);
			SelectObject(di->hDC,hOldBrush);
			DeleteObject(hBrush);
		};
	};
	return 0;
}

static char acedit_text[32];
static int acedit_allow;

static BOOL CALLBACK ACeditProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		SendDlgItemMessage(hwndDlg,IDC_EDIT_IP_AND_MASK,EM_LIMITTEXT,sizeof(acedit_text)-1,0);
		SetDlgItemText(hwndDlg,IDC_EDIT_IP_AND_MASK,acedit_text);
		CheckDlgButton(hwndDlg,acedit_allow?IDC_RADIO_ALLOW_ACCESS:IDC_RADIO_DENY_ACCESS,BST_CHECKED);
	};
	if (uMsg == WM_CLOSE) EndDialog(hwndDlg,0);
	if (uMsg == WM_COMMAND) {
		if (LOWORD(wParam) == IDOK) {
			acedit_allow=!!IsDlgButtonChecked(hwndDlg,IDC_RADIO_ALLOW_ACCESS);
			GetDlgItemText(hwndDlg,IDC_EDIT_IP_AND_MASK,acedit_text,sizeof(acedit_text));
			acedit_text[sizeof(acedit_text)-1]=0;
			char tmp[32];
			safe_strncpy(tmp,acedit_text,sizeof(tmp));
			if (tmp[0] && strstr(tmp,"/")) {
				char *p=strstr(tmp,"/");
				*p++=0;
				if (*p <= '9' && *p >= '0') {
					unsigned int ip=inet_addr(tmp);
					unsigned int mask=atoi(p);
					if (mask!=0x20) mask&=0x1f; // need /32 too
					if (inet_addr(tmp) != INADDR_NONE) {
						ip=ip&IPv4NetMask(mask);
						struct in_addr in;
						in.s_addr=ip;
						char *t=inet_ntoa(in);
						sprintf(acedit_text,"%s/%i",t,mask);
						EndDialog(hwndDlg,1);
					};
				};
			};
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hwndDlg,0);
		};

	};
	return 0;
}

static void editAC(HWND hwndDlg, int idx, W_ListView *lv) //return 1 on
{
	if (idx >= 0) {
		lv->GetText(idx,0,acedit_text,sizeof(acedit_text));
		acedit_allow = acedit_text[0]!='D';

		lv->GetText(idx,1,acedit_text,sizeof(acedit_text));
	}
	else {
		strcpy(acedit_text,"0.0.0.0/0");
		acedit_allow=1;
	};
	if (DialogBox(g_hInst,MAKEINTRESOURCE(IDD_ACEDIT),hwndDlg,ACeditProc)) {
		if (idx >= 0) {
			lv->SetItemText(idx,0,acedit_allow?"Allow":"Deny");
			lv->SetItemText(idx,1,acedit_text);
		}
		else {
			int p=lv->InsertItem(lv->GetCount(),acedit_allow?"Allow":"Deny",0);
			lv->SetItemText(p,1,acedit_text);
		};
		updateACList(lv);
	};
}

static void updPortText(HWND hwndDlg)
{
	char buf[128];

	if (g_port) {
		if (!g_listen || g_listen->is_error())
			sprintf(buf,"Error listening on port %d",g_port);
		else sprintf(buf,"Listening on port %d",g_port);
	}
	else {
		strcpy(buf,"Listening disabled");
	};
	SetDlgItemText(hwndDlg,IDC_CURPORT,buf);
}

static BOOL CALLBACK Pref_NetworkProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		int x;
		for (x = 0; x < sizeof(conspeed_strs)/sizeof(conspeed_strs[0]); x ++) {
			SendDlgItemMessage(hwndDlg,IDC_CONSPEED,CB_ADDSTRING,0,(long)conspeed_strs[x]);
		};
		for (x = 0; x < sizeof(conspeed_strs)/sizeof(conspeed_strs[0]); x ++) {
			if (g_conspeed <= conspeed_speeds[x]) break;
		};
		if (x == sizeof(conspeed_strs)/sizeof(conspeed_strs[0])) x--;
		SendDlgItemMessage(hwndDlg,IDC_CONSPEED,CB_SETCURSEL,x,0);

		if (g_route_traffic)
			CheckDlgButton(hwndDlg,IDC_CHECK_ROUTE,BST_CHECKED);
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_LISTEN),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_PORT),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_UPDPORT),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_ADVERTISE),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_LIMIT_IN),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_LIMIT_IN_HO),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN_HO),0);
		};

		if (g_config->ReadInt(CONFIG_advertise_listen,CONFIG_advertise_listen_DEFAULT))
			CheckDlgButton(hwndDlg,IDC_CHECK_ADVERTISE,BST_CHECKED);

		if (g_config->ReadInt(CONFIG_listen,CONFIG_listen_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_LISTEN,BST_CHECKED);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_PORT),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_UPDPORT),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_ADVERTISE),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_LIMIT_IN),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_LIMIT_IN_HO),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN_HO),0);
		};
		SetDlgItemInt(hwndDlg,IDC_PORT,g_config->ReadInt(CONFIG_port,CONFIG_port_DEFAULT),FALSE);
		updPortText(hwndDlg);

		int in1=g_config->ReadInt(CONFIG_limitInCons,CONFIG_limitInCons_DEFAULT);
		in1=max(0,in1);
		int in2=g_config->ReadInt(CONFIG_limitInConsPHost,CONFIG_limitInConsPHost_DEFAULT);
		in2=max(0,in2);
		if (in1>0) {
			CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_IN,BST_CHECKED);
			SetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_IN,in1,FALSE);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN),0);
		};
		if (in2>0) {
			CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_IN_HO,BST_CHECKED);
			SetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_IN_HO,in2,FALSE);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN_HO),0);
		};
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_EDIT_LIMIT_IN:
		case IDC_EDIT_LIMIT_IN_HO:
			{
				if (HIWORD(wParam) != EN_KILLFOCUS) return 0;
				// Fallthrough
			};
		case IDC_CHECK_LIMIT_IN:
		case IDC_CHECK_LIMIT_IN_HO:
			{
				BOOL t;
				int ic1=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_LIMIT_IN);
				int ic2=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_LIMIT_IN_HO);
				int in1,in2;
				if (ic1) {
					in1=GetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_IN,&t,FALSE);
					if (!t) in1=CONFIG_limitInCons_DEFAULT;
					in1=max(1,in1);
				}
				else {
					in1=0;
				};
				if (ic2) {
					in2=GetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_IN_HO,&t,FALSE);
					if (!t) in2=CONFIG_limitInConsPHost_DEFAULT;
					in2=max(1,in2);
				}
				else {
					in2=0;
				};

				g_config->WriteInt(CONFIG_limitInCons,in1);
				g_config->WriteInt(CONFIG_limitInConsPHost,in2);

				if (in1>0) {
					CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_IN,BST_CHECKED);
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN),1);
					SetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_IN,in1,FALSE);
				}
				else {
					CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_IN,BST_UNCHECKED);
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN),0);
					SetDlgItemText(hwndDlg,IDC_EDIT_LIMIT_IN,"");
				};

				if (in2>0) {
					CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_IN_HO,BST_CHECKED);
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN_HO),1);
					SetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_IN_HO,in2,FALSE);
				}
				else {
					CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_IN_HO,BST_UNCHECKED);
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN_HO),0);
					SetDlgItemText(hwndDlg,IDC_EDIT_LIMIT_IN_HO,"");
				};
				return 0;
			};
		case IDC_CONSPEED:
			{
				if (HIWORD(wParam) == CBN_SELCHANGE) {
					int x=SendDlgItemMessage(hwndDlg,IDC_CONSPEED,CB_GETCURSEL,0,0);
					if (x >= 0 && x < sizeof(conspeed_strs)/sizeof(conspeed_strs[0])) {
						if (conspeed_speeds[x] != g_conspeed) {
							g_conspeed=conspeed_speeds[x];
							g_config->WriteInt(CONFIG_conspeed,conspeed_speeds[x]);
							g_route_traffic=conspeed_speeds[x]>MIN_ROUTE_SPEED;
							CheckDlgButton(hwndDlg,IDC_CHECK_ROUTE,conspeed_speeds[x]>MIN_ROUTE_SPEED?BST_CHECKED:BST_UNCHECKED);
							RebroadcastCaps(g_mql);
						};
					};
					// Fallthrough
				}
				else {
					return 0;
				};
			};
		case IDC_CHECK_ROUTE:
		case IDC_CHECK_LISTEN:
			{
				g_route_traffic=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_ROUTE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_LISTEN),g_route_traffic);
				g_config->WriteInt(CONFIG_route,g_route_traffic);
				int a=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_LISTEN);
				g_config->WriteInt(CONFIG_listen,a);
				if (!g_route_traffic) a=0;
				update_set_port();
				updPortText(hwndDlg);
				EnableWindow(GetDlgItem(hwndDlg,IDC_PORT),a);
				EnableWindow(GetDlgItem(hwndDlg,IDC_UPDPORT),a);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_ADVERTISE),a);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_LIMIT_IN),a);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_LIMIT_IN_HO),a);
				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN),a);
				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_IN_HO),a);
				return 0;
			};
		case IDC_UPDPORT:
			{
				BOOL t;
				int r=GetDlgItemInt(hwndDlg,IDC_PORT,&t,FALSE);
				if (t) g_config->WriteInt(CONFIG_port,r);
				update_set_port();
				updPortText(hwndDlg);
				return 0;
			};
		case IDC_CHECK_ADVERTISE:
			{
				g_config->WriteInt(CONFIG_advertise_listen,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_ADVERTISE));
				return 0;
			};
		};
		return 0;
	};
	return 0;
}

static BOOL CALLBACK Pref_Network2Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static W_ListView *lv;
	if (uMsg == WM_INITDIALOG) {
		if (lv) {
			delete lv;
			lv=NULL;
		};
		lv=new W_ListView;
		lv->setwnd(GetDlgItem(hwndDlg,IDC_LIST_ACL));
		lv->AddCol("Type",80);
		lv->AddCol("Address/Mask",140);

		if (g_config->ReadInt(CONFIG_ac_use,CONFIG_ac_use_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_USE_ACL,BST_CHECKED);
		};
		int n=g_config->ReadInt(CONFIG_ac_cnt,CONFIG_ac_cnt_DEFAULT);
		int x;
		int p=0;
		for (x = 0; x < n; x ++) {
			char buf[64];
			sprintf(buf,"ac_%d",x);
			const char *t=g_config->ReadString(buf,"");
			if (*t == 'A' || *t == 'D') {
				int a=lv->InsertItem(p,*t == 'A' ? "Allow" : "Deny",0);
				lv->SetItemText(a,1,t+1);
				p++;
			};
		};
	};
	if (uMsg == WM_DESTROY) {
		if (lv) {
			delete lv;
			lv=NULL;
		};
	};
	if (uMsg == WM_NOTIFY) {
		LPNMHDR l=(LPNMHDR)lParam;
		if (l->idFrom==IDC_LIST_ACL) {
			if (l->code == NM_DBLCLK)
				SendMessage(hwndDlg,WM_COMMAND,IDC_BUTTON_EDIT,0);
			else {
				int s=!!ListView_GetSelectedCount(l->hwndFrom);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_EDIT),s);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_REMOVE),s);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_MOVEUP),s);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_MOVEDOWN),s);
			};
		};
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_EDIT:
			{
				int x;
				for (x = 0; x < lv->GetCount() && !lv->GetSelected(x); x ++);
				if (x < lv->GetCount()) {
					editAC(hwndDlg,x,lv);
				};
				return 0;
			};
		case IDC_BUTTON_ADD:
			{
				editAC(hwndDlg,-1,lv);
				return 0;
			};
		case IDC_BUTTON_REMOVE:
			{
				int x;
				for (x = 0; x < lv->GetCount() && !lv->GetSelected(x); x ++);
				if (x < lv->GetCount()) {
					lv->DeleteItem(x);
					updateACList(lv);
				};
				return 0;
			};
		case IDC_BUTTON_MOVEUP:
		case IDC_BUTTON_MOVEDOWN:
			{
				int x;
				for (x = 0; x < lv->GetCount() && !lv->GetSelected(x); x ++);
				if (x < lv->GetCount() && (LOWORD(wParam) == IDC_BUTTON_MOVEDOWN || x) &&
					(LOWORD(wParam) == IDC_BUTTON_MOVEUP || x < lv->GetCount()-1))
				{
					char buf[4][256];
					int a;
					if (LOWORD(wParam) == IDC_BUTTON_MOVEUP) x--;
					for (a = 0; a < 4; a ++) lv->GetText(x+(a&1),a/2,buf[a],256);
					for (a = 0; a < 4; a ++) lv->SetItemText(x+!(a&1),a/2,buf[a]);

					if (LOWORD(wParam) == IDC_BUTTON_MOVEUP) lv->SetSelected(x);
					else lv->SetSelected(x+1);
					updateACList(lv);
				};
				break;
			};
		case IDC_CHECK_USE_ACL:
			{
				g_config->WriteInt(CONFIG_ac_use,g_use_accesslist=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_USE_ACL));
				break;
			};
		};
	};
	return 0;
}

static void Pref_Network3Proc_Help1(HWND hwndDlg)
{
	if(strlen(g_forceip_name) == 0) {
		struct in_addr in;
		in.s_addr=g_forceip_dynip_addr;
		char *t=inet_ntoa(in);
		if (t && in.s_addr != INADDR_NONE) {
			SetDlgItemText(hwndDlg,IDC_EDIT_FORCEIP,t);
		}
		else {
			SetDlgItemText(hwndDlg,IDC_EDIT_FORCEIP,"");
		};
	}
	else {
		//dns might have changed, resolve now
		update_forceip_dns_resolution();
		if (g_forceip_dynip_addr!=INADDR_NONE) {
			struct in_addr in;
			in.s_addr=g_forceip_dynip_addr;
			SetDlgItemText(hwndDlg,IDC_EDIT_SHOW_RESOLVE, inet_ntoa(in));
		}
		else {
			SetDlgItemText(hwndDlg,IDC_EDIT_SHOW_RESOLVE, "");
		};
		SetDlgItemText(hwndDlg,IDC_EDIT_FORCEIP, g_forceip_name);
	};
}

static void Pref_Network3Proc_Help2(HWND hwndDlg)
{
	struct in_addr in;
	in.s_addr=g_forceip_dynip_addr;
	char *t=inet_ntoa(in);
	if (t && in.s_addr != INADDR_NONE) {
		SetDlgItemText(hwndDlg,IDC_EDIT_SHOW_CURRENTIP, t);
	}
	else {
		SetDlgItemText(hwndDlg,IDC_EDIT_SHOW_CURRENTIP, "");
	};
}

static BOOL CALLBACK Pref_Network3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		//------------------------------------------------------------
		if (g_forceip_dynip_mode==0) {
			CheckDlgButton(hwndDlg,IDC_RADIO_IP_NORMAL,BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_FORCEIP),0);
		}
		//------------------------------------------------------------
		else if (g_forceip_dynip_mode==1) {
			CheckDlgButton(hwndDlg,IDC_RADIO_IP_FORCED,BST_CHECKED);
			Pref_Network3Proc_Help1(hwndDlg);
		}
		//------------------------------------------------------------
		else if (g_forceip_dynip_mode==2) {
			CheckDlgButton(hwndDlg,IDC_RADIO_IP_AUTO,BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_FORCEIP),0);
			Pref_Network3Proc_Help2(hwndDlg);
		};
		//------------------------------------------------------------
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_RADIO_IP_NORMAL:
		case IDC_RADIO_IP_FORCED:
		case IDC_RADIO_IP_AUTO:
			{
				if (!!IsDlgButtonChecked(hwndDlg, IDC_RADIO_IP_NORMAL)) {
					g_forceip_dynip_mode=0;
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_FORCEIP),0);
					g_config->WriteInt(CONFIG_forceip_dynip_mode,g_forceip_dynip_mode);
				}
				else if (!!IsDlgButtonChecked(hwndDlg,IDC_RADIO_IP_FORCED)) {
					g_forceip_dynip_mode=1;
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_FORCEIP),1);
					Pref_Network3Proc_Help1(hwndDlg);
					g_config->WriteInt(CONFIG_forceip_dynip_mode,g_forceip_dynip_mode);
				}
				else if (!!IsDlgButtonChecked(hwndDlg,IDC_RADIO_IP_AUTO)) {
					g_forceip_dynip_mode=2;
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_FORCEIP),0);
					Pref_Network3Proc_Help2(hwndDlg);
					g_config->WriteInt(CONFIG_forceip_dynip_mode,g_forceip_dynip_mode);
				};
				break;
			};
		case IDC_EDIT_FORCEIP:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS) {
					char buf[256];
					GetDlgItemText(hwndDlg,IDC_EDIT_FORCEIP,buf,256);
					safe_strncpy(g_forceip_name, buf, 256);

					update_forceip_dns_resolution();
					Pref_Network3Proc_Help1(hwndDlg);

					g_config->WriteString(CONFIG_forceip_name,g_forceip_name);
				};
				break;
			};
		};
	};
	return 0;
}

static void Pref_Network4Proc_Help(HWND hwnd)
{
	static const char szShortPwHint[]=
		"You have entered a short networkname/password.\r\n"
		"You should not enable stealth mode with a short password!\r\n"
		"Using that feature only makes sense with long passwords.";
	MessageBox(hwnd,szShortPwHint,APP_NAME " Warning",MB_ICONWARNING);
}

static void Pref_Network4Proc_Help2(HWND hwnd)
{
	static const char szPSKHint[]=
		"If this option is selected, WASTE tries to do several things to make it more obfuscated.\r\n"
		"It will make the initial handshake(saying hallo to the other WASTE clients) harder to detect.\r\n"
		"It will also make it harder to \"denial of service\"-attack WASTE.\r\n"
		"It is always a good thing to enable this option.\r\n"
		"Unfortunately you must supply a long password for security reasons.\r\n"
		"You should only use this option if your password is at least 15 characters long.\r\n"
		"If you have a short password (\"hallo\" for example) you should NOT use this option.\r\n"
		"This option should help to be unseen by many P2P-detection systems.\r\n"
		"It is still possible and will always be possible to \"guess\" that you are using WASTE. Although there is not much evidence for it.\r\n";
	MessageBox(hwnd,szPSKHint,APP_NAME " Help",MB_ICONINFORMATION);
}

static const char* Pref_Network4Proc_Help3()
{
	static const char szKEYHint[]=
	"If your network uses a network name/ID/passphrase, enter it here.\r\n"
	"You will only be able to connect to nodes with the same name/ID.\r\n"
	"Be very careful when entering this password. Every user must have exactly the same password.\r\n"
	"Look for leading and trailing spaces if error occur! In stealth mode, connection will fail without concrete warning!";
	return szKEYHint;
}

static BOOL CALLBACK Pref_Network4Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	char buf1[8192];
	char buf2[4096];

	if (uMsg == WM_INITDIALOG) {
		str_return_unpack(buf1,g_config->ReadString(CONFIG_networkname,CONFIG_networkname_DEFAULT),sizeof(buf1),';');
		SetDlgItemText(hwndDlg,IDC_EDIT_NETWORK_NAME,buf1);
		memset(buf1,0,sizeof(buf1));
		//------------------------------------------------------------
		if (g_networkhash_PSK) {
			CheckDlgButton(hwndDlg,IDC_CHECK_PREAUTH,BST_CHECKED);
		};
		//------------------------------------------------------------
		SetDlgItemText(hwndDlg,IDC_LABEL_PASSWDED,Pref_Network4Proc_Help3());
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_CHECK_PREAUTH:
			{
				g_networkhash_PSK=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_PREAUTH);
				g_config->WriteInt(CONFIG_USE_PSK,g_networkhash_PSK);
				if (g_use_networkhash && g_networkhash_PSK) {
					int len=strlen(g_config->ReadString(CONFIG_networkname,CONFIG_networkname_DEFAULT));
					if (len<15) {
						Pref_Network4Proc_Help(hwndDlg);
					};
				};
				break;
			};
		case IDC_BUTTON_PREAUTH_HELP:
			{
				Pref_Network4Proc_Help2(hwndDlg);
				break;
			};
		case IDC_EDIT_NETWORK_NAME:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS) {
					GetDlgItemText(hwndDlg,IDC_EDIT_NETWORK_NAME,buf1,sizeof(buf1));
					str_return_pack(buf2,buf1,sizeof(buf2),';');
					g_config->WriteString(CONFIG_networkname,buf2);
					InitNeworkHash();
					memset(buf1,0,sizeof(buf1));
					memset(buf2,0,sizeof(buf2));
					if (g_use_networkhash && g_networkhash_PSK) {
						int len=strlen(g_config->ReadString(CONFIG_networkname,CONFIG_networkname_DEFAULT));
						if (len<15) {
							Pref_Network4Proc_Help(hwndDlg);
						};
					};
				};
				break;
			};
		};
	};
	return 0;
}

static BOOL CALLBACK Pref_AboutProc_HttpBitch(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hc;
	static char *szHP2;
	WNDPROC ww=(WNDPROC)GetWindowLong(hwnd,GWL_USERDATA);
	if (uMsg==WM_INITDIALOG) {
		szHP2=(char*)malloc(sK0[0]);
		safe_strncpy(szHP2,(char*)sK1[0],sK0[0]);
		dpi(szHP2,1);
		SetWindowText(hwnd,szHP2);
		if (!hc) hc=LoadImage(g_hInst,MAKEINTRESOURCE(IDC_HTMLHAND),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE);
	}
	else if (uMsg==WM_DESTROY) {
		memset(szHP2,0,sK0[0]);free(szHP2);
		if (hc) DeleteObject(hc);
		hc=NULL;
	}
	else if (uMsg==WM_SETCURSOR) {
		SetCursor((HCURSOR)hc);
		return 1;
	}
	else if (uMsg==WM_LBUTTONDOWN) {
		static bool bLock;
		if (!bLock) {
			bLock=true;
			ShellExecute(0,"open",szHP2,NULL,NULL,SW_SHOWNORMAL);
			bLock=false;
		};
	};
	return CallWindowProc(ww,hwnd,uMsg,wParam,lParam);
}

static BOOL CALLBACK Pref_AboutProc_CredBitch(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC ww=(WNDPROC)GetWindowLong(hwnd,GWL_USERDATA);
	if (uMsg==WM_INITDIALOG) {
		char *szCR2;
		szCR2=(char*)malloc(sK0[2]);
		safe_strncpy(szCR2,(char*)sK1[2],sK0[2]);
		dpi(szCR2,3);
		SetWindowText(hwnd,szCR2);
		memset(szCR2,0,sK0[2]);free(szCR2);
	}
	else if (uMsg==WM_NCHITTEST) {
		LRESULT ret=CallWindowProc(ww,hwnd,uMsg,wParam,lParam);
		if (ret==HTCLIENT) ret=HTNOWHERE;
		return ret;

	};
	return CallWindowProc(ww,hwnd,uMsg,wParam,lParam);
}

static BOOL CALLBACK Pref_AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND bitch;
	static HWND bitch2;
	if (uMsg == WM_INITDIALOG) {
		char *szCP2;
		szCP2=(char*)malloc(sK0[1]);
		safe_strncpy(szCP2,(char*)sK1[1],sK0[1]);
		dpi(szCP2,2);
		bitch=GetDlgItem(hwndDlg,IDC_HOMEPAGE);
		bitch2=GetDlgItem(hwndDlg,IDC_CREDITS);
		WNDPROC ww;
		ww=(WNDPROC)GetWindowLong(bitch,GWL_WNDPROC);
		SetWindowLong(bitch,GWL_USERDATA,(LONG)ww);
		SetWindowLong(bitch,GWL_WNDPROC,(LONG)Pref_AboutProc_HttpBitch);
		SendMessage(bitch,WM_INITDIALOG,0,0);
		ww=(WNDPROC)GetWindowLong(bitch2,GWL_WNDPROC);
		SetWindowLong(bitch2,GWL_USERDATA,(LONG)ww);
		SetWindowLong(bitch2,GWL_WNDPROC,(LONG)Pref_AboutProc_CredBitch);
		SendMessage(bitch2,WM_INITDIALOG,0,0);
		SetDlgItemText(hwndDlg,IDC_TEXT_VERSION,g_nameverstr);
		SetDlgItemText(hwndDlg,IDC_TEXT_CRIGHT,szCP2);
		memset(szCP2,0,sK0[1]);free(szCP2);
	}
	else if (uMsg==WM_DESTROY) {
		bitch=NULL;
	}
	else if (uMsg==WM_CTLCOLORSTATIC) {
		if ((HWND)lParam==bitch) {
			SetTextColor((HDC)wParam, RGB(0,0,255));
			SetBkMode((HDC)wParam,TRANSPARENT);
			return (BOOL)GetSysColorBrush(COLOR_BTNFACE);
		};
	};
	return 0;
}

static void playSong(HWND hwndNotify, UINT &id, const char* fn)
{
    UINT wDeviceID;
    MCI_OPEN_PARMS mciOpenParms;memset(&mciOpenParms,0,sizeof(mciOpenParms));
    MCI_PLAY_PARMS mciPlayParms;memset(&mciPlayParms,0,sizeof(mciPlayParms));
    MCI_STATUS_PARMS mciStatusParms;memset(&mciStatusParms,0,sizeof(mciStatusParms));
    MCI_SEQ_SET_PARMS mciSeqSetParms;memset(&mciSeqSetParms,0,sizeof(mciSeqSetParms));

	if (fn) {
		mciOpenParms.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_SEQUENCER;
		mciOpenParms.lpstrElementName = fn;
		if (mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID|MCI_OPEN_ELEMENT, (DWORD)&mciOpenParms)) return;
		wDeviceID = mciOpenParms.wDeviceID;
		mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;
		if (mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mciStatusParms))
		{
			mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
			return;
		};
		mciPlayParms.dwCallback =(DWORD)hwndNotify;
		if (mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD)&mciPlayParms))
		{
			mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
			return;
		}
		id=wDeviceID;
		return;
	};
	if (id) mciSendCommand(id, MCI_CLOSE, 0, NULL);
}

struct _THELP
{
	//simple stupid synchro. no need true events here...
	//0 none
	//1 start
	//2 inplay
	//3 doquit
	//4 onquit
	//XX
	volatile int sync;
	volatile int replay;
	HWND hwnd;
};
static _THELP _thelp;

static DWORD WINAPI Pref_LicenseThread(LPVOID /*data*/)
{
	struct _LIHELP2
	{
		char* data;
		UINT len;
		UINT mci;
		HANDLE hh;
		char tfi[MAX_PATH+1];
	};
	_LIHELP2 li2;memset(&li2,0,sizeof(li2));
	//_thelp.sync==1 now, no need locking, mom's waiting ;)
	_thelp.replay=0;
	_thelp.sync=2;
	//dbg_printf(ds_Debug,"DEBUG: inthread before mci");
	HRSRC hs=FindResource(g_hInst,MAKEINTRESOURCE(IDR_METALLICA),"DAT");
	if (hs) {
		HGLOBAL hg=LoadResource(g_hInst,hs);
		if (hg) {
			char* data=(char*)LockResource(hg);
			DWORD len=SizeofResource(g_hInst,hs);
			if (len>0) {
				li2.data=new char[len];
				li2.len=len;
				memcpy(li2.data,data,len);
				FreeResource(hg);
				char tdir[MAX_PATH+1];
				char tfi[MAX_PATH+1];
				int l=GetTempPath(sizeof(tdir),tdir);
				if (l>0 && l<sizeof(tdir)) {
					if (GetTempFileName(tdir,"Md5Chap",0,tfi)) {
						safe_strncpy(li2.tfi,tfi,sizeof(li2.tfi));
						HANDLE hh=CreateFile(tfi,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_ALWAYS,0/*FILE_ATTRIBUTE_TEMPORARY*/,NULL);
						li2.hh=hh;
						DWORD dw;
						if (WriteFile(hh,li2.data,li2.len,&dw,0)) {
							CloseHandle(hh);
							hh=CreateFile(tfi,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
							li2.hh=hh;
							playSong(_thelp.hwnd,li2.mci,li2.tfi);
						};
					};
				};
			}
			else {
				FreeResource(hg);
			};
		};
	};
	//dbg_printf(ds_Debug,"DEBUG: inthread after mci play");	
	for (;;) {
		int bc;
		bc=(int)InterlockedCompareExchangePointer((volatile PVOID*)&_thelp.sync,(PVOID)4,(PVOID)3);
		if (bc==3) break;
		bc=(int)InterlockedCompareExchangePointer((volatile PVOID*)&_thelp.replay,(LPVOID)0,(LPVOID)1);
		if (bc==1) {
			//dbg_printf(ds_Debug,"DEBUG: inthread doing replay");
			MCI_SEEK_PARMS mciSeekParms;memset(&mciSeekParms,0,sizeof(mciSeekParms));
			if (mciSendCommand(li2.mci, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)&mciSeekParms))
			{
				mciSendCommand(li2.mci, MCI_CLOSE, 0, NULL);
				li2.mci=0;
			};
			MCI_PLAY_PARMS mciPlayParms;memset(&mciPlayParms,0,sizeof(mciPlayParms));
			mciPlayParms.dwCallback=(DWORD)_thelp.hwnd;
			if (mciSendCommand(li2.mci, MCI_PLAY, MCI_NOTIFY, (DWORD)&mciPlayParms))
			{
				mciSendCommand(li2.mci, MCI_CLOSE, 0, NULL);
				li2.mci=0;
			};
		};
		Sleep(300);
	};
	//dbg_printf(ds_Debug,"DEBUG: inthread got quit");
	if (li2.data) {
		delete li2.data;
	};
	if (li2.hh && li2.hh!=INVALID_HANDLE_VALUE) {
		playSong(NULL,li2.mci,NULL);
		CloseHandle(li2.hh);
		int i=10;
		while ((i--)>0 && !DeleteFile(li2.tfi)) Sleep(200);
	};
	//dbg_printf(ds_Debug,"DEBUG: inthread exiting");
	_thelp.hwnd=NULL;
	_thelp.sync=0;
	return 0;
}

#define LIC_ADD_SIZE 300
void WndModSize(HWND hwnd,int mod)
{
	if (!mod) return;
	RECT rr;
	GetWindowRect(hwnd,&rr);
	rr.right-=rr.left;
	rr.bottom-=rr.top;
	SetWindowPos(hwnd,
		0,
		0,0,
		rr.right+mod,rr.bottom,
		SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOOWNERZORDER
		);
}

static BOOL CALLBACK Pref_LicenseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	struct _LIHELP1
	{
		HFONT fo;
	};
	if (uMsg == WM_INITDIALOG) {
		WndModSize(hwndDlg,LIC_ADD_SIZE);
		WndModSize(GetDlgItem(hwndDlg,IDC_LICENSE_GBOX),LIC_ADD_SIZE);
		WndModSize(GetDlgItem(hwndDlg,IDC_EDIT_LICENSE),LIC_ADD_SIZE);
		int old=(int)InterlockedCompareExchangePointer((volatile PVOID*)&_thelp.sync,(PVOID)1,(PVOID)0);
		if (old==0) {
			_thelp.hwnd=hwndDlg;
			DWORD dwTid;
			//dbg_printf(ds_Debug,"DEBUG: Creating thread");
			HANDLE th=CreateThread(NULL,NULL,Pref_LicenseThread,0,0,&dwTid);
			if (th!=INVALID_HANDLE_VALUE) {
				CloseHandle(th);//free ref
				while (_thelp.sync!=2) Sleep(200);
				//dbg_printf(ds_Debug,"DEBUG: thread synced 2");
			}else {
				//WTF ...
				InterlockedCompareExchangePointer((volatile PVOID*)&_thelp.sync,(PVOID)0,(PVOID)1);
			};
		};
		char *szLI2;
		szLI2=(char*)malloc(sK0[3]);
		safe_strncpy(szLI2,(char*)sK1[3],sK0[3]);
		dpi(szLI2,4);
		HFONT fo2;
		LOGFONT lf;
		memset(&lf,0,sizeof(lf));
		lf.lfHeight=-12;
		safe_strncpy(lf.lfFaceName,"Arial",sizeof(lf.lfFaceName));
		fo2=CreateFontIndirect(&lf);
		_LIHELP1* li=new _LIHELP1;
		memset(li,0,sizeof(li));
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)li);
		if (fo2) {
			li->fo=fo2;
			SendDlgItemMessage(hwndDlg,IDC_EDIT_LICENSE,WM_SETFONT,(WPARAM)fo2,1);
		};
		SetDlgItemText(hwndDlg,IDC_EDIT_LICENSE,szLI2);
		memset(szLI2,0,sK0[3]);free(szLI2);
	}
	else if (uMsg==MM_MCINOTIFY) {
		if (wParam==MCI_NOTIFY_SUCCESSFUL) {
			//dbg_printf(ds_Debug,"DEBUG: thread pushing replay");
			InterlockedCompareExchangePointer((volatile PVOID*)&_thelp.replay,(LPVOID)1,(LPVOID)0);
		};
	}
	else if (uMsg==WM_DESTROY) {
		_LIHELP1* li=(_LIHELP1*)GetWindowLong(hwndDlg,GWL_USERDATA);
		if (li) {
			SendDlgItemMessage(hwndDlg,IDC_EDIT_LICENSE,WM_SETFONT,0,0);
			DeleteObject(li->fo);
			delete li;
			SetWindowLong(hwndDlg,GWL_USERDATA,0);
		};
		//dbg_printf(ds_Debug,"DEBUG: thread sync ->3");
		InterlockedCompareExchangePointer((volatile PVOID*)&_thelp.sync,(LPVOID)3,(LPVOID)2);
		//dbg_printf(ds_Debug,"DEBUG: thread sync done ->3");
	};
	return 0;
}

static BOOL CALLBACK Pref_PerformProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	char buf[2*sizeof(g_performs)];
	if (uMsg == WM_INITDIALOG) {
		str_return_unpack(buf,g_performs,sizeof(buf),';');
		SetDlgItemText(hwndDlg, IDC_PERFORMS, buf);
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_PERFORMS:
			{
				GetDlgItemText(hwndDlg,IDC_PERFORMS, buf, sizeof(buf));
				str_return_pack(g_performs,buf,sizeof(g_performs),';');
				g_config->WriteString(CONFIG_performs,g_performs);
				break;
			};
		};
	};
	return 0;
}

static BOOL CALLBACK Pref_IdentProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		SetDlgItemText(hwndDlg,IDC_EDIT_NICK,g_regnick);
		if (!g_regnick[0])
			SetDlgItemText(hwndDlg,IDC_NICKSTATUS,"No nickname");
		else {
			char buf[512];
			sprintf(buf,"Current nickname: %s\n",g_regnick);
			SetDlgItemText(hwndDlg,IDC_NICKSTATUS,buf);
		};
		SetDlgItemText(hwndDlg,IDC_EDIT_USERNAME,g_config->ReadString(CONFIG_userinfo,CONFIG_userinfo_DEFAULT));
		SetDlgItemText(hwndDlg,IDC_EDIT_CLIENTID,g_client_id_str);
		CreateTooltip(
			GetDlgItem(hwndDlg,IDC_BUTTON_RENEW_CLIENTID),
			"Warning! Read the help!"
			);
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_CLIENTID_HELP:
			{
				static const char szCidhelp[]=
					"Use renew of Client ID only in certain circumstances!\r\n"
					"It is only necessary if you have copied your profile and cannot download from another client!\r\n"
					"If you click this button, other WASTE clients may lose their downloads from you!\r\n"
					"You have been warned!";
				MessageBox(hwndDlg,szCidhelp,APP_NAME " Help",MB_ICONINFORMATION);
				break;
			};
		case IDC_BUTTON_RENEW_CLIENTID:
			{
				static const char szCidwarn[]=
					"Are you really sure?\r\n"
					"Read the help before using this!!!";
				int ret=MessageBox(hwndDlg,szCidwarn,APP_NAME " Help",MB_YESNO|MB_ICONWARNING);
				if (ret==IDYES) {
					CreateID128(&g_client_id);
					MakeID128Str(&g_client_id,g_client_id_str);
					g_config->WriteString(CONFIG_clientid128,g_client_id_str);
					SetDlgItemText(hwndDlg,IDC_EDIT_CLIENTID,g_client_id_str);
				};
				break;
			};
		case IDC_EDIT_USERNAME:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					char buf[256];
					GetDlgItemText(hwndDlg,IDC_EDIT_USERNAME,buf,sizeof(buf));
					buf[255]=0;
					g_config->WriteString(CONFIG_userinfo,buf);
				};
				break;
			};
		case IDC_BUTTON_UPDATE_NICK:
			{
				char buf[32];
				GetDlgItemText(hwndDlg,IDC_EDIT_NICK,buf,sizeof(buf));
				buf[31]=0;
				if (stricmp(buf,g_regnick)) {
					char oldnick[32];
					safe_strncpy(oldnick,g_regnick,sizeof(oldnick));
					safe_strncpy(g_regnick,buf,sizeof(g_regnick));
					if (g_regnick[0] == '#' || g_regnick[0] == '&') g_regnick[0]=0;
					g_config->WriteString(CONFIG_nick,g_regnick);
					if (!g_regnick[0])
						SetDlgItemText(hwndDlg,IDC_NICKSTATUS,"No nickname");
					else {
						char buf[512];
						sprintf(buf,"Current nickname: %s\n",g_regnick);
						SetDlgItemText(hwndDlg,IDC_NICKSTATUS,buf);
						//send nick change notification
						chat_sendNickChangeFrom(oldnick);
					};
				};
			break;
			};
		};
	};
	return 0;
}

static BOOL CALLBACK Pref_Chat3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	bool checkpath=false;
	if (uMsg == WM_INITDIALOG) {
		int lopt=g_config->ReadInt(CONFIG_chatlog, CONFIG_chatlog_DEFAULT);
		if (lopt) {
			SetDlgItemText(hwndDlg,IDC_EDITLOGLOCATION,g_config->ReadString(CONFIG_chatlogpath,CONFIG_chatlogpath_DEFAULT));
			CheckDlgButton(hwndDlg,IDC_CHECK_LOG_PM,(lopt&1)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_CHECK_LOG_ROOM,(lopt&2)?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_CHECK_LOG_BCAST,(lopt&4)?BST_CHECKED:BST_UNCHECKED);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDITLOGLOCATION),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTONLOGLOCATION),0);
		};
		SetDlgItemText(hwndDlg,IDC_STATUSLOGERROR,"Path is checked here");
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_CHECK_LOG_PM:
		case IDC_CHECK_LOG_ROOM:
		case IDC_CHECK_LOG_BCAST:
			{
				int lopt=0;
				lopt+=IsDlgButtonChecked(hwndDlg,IDC_CHECK_LOG_PM)?1:0;
				lopt+=IsDlgButtonChecked(hwndDlg,IDC_CHECK_LOG_ROOM)?2:0;
				lopt+=IsDlgButtonChecked(hwndDlg,IDC_CHECK_LOG_BCAST)?4:0;
				g_config->WriteInt(CONFIG_chatlog, lopt);
				if (lopt) {
					SetDlgItemText(hwndDlg,IDC_EDITLOGLOCATION,g_config->ReadString(CONFIG_chatlogpath,CONFIG_chatlogpath_DEFAULT));
				};
				EnableWindow(GetDlgItem(hwndDlg,IDC_EDITLOGLOCATION),lopt?1:0);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTONLOGLOCATION),lopt?1:0);
				break;
			};
		case IDC_BUTTONLOGLOCATION:
			{
				BROWSEINFO bi={0};
				ITEMIDLIST *idlist;
				char name[1024];
				GetDlgItemText(hwndDlg,IDC_EDITLOGLOCATION,name,sizeof(name));
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = name;
				bi.lpfn=BrowseCallbackProc;
				bi.lParam=(LPARAM)GetDlgItem(hwndDlg,IDC_EDITLOGLOCATION);
				bi.lpszTitle = "Select a directory:";
				bi.ulFlags = BIF_RETURNONLYFSDIRS;
				idlist = SHBrowseForFolder( &bi );
				if (idlist) {
					SHGetPathFromIDList( idlist, name );
					IMalloc *m;
					SHGetMalloc(&m);
					m->Free(idlist);
					m->Release();
					SetDlgItemText(hwndDlg,IDC_EDITLOGLOCATION,name);
					SetFocus(GetDlgItem(hwndDlg,IDC_EDITLOGLOCATION));
					checkpath=true;
				};
				break;
			};
		case IDC_EDITLOGLOCATION:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS) {
					checkpath=true;
				};
				break;
			};
		};
		if (checkpath) {
			char name[1024];
			name[0]=0;
			GetDlgItemText(hwndDlg,IDC_EDITLOGLOCATION,name,sizeof(name));
			DWORD att=GetFileAttributes(name);
			if ((att!=INVALID_FILE_ATTRIBUTES)&&((att&FILE_ATTRIBUTE_DIRECTORY)!=0)) {
				g_config->WriteString(CONFIG_chatlogpath,name);
				SetDlgItemText(hwndDlg,IDC_STATUSLOGERROR,"The directory is accessible");
			}
			else {
				SetDlgItemText(hwndDlg,IDC_STATUSLOGERROR,"The directory or file is not accesible");
			};
		};
	};
	return 0;
}


static BOOL CALLBACK Pref_Chat2Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		int f;
		if (g_config->ReadInt(CONFIG_allowpriv,CONFIG_allowpriv_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHAT_ALLOW_USER,BST_CHECKED);
		};
		if (g_config->ReadInt(CONFIG_allowbcast,CONFIG_allowbcast_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHAT_ALLOW_BCAST,BST_CHECKED);
		};

		f=g_config->ReadInt(CONFIG_gayflash,CONFIG_gayflash_DEFAULT);
		if (f&1) CheckDlgButton(hwndDlg,IDC_CHAT_FLASH_ROOM,BST_CHECKED);
		if (f&2) CheckDlgButton(hwndDlg,IDC_CHAT_FLASH_ROOM_STOP,BST_CHECKED);
		SetDlgItemInt(hwndDlg,IDC_CHAT_FLASH_ROOM_EDIT,(f>>2),FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_ROOM_STOP),(f&1));
		EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_ROOM_EDIT),(f&1)&&(f&2));

		f=g_config->ReadInt(CONFIG_gayflashp,CONFIG_gayflashp_DEFAULT);
		if (f&1) CheckDlgButton(hwndDlg,IDC_CHAT_FLASH_USER,BST_CHECKED);
		if (f&2) CheckDlgButton(hwndDlg,IDC_CHAT_FLASH_USER_STOP,BST_CHECKED);
		SetDlgItemInt(hwndDlg,IDC_CHAT_FLASH_USER_EDIT,(f>>2),FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_USER_STOP),(f&1));
		EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_USER_EDIT),(f&1)&&(f&2));

		f=g_config->ReadInt(CONFIG_gayflashb,CONFIG_gayflashb_DEFAULT);
		if (f&1) CheckDlgButton(hwndDlg,IDC_CHAT_FLASH_BCAST,BST_CHECKED);
		if (f&2) CheckDlgButton(hwndDlg,IDC_CHAT_FLASH_BCAST_STOP,BST_CHECKED);
		SetDlgItemInt(hwndDlg,IDC_CHAT_FLASH_BCAST_EDIT,(f>>2),FALSE);
		EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_BCAST_STOP),(f&1));
		EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_BCAST_EDIT),(f&1)&&(f&2));

	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_CHAT_ALLOW_USER:
			{
				g_config->WriteInt(CONFIG_allowpriv,!!IsDlgButtonChecked(hwndDlg,IDC_CHAT_ALLOW_USER));
				break;
			}
		case IDC_CHAT_ALLOW_BCAST:
			{
				g_config->WriteInt(CONFIG_allowbcast,!!IsDlgButtonChecked(hwndDlg,IDC_CHAT_ALLOW_BCAST));
				break;
			}
		case IDC_CHAT_FLASH_BCAST_EDIT:
			{
				if (HIWORD(wParam) != EN_CHANGE) return 0;
			}
		case IDC_CHAT_FLASH_BCAST_STOP:
		case IDC_CHAT_FLASH_BCAST:
			{
				BOOL t;
				int f=(IsDlgButtonChecked(hwndDlg,IDC_CHAT_FLASH_BCAST)?1:0) |
					  (IsDlgButtonChecked(hwndDlg,IDC_CHAT_FLASH_BCAST_STOP)?2:0) |
					  (GetDlgItemInt(hwndDlg,IDC_CHAT_FLASH_BCAST_EDIT,&t,FALSE)<<2);
				g_config->WriteInt(CONFIG_gayflashb,f);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_BCAST_STOP),(f&1));
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_BCAST_EDIT),(f&1)&&(f&2));
				break;
			};
		case IDC_CHAT_FLASH_ROOM_EDIT:
			{
				if (HIWORD(wParam) != EN_CHANGE) return 0;
			}
		case IDC_CHAT_FLASH_ROOM_STOP:
		case IDC_CHAT_FLASH_ROOM:
			{
				BOOL t;
				int f=(IsDlgButtonChecked(hwndDlg,IDC_CHAT_FLASH_ROOM)?1:0) |
					  (IsDlgButtonChecked(hwndDlg,IDC_CHAT_FLASH_ROOM_STOP)?2:0) |
					  (GetDlgItemInt(hwndDlg,IDC_CHAT_FLASH_ROOM_EDIT,&t,FALSE)<<2);
				g_config->WriteInt(CONFIG_gayflash,f);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_ROOM_STOP),(f&1));
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_ROOM_EDIT),(f&1)&&(f&2));
				break;
			};
		case IDC_CHAT_FLASH_USER_EDIT:
			{
				if (HIWORD(wParam) != EN_CHANGE) return 0;
			}
		case IDC_CHAT_FLASH_USER_STOP:
		case IDC_CHAT_FLASH_USER:
			{
				BOOL t;
				int f=(IsDlgButtonChecked(hwndDlg,IDC_CHAT_FLASH_USER)?1:0) |
					  (IsDlgButtonChecked(hwndDlg,IDC_CHAT_FLASH_USER_STOP)?2:0) |
					  (GetDlgItemInt(hwndDlg,IDC_CHAT_FLASH_USER_EDIT,&t,FALSE)<<2);
				g_config->WriteInt(CONFIG_gayflashp,f);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_USER_STOP),(f&1));
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_FLASH_USER_EDIT),(f&1)&&(f&2));
				break;
			};
		};
	};
	return 0;
}

static BOOL CALLBACK Pref_ChatProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG) {
		SendDlgItemMessage(hwndDlg,IDC_BUTTON_CHAT_SOUND_BROWSE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_SEARCH),IMAGE_ICON,16,16,0));
		SendDlgItemMessage(hwndDlg,IDC_BUTTON_CHAT_SOUND_PLAY,BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_PLAY),IMAGE_ICON,16,16,0));

		if (g_chat_timestamp&1) CheckDlgButton(hwndDlg,IDC_CHAT_STAMP_USER,BST_CHECKED);
		if (g_chat_timestamp&2) CheckDlgButton(hwndDlg,IDC_CHAT_STAMP_ROOM,BST_CHECKED);
		if (g_chat_timestamp&4) CheckDlgButton(hwndDlg,IDC_CHAT_STAMP_DATED,BST_CHECKED);
		if (g_config->ReadInt(CONFIG_cwhmin,CONFIG_cwhmin_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHAT_HIDEONMIN,BST_CHECKED);
		};

		SendDlgItemMessage(hwndDlg,IDC_CHAT_LIMIT_EDIT,EM_LIMITTEXT,16,0);

		if (g_config->ReadInt(CONFIG_limitchat,CONFIG_limitchat_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHAT_LIMIT,BST_CHECKED);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_LIMIT_EDIT),0);
		};
		SetDlgItemInt(hwndDlg,IDC_CHAT_LIMIT_EDIT,g_config->ReadInt(CONFIG_limitchatn,CONFIG_limitchatn_DEFAULT),FALSE);

		uMsg=WM_USER+500;

		const char *fn;
		int i;
		i=g_config->ReadInt(CONFIG_chatsnd,CONFIG_chatsnd_DEFAULT);
		fn=g_config->ReadString(CONFIG_chatsndfn,CONFIG_chatsndfn_DEFAULT);
		if (i) {
			CheckDlgButton(hwndDlg,IDC_CHECK_CHAT_SOUND,BST_CHECKED);
			SetDlgItemText(hwndDlg,IDC_EDIT_CHAT_SOUND_FILENAME,fn);
		}
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_CHAT_SOUND_BROWSE),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_CHAT_SOUND_PLAY),0);
		};
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_CHECK_CHAT_SOUND:
			{
				const char *fn;
				int i=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_CHAT_SOUND);
				g_config->WriteInt(CONFIG_chatsnd,i);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_CHAT_SOUND_BROWSE),i);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_CHAT_SOUND_PLAY),i);
				if (i) {
					fn=g_config->ReadString(CONFIG_chatsndfn,CONFIG_chatsndfn_DEFAULT);
				}
				else {
					fn="";
				};
				SetDlgItemText(hwndDlg,IDC_EDIT_CHAT_SOUND_FILENAME,fn);
				break;
			};
		case IDC_BUTTON_CHAT_SOUND_BROWSE:
			{
				char fnbuf[1024];fnbuf[0]=0;
				char szDir[1024];szDir[1023]=0;
				const char *fn=g_config->ReadString(CONFIG_chatsndfn,CONFIG_chatsndfn_DEFAULT);
				safe_strncpy(fnbuf,fn,sizeof(fnbuf));
				GetModuleFileName(NULL,szDir,sizeof(szDir));
				char *p;
				if ((p=strrchr(szDir,DIRCHAR))!=0) *p=0;
				OPENFILENAME of;
				memset(&of,0,sizeof(of));
				of.lStructSize=sizeof(of);
				of.hwndOwner=hwndDlg;
				of.lpstrTitle="Open sound file";
				of.lpstrFilter="Sound Files(*.wav)\0*.wav\0";
				of.lpstrFile=fnbuf;
				of.nMaxFile=sizeof(fnbuf);
				if (fnbuf[0]==0) of.lpstrInitialDir=szDir;
				if (GetOpenFileName(&of)) {
					g_config->WriteString(CONFIG_chatsndfn,fnbuf);
					SetDlgItemText(hwndDlg,IDC_EDIT_CHAT_SOUND_FILENAME,fnbuf);
				}
				else {
					g_config->WriteString(CONFIG_chatsndfn,"");
					SetDlgItemText(hwndDlg,IDC_EDIT_CHAT_SOUND_FILENAME,"");
				};
				break;
			};
		case IDC_BUTTON_CHAT_SOUND_PLAY:
			{
				const char *fn;
				fn=g_config->ReadString(CONFIG_chatsndfn,CONFIG_chatsndfn_DEFAULT);
				if (GetFileAttributes(fn)!=INVALID_FILE_ATTRIBUTES) {
					sndPlaySound(fn,SND_ASYNC);
				};
				break;
			};
		case IDC_BUTTON_FONT: //chat font
			{
				LOGFONT lf={0,};
				CHOOSEFONT cf={sizeof(cf),0,0,0,0,
					CF_EFFECTS|CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT,
					0,};
				cf.hwndOwner=hwndDlg;
				cf.lpLogFont=&lf;
				const char *p=g_config->ReadString(CONFIG_cfont_face,CONFIG_cfont_face_DEFAULT);
				if (*p) safe_strncpy(lf.lfFaceName,p,sizeof(lf.lfFaceName));
				int effects=g_config->ReadInt(CONFIG_cfont_fx,CONFIG_cfont_fx_DEFAULT);
				cf.rgbColors=g_config->ReadInt(CONFIG_cfont_color,CONFIG_cfont_color_DEFAULT);
				if (cf.rgbColors & 0xFF000000) cf.rgbColors=0xFFFFFF;

				lf.lfHeight=(g_config->ReadInt(CONFIG_cfont_yh,CONFIG_cfont_yh_DEFAULT))/15;
				lf.lfItalic=(effects&CFE_ITALIC)?1:0;
				lf.lfWeight=(effects&CFE_BOLD)?FW_BOLD:FW_NORMAL;
				lf.lfUnderline=(effects&CFE_UNDERLINE)?1:0;
				lf.lfStrikeOut=(effects&CFE_STRIKEOUT)?1:0;
				lf.lfCharSet=DEFAULT_CHARSET;
				lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
				lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
				lf.lfQuality=DEFAULT_QUALITY;
				lf.lfPitchAndFamily=FIXED_PITCH;

				if (ChooseFont(&cf)) {
					effects=0;
					if (lf.lfItalic) effects |= CFE_ITALIC;
					if (lf.lfUnderline) effects |= CFE_UNDERLINE;
					if (lf.lfStrikeOut) effects |= CFE_STRIKEOUT;
					if (lf.lfWeight!=FW_NORMAL)effects |= CFE_BOLD;
					g_config->WriteInt(CONFIG_cfont_fx,effects);
					g_config->WriteInt(CONFIG_cfont_yh,cf.iPointSize*2);
					g_config->WriteInt(CONFIG_cfont_color,cf.rgbColors);
					g_config->WriteString(CONFIG_cfont_face,lf.lfFaceName);
					SendMessage(g_mainwnd,WM_USER_TITLEUPDATE,0,0);
					uMsg = WM_USER+500;
				};
				break;
			};
		case IDC_BUTTON_BACK: //chat bg color
			{
				const char *p=g_config->ReadString(CONFIG_cfont_face,CONFIG_cfont_face_DEFAULT);
				if (*p) {
					int c=g_config->ReadInt(CONFIG_cfont_bgc,CONFIG_cfont_bgc_DEFAULT);
					CHOOSECOLOR cs;
					cs.lStructSize = sizeof(cs);
					cs.hwndOwner = hwndDlg;
					cs.hInstance = 0;
					cs.rgbResult=c;
					cs.lpCustColors = custcolors;
					cs.Flags = CC_RGBINIT|CC_FULLOPEN;
					if (ChooseColor(&cs)) {
						g_config->WriteInt(CONFIG_cfont_bgc,cs.rgbResult);
						SendMessage(g_mainwnd,WM_USER_TITLEUPDATE,0,0);
						uMsg = WM_USER+500;
					};
				};
				break;
			};
		case IDC_BUTTON_OTHERS: //chat my own name's color
			{
				const char *p=g_config->ReadString(CONFIG_cfont_face,CONFIG_cfont_face_DEFAULT);
				if (*p) {
					int c=g_config->ReadInt(CONFIG_cfont_others_color,CONFIG_cfont_others_color_DEFAULT);
					CHOOSECOLOR cs;
					cs.lStructSize = sizeof(cs);
					cs.hwndOwner = hwndDlg;
					cs.hInstance = 0;
					cs.rgbResult=c;
					cs.lpCustColors = custcolors;
					cs.Flags = CC_RGBINIT|CC_FULLOPEN;
					if (ChooseColor(&cs)) {
						g_config->WriteInt(CONFIG_cfont_others_color,cs.rgbResult);
						SendMessage(g_mainwnd,WM_USER_TITLEUPDATE,0,0);
						uMsg = WM_USER+500;
					};
				};
				break;
			};
		case IDC_CHAT_LIMIT_EDIT:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					BOOL t;
					int a=GetDlgItemInt(hwndDlg,IDC_CHAT_LIMIT_EDIT,&t,FALSE);
					if (t) g_config->WriteInt(CONFIG_limitchatn,a);
				};
				break;
			}
		case IDC_CHAT_STAMP_DATED:
		case IDC_CHAT_STAMP_USER:
		case IDC_CHAT_STAMP_ROOM:
			{
				g_chat_timestamp=
					(IsDlgButtonChecked(hwndDlg,IDC_CHAT_STAMP_USER)?1:0) |
					(IsDlgButtonChecked(hwndDlg,IDC_CHAT_STAMP_ROOM)?2:0) |
					(IsDlgButtonChecked(hwndDlg,IDC_CHAT_STAMP_DATED)?4:0);
				g_config->WriteInt(CONFIG_chat_timestamp,g_chat_timestamp);
				break;
			};
		case IDC_CHAT_HIDEONMIN:
			{
				g_config->WriteInt(CONFIG_cwhmin,!!IsDlgButtonChecked(hwndDlg,IDC_CHAT_HIDEONMIN));
				break;
			}
		case IDC_CHAT_LIMIT:
			{
				int a;
				g_config->WriteInt(CONFIG_limitchat,a=!!IsDlgButtonChecked(hwndDlg,IDC_CHAT_LIMIT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHAT_LIMIT_EDIT),a);
				break;
			};
		};
	};
	if (uMsg == WM_DRAWITEM) {
		DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT *)lParam;
		if (di->CtlID == IDC_CHAT_BOX_COLOR_BACK ||
			di->CtlID == IDC_CHAT_BOX_COLOR_FONT ||
			di->CtlID == IDC_CHAT_BOX_COLOR_OTHERS
			)
		{
			const char *p=g_config->ReadString(CONFIG_cfont_face,CONFIG_cfont_face_DEFAULT);
			int color=0;
			if (*p) {
				char* readwhat;
				int colordefault;
				switch (di->CtlID)
				{
				case IDC_CHAT_BOX_COLOR_BACK:
					{
						readwhat=CONFIG_cfont_bgc;
						colordefault=CONFIG_cfont_bgc_DEFAULT;
						break;
					};
				case IDC_CHAT_BOX_COLOR_FONT:
					{
						readwhat=CONFIG_cfont_color;
						colordefault=CONFIG_cfont_color_DEFAULT;
						break;
					};
				case IDC_CHAT_BOX_COLOR_OTHERS:
					{
						readwhat=CONFIG_cfont_others_color;
						colordefault=CONFIG_cfont_others_color_DEFAULT;
						break;
					};
				default:
					readwhat=NULL;
					colordefault=0;
				};
				if (readwhat) {
					color=g_config->ReadInt(readwhat,colordefault);
					HBRUSH hBrush,hOldBrush;
					LOGBRUSH lb={BS_SOLID,color,0};
					hBrush = CreateBrushIndirect(&lb);
					hOldBrush=(HBRUSH)SelectObject(di->hDC,hBrush);
					Rectangle(di->hDC,di->rcItem.left,di->rcItem.top,di->rcItem.right,di->rcItem.bottom);
					SelectObject(di->hDC,hOldBrush);
					DeleteObject(hBrush);
				};
			};
		};
	};

	if (uMsg == WM_USER+500) {
		char buf[512];
		const char *p=g_config->ReadString(CONFIG_cfont_face,CONFIG_cfont_face_DEFAULT);
		if (!*p) {
			strcpy(buf,"(Windows Default)");
		}
		else {
			sprintf(buf,"%s @ %dpt",p,g_config->ReadInt(CONFIG_cfont_yh,CONFIG_cfont_yh_DEFAULT)/20);
		};

		SetDlgItemText(hwndDlg,IDC_CHATFONTTEXT,buf);
		InvalidateRect(hwndDlg,NULL,FALSE);
	};
	return 0;
}

static BOOL CALLBACK Pref_RecvProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		SetDlgItemText(hwndDlg,IDC_EDIT_DL_PATH,g_config->ReadString(CONFIG_downloadpath,""));

		int a=g_config->ReadInt(CONFIG_dlppath,CONFIG_dlppath_DEFAULT);
		if (a&4)
			CheckDlgButton(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_SEARCH,BST_CHECKED);
		if (a&16)
			CheckDlgButton(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_BROWSED,BST_CHECKED);
		if (a&32)
			CheckDlgButton(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_REC,BST_CHECKED);
		if (a&1)
			CheckDlgButton(hwndDlg,IDC_CHECK_USE_PATH_ON_REC_DL,BST_CHECKED);
		else EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_REC),0);

		if (a&2)
			CheckDlgButton(hwndDlg,IDC_CHECK_USE_PATH_ON_REC_DL_SEARCH,BST_CHECKED);
		else EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_SEARCH),0);

		if (a&8)
			CheckDlgButton(hwndDlg,IDC_CHECK_USE_PATH_ON_REC_DL_BROWSED,BST_CHECKED);
		else EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_BROWSED),0);

		if (g_config->ReadInt(CONFIG_accept_uploads,CONFIG_accept_uploads_DEFAULT)&4) CheckDlgButton(hwndDlg,IDC_CHECK_UPLOAD_USE_RELPATHS,BST_CHECKED);
		if (g_config->ReadInt(CONFIG_accept_uploads,CONFIG_accept_uploads_DEFAULT)&2) CheckDlgButton(hwndDlg,IDC_CHECK_QUERY_ON_UPLOAD,BST_CHECKED);
		if (g_config->ReadInt(CONFIG_accept_uploads,CONFIG_accept_uploads_DEFAULT)&1) CheckDlgButton(hwndDlg,IDC_CHECK_ALLOW_UPLOADING,BST_CHECKED);
		else {
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_QUERY_ON_UPLOAD),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_UPLOAD_USE_RELPATHS),0);
		};
		SetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_DL,g_max_simul_dl_host&0x7FFFFFFF,FALSE);
		if (g_max_simul_dl_host&0x80000000)
			EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_DL),0);
		else
			CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_DL,BST_CHECKED);
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_CHECK_LIMIT_DL:
			{
				g_max_simul_dl_host&=0x7FFFFFFF;
				if (!IsDlgButtonChecked(hwndDlg,IDC_CHECK_LIMIT_DL)) {
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_DL),0);
					g_max_simul_dl_host|=0x80000000;
				}
				else EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT_LIMIT_DL),1);
				g_config->WriteInt(CONFIG_recv_maxdl_host,g_max_simul_dl_host);
				break;
			};
		case IDC_EDIT_LIMIT_DL:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					BOOL t=0;
					int inter=GetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_DL,&t,FALSE);
					if (inter<0)inter=0;
					if (t) {
						g_max_simul_dl_host&=0x80000000;
						g_max_simul_dl_host|=inter;
						g_config->WriteInt(CONFIG_recv_maxdl_host,g_max_simul_dl_host);
					};
				};
				break;
			};
		case IDC_CHECK_DL_PARENTPATH_ON_SEARCH:
		case IDC_CHECK_DL_PARENTPATH_ON_BROWSED:
		case IDC_CHECK_USE_PATH_ON_REC_DL:
		case IDC_CHECK_USE_PATH_ON_REC_DL_SEARCH:
		case IDC_CHECK_USE_PATH_ON_REC_DL_BROWSED:
		case IDC_CHECK_DL_PARENTPATH_ON_REC:
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_REC),IsDlgButtonChecked(hwndDlg,IDC_CHECK_USE_PATH_ON_REC_DL)?1:0);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_SEARCH),IsDlgButtonChecked(hwndDlg,IDC_CHECK_USE_PATH_ON_REC_DL_SEARCH)?1:0);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_BROWSED),IsDlgButtonChecked(hwndDlg,IDC_CHECK_USE_PATH_ON_REC_DL_BROWSED)?1:0);

				g_config->WriteInt(CONFIG_dlppath,(IsDlgButtonChecked(hwndDlg,IDC_CHECK_USE_PATH_ON_REC_DL)?1:0) |
					(IsDlgButtonChecked(hwndDlg,IDC_CHECK_USE_PATH_ON_REC_DL_SEARCH) ? 2 : 0) |
					(IsDlgButtonChecked(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_SEARCH) ? 4 : 0) |
					(IsDlgButtonChecked(hwndDlg,IDC_CHECK_USE_PATH_ON_REC_DL_BROWSED) ? 8 : 0) |
					(IsDlgButtonChecked(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_BROWSED) ? 16 : 0) |
					(IsDlgButtonChecked(hwndDlg,IDC_CHECK_DL_PARENTPATH_ON_REC) ? 32 : 0)
					);
				break;
			};
		case IDC_CHECK_ALLOW_UPLOADING:
		case IDC_CHECK_QUERY_ON_UPLOAD:
		case IDC_CHECK_UPLOAD_USE_RELPATHS:
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_QUERY_ON_UPLOAD),IsDlgButtonChecked(hwndDlg,IDC_CHECK_ALLOW_UPLOADING)?1:0);
				EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_UPLOAD_USE_RELPATHS),IsDlgButtonChecked(hwndDlg,IDC_CHECK_ALLOW_UPLOADING)?1:0);
				g_config->WriteInt("accept_uploads",
					(IsDlgButtonChecked(hwndDlg,IDC_CHECK_ALLOW_UPLOADING)?1:0)|
					(IsDlgButtonChecked(hwndDlg,IDC_CHECK_QUERY_ON_UPLOAD)?2:0)|
					(IsDlgButtonChecked(hwndDlg,IDC_CHECK_UPLOAD_USE_RELPATHS)?4:0));
				break;
			};
		case IDC_BROWSESAVE:
			{
				BROWSEINFO bi={0,};
				ITEMIDLIST *idlist;
				char name[1024];
				GetDlgItemText(hwndDlg,IDC_EDIT_DL_PATH,name,sizeof(name));
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = name;
				bi.lpfn=BrowseCallbackProc;
				bi.lParam=(LPARAM)GetDlgItem(hwndDlg,IDC_EDIT_DL_PATH);
				bi.lpszTitle = "Select a directory:";
				bi.ulFlags = BIF_RETURNONLYFSDIRS;
				idlist = SHBrowseForFolder( &bi );
				if (idlist) {
					SHGetPathFromIDList( idlist, name );
					IMalloc *m;
					SHGetMalloc(&m);
					m->Free(idlist);
					SetDlgItemText(hwndDlg,IDC_EDIT_DL_PATH,name);
					g_config->WriteString(CONFIG_downloadpath,name);
				};
			};
			return 0;
		case IDC_EDIT_DL_PATH:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					char name[1024];
					GetDlgItemText(hwndDlg,IDC_EDIT_DL_PATH,name,sizeof(name));
					g_config->WriteString(CONFIG_downloadpath,name);
				};
				return 0;
			};
		};
	};
	return 0;
}

static void PS_EnableWnds(HWND hwndDlg)
{
	static int a[]=
	{
		IDC_ADDDIR,
		IDC_CHECK_AUTORESCAN,
		IDC_CHECK_CACHE_ON_EXIT,
		IDC_CHECK_DL_ALLOW_BROWSE,
		IDC_CHECK_DL_ALLOW_SEARCH,
		IDC_CHECK_LIMIT_EXTENSIONS,
		IDC_CHECK_RESCAN_ON_START,
		IDC_EDIT_EXTLIST,
		IDC_EDIT_LIMIT_UL,
		IDC_EDIT_RESCAN_MINUTES,
		IDC_FILEPATHLIST,
		IDC_RESCAN,
		IDC_SCANSTATUS,
		IDC_SHAMB
	};
	int x;
	int en=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_INDEX_FILES);
	for (x=0;x<sizeof(a)/sizeof(a[0]);x++) {
		int een=en;
		if (a[x]==IDC_EDIT_LIMIT_UL) {
			een=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_LIMIT_UL);
		}
		else if (a[x]==IDC_SHAMB) {
			een=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHA_FILES);
		}
		else if (en) {
			if (a[x]==IDC_EDIT_EXTLIST) {
				een=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_LIMIT_EXTENSIONS);
			}
			else if (a[x]==IDC_EDIT_RESCAN_MINUTES) {
				een=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_AUTORESCAN);
			};
		};
		EnableWindow(GetDlgItem(hwndDlg,a[x]),een);
	};
}

static BOOL CALLBACK Pref_SendProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_TIMER) {
		if (wParam == 1 && g_scan_status_buf[0]) {
			SetDlgItemText(hwndDlg,IDC_SCANSTATUS,g_scan_status_buf);
			g_scan_status_buf[0]=0;
		};
	};
	if (uMsg == WM_INITDIALOG) {
		SetTimer(hwndDlg,1,50,NULL);
		if (g_scan_status_buf[0]) {
			SetDlgItemText(hwndDlg,IDC_SCANSTATUS,g_scan_status_buf);
			g_scan_status_buf[0]=0;
		}
		else {
			if (g_database) {
				char buf[512];
				sprintf(buf,"Scanned %d",g_database->GetNumFiles());
				SetDlgItemText(hwndDlg,IDC_SCANSTATUS,buf);
			};
		};

		if (g_accept_downloads&1) CheckDlgButton(hwndDlg,IDC_CHECK_INDEX_FILES,BST_CHECKED);
		if (g_accept_downloads&2) CheckDlgButton(hwndDlg,IDC_CHECK_DL_ALLOW_SEARCH,BST_CHECKED);
		if (g_accept_downloads&4) CheckDlgButton(hwndDlg,IDC_CHECK_DL_ALLOW_BROWSE,BST_CHECKED);
		if (g_config->ReadInt(CONFIG_use_extlist,CONFIG_use_extlist_DEFAULT)) CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_EXTENSIONS,BST_CHECKED);
		if (g_config->ReadInt(CONFIG_limit_uls,CONFIG_limit_uls_DEFAULT)) CheckDlgButton(hwndDlg,IDC_CHECK_LIMIT_UL,BST_CHECKED);
		SetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_UL,g_config->ReadInt(CONFIG_ul_limit,CONFIG_ul_limit_DEFAULT),FALSE);
		if (g_config->ReadInt(CONFIG_ulfullpaths,CONFIG_ulfullpaths_DEFAULT))
			CheckDlgButton(hwndDlg,IDC_CHECK_SHOW_PARTIAL_UPLOADPATH,BST_CHECKED);

		if (g_config->ReadInt(CONFIG_shafiles,CONFIG_shafiles_DEFAULT)) CheckDlgButton(hwndDlg,IDC_CHECK_SHA_FILES,BST_CHECKED);
		SetDlgItemInt(hwndDlg,IDC_SHAMB,g_config->ReadInt(CONFIG_maxsizesha,CONFIG_maxsizesha_DEFAULT),FALSE);

		SetDlgItemText(hwndDlg,IDC_FILEPATHLIST,g_config->ReadString(CONFIG_databasepath,CONFIG_databasepath_DEFAULT));
		SetDlgItemText(hwndDlg,IDC_EDIT_EXTLIST,g_config->ReadString(CONFIG_extlist,g_def_extlist));
		SetDlgItemInt(hwndDlg,IDC_EDIT_RESCAN_MINUTES,g_config->ReadInt(CONFIG_refreshint,DEFAULT_DB_REFRESH_DELAY),FALSE);
		if (g_do_autorefresh) {
			CheckDlgButton(hwndDlg,IDC_CHECK_AUTORESCAN,BST_CHECKED);
		};
		if (g_config->ReadInt(CONFIG_db_save,CONFIG_db_save_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_CACHE_ON_EXIT,BST_CHECKED);
		};
		if (g_config->ReadInt(CONFIG_scanonstartup,CONFIG_scanonstartup_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_RESCAN_ON_START,BST_CHECKED);
		};
		PS_EnableWnds(hwndDlg);
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_CHECK_SHOW_PARTIAL_UPLOADPATH:
			{
				g_config->WriteInt(CONFIG_ulfullpaths,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOW_PARTIAL_UPLOADPATH));
				break;
			};
		case IDC_SHAMB:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					BOOL t=0;
					int inter=GetDlgItemInt(hwndDlg,IDC_SHAMB,&t,FALSE);
					if (inter<0)inter=0;
					if (t) g_config->WriteInt(CONFIG_maxsizesha,inter);
				};
				return 0;
			};
		case IDC_EDIT_LIMIT_UL:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					BOOL t=0;
					int inter=GetDlgItemInt(hwndDlg,IDC_EDIT_LIMIT_UL,&t,FALSE);
					if (inter<0)inter=0;
					if (t) g_config->WriteInt(CONFIG_ul_limit,inter);
				};
				return 0;
			};
		case IDC_CHECK_LIMIT_UL:
			{
				g_config->WriteInt(CONFIG_limit_uls,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_LIMIT_UL));
				PS_EnableWnds(hwndDlg);
				return 0;
			};
		case IDC_CHECK_SHA_FILES:
			{
				g_config->WriteInt(CONFIG_shafiles,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHA_FILES));
				PS_EnableWnds(hwndDlg);
				return 0;
			};
		case IDC_CHECK_RESCAN_ON_START:
			{
				g_config->WriteInt(CONFIG_scanonstartup,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_RESCAN_ON_START));
				return 0;
			};
		case IDC_CHECK_CACHE_ON_EXIT:
			{
				g_config->WriteInt(CONFIG_db_save,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_CACHE_ON_EXIT));
				return 0;
			};
		case IDC_CHECK_AUTORESCAN:
			{
				g_config->WriteInt(CONFIG_dorefresh,g_do_autorefresh=!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_AUTORESCAN));
				g_next_refreshtime = time(NULL)+60*g_config->ReadInt(CONFIG_refreshint,DEFAULT_DB_REFRESH_DELAY);
				PS_EnableWnds(hwndDlg);
				return 0;
			};
		case IDC_EDIT_RESCAN_MINUTES:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					BOOL t=0;
					int inter=GetDlgItemInt(hwndDlg,IDC_EDIT_RESCAN_MINUTES,&t,FALSE);
					if (inter<1)inter=1;
					if (t) g_config->WriteInt(CONFIG_refreshint,inter);
					g_next_refreshtime = time(NULL)+60*g_config->ReadInt(CONFIG_refreshint,DEFAULT_DB_REFRESH_DELAY);
				};
				return 0;
			};
		case IDC_ADDDIR:
			{
				char name[MAX_PATH]="";
				BROWSEINFO bi={0,};
				ITEMIDLIST *idlist;
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = name;
				bi.lpfn=BrowseCallbackProc;
				bi.lpszTitle = "Select a directory:";
				bi.ulFlags = BIF_RETURNONLYFSDIRS;
				idlist = SHBrowseForFolder( &bi );
				if (idlist) {
					char buf[4096];
					SHGetPathFromIDList( idlist, name );
					IMalloc *m;
					SHGetMalloc(&m);
					m->Free(idlist);
					GetDlgItemText(hwndDlg,IDC_FILEPATHLIST,buf,sizeof(buf));
					if (buf[0] && buf[strlen(buf)-1]!=';') strcat(buf,";");
					strcat(buf,name);
					SetDlgItemText(hwndDlg,IDC_FILEPATHLIST,buf);
					g_config->WriteString(CONFIG_databasepath,buf);
				};
				return 0;
			};
		case IDC_FILEPATHLIST:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					char buf[4096];
					GetDlgItemText(hwndDlg,IDC_FILEPATHLIST,buf,sizeof(buf));
					g_config->WriteString(CONFIG_databasepath,buf);
				};
				return 0;
			};
		case IDC_EDIT_EXTLIST:
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					char buf[4096];
					GetDlgItemText(hwndDlg,IDC_EDIT_EXTLIST,buf,sizeof(buf));
					g_config->WriteString(CONFIG_extlist,buf);
				};
				return 0;
			};
		case IDC_RESCAN:
			{
				doDatabaseRescan();
				return 0;
			};
		case IDC_CHECK_INDEX_FILES:
			{
				g_accept_downloads&=~1;
				g_config->WriteInt(CONFIG_downloadflags,
					g_accept_downloads|=IsDlgButtonChecked(hwndDlg,IDC_CHECK_INDEX_FILES)?1:0);
				PS_EnableWnds(hwndDlg);
				break;
			};
		case IDC_CHECK_DL_ALLOW_SEARCH:
			{
				g_accept_downloads&=~2;
				g_config->WriteInt(CONFIG_downloadflags,
					g_accept_downloads|=IsDlgButtonChecked(hwndDlg,IDC_CHECK_DL_ALLOW_SEARCH)?2:0);
				break;
			};
		case IDC_CHECK_DL_ALLOW_BROWSE:
			{
				g_accept_downloads&=~4;
				g_config->WriteInt(CONFIG_downloadflags,
					g_accept_downloads|=IsDlgButtonChecked(hwndDlg,IDC_CHECK_DL_ALLOW_BROWSE)?4:0);
				break;
			};
		case IDC_CHECK_LIMIT_EXTENSIONS:
			{
				g_config->WriteInt(CONFIG_use_extlist,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_LIMIT_EXTENSIONS));
				PS_EnableWnds(hwndDlg);
				break;
			};
		};
	};
	return 0;
}

static UINT CALLBACK importTextProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	if (uMsg == WM_COMMAND) {
		if (LOWORD(wParam) == IDC_BUTTON_LOAD_KEYTEXT) {
			char temppath[1024];
			char tempfn[2048];
			int num=0;
			GetTempPath(sizeof(temppath),temppath);
			GetTempFileName(temppath,"wst",0,tempfn);
			FILE *a=fopen(tempfn,"wt");
			if (a) {

				int tlen=SendDlgItemMessage(hwndDlg,IDC_KEYTEXT,WM_GETTEXTLENGTH,0,0)+1;
				char *buf=(char*)malloc(tlen+1);
				if (buf) {
					GetDlgItemText(hwndDlg,IDC_KEYTEXT,buf,tlen);
					buf[tlen]=0;
					fwrite(buf,1,strlen(buf),a);
					free(buf);
				};
				fclose(a);
				num=loadPKList(tempfn);
			};
			DeleteFile(tempfn);
			if (!num) {
				MessageBox(hwndDlg,"Error: No key(s) found in text",APP_NAME " Public Key Import Error",MB_OK|MB_ICONSTOP);
				return 0;
			};
			savePKList();
			sprintf(tempfn,"Imported %d public keys successfully.",num);
			MessageBox(hwndDlg,tempfn,APP_NAME " Public Key Import",MB_OK|MB_ICONINFORMATION);
			PostMessage(GetParent(hwndDlg),WM_COMMAND,IDCANCEL,0);
			return 1;
		};
	};

	return 0;
}

static void importPubDlg(HWND hwndDlg)
{
	char *fnroot=(char*)malloc(65536*4);
	fnroot[0]=0;
	OPENFILENAME l={sizeof(l),};
	l.hwndOwner = hwndDlg;
	l.lpstrFilter = "Public key files (*.pub;*.txt)\0*.pub;*.txt\0All files (*.*)\0*.*\0";
	l.lpstrFile = fnroot;
	l.nMaxFile = 65535*4;
	l.lpstrTitle = "Import public key file(s)";
	l.lpstrDefExt = "pub";
	l.hInstance=g_hInst;
	l.lpfnHook=importTextProc;
	l.lpTemplateName=MAKEINTRESOURCE(IDD_KEYIMPORTDLG);

	l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_ALLOWMULTISELECT|OFN_ENABLETEMPLATE|OFN_ENABLEHOOK|OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&l)) {
		int num=0;
		char *fn=fnroot;
		char *pathstr="";
		if (fn[strlen(fn)+1]) { //multiple files
			pathstr=fn;
			fn+=strlen(fn)+1;
		};
		while (*fn) {
			char fullfn[4096];
			fullfn[0]=0;
			if (*pathstr) {
				safe_strncpy(fullfn,pathstr,sizeof(fullfn));
				if (fullfn[strlen(fullfn)-1]!='\\') strcat(fullfn,"\\");
			};
			strcat(fullfn,fn);

			num+=loadPKList(fullfn);
			fn+=strlen(fn)+1;
		};
		savePKList();
		if (num) {
			char buf[128];
			sprintf(buf,"Imported %d public keys successfully.",num);
			MessageBox(hwndDlg,buf,APP_NAME " Public Key Import",MB_OK|MB_ICONINFORMATION);
		}
		else {
			MessageBox(hwndDlg,"Error: No key(s) found in text",APP_NAME " Public Key Import Error",MB_OK|MB_ICONSTOP);
		};
	};
	free(fnroot);

	return;
}

static BOOL CALLBACK Pref_KeyDist2Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static W_ListView *lv;
	if (uMsg == WM_INITDIALOG) {
		delete lv;
		lv=new W_ListView;
		lv->setwnd(GetDlgItem(hwndDlg,IDC_LIST_PENDING_KEYS));
		lv->AddCol("Key name",100);
		lv->AddCol("Signature",160);
		int x;
		for (x = 0; x < g_pklist_pending.GetSize(); x ++) {
			PKitem *p=g_pklist_pending.Get(x);
			lv->InsertItem(x,p->name,0);
			char buf[128];
			int a;
			for (a = 0; a < SHA_OUTSIZE; a ++) sprintf(buf+a*2,"%02X",p->hash[a]);
			lv->SetItemText(x,1,buf);
		};
		if (g_keydist_flags&4) CheckDlgButton(hwndDlg,IDC_CHECK_REBROADCAST_KEYS,BST_CHECKED);
		if (g_keydist_flags&1) {
			CheckDlgButton(hwndDlg,IDC_CHECK_AUTO_ACCEPT_KEYS,BST_CHECKED);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_PROMPT_ON_NEW_KEY),0);
		};
		if (g_keydist_flags&2) CheckDlgButton(hwndDlg,IDC_CHECK_PROMPT_ON_NEW_KEY,BST_CHECKED);
	}
	else if (uMsg == WM_DESTROY) {
		if (lv) {
			delete lv;
			lv=NULL;
		};
	};
	if (uMsg == WM_NOTIFY) {
		LPNMHDR l=(LPNMHDR)lParam;
		if (l->idFrom==IDC_LIST_PENDING_KEYS) {
			if (l->code == NM_DBLCLK)
				SendMessage(hwndDlg,WM_COMMAND,IDC_AUTH,0);
		};
	};

	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
		case IDC_REMOVE:
			{
				int x;
				for (x = 0; x < lv->GetCount(); x ++) {
					if (lv->GetSelected(x)) {
						free(g_pklist_pending.Get(x));
						g_pklist_pending.Del(x);
						lv->DeleteItem(x);
						x--;
					};
				};
				savePKList();
				return 0;
			};
		case IDC_AUTH:
			{
				int x;
				for (x = 0; x < lv->GetCount(); x ++) {
					if (lv->GetSelected(x)) {
						g_pklist.Add(g_pklist_pending.Get(x));
						g_pklist_pending.Del(x);
						lv->DeleteItem(x);
						x--;

					};
				};
				savePKList();
				return 0;
			};
		case IDC_CHECK_REBROADCAST_KEYS:
		case IDC_CHECK_AUTO_ACCEPT_KEYS:
		case IDC_CHECK_PROMPT_ON_NEW_KEY:
			{
			g_keydist_flags=(IsDlgButtonChecked(hwndDlg,IDC_CHECK_AUTO_ACCEPT_KEYS)?1:0)|
				(IsDlgButtonChecked(hwndDlg,IDC_CHECK_REBROADCAST_KEYS)?4:0)|
				(IsDlgButtonChecked(hwndDlg,IDC_CHECK_PROMPT_ON_NEW_KEY)?2:0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_PROMPT_ON_NEW_KEY),!(g_keydist_flags&1));
			g_config->WriteInt(CONFIG_keydistflags,g_keydist_flags);
			return 0;
			};
		};
	};
	return 0;
}

static void CALLBACK Pref_KeyDistProc_HELP1(W_ListView *lv)
{
	int x;
	for ( x=0 ; x<g_pklist.GetSize() ; x ++ ) {
		PKitem *p=g_pklist.Get(x);
		lv->InsertItem(x,p->name,0);
		char buf[128];
		Bin2Hex(buf,p->hash,sizeof(p->hash));
		lv->SetItemText(x,1,buf);
	};
}

static BOOL CALLBACK Pref_KeyDistProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	static W_ListView *lv;
	if (uMsg == WM_INITDIALOG) {
		delete lv;
		lv=new W_ListView;
		lv->setwnd(GetDlgItem(hwndDlg,IDC_LIST_PUBLIC_KEYS));
		lv->AddCol("Key name",100);
		lv->AddCol("Signature",160);
		Pref_KeyDistProc_HELP1(lv);
	}
	else if (uMsg == WM_DESTROY) {
		if (lv) {
			delete lv;
			lv=NULL;
		};
	};
	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
		case IDC_VIEWKEYS:
			{
				char str[2048];
				sprintf(str,"%s.pr3",g_config_prefix);
				char dir[MAX_PATH];
				GetCurrentDirectory(sizeof(dir),dir);
				ShellExecute(hwndDlg,NULL,"notepad",str,dir,SW_SHOWMAXIMIZED);
				return 0;
			};
		case IDC_REMOVE:
			{
				int x;
				for (x = 0; x < lv->GetCount(); x ++) {
					if (lv->GetSelected(x)) {
						free(g_pklist.Get(x));
						g_pklist.Del(x);
						lv->DeleteItem(x);
						x--;
					};
				};
				savePKList();
				return 0;
			};
		case IDC_IMPORT:
			{
				importPubDlg(hwndDlg);
				SendMessage(hwndDlg,WM_COMMAND,IDC_RELOAD,0);
				return 0;
			};
		case IDC_RELOAD:
			{
				KillPkList();
				loadPKList();
				lv->Clear();
				Pref_KeyDistProc_HELP1(lv);
				return 0;
			};
		};
	};
	return 0;
}

static BOOL CALLBACK keyPassChangeProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	if (uMsg == WM_CLOSE) EndDialog(hwndDlg,1);
	if (uMsg == WM_COMMAND) {
		if (LOWORD(wParam) == IDCANCEL) EndDialog(hwndDlg,1);
		if (LOWORD(wParam) == IDOK) {
			char oldpass[1024],buf2[1024],buf3[1024];
			GetDlgItemText(hwndDlg,IDC_OLDPASS,oldpass,sizeof(oldpass));
			GetDlgItemText(hwndDlg,IDC_NEWPASS1,buf2,sizeof(buf2));
			GetDlgItemText(hwndDlg,IDC_NEWPASS2,buf3,sizeof(buf3));
			if (strcmp(buf2,buf3)) {
				MessageBox(hwndDlg,"New passphrase mistyped",APP_NAME " Error",MB_OK|MB_ICONSTOP);
				return 0;
			};
			R_RSA_PRIVATE_KEY key;
			char keyfn[1024];
			sprintf(keyfn,"%s.pr4",g_config_prefix);
			memset(&key,0,sizeof(key));
			if (doLoadKey(hwndDlg,oldpass,keyfn,&key) || !key.bits) return 0;

			char newpasshash[SHA_OUTSIZE];
			SHAify c;
			c.add((unsigned char *)buf2,strlen(buf2));
			c.final((unsigned char *)newpasshash);

			kg_writePrivateKey(keyfn,&key,&g_random,newpasshash);

			memset(newpasshash,0,sizeof(newpasshash));
			memset(buf2,0,sizeof(buf2));
			memset(buf3,0,sizeof(buf3));
			memset(oldpass,0,sizeof(oldpass));
			memset(&key,0,sizeof(key));
			EndDialog(hwndDlg,0);
		};
	};
	return 0;
}

static BOOL CALLBACK Pref_SecurityProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM /*lParam*/)
{
	static W_ListView *lv;
	if (uMsg == WM_INITDIALOG) {
		if (g_config->ReadInt(CONFIG_bcastkey,CONFIG_bcastkey_DEFAULT)) {
			CheckDlgButton(hwndDlg,IDC_CHECK_PERIODIC_BCAST,BST_CHECKED);
		};
		#if (defined(_WIN32) && defined(_CHECK_RSA_BLINDING))
			SetWindowPos(
				GetDlgItem(hwndDlg,IDC_BUTTON_CHECKBLIND),NULL,
				0,0,0,0,
				SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOZORDER|
				SWP_SHOWWINDOW
				);
		#endif
		SendMessage(hwndDlg,WM_USER+0x109,0,0);
	};
	if (uMsg == WM_USER+0x109) {
		char sign[SHA_OUTSIZE*2+64];
		int x;
		if (g_key.bits) {
			strcpy(sign,"Key signature:");
			int t=strlen(sign);
			for (x = 0; x < SHA_OUTSIZE; x ++) sprintf(sign+t+x*2,"%02X",g_pubkeyhash[x]);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_COPY_PUBKEY),1);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_BROADCAST_PUB),1);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_EXPORT_PKEY),1);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHPASS),1);
		}
		else {
			strcpy(sign,"Key not loaded.");
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_COPY_PUBKEY),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_BROADCAST_PUB),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_EXPORT_PKEY),0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_CHPASS),0);
		};
		SetDlgItemText(hwndDlg,IDC_EDIT_SHOW_PKHASH,sign);
	};
	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_CHECK_PERIODIC_BCAST:
			{
				g_config->WriteInt(CONFIG_bcastkey,!!IsDlgButtonChecked(hwndDlg,IDC_CHECK_PERIODIC_BCAST));
				return 0;
			};
		case IDC_BUTTON_BROADCAST_PUB:
			{
				main_BroadcastPublicKey();
				return 0;
			};
		case IDC_BUTTON_COPY_PUBKEY:
			{
				copyMyPubKey(hwndDlg);
				return 0;
			};
		case IDC_CHPASS:
			{
				if (DialogBox(g_hInst,MAKEINTRESOURCE(IDD_CHPASS),hwndDlg,keyPassChangeProc))
					return 0;
			};
		case IDC_RELOADKEY:
			{
				reloadKey(NULL,hwndDlg);
				SendMessage(hwndDlg,WM_USER+0x109,0,0);
				return 0;
			};
		case IDC_BUTTON_EXPORT_PKEY:
			{
				char str[1024];
				sprintf(str,"%s.pr4",g_config_prefix);
				FILE *fp=fopen(str,"rb");
				if (fp) {
					fclose(fp);
					char buf[1024];buf[0]=0;
					OPENFILENAME l={sizeof(l),};
					l.hwndOwner = hwndDlg;
					l.lpstrFilter = "Private key files (*.prv)\0*.prv\0All files (*.*)\0*.*\0";
					l.lpstrFile = buf;
					l.nMaxFile = sizeof(buf);
					l.lpstrTitle = "Select key output file";
					l.lpstrDefExt = "";

					l.Flags = OFN_EXPLORER;
					if (GetSaveFileName(&l)) {
						if (!CopyFile(str,buf,FALSE))
							MessageBox(hwndDlg,"Error exporting key",APP_NAME " Error",MB_OK|MB_ICONEXCLAMATION);
					};
				}
				else {
					MessageBox(hwndDlg,"No Private Key available for export",APP_NAME " Error",MB_OK|MB_ICONEXCLAMATION);
				};
				return 0;
			};
		#if (defined(_WIN32) && defined(_CHECK_RSA_BLINDING))
		case IDC_BUTTON_CHECKBLIND:
			{
				CheckRsaBlinding();
				return 0;
			};
		#endif
		case IDC_KEYGEN:
			{
				int warn=0;
				char str[1024];
				sprintf(str,"%s.pr4",g_config_prefix);
				FILE *fp=fopen(str,"rb");
				if (fp) {
					fclose(fp);
					warn=1;
				};

				if (!warn || MessageBox(hwndDlg,"Warning - Generating a new key may remove your old private key.\r\n"
					"To save a copy of your private key, hit cancel and select\r\n"
					"\"Export Private Key\". To go ahead and generate a new key\r\n"
					"hit OK.",APP_NAME " Private Key Warning",MB_ICONQUESTION|MB_OKCANCEL) == IDOK)
				{
					char parms[2048];
					sprintf(parms,"%s.pr4",g_config_prefix);
					RunKeyGen(hwndDlg,parms);
					SendMessage(hwndDlg,WM_USER+0x109,0,0);
				};
				return 0;
			};
		case IDC_BUTTON_IMPORT_PKEY:
			{
				int warn=0;
				char str[1024];
				sprintf(str,"%s.pr4",g_config_prefix);
				FILE *fp=fopen(str,"rb");
				if (fp) {
					fclose(fp);
					warn=1;
				};

				if (!warn || MessageBox(hwndDlg,"Warning - Importing a new key may remove your old private key.\r\n"
					"To save a copy of your private key, hit cancel and select\r\n"
					"\"Export Private Key\". To go ahead and import a new key\r\n"
					"hit OK.",APP_NAME " Private Key Warning",MB_ICONQUESTION|MB_OKCANCEL) == IDOK)
				{
					char temp[1024];temp[0]=0;
					OPENFILENAME l={sizeof(l),};
					l.hwndOwner = hwndDlg;
					l.lpstrFilter = "Private key files (*.prv)\0*.prv\0All files (*.*)\0*.*\0";
					l.lpstrFile = temp;
					l.nMaxFile = sizeof(temp);
					l.lpstrTitle = "Import private key file";
					l.lpstrDefExt = "prv";

					l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST;

					if (GetOpenFileName(&l)) {
						char str[1024];
						sprintf(str,"%s.pr4",g_config_prefix);
						CopyFile(temp,str,FALSE);
						reloadKey(NULL,hwndDlg);
						SendMessage(hwndDlg,WM_USER+0x109,0,0);
					};
				};
				return 0;
			};
		};
	};
	return 0;
}

BOOL CALLBACK PrefsOuterProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWndTitle(hwndDlg,APP_NAME " Preferences");
			prefs_last_page=g_config->ReadInt(CONFIG_prefslp,CONFIG_prefslp_DEFAULT);
			lp_v=NULL;
			HWND hwnd=GetDlgItem(hwndDlg,IDC_TREE_PREFERENCES);
			HTREEITEM h;
			_additem(hwnd,TVI_ROOT,"About",0,12);
			_additem(hwnd,TVI_ROOT,"License",0,18);
			_additem(hwnd,TVI_ROOT,"Profiles",0,8);
			_additem(hwnd,TVI_ROOT,"Identity",0,13);
			h=_additem(hwnd,TVI_ROOT,"Network",1,0);
			{
				_additem(hwnd,h,"Password",0,16);
				_additem(hwnd,h,"Private Key",0,1);
				_additem(hwnd,h,"Public Keys",0,10);
				_additem(hwnd,h,"Pending Keys",0,11);
				_additem(hwnd,h,"Access Control",0,4);
				_additem(hwnd,h,"Your IP Addr.",0,15);
				_additem(hwnd,h,"Bandwidth",0,9);
				SendMessage(hwnd,TVM_EXPAND,TVE_EXPAND,(long)h);
			};
			_additem(hwnd,TVI_ROOT,"Display",0,5);
			h=_additem(hwnd,TVI_ROOT,"Chat",1,2);
			{
				_additem(hwnd,h,"Windows",0,17);
				_additem(hwnd,h,"Logging",0,19);
				_additem(hwnd,h,"Perform",0,14);
				SendMessage(hwnd,TVM_EXPAND,TVE_EXPAND,(long)h);
			};
			h=_additem(hwnd,TVI_ROOT,"File Transfers",1,3);
			{
				_additem(hwnd,h,"Receiving",0,6);
				_additem(hwnd,h,"Sending",0,7);
				SendMessage(hwnd,TVM_EXPAND,TVE_EXPAND,(long)h);
			};
			if (lp_v) SendMessage(hwnd,TVM_SELECTITEM,TVGN_CARET,(long)lp_v);
			return TRUE;
		};
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
			case IDOK:
				{
				DestroyWindow(hwndDlg);
				return FALSE;
				};
			};
			break;
		};
	case WM_NOTIFY:
		{
			NM_TREEVIEW *p;
			p=(NM_TREEVIEW *)lParam;

			if (p->hdr.code==TVN_SELCHANGED) {
				HTREEITEM hTreeItem = TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_TREE_PREFERENCES));
				TV_ITEM i={TVIF_HANDLE,hTreeItem,0,0,0,0,0};
				TreeView_GetItem(GetDlgItem(hwndDlg,IDC_TREE_PREFERENCES),&i);
				{
					int id=-1;
					DLGPROC proc=NULL;
					prefs_last_page=i.lParam;
					g_config->WriteInt(CONFIG_prefslp,prefs_last_page);
					switch (i.lParam)
					{
					case 0:		id=IDD_PREF_NETWORK;			proc=Pref_NetworkProc; break;
					case 1:		id=IDD_PREF_SECURITY;			proc=Pref_SecurityProc; break;
					case 2:		id=IDD_PREF_CHAT;				proc=Pref_ChatProc; break;
					case 3:		id=IDD_PREF_FILES;				proc=Pref_FilesProc; break;
					case 4:		id=IDD_PREF_NETWORK_A;			proc=Pref_Network2Proc; break;
					case 5:		id=IDD_PREF_DISPLAY;			proc=Pref_DisplayProc; break;
					case 6:		id=IDD_PREF_FILES_RECV;			proc=Pref_RecvProc; break;
					case 7:		id=IDD_PREF_FILES_SEND;			proc=Pref_SendProc; break;
					case 8:		id=IDD_PREF_PROFILES;			proc=Pref_ProfilesProc; break;
					case 9:		id=IDD_PREF_THROTTLE;			proc=Pref_ThrottleProc; break;
					case 10:	id=IDD_PREF_KEYDIST;			proc=Pref_KeyDistProc; break;
					case 11:	id=IDD_PREF_KEYDIST_PENDING;	proc=Pref_KeyDist2Proc; break;
					case 12:	id=IDD_PREF_ABOUT;				proc=Pref_AboutProc; break;
					case 13:	id=IDD_PREF_IDENT;				proc=Pref_IdentProc; break;
					case 14:	id=IDD_PREF_PERFORM;			proc=Pref_PerformProc; break;
					case 15:	id=IDD_PREF_NETWORK_B;			proc=Pref_Network3Proc; break;
					case 16:	id=IDD_PREF_NETWORK_C;			proc=Pref_Network4Proc; break;
					case 17:	id=IDD_PREF_CHAT_A;				proc=Pref_Chat2Proc; break;
					case 18:	id=IDD_PREF_LICENSE;			proc=Pref_LicenseProc; break;
					case 19:	id=IDD_PREF_CHAT_B;				proc=Pref_Chat3Proc; break;
					};

					static int radd;

					if (prefs_cur_wnd) {
						DestroyWindow(prefs_cur_wnd);
						prefs_cur_wnd=0;
						WndModSize(hwndDlg,-radd);
						radd=0;
					}
					else {
						radd=0;
					};

					if (id==IDD_PREF_LICENSE) {
						radd=LIC_ADD_SIZE;
						WndModSize(hwndDlg,radd);
					};

					if (id != -1) {
						RECT r;
						prefs_cur_wnd=CreateDialog(g_hInst,MAKEINTRESOURCE(id),hwndDlg,proc);
						GetWindowRect(GetDlgItem(hwndDlg,IDC_RECT),&r);
						ScreenToClient(hwndDlg,(LPPOINT)&r);
						SetWindowPos(
							prefs_cur_wnd,
							GetDlgItem(hwndDlg,IDC_TREE_PREFERENCES),
							r.left,r.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE
							);
						ShowWindow(prefs_cur_wnd,SW_SHOWNA);
					};
				};
			};
			break;
		};
	case WM_DESTROY:
		{
			if (prefs_cur_wnd) DestroyWindow(prefs_cur_wnd);
			prefs_cur_wnd=0;
			prefs_hwnd=NULL;
			CheckMenuItem(GetMenu(g_mainwnd),ID_VIEW_PREFERENCES,MF_UNCHECKED|MF_BYCOMMAND);
			g_config->Flush();
			SaveNetQ();
			break;
		};
	};
	return FALSE;
}

///////////////////////////wizard shit ////////////////////////////////////

static int sw_pos;
static HWND sw_wnd;

static BOOL WINAPI SW_Proc1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		int x;
		for (x = 0; x < sizeof(conspeed_strs)/sizeof(conspeed_strs[0]); x ++) {
			SendDlgItemMessage(hwndDlg,IDC_CONSPEED,CB_ADDSTRING,0,(long)conspeed_strs[x]);
		};
		for (x = 0; x < sizeof(conspeed_strs)/sizeof(conspeed_strs[0]); x ++) {
			if (g_config->ReadInt(CONFIG_conspeed,DEFAULT_CONSPEED) <= conspeed_speeds[x]) break;
		};
		if (x == sizeof(conspeed_strs)/sizeof(conspeed_strs[0])) x--;
		SendDlgItemMessage(hwndDlg,IDC_CONSPEED,CB_SETCURSEL,x,0);
		SetDlgItemText(hwndDlg,IDC_EDIT_NICK,g_config->ReadString(CONFIG_nick,""));
		SetDlgItemText(hwndDlg,IDC_EDIT_USERNAME,g_config->ReadString(CONFIG_userinfo,CONFIG_userinfo_DEFAULT));
		SetDlgItemText(hwndDlg,IDC_EDIT_NETPASSWORD,g_config->ReadString(CONFIG_networkname,CONFIG_networkname_DEFAULT));
	};
	if (uMsg == WM_COMMAND && wParam == MAKEWPARAM(IDC_EDIT_NETPASSWORD,EN_CHANGE)) {
		char buf[4096];
		GetDlgItemText(hwndDlg,IDC_EDIT_NETPASSWORD,buf,sizeof(buf));
		g_config->WriteString(CONFIG_networkname,buf);
		InitNeworkHash();
		memset(buf,0,sizeof(buf));
	};
	if (uMsg == WM_COMMAND && wParam == MAKEWPARAM(IDC_EDIT_NICK,EN_CHANGE)) {
		GetDlgItemText(hwndDlg,IDC_EDIT_NICK,g_regnick,sizeof(g_regnick));
		g_regnick[sizeof(g_regnick)-1]=0;
		g_config->WriteString(CONFIG_nick,g_regnick);
	};
	if (uMsg == WM_COMMAND && wParam == MAKEWPARAM(IDC_EDIT_USERNAME,EN_CHANGE)) {
		char buf[256];
		GetDlgItemText(hwndDlg,IDC_EDIT_USERNAME,buf,sizeof(buf));
		buf[sizeof(buf)-1]=0;
		g_config->WriteString(CONFIG_userinfo,buf);
	};
	if (uMsg == WM_COMMAND && wParam == MAKEWPARAM(IDC_CONSPEED,CBN_SELCHANGE)) {
		int x=SendDlgItemMessage(hwndDlg,IDC_CONSPEED,CB_GETCURSEL,0,0);
		if (x >= 0 && x < sizeof(conspeed_strs)/sizeof(conspeed_strs[0])) {
			g_config->WriteInt(CONFIG_conspeed,conspeed_speeds[x]);
			int tab[sizeof(conspeed_strs)/sizeof(conspeed_strs[0])]={1,2,2,3,4};
			g_config->WriteInt(CONFIG_keepupnet,tab[x]);
		};
	};
	return 0;
}

static BOOL WINAPI SW_Proc2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		SetDlgItemText(hwndDlg,IDC_SAVEPATH,g_config->ReadString(CONFIG_downloadpath,""));
		SetDlgItemText(hwndDlg,IDC_FILEPATHLIST,g_config->ReadString(CONFIG_databasepath,CONFIG_databasepath_DEFAULT));
	};
	if (uMsg == WM_COMMAND) {
		if (LOWORD(wParam) == IDC_ADDDIR) {
			char name[MAX_PATH]="";
			BROWSEINFO bi={0,};
			ITEMIDLIST *idlist;
			bi.hwndOwner = hwndDlg;
			bi.pszDisplayName = name;
			bi.lpfn=BrowseCallbackProc;
			bi.lpszTitle = "Select a directory:";
			bi.ulFlags = BIF_RETURNONLYFSDIRS;
			idlist = SHBrowseForFolder( &bi );
			if (idlist) {
				char buf[4096];
				SHGetPathFromIDList( idlist, name );
				IMalloc *m;
				SHGetMalloc(&m);
				m->Free(idlist);
				GetDlgItemText(hwndDlg,IDC_FILEPATHLIST,buf,sizeof(buf));
				if (buf[0] && buf[strlen(buf)-1]!=';') strcat(buf,";");
				strcat(buf,name);
				SetDlgItemText(hwndDlg,IDC_FILEPATHLIST,buf);
				g_config->WriteString(CONFIG_databasepath,buf);
			};
		};
		if (wParam == MAKEWPARAM(IDC_FILEPATHLIST,EN_CHANGE) ) {
			char buf[4096];
			GetDlgItemText(hwndDlg,IDC_FILEPATHLIST,buf,sizeof(buf));
			g_config->WriteString(CONFIG_databasepath,buf);
		};
	};
	if (uMsg == WM_COMMAND && LOWORD(wParam) == IDC_SAVEPATH) {
		char name[MAX_PATH];
		BROWSEINFO bi={0,};
		ITEMIDLIST *idlist;
		GetDlgItemText(hwndDlg,IDC_SAVEPATH,name,MAX_PATH);
		bi.hwndOwner = hwndDlg;
		bi.pszDisplayName = name;
		bi.lpfn=BrowseCallbackProc;
		bi.lParam=(LPARAM)GetDlgItem(hwndDlg,IDC_SAVEPATH);
		bi.lpszTitle = "Select a directory:";
		bi.ulFlags = BIF_RETURNONLYFSDIRS;
		idlist = SHBrowseForFolder( &bi );
		if (idlist) {
			SHGetPathFromIDList( idlist, name );
			IMalloc *m;
			SHGetMalloc(&m);
			m->Free(idlist);
			SetDlgItemText(hwndDlg,IDC_SAVEPATH,name);
			g_config->WriteString(CONFIG_downloadpath,name);
		};
	};
	return 0;
}
static BOOL WINAPI SW_Proc4(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		char buf[128];
		sprintf(buf,"%d keys loaded total",g_pklist.GetSize());
		SetDlgItemText(hwndDlg,IDC_KEYINF,buf);
		char str[1024];
		sprintf(str,"%s.pr4",g_config_prefix);
		FILE *fp=fopen(str,"rb");
		if (fp) {
			fclose(fp);
			if (!g_key.bits) reloadKey(NULL,hwndDlg);
		};
		SendMessage(hwndDlg,WM_USER+0x132,0,0);
	};

	if (uMsg == WM_USER+0x132) {
		char sign[SHA_OUTSIZE*2+64];
		if (g_key.bits) {
			strcpy(sign,"Key signature:");
			int t=strlen(sign);
			Bin2Hex(sign+t,g_pubkeyhash,SHA_OUTSIZE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_COPY_PUBKEY),1);
		}
		else {
			strcpy(sign,"Key not loaded.");
			EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_COPY_PUBKEY),0);
		};
		SetDlgItemText(hwndDlg,IDC_KEYINFO,sign);
	};

	if (uMsg == WM_COMMAND) {
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_COPY_PUBKEY:
			{
				copyMyPubKey(hwndDlg);
				return 0;
			};
		case IDC_BUTTON_IMPORT_PUBKEYS:
			{
				importPubDlg(hwndDlg);
				char buf[128];
				sprintf(buf,"%d keys loaded total",g_pklist.GetSize());
				SetDlgItemText(hwndDlg,IDC_KEYINF,buf);
				return 0;
			};
		case IDC_BUTTON_RUN_KEYGEN:
			{
				char parms[2048];
				sprintf(parms,"%s.pr4",g_config_prefix);
				RunKeyGen(hwndDlg,parms);
				SendMessage(hwndDlg,WM_USER+0x132,0,0);
				return 0;
			};
		case IDC_BUTTON_IMPORT_KEY:
			{
				char temp[1024];temp[0]=0;
				OPENFILENAME l={sizeof(l),};
				l.hwndOwner = hwndDlg;
				l.lpstrFilter = "Private key files (*.prv)\0*.prv\0All files (*.*)\0*.*\0";
				l.lpstrFile = temp;
				l.nMaxFile = sizeof(temp);
				l.lpstrTitle = "Import private key file";
				l.lpstrDefExt = "prv";

				l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST;;

				if (GetOpenFileName(&l)) {
					char str[1024];
					sprintf(str,"%s.pr4",g_config_prefix);
					CopyFile(temp,str,FALSE);
					reloadKey(NULL,hwndDlg);
					SendMessage(hwndDlg,WM_USER+0x132,0,1);
				};
				return 0;
			};
		};
	};
	return 0;
}

static void dosw(HWND hwndDlg)
{
	void *procs[]={SW_Proc1,SW_Proc4,SW_Proc2};
	if (sw_wnd) DestroyWindow(sw_wnd);
	sw_wnd=0;
	int tab[]={IDD_WIZ1,IDD_WIZ4,IDD_WIZ2};
	sw_wnd=CreateDialog(g_hInst,MAKEINTRESOURCE(tab[sw_pos]),hwndDlg,(BOOL (WINAPI*)(HWND,UINT,WPARAM,LPARAM))procs[sw_pos]);
	RECT r;
	GetWindowRect(GetDlgItem(hwndDlg,IDC_RECTANGLE),&r);
	ScreenToClient(hwndDlg,(LPPOINT)&r);
	ScreenToClient(hwndDlg,((LPPOINT)&r)+1);
	SetWindowPos(sw_wnd,NULL,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER|SWP_NOACTIVATE);
	ShowWindow(sw_wnd,SW_SHOWNA);
	char str[64];
	sprintf(str,APP_NAME " Profile Setup Wizard (Step %d/4)",sw_pos+1);
	SetWindowText(hwndDlg,str);
}

BOOL WINAPI SetupWizProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			sw_pos=0;
			dosw(hwndDlg);
			return 0;
		};
	case WM_CLOSE:
		{
			if (MessageBox(hwndDlg,"Abort " APP_NAME " Profile Setup Wizard?",APP_NAME,MB_YESNO|MB_ICONQUESTION) == IDYES) {
				if (sw_wnd) DestroyWindow(sw_wnd);
				sw_wnd=0;
				EndDialog(hwndDlg,0);
			};
			return 0;
		};
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_BACK:
			case IDOK:
				{
					if (LOWORD(wParam) == IDC_BACK) {
						if (sw_pos>0) sw_pos--;
						else SendMessage(hwndDlg,WM_CLOSE,0,0);
					}
					else {  //IDOK
						if (sw_pos < 2) sw_pos++;
						else {
							if (sw_wnd) DestroyWindow(sw_wnd);
							sw_wnd=0;
							EndDialog(hwndDlg,1);
							return 0;
						};
					};
					dosw(hwndDlg);
					SetDlgItemText(hwndDlg,IDOK,(sw_pos==2)?"Run":"Next>>");
					SetDlgItemText(hwndDlg,IDC_BACK,(sw_pos==0)?"Quit":"<<Back");
					break;
				};
			};
			return 0;
		};
	};
	return 0;
}

//////////////////////////////profile selection dialog //////////////////////////////////////////

static int pep_mode; //copy,rename,create
static char pep_n[128];

static BOOL WINAPI ProfEditProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(hwndDlg,pep_mode?pep_mode==2?"Create Profile":"Rename Profile":"Copy Profile");
			SetDlgItemText(hwndDlg,IDC_EDIT_SOURCENAME,pep_n);
			SetDlgItemText(hwndDlg,IDC_EDIT_DESTNAME,pep_mode != 1 ? "new profile" : pep_n);
			return 0;
		};
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
			case IDCANCEL:
				{
					GetDlgItemText(hwndDlg,IDC_EDIT_DESTNAME,pep_n,sizeof(pep_n));
					EndDialog(hwndDlg,LOWORD(wParam)==IDCANCEL);
					return 0;
				};
			};
			return 0;
		};
	};
	return 0;
}

static BOOL WINAPI ProfilesProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(hwndDlg,APP_NAME " Profile Manager");

			if (GetPrivateProfileInt("config","showprofiles",0,g_config_mainini)) {
				CheckDlgButton(hwndDlg,IDC_CHECK_SHOW_PROFILEMAN_ON_START,BST_CHECKED);
			};

			HWND hwndList=GetDlgItem(hwndDlg,IDC_LIST_PROFILES);
			WIN32_FIND_DATA fd;
			HANDLE h;

			int gotAny=0;
			char tmp[1024];
			getProfilePath(tmp);
			strcat(tmp,"*.pr0");
			h=FindFirstFile(tmp,&fd);
			if (h) {
				do{
					if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						char *p=fd.cFileName;
						while (*p) p++;
						while (p >= fd.cFileName && *p != '.') p--;
						if (p > fd.cFileName) {
							*p=0;
							if (strlen(fd.cFileName) < sizeof(g_profile_name)) {
								SendMessage(hwndList,LB_ADDSTRING,0,(LPARAM)fd.cFileName);
								gotAny=1;
							};
						};
					};
				}
				while (FindNextFile(h,&fd));
				FindClose(h);
			};
			if (!gotAny) {
				SendMessage(hwndList,LB_ADDSTRING,0,(LPARAM)"Default");
			};
			int l=SendMessage(hwndList,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)g_profile_name);
			if (l == LB_ERR) l=SendMessage(hwndList,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)"Default");
			if (l != LB_ERR) SendMessage(hwndList,LB_SETCURSEL,(WPARAM)l,0);

			return 0;
		};
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_RENAME:
			case IDC_CLONE:
				{
					int l=SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_GETCURSEL,0,0);
					if (l != LB_ERR) {
						pep_mode=LOWORD(wParam) == IDC_RENAME;
						SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_GETTEXT,(WPARAM)l,(LPARAM)pep_n);
						char oldfn[1024+1024];
						getProfilePath(oldfn);
						strcat(oldfn,pep_n);
						strcat(oldfn,".pr0");

						if (!DialogBox(g_hInst,MAKEINTRESOURCE(IDD_PROFILE_MOD),hwndDlg,ProfEditProc) && pep_n[0]) {
							char tmp[1024+1024];
							getProfilePath(tmp);
							strcat(tmp,pep_n);
							strcat(tmp,".pr0");
							if (stricmp(tmp,oldfn)) {
								BOOL ret;
								if (LOWORD(wParam) == IDC_RENAME) ret=MoveFile(oldfn,tmp);
								else ret=CopyFile(oldfn,tmp,TRUE);
								if (ret) {
									tmp[strlen(tmp)-1]++;
									oldfn[strlen(oldfn)-1]++;
									if (LOWORD(wParam) == IDC_RENAME) MoveFile(oldfn,tmp);
									//else CopyFile(oldfn,tmp,FALSE);

									tmp[strlen(tmp)-1]++;
									oldfn[strlen(oldfn)-1]++;
									if (LOWORD(wParam) == IDC_RENAME) MoveFile(oldfn,tmp);
									else CopyFile(oldfn,tmp,FALSE);

									tmp[strlen(tmp)-1]++;
									oldfn[strlen(oldfn)-1]++;
									if (LOWORD(wParam) == IDC_RENAME) MoveFile(oldfn,tmp);
									else CopyFile(oldfn,tmp,FALSE);

									tmp[strlen(tmp)-1]++;
									oldfn[strlen(oldfn)-1]++;
									if (LOWORD(wParam) == IDC_RENAME) MoveFile(oldfn,tmp);
									else CopyFile(oldfn,tmp,FALSE);

									if (LOWORD(wParam) == IDC_RENAME) SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_DELETESTRING,(WPARAM)l,0);

									l=SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_ADDSTRING,0,(WPARAM)pep_n);
									SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_SETCURSEL,(WPARAM)l,0);
								}
								else MessageBox(hwndDlg,LOWORD(wParam) == IDC_RENAME ? "Error renaming profile" : "Error copying profile",APP_NAME " Error",MB_OK|MB_ICONEXCLAMATION);
							};
						};
					};
					return 0;
				};
			case IDC_CREATE:
				{
					pep_mode=2;
					strcpy(pep_n,"NULL");
					if (!DialogBox(g_hInst,MAKEINTRESOURCE(IDD_PROFILE_MOD),hwndDlg,ProfEditProc) && pep_n[0]) {
						char tmp[1024+1024];
						getProfilePath(tmp);
						strcat(tmp,pep_n);
						strcat(tmp,".pr0");
						HANDLE h=CreateFile(tmp,0,0,NULL,CREATE_NEW,0,NULL);
						if (h != INVALID_HANDLE_VALUE) {
							CloseHandle(h);
							int l=SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_ADDSTRING,0,(WPARAM)pep_n);
							SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_SETCURSEL,(WPARAM)l,0);
						}
						else MessageBox(hwndDlg,"Error creating profile",APP_NAME " Error",MB_OK|MB_ICONEXCLAMATION);
					};
					return 0;
				};
			case IDC_DELETE:
				{
					int l=SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_GETCURSEL,0,0);
					if (l != LB_ERR) {
						char tmp[1024+1024];
						getProfilePath(tmp);
						SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_GETTEXT,(WPARAM)l,(LPARAM)(tmp+strlen(tmp)));
						strcat(tmp,".pr0");
						if (DeleteFile(tmp)) {
							tmp[strlen(tmp)-1]++;
							DeleteFile(tmp);
							tmp[strlen(tmp)-1]++;
							DeleteFile(tmp);
							tmp[strlen(tmp)-1]++;
							DeleteFile(tmp);
							tmp[strlen(tmp)-1]++;
							DeleteFile(tmp);
							SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_DELETESTRING,l,0);
						}
						else MessageBox(hwndDlg,"Error deleting profile",APP_NAME " Error",MB_OK|MB_ICONEXCLAMATION);
					};
					return 0;
				};
			case IDC_LIST_PROFILES:
				{
					if (HIWORD(wParam) != LBN_DBLCLK) return 0;
				};
			case IDOK:
				{
					int l=SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_GETCURSEL,0,0);
					if (l != LB_ERR) {
						SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_GETTEXT,(WPARAM)l,(LPARAM)g_profile_name);
						EndDialog(hwndDlg,IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOW_PROFILEMAN_ON_START)?3:1);
					};
					return 0;
				};
			case IDCANCEL:
				{

					int l=SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_GETCURSEL,0,0);
					if (l != LB_ERR) {
						SendDlgItemMessage(hwndDlg,IDC_LIST_PROFILES,LB_GETTEXT,(WPARAM)l,(LPARAM)g_profile_name);
					};
					EndDialog(hwndDlg,IsDlgButtonChecked(hwndDlg,IDC_CHECK_SHOW_PROFILEMAN_ON_START)?2:0);
					return 0;
				};
			};
			return 0;
		};
	};

	return 0;
}

int Prefs_SelectProfile(int force) //returns 1 on user abort
{
	if (g_profile_name[0]) return 0;

	if (force || GetPrivateProfileInt("config","showprofiles",0,g_config_mainini)) { //bring up profile manager
		GetPrivateProfileString("config","lastprofile","Default",g_profile_name,sizeof(g_profile_name),g_config_mainini);
		int rv=DialogBox(g_hInst,MAKEINTRESOURCE(IDD_PROFILES),GetDesktopWindow(),ProfilesProc);
		if (!g_profile_name[0]) strcpy(g_profile_name,"Default");
		WritePrivateProfileString("config","showprofiles",(rv&2)?"1":"0",g_config_mainini);
		WritePrivateProfileString("config","lastprofile",g_profile_name,g_config_mainini);
		return !(rv&1);
	};
	strcpy(g_profile_name,"Default");
	char proffn[4096];
	sprintf(proffn,"%s\\%s.pr0",g_config_prefix,g_profile_name);
	if (!GetPrivateProfileInt("profile","valid",0,proffn)) {
		sprintf(proffn,"%s\\*.pr0",g_config_prefix);
		WIN32_FIND_DATA d;
		HANDLE h=FindFirstFile(proffn,&d);
		if (h != INVALID_HANDLE_VALUE) {
			do{
				sprintf(proffn,"%s\\%s",g_config_prefix,d.cFileName);
				if (GetPrivateProfileInt("profile","valid",0,proffn)) {
					char *p=d.cFileName;
					while (*p) p++;
					while (p > d.cFileName && *p != '.') p--;
					if (p > d.cFileName) {
						*p=0;
						safe_strncpy(g_profile_name,d.cFileName,sizeof(g_profile_name));
						break;
					};
				};
			} while (FindNextFile(h,&d));
			FindClose(h);
		};
	};
	return 0;

}
