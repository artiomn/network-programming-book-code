/*
WASTE - main.cpp (Windows main entry point and a lot of code :)
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

#include "resource.hpp"

#include "rsa/md5.hpp"
#include "childwnd.hpp"
#include "m_upload.hpp"
#include "m_chat.hpp"
#include "m_search.hpp"
#include "m_ping.hpp"
#include "m_keydist.hpp"
#include "m_lcaps.hpp"
#include "d_chat.hpp"
#include "prefs.hpp"
#include "xferwnd.hpp"
#include "netq.hpp"
#include "netkern.hpp"
#include "srchwnd.hpp"

static UINT CALLBACK fileHookProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		SetDlgItemText(hwndDlg,IDC_UPATH,g_filedlg_ulpath);
	};
	if (uMsg == WM_COMMAND) {
		if (LOWORD(wParam) == IDC_UPATH && HIWORD(wParam) == EN_CHANGE) {
			GetDlgItemText(hwndDlg,IDC_UPATH,g_filedlg_ulpath,sizeof(g_filedlg_ulpath));
			return 1;
		};
	};
	return 0;
}

void UserListContextMenu(HWND htree)
{
	TVHITTESTINFO hit;
	hit.pt.x=GET_X_LPARAM(GetMessagePos());
	hit.pt.y=GET_Y_LPARAM(GetMessagePos());
	ScreenToClient(htree,&hit.pt);
	HTREEITEM h=TreeView_HitTest(htree,&hit);
	if (!h) h=TreeView_GetSelection(htree);
	if (h) {
		char text[256];
		TVITEM i;
		i.mask=TVIF_TEXT|TVIF_HANDLE;
		i.hItem=h;
		i.pszText=text;
		i.cchTextMax=sizeof(text);
		text[0]=0;
		TreeView_GetItem(htree,&i);
		if (i.pszText[0]) {
			HMENU hMenu=GetSubMenu(g_context_menus,2);
			POINT p;
			GetCursorPos(&p);
			int x=TrackPopupMenu(hMenu,TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,p.x,p.y,0,GetParent(htree),NULL);
			if (x == ID_SENDFILENODE) {
				char *fnroot=(char*)malloc(65536*4);
				OPENFILENAME l={sizeof(l),};
				fnroot[0]=0;
				l.hwndOwner = GetParent(htree);
				l.lpstrFilter = "All files (*.*)\0*.*\0";
				l.lpstrFile = fnroot;
				l.nMaxFile = 65535*4;
				l.lpstrTitle = "Open file(s) to send";
				l.lpstrDefExt = "";
				l.hInstance=g_hInst;
				l.lpfnHook=fileHookProc;
				l.lpTemplateName=MAKEINTRESOURCE(IDD_FILESUBDLG);
				l.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_ALLOWMULTISELECT|OFN_ENABLETEMPLATE|OFN_ENABLEHOOK|OFN_FILEMUSTEXIST;
				if (GetOpenFileName(&l)) {
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
						Xfer_UploadFileToUser(GetParent(htree),fullfn,i.pszText,g_filedlg_ulpath);
						fn+=strlen(fn)+1;
					};
				};
				free(fnroot);
			}
			else if (x == ID_BROWSEUSER) {
				char buf[1024];
				sprintf(buf,"/%s",i.pszText);
				SendMessage(g_mainwnd,WM_COMMAND,IDC_SEARCH,0);
				Search_Search(buf);
			}
			else if (x == ID_PRIVMSGNODE) {
				chat_ShowRoom(i.pszText,1);
			}
			else if (x == ID_WHOISUSER) {
				T_Message msg;
				//send a message to text that is /whois
				C_MessageChat req;
				req.set_chatstring("/whois");
				req.set_dest(i.pszText);
				req.set_src(g_regnick);
				msg.data=req.Make();
				msg.message_type=MESSAGE_CHAT;
				if (msg.data) {
					msg.message_length=msg.data->GetLength();
					g_mql->send(&msg);
				};
			};
		};
	};
}

void handleWasteURL(char *url)
{
	char tmp[2048];
	safe_strncpy(tmp,url,sizeof(tmp));
	char *in=tmp+6,*out=tmp;
	while (*in) {
		if (!strncmp(in,"%20",3)) { in+=3; *out++=' '; }
		else *out++=*in++;
	};
	*out=0;

	if (tmp[0] == '?') {
		if (!strnicmp(tmp,"?chat=",6)) {
			chat_ShowRoom(tmp+6,2);
		}
		else if (!strnicmp(tmp,"?browse=",8)) {
			SendMessage(g_mainwnd,WM_COMMAND,IDC_SEARCH,0);
			Search_Search(tmp+8);
		};
	}
	else {
		SendMessage(g_mainwnd,WM_COMMAND,IDC_SEARCH,0);
		Search_Search(tmp);
	};
}

static void SendDirectoryToUser(HWND hwndDlg, char *fullfn, char *text, int offs)
{
	char maskstr[2048];
	WIN32_FIND_DATA d;
	sprintf(maskstr,"%s\\*.*",fullfn);
	HANDLE h=FindFirstFile(maskstr,&d);
	if (h != INVALID_HANDLE_VALUE) {
		do{
			if (d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (d.cFileName[0] != '.') {
					sprintf(maskstr,"%s\\%s",fullfn,d.cFileName);
					SendDirectoryToUser(hwndDlg,maskstr,text,offs);
				};
			}
			else {
				sprintf(maskstr,"%s\\%s",fullfn,d.cFileName);
				Xfer_UploadFileToUser(hwndDlg,maskstr,text,fullfn+offs);
			};
		}
		while (FindNextFile(h,&d));
		FindClose(h);
	};
}

void UserListOnDropFiles(HWND hwndDlg, HWND htree, HDROP hdrop, char *forcenick)
{
	TVHITTESTINFO i;
	DragQueryPoint(hdrop,&i.pt);
	ClientToScreen(hwndDlg,&i.pt);
	ScreenToClient(htree,&i.pt);
	HTREEITEM htt=0;
	if (!forcenick) htt=TreeView_HitTest(htree,&i);
	if (forcenick || htt) {
		char fullfn[1024];
		int x,y = DragQueryFile(hdrop,0xffffffff,fullfn,sizeof(fullfn));
		char texto[64];
		TVITEM tvi;
		tvi.mask=TVIF_TEXT|TVIF_HANDLE;
		tvi.hItem=htt;
		texto[0]=0;
		tvi.pszText=texto;
		tvi.cchTextMax=sizeof(texto);
		if (!forcenick) TreeView_GetItem(htree,&tvi);
		if (forcenick || tvi.pszText[0]) {
			char *text=forcenick?forcenick:tvi.pszText;
			if (text[0]) for (x = 0; x < y; x ++) {
				DragQueryFile(hdrop,x,fullfn,sizeof(fullfn));
				DWORD attr=GetFileAttributes(fullfn);
				if (attr != 0xFFFFFFFF) {
					if (attr & FILE_ATTRIBUTE_DIRECTORY) {
						int a=strlen(fullfn);
						while (a>=0 && fullfn[a] != '\\') a--;
						a++;
						SendDirectoryToUser(hwndDlg,fullfn,text,a);
					}
					else Xfer_UploadFileToUser(hwndDlg,fullfn,text,"");
				};
			};
		};
		DragFinish(hdrop);
	};
}

static int mainwnd_old_yoffs;
static ChildWndResizeItem mainwnd_rlist[]={
	{IDC_USERSLABEL,0x0010},
	{IDC_USERS,0x0011},
	{IDC_DIVIDER,0x0111},
	{IDC_CHATROOMSLABEL,0x0111},
	{IDC_CHATROOMS,0x0111},
	{IDC_CREATECHATROOM,0x1010},
	{IDC_SEARCH,0x1010},
	{IDC_NETSTATUS,0x0010},
};

//unfortunately this code is super super dumb. I actually did a much better
//version of it for gen_ml later on, but alas, too late now.
static void MainDiv_UpdPos(int yp)
{
	RECT r,r2;
	GetClientRect(g_mainwnd,&r);

	GetWindowRect(GetDlgItem(g_mainwnd,mainwnd_rlist[2].id),&r2);
	int tmp=mainwnd_old_yoffs-(r.bottom-(yp+r2.top));

	{ //see if bottom of IDC_USERS is < top of IDC_USERS+16
		GetWindowRect(GetDlgItem(g_mainwnd,mainwnd_rlist[1].id),&r2);
		int newy=yp+r2.top;
		ScreenToClient(g_mainwnd,(LPPOINT)&r2);
		ScreenToClient(g_mainwnd,((LPPOINT)&r2)+1);
		if (((r2.bottom-r2.top)+newy) < mainwnd_rlist[1].rinfo.top+16)
			tmp=80;
	};
	if (tmp < 80) {
		int x;
		for (x = 1; x < 5; x ++) {
			GetWindowRect(GetDlgItem(g_mainwnd,mainwnd_rlist[x].id),&r2);
			int newy=yp+r2.top;
			ScreenToClient(g_mainwnd,(LPPOINT)&r2);
			ScreenToClient(g_mainwnd,((LPPOINT)&r2)+1);
			if (x > 1) mainwnd_rlist[x].rinfo.top=r.bottom-newy;
			if (x < 4) mainwnd_rlist[x].rinfo.bottom=r.bottom-((r2.bottom-r2.top)+newy);
		};
		g_config->WriteInt(CONFIG_main_divpos,mainwnd_old_yoffs-mainwnd_rlist[2].rinfo.top);
	};

	childresize_resize(g_mainwnd,mainwnd_rlist,sizeof(mainwnd_rlist)/sizeof(mainwnd_rlist[0]));
}

static UINT WM_TASKBARCREATED;

static WNDPROC div_oldWndProc;
static BOOL CALLBACK div_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_LBUTTONDOWN) {
		SetForegroundWindow(hwndDlg);
		SetCapture(hwndDlg);
		SetCursor(LoadCursor(NULL,IDC_SIZENS));
	}
	else if (uMsg == WM_SETCURSOR) {
		SetCursor(LoadCursor(NULL,IDC_SIZENS));
		return TRUE;
	}
	else if (uMsg == WM_MOUSEMOVE && GetCapture()==hwndDlg) {
		POINT p;
		RECT r3;
		GetCursorPos(&p);
		ScreenToClient(GetParent(hwndDlg),(LPPOINT)&p);
		GetWindowRect(hwndDlg,&r3);
		MainDiv_UpdPos(p.y-r3.top);
	}
	else if (uMsg == WM_MOUSEMOVE) {
		SetCursor(LoadCursor(NULL,IDC_SIZENS));
	}
	else if (uMsg == WM_LBUTTONUP) {
		ReleaseCapture();
	};
	return CallWindowProc(div_oldWndProc,hwndDlg,uMsg,wParam,lParam);
}

#define CHATMENU_HIDEALL 4000
#define CHATMENU_SHOWALL 4001
#define CHATMENU_BASE 4002
#define CHATMENU_MAX 5000

static void MakeChatSubMenu(HMENU hMenu)
{
	MENUITEMINFO i={sizeof(i),};
	while (RemoveMenu(hMenu,0,MF_BYPOSITION));

	i.fMask=MIIM_TYPE|MIIM_DATA|MIIM_ID|MIIM_STATE;
	i.fType=MFT_STRING;
	i.fState=MFS_ENABLED;
	chatroom_item *p=L_Chatroom;
	if (!p) {
		i.dwTypeData="No active chats";
		i.fState=MFS_DISABLED;
		InsertMenuItem(hMenu,0,TRUE,&i);
	}
	else {
		i.dwTypeData="Hide all chat windows";
		i.wID=CHATMENU_HIDEALL;
		InsertMenuItem(hMenu,0,TRUE,&i);

		i.dwTypeData="Show all chat windows";
		i.wID=CHATMENU_SHOWALL;
		InsertMenuItem(hMenu,1,TRUE,&i);

		i.wID=0;
		i.fType=MFT_SEPARATOR;
		InsertMenuItem(hMenu,2,TRUE,&i);
		i.fType=MFT_STRING;

		i.wID = CHATMENU_BASE;

		while (p && i.wID < CHATMENU_MAX) {
			i.fState = IsWindowVisible(p->hwnd)?MFS_CHECKED:MFS_ENABLED;
			i.dwTypeData=p->channel;
			InsertMenuItem(hMenu,3+i.wID - CHATMENU_BASE,TRUE,&i);
			i.wID++;
			p=p->next;
		};
	};
}

static BOOL WINAPI Main_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	static int m_isinst;
	static dlgSizeInfo sizeinf={"m",100,100,112,335};
	if (uMsg == WM_TASKBARCREATED) {
		if (systray_state) {
			systray_del(g_mainwnd);
			systray_add(g_mainwnd,g_hSmallIcon);
		};
		return 0;
	};
	switch (uMsg)
	{
	case WM_INITMENU:
		{
			MakeChatSubMenu(GetSubMenu(GetSubMenu((HMENU)wParam,1),5));
			return 0;
		};
	case WM_DROPFILES:
		{
			UserListOnDropFiles(hwndDlg,GetDlgItem(hwndDlg,IDC_USERS),(HDROP)wParam,NULL);
			return 0;
		};
	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO m=(LPMINMAXINFO)lParam;
			if (m) {
				m->ptMinTrackSize.x=112;
				m->ptMinTrackSize.y=260 - g_config->ReadInt(CONFIG_main_divpos,CONFIG_main_divpos_DEFAULT);
			};
			return 0;
		};
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
		{
			POINT p;
			RECT r3;
			GetWindowRect(GetDlgItem(hwndDlg,IDC_DIVIDER),&r3);
			GetCursorPos(&p);
			int d=p.y-r3.bottom;
			int d2=p.y-r3.top;
			if (d<0)d=-d;
			if (d2<0)d2=-d2;
			if (d2<d) d=d2;
			if (d < 6) SendDlgItemMessage(hwndDlg,IDC_DIVIDER,uMsg,0,0);
			return 0;
		};
	case WM_USER_TITLEUPDATE:
		{
			SetWndTitle(hwndDlg,APP_NAME);
			if (g_search_wnd) SendMessage(g_search_wnd,uMsg,wParam,lParam);
			SendMessage(g_netstatus_wnd,uMsg,wParam,lParam);
			SendMessage(g_xferwnd,uMsg,wParam,lParam);
			chat_updateTitles();
			if (systray_state) {
				systray_del(g_mainwnd);
				systray_add(g_mainwnd,g_hSmallIcon);
			};
			TreeView_SetTextColor(GetDlgItem(g_mainwnd,IDC_USERS),g_config->ReadInt(CONFIG_mul_color,CONFIG_mul_color_DEFAULT));
			TreeView_SetTextColor(GetDlgItem(g_mainwnd,IDC_CHATROOMS),g_config->ReadInt(CONFIG_mul_color,CONFIG_mul_color_DEFAULT));
			TreeView_SetBkColor(GetDlgItem(g_mainwnd,IDC_USERS),g_config->ReadInt(CONFIG_mul_bgc,CONFIG_mul_bgc_DEFAULT));
			TreeView_SetBkColor(GetDlgItem(g_mainwnd,IDC_CHATROOMS),g_config->ReadInt(CONFIG_mul_bgc,CONFIG_mul_bgc_DEFAULT));
			return 0;
		};
	case WM_INITDIALOG:
		{
			g_mainwnd=hwndDlg;
			div_oldWndProc=(WNDPROC) SetWindowLong(GetDlgItem(hwndDlg,IDC_DIVIDER),GWL_WNDPROC,(LONG)div_newWndProc);

			childresize_init(hwndDlg,mainwnd_rlist,sizeof(mainwnd_rlist)/sizeof(mainwnd_rlist[0]));

			mainwnd_old_yoffs=mainwnd_rlist[2].rinfo.top;
			POINT p={0,0};
			ScreenToClient(hwndDlg,&p);
			MainDiv_UpdPos(g_config->ReadInt(CONFIG_main_divpos,CONFIG_main_divpos_DEFAULT)+p.y);

			SetWndTitle(hwndDlg,APP_NAME);

			if (g_config->ReadInt(CONFIG_systray,CONFIG_systray_DEFAULT)) systray_add(hwndDlg, g_hSmallIcon);

			handleDialogSizeMsgs(hwndDlg,uMsg,wParam,lParam,&sizeinf);
			ShowWindow(hwndDlg,SW_SHOW);

			g_netstatus_wnd=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_NET),NULL,Net_DlgProc);
			g_xferwnd=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_XFERS),NULL,Xfers_DlgProc);

			LoadNetQ();

			toolWindowSet(g_config->ReadInt(CONFIG_toolwindow,CONFIG_toolwindow_DEFAULT));

			if (g_config->ReadInt(CONFIG_net_vis,CONFIG_net_vis_DEFAULT))
				SendMessage(hwndDlg,WM_COMMAND,IDC_NETSETTINGS,0);
			if (g_config->ReadInt(CONFIG_xfers_vis,CONFIG_xfers_vis_DEFAULT))
				SendMessage(hwndDlg,WM_COMMAND,ID_VIEW_TRANSFERS,0);
			if (g_config->ReadInt(CONFIG_search_vis,CONFIG_search_vis_DEFAULT))
				SendMessage(hwndDlg,WM_COMMAND,IDC_SEARCH,0);

			SetForegroundWindow(hwndDlg);

			if (g_config->ReadInt(CONFIG_aot,CONFIG_aot_DEFAULT)) {
				SetWindowPos(hwndDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
				CheckMenuItem(GetMenu(hwndDlg),ID_VIEW_ALWAYSONTOP,MF_CHECKED|MF_BYCOMMAND);
			};
			TreeView_SetTextColor(GetDlgItem(g_mainwnd,IDC_USERS),g_config->ReadInt(CONFIG_mul_color,CONFIG_mul_color_DEFAULT));
			TreeView_SetTextColor(GetDlgItem(g_mainwnd,IDC_CHATROOMS),g_config->ReadInt(CONFIG_mul_color,CONFIG_mul_color_DEFAULT));
			TreeView_SetBkColor(GetDlgItem(g_mainwnd,IDC_USERS),g_config->ReadInt(CONFIG_mul_bgc,CONFIG_mul_bgc_DEFAULT));
			TreeView_SetBkColor(GetDlgItem(g_mainwnd,IDC_CHATROOMS),g_config->ReadInt(CONFIG_mul_bgc,CONFIG_mul_bgc_DEFAULT));

			CreateTooltip(GetDlgItem(hwndDlg,IDC_CREATECHATROOM),"Create/join chat");
			SendDlgItemMessage(hwndDlg,IDC_CREATECHATROOM,BM_SETIMAGE,IMAGE_ICON,
				(LPARAM)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_CHAT),IMAGE_ICON,16,16,0));

			CreateTooltip(GetDlgItem(hwndDlg,IDC_NETSETTINGS),"Open network status");
			SendDlgItemMessage(hwndDlg,IDC_NETSETTINGS,BM_SETIMAGE,IMAGE_ICON,
				(LPARAM)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_CONNECT),IMAGE_ICON,16,16,0));

			CreateTooltip(GetDlgItem(hwndDlg,IDC_SEARCH),"Open browser");

			SendDlgItemMessage(hwndDlg,IDC_SEARCH,BM_SETIMAGE,IMAGE_ICON,
				(LPARAM)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_SEARCH),IMAGE_ICON,16,16,0));

			SetTimer(hwndDlg,1,5,NULL);
			return 0;
		};
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case ID_FILE_QUIT:
				{
					DestroyWindow(hwndDlg);
					break;
				};
			case ID_VIEW_MAINWND:
				{
					WINDOWPLACEMENT wp={sizeof(wp),};
					GetWindowPlacement(hwndDlg,&wp);
					if (wp.showCmd == SW_SHOWMINIMIZED)
						ShowWindow(hwndDlg,SW_RESTORE);
					else
						ShowWindow(hwndDlg,SW_SHOW);
					SetForegroundWindow(hwndDlg);
					break;
				};
			case IDC_NETSETTINGS:
			case ID_VIEW_NETWORKCONNECTIONS:
			case ID_VIEW_NETWORKCONNECTIONS2:
				{
					CheckMenuItem(GetMenu(hwndDlg),ID_VIEW_NETWORKCONNECTIONS,MF_CHECKED|MF_BYCOMMAND);
					{
						WINDOWPLACEMENT wp={sizeof(wp),};
						GetWindowPlacement(g_netstatus_wnd,&wp);
						if (wp.showCmd == SW_SHOWMINIMIZED)
							ShowWindow(g_netstatus_wnd,SW_RESTORE);
						else
							ShowWindow(g_netstatus_wnd,SW_SHOW);
					};
					SetForegroundWindow(g_netstatus_wnd);
					g_config->WriteInt(CONFIG_net_vis,1);
					break;
				};
			case ID_VIEW_TRANSFERS:
			case ID_VIEW_TRANSFERS2:
				{
					CheckMenuItem(GetMenu(hwndDlg),ID_VIEW_TRANSFERS,MF_CHECKED|MF_BYCOMMAND);
					{
						WINDOWPLACEMENT wp={sizeof(wp),};
						GetWindowPlacement(g_xferwnd,&wp);
						if (wp.showCmd == SW_SHOWMINIMIZED)
							ShowWindow(g_xferwnd,SW_RESTORE);
						else
							ShowWindow(g_xferwnd,SW_SHOW);
					};
					SetForegroundWindow(g_xferwnd);
					g_config->WriteInt(CONFIG_xfers_vis,1);
					break;
				};
			case ID_VIEW_ALWAYSONTOP2:
			case ID_VIEW_ALWAYSONTOP:
				{
					int newaot=!g_config->ReadInt(CONFIG_aot,CONFIG_aot_DEFAULT);
					g_config->WriteInt(CONFIG_aot,newaot);
					if (newaot) {
						SetWindowPos(hwndDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
						CheckMenuItem(GetMenu(hwndDlg),ID_VIEW_ALWAYSONTOP,MF_CHECKED|MF_BYCOMMAND);
					}
					else {
						SetWindowPos(hwndDlg,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
						CheckMenuItem(GetMenu(hwndDlg),ID_VIEW_ALWAYSONTOP,MF_UNCHECKED|MF_BYCOMMAND);
					};
					break;
				};
			case ID_VIEW_PREFERENCES:
			case ID_VIEW_PREFERENCES2:
				{
					if (prefs_hwnd) {
						ShowWindow(prefs_hwnd,SW_SHOW);
						SetForegroundWindow(prefs_hwnd);
					}
					else {
						prefs_hwnd=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_PREFS),g_mainwnd,PrefsOuterProc);
						CheckMenuItem(GetMenu(hwndDlg),ID_VIEW_PREFERENCES,MF_CHECKED|MF_BYCOMMAND);
						ShowWindow(prefs_hwnd,SW_SHOW);
					};
					break;
				};
			case IDC_SEARCH:
			case IDC_SEARCH2:
				{
					if (!g_search_wnd) g_search_wnd=CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_SEARCH),NULL,Search_DlgProc);{
						WINDOWPLACEMENT wp={sizeof(wp),};
						GetWindowPlacement(g_search_wnd,&wp);
						if (wp.showCmd == SW_SHOWMINIMIZED)
							ShowWindow(g_search_wnd,SW_RESTORE);
						else
							ShowWindow(g_search_wnd,SW_SHOW);
					};
					SetForegroundWindow(g_search_wnd);
					CheckMenuItem(GetMenu(hwndDlg),IDC_SEARCH,MF_CHECKED|MF_BYCOMMAND);
					g_config->WriteInt(CONFIG_search_vis,1);
					break;
				};
			case IDC_CREATECHATROOM:
			case IDC_CREATECHATROOM2:
				{
					char text[64];
					text[0]=0;
					HTREEITEM h=TreeView_GetSelection(GetDlgItem(hwndDlg,IDC_CHATROOMS));
					if (h) {
						TVITEM i;
						i.mask=TVIF_TEXT|TVIF_HANDLE;
						i.hItem=h;
						i.pszText=text;
						i.cchTextMax=sizeof(text);
						TreeView_GetItem(GetDlgItem(hwndDlg,IDC_CHATROOMS),&i);
						if (i.pszText != text)
							safe_strncpy(text,i.pszText,sizeof(text));
					};
					DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_CHATROOMCREATE),hwndDlg,CreateChat_DlgProc,(LPARAM)text);
					break;
				};
			case CHATMENU_SHOWALL:
			case CHATMENU_HIDEALL:
				{
					chatroom_item *p=L_Chatroom;
					while (p) {
						if (LOWORD(wParam) == CHATMENU_HIDEALL) {
							ShowWindow(p->hwnd,SW_HIDE);
						}
						else {
							WINDOWPLACEMENT wp={sizeof(wp),};
							GetWindowPlacement(p->hwnd,&wp);
							if (wp.showCmd == SW_SHOWMINIMIZED) ShowWindow(p->hwnd,SW_RESTORE);
							if (!IsWindowVisible(p->hwnd)) ShowWindow(p->hwnd,SW_SHOW);
							SetForegroundWindow(p->hwnd);
						};
						p=p->next;
					};
					break;
				};
			default:
				{
					if (LOWORD(wParam) >= CHATMENU_BASE && LOWORD(wParam) < CHATMENU_MAX) {
						int n=LOWORD(wParam)-CHATMENU_BASE;
						chatroom_item *p=L_Chatroom;
						while (n-- && p) {
							p=p->next;
						};
						if (p) {
							WINDOWPLACEMENT wp={sizeof(wp),};
							GetWindowPlacement(p->hwnd,&wp);
							if (wp.showCmd == SW_SHOWMINIMIZED)
								ShowWindow(p->hwnd,SW_RESTORE);
							else if (!IsWindowVisible(p->hwnd)) ShowWindow(p->hwnd,SW_SHOW);
							SetForegroundWindow(p->hwnd);
						};
					};
					break;
				};
			};
			return 0;
		};
	case WM_ENDSESSION:
	case WM_DESTROY:
		{
			SaveNetQ();
			SaveDbToDisk();
			if (g_netstatus_wnd) DestroyWindow(g_netstatus_wnd);
			if (g_xferwnd) DestroyWindow(g_xferwnd);
			if (g_search_wnd) DestroyWindow(g_search_wnd);
			if (systray_state) systray_del(hwndDlg);
			PostQuitMessage(0);
			return 0;
		};
	case WM_NOTIFY:
		{
			LPNMHDR p=(LPNMHDR) lParam;
			if (p->code == NM_RCLICK) {
				if (p->idFrom == IDC_USERS) {
					UserListContextMenu(p->hwndFrom);
				};
			};
			if (p->code == NM_DBLCLK) {
				if (p->idFrom == IDC_CHATROOMS || p->idFrom == IDC_USERS) {
					HWND htree=p->hwndFrom;
					HTREEITEM h=TreeView_GetSelection(htree);
					if (h) {
						char text[256];
						TVITEM i;
						i.mask=TVIF_TEXT|TVIF_HANDLE;
						i.hItem=h;
						i.pszText=text;
						i.cchTextMax=sizeof(text);
						text[0]=0;
						TreeView_GetItem(htree,&i);
						if (text[0]) {
							chat_ShowRoom(text,2);
							return 1;
						};
					};
				};
			};
			return 0;
		};
	case WM_CLOSE:
		{
			if (!g_config->ReadInt(CONFIG_confirmquit,CONFIG_confirmquit_DEFAULT) || MessageBox(hwndDlg,"Closing this window will quit " APP_NAME " and disconnect you from the private network.\n"
				"Are you sure you want to do this?",
				"Close " APP_NAME,MB_YESNO) == IDYES) DestroyWindow(hwndDlg);
			return 0;
		};
	case WM_TIMER:
		{
			if (wParam == 5) { //update user list
				//check treeviews for any nicks/channels that we haven't seen in 4 minutes.
				main_onGotNick(NULL,0);
				main_onGotChannel(NULL);
			};

			if (wParam==1) {
				static int upcnt;
				if (uploadPrompts.GetSize() && !upcnt) {
					upcnt=1;
					C_UploadRequest *t=uploadPrompts.Get(0);
					uploadPrompts.Del(0);
					char buf[1024];
					char str[64];
					MakeID128Str(t->get_guid(),str);
					sprintf(str+strlen(str),":%d",t->get_idx());
					if (strlen(t->get_fn()) > 768)
						t->get_fn()[768]=0;
					int f=MB_YESNO;
					char sizebuf[64];
					sizebuf[0]=0;
					int fs_l,fs_h;
					t->get_fsize(&fs_l,&fs_h);
					if (fs_l != -1 || fs_h != -1) {
						strcpy(sizebuf," (");
						FormatSizeStr64(sizebuf+2,fs_l,fs_h);
						strcat(sizebuf,")");
					};
					if (uploadPrompts.GetSize() > 1) {
						f=MB_YESNOCANCEL;
						sprintf(buf,"Accept upload of file named '%s'%s from user '%s'?\r\n(Cancel will remove all %d pending uploads)",t->get_fn(),sizebuf,t->get_nick(),uploadPrompts.GetSize()-1);
					}
					else {
						sprintf(buf,"Accept upload of file named '%s'%s from user '%s'?",t->get_fn(),sizebuf,t->get_nick());
					};

					f=MessageBox(NULL,buf,APP_NAME " - Accept Upload?",f|MB_TOPMOST|MB_ICONQUESTION);
					if (f==IDYES) main_handleUpload(str,t->get_fn(),t);
					else if (f == IDCANCEL) {
						while (uploadPrompts.GetSize()>0) {
							delete uploadPrompts.Get(0);
							uploadPrompts.Del(0);
						};
					};

					delete t;
					upcnt=0;
				};
				static int kdcnt;
				if (keydistPrompts.GetSize() && !kdcnt) {
					kdcnt=1;
					C_KeydistRequest *t=keydistPrompts.Get(0);
					keydistPrompts.Del(0);

					if (!findPublicKeyFromKey(t->get_key())) {
						char buf[1024];
						char sign[SHA_OUTSIZE*2+1];
						unsigned char hash[SHA_OUTSIZE];
						SHAify m;
						m.add((unsigned char *)t->get_key()->modulus,MAX_RSA_MODULUS_LEN);
						m.add((unsigned char *)t->get_key()->exponent,MAX_RSA_MODULUS_LEN);
						m.final(hash);
						Bin2Hex(sign,hash,SHA_OUTSIZE);

						sprintf(buf,"Authorize public key with signature '%s' from user '%s'?\r\n(Cancel will keep this key and any remaining prompts in the pending key list)",sign,t->get_nick());

						int f=MessageBox(NULL,buf,APP_NAME " - Accept Public Key?",MB_YESNOCANCEL|MB_TOPMOST|MB_ICONQUESTION);

						if (f==IDYES) main_handleKeyDist(t,0);
						else if (f == IDCANCEL) {
							main_handleKeyDist(t,1);
							while (keydistPrompts.GetSize()>0) {
								main_handleKeyDist(keydistPrompts.Get(0),1);
								delete keydistPrompts.Get(0);
								keydistPrompts.Del(0);
							};
						};
					};
					delete t;
					kdcnt=0;
				};

				static int in_timer;
				if (!in_timer) {
					in_timer=1;

					DoMainLoop();

					in_timer=0;
				};
			};
			return 0;
		};
	case WM_USER_SYSTRAY:
		{
			switch (LOWORD(lParam))
			{
			case WM_RBUTTONUP:
				{
					HMENU hMenu=GetSubMenu(g_context_menus,5);
					POINT p;
					GetCursorPos(&p);
					SetForegroundWindow(hwndDlg);

					MakeChatSubMenu(GetSubMenu(hMenu,5));

					CheckMenuItem(hMenu,ID_VIEW_NETWORKCONNECTIONS,MF_BYCOMMAND |
						(IsWindowVisible(g_netstatus_wnd)?MF_CHECKED:MF_UNCHECKED));
					CheckMenuItem(hMenu,IDC_SEARCH,MF_BYCOMMAND |
						(IsWindowVisible(g_search_wnd)?MF_CHECKED:MF_UNCHECKED));
					CheckMenuItem(hMenu,ID_VIEW_TRANSFERS,MF_BYCOMMAND |
						(IsWindowVisible(g_xferwnd)?MF_CHECKED:MF_UNCHECKED));
					CheckMenuItem(hMenu,ID_VIEW_PREFERENCES,MF_BYCOMMAND |
						(prefs_hwnd?MF_CHECKED:MF_UNCHECKED));

					TrackPopupMenu(hMenu,TPM_RIGHTBUTTON|TPM_LEFTBUTTON,p.x,p.y,0,hwndDlg,NULL);
					return 0;
				};
			case WM_LBUTTONDOWN:
				{
					SendMessage(hwndDlg,WM_COMMAND,ID_VIEW_MAINWND,0);
					return 0;
				};
			};
			return 0;
		};
	case WM_COPYDATA:
		{
			if (((COPYDATASTRUCT *)lParam)->dwData == 0xB3EF) {
				handleWasteURL((char*)((COPYDATASTRUCT *)lParam)->lpData);
			}
			else if (((COPYDATASTRUCT *)lParam)->dwData == 0xF00D) {
				m_isinst=!stricmp((char*)((COPYDATASTRUCT *)lParam)->lpData,g_config_prefix);
			};
			return TRUE;
		};
	case WM_USER_PROFILECHECK:
		{
			SetWindowLong(hwndDlg,DWL_MSGRESULT,m_isinst);
			return TRUE;
		};
	case WM_MOVE:
		{
			handleDialogSizeMsgs(hwndDlg,uMsg,wParam,lParam,&sizeinf);
			break;
		};
	case WM_SIZE:
		{
			if (wParam==SIZE_MINIMIZED) {
				if (toolwnd_state || (GetAsyncKeyState(VK_SHIFT)&0x8000) || g_config->ReadInt(CONFIG_systray_hide,CONFIG_systray_hide_DEFAULT)) {
					if (!systray_state) systray_add(hwndDlg,g_hSmallIcon);
					ShowWindow(hwndDlg,SW_HIDE);
				};
				if (g_config->ReadInt(CONFIG_hideallonmin,CONFIG_hideallonmin_DEFAULT) && !g_hidewnd_state) {
					g_hidewnd_state=1;
					chatroom_item *p=L_Chatroom;
					while(p!=NULL) {
						ShowWindow(p->hwnd,SW_HIDE);
						p=p->next;
					};
					if (g_search_wnd && IsWindowVisible(g_search_wnd)) {
						ShowWindow(g_search_wnd,SW_HIDE);
						g_hidewnd_state|=2;
					};
					if (IsWindowVisible(g_netstatus_wnd)) {
						ShowWindow(g_netstatus_wnd,SW_HIDE);
						g_hidewnd_state|=4;
					};
					if (IsWindowVisible(g_xferwnd)) {
						ShowWindow(g_xferwnd,SW_HIDE);
						g_hidewnd_state|=8;
					};
				};
			}
			else {
				if (systray_state && !g_config->ReadInt(CONFIG_systray,CONFIG_systray_DEFAULT))
					systray_del(hwndDlg);
				childresize_resize(hwndDlg,mainwnd_rlist,sizeof(mainwnd_rlist)/sizeof(mainwnd_rlist[0]));
				if (g_hidewnd_state) {
					chatroom_item *p=L_Chatroom;
					while(p!=NULL) {
						ShowWindow(p->hwnd,SW_SHOWNA);
						p=p->next;
					};
					if (g_hidewnd_state&2) { //browser
						if (g_search_wnd) ShowWindow(g_search_wnd,SW_SHOWNA);
					};
					if (g_hidewnd_state&4) { //network
						ShowWindow(g_netstatus_wnd,SW_SHOWNA);
					};
					if (g_hidewnd_state&8) { //transfers
						ShowWindow(g_xferwnd,SW_SHOWNA);
					};
					g_hidewnd_state=0;
				};
			};
			handleDialogSizeMsgs(hwndDlg,uMsg,wParam,lParam,&sizeinf);
			return 0;
		};
	};
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR lpszCmdParam, int /*nShowCmd*/)
{
	const char *mainwndclassname=APP_NAME "mainwnd";
	SetProgramDirectory();

	#if 0
		#ifndef _DEBUG
			#error remove 1
		#endif
		//HACK md5chap
		int tmpf=_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmpf|=_CRTDBG_LEAK_CHECK_DF;
		tmpf|=_CRTDBG_DELAY_FREE_MEM_DF;
		_CrtSetDbgFlag(tmpf);
	#endif

	int x;
	g_hInst=hInstance;
	InitCommonControls();
	CoInitialize(0);
	{ //load richedit DLL
		WNDCLASS wc={0,};
		if (!LoadLibrary("RichEd20.dll")) LoadLibrary("RichEd32.dll");

		//make richedit20a point to RICHEDIT
		if (!GetClassInfo(NULL,"RichEdit20A",&wc)) {
			GetClassInfo(NULL,"RICHEDIT",&wc);
			wc.lpszClassName = "RichEdit20A";
			RegisterClass(&wc);
		};
	};

	g_hSmallIcon = (HICON)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON,16,16,0);

	{
		WNDCLASS wc={0,};
		GetClassInfo(NULL,"#32770",&wc);
		wc.hIcon=LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_ICON1));
		RegisterClass(&wc);
		wc.lpszClassName = mainwndclassname;
		RegisterClass(&wc);
		wc.lpszClassName = APP_NAME "wnd";
		RegisterClass(&wc);

		wc.hIcon = LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_CHAT));
		wc.lpszClassName = APP_NAME "chatwnd";
		RegisterClass(&wc);

		wc.hIcon = LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_BROWSE));
		wc.lpszClassName = APP_NAME "searchwnd";
		RegisterClass(&wc);

		wc.hIcon = LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_XFERS));
		wc.lpszClassName = APP_NAME "xferwnd";
		RegisterClass(&wc);

		wc.hIcon = LoadIcon(g_hInst,MAKEINTRESOURCE(IDI_CONNECT));
		wc.lpszClassName = APP_NAME "netwnd";
		RegisterClass(&wc);
	};

	char *wasteOpen=NULL;
	int force_profiledlg=0;
	for (;;) {
		while (*lpszCmdParam == ' ') lpszCmdParam++;
		if (!strnicmp(lpszCmdParam,"/profile=",9)) {
			if (lpszCmdParam[9] && lpszCmdParam[9] != ' ') {
				char *p;
				if (lpszCmdParam[9] == '"') {
					safe_strncpy(g_profile_name,lpszCmdParam+10,sizeof(g_profile_name));
					if ((p=strstr(g_profile_name,"\""))!=0) *p=0;
					lpszCmdParam+=10;
					while (*lpszCmdParam != '"' && *lpszCmdParam) lpszCmdParam++;
					if (*lpszCmdParam) lpszCmdParam++;
				}
				else {
					safe_strncpy(g_profile_name,lpszCmdParam+9,sizeof(g_profile_name));
					if ((p=strstr(g_profile_name," "))!=0) *p=0;
					lpszCmdParam+=9+strlen(g_profile_name);
				};
				if (!g_profile_name[0]) force_profiledlg=1;
			}
			else {
				force_profiledlg=1;
				break;
			};
		}
		else if (*lpszCmdParam) {
			char scchar=' ';
			if (*lpszCmdParam == '\"') scchar=*lpszCmdParam;
			if (!strnicmp(lpszCmdParam+(scchar != ' '),"waste:",6) && !wasteOpen) {
				if (scchar != ' ') lpszCmdParam++;

				wasteOpen=strdup(lpszCmdParam);
				char *t=wasteOpen;
				while (*t != scchar && *t) t++;
				*t=0;

				lpszCmdParam+=6;
				while (*lpszCmdParam != scchar && *lpszCmdParam) lpszCmdParam++;
			}
			else break;
		}
		else break;
	};

	MYSRAND();

	if (Prefs_SelectProfile(force_profiledlg)) return 1;

	strcat(g_config_prefix,g_profile_name);

	HWND wndpos=NULL;
	while ((wndpos=FindWindowEx(NULL,wndpos,mainwndclassname,NULL))!=0) {
		COPYDATASTRUCT cds;
		cds.cbData=strlen(g_config_prefix)+1;
		cds.lpData=(PVOID)g_config_prefix;
		cds.dwData=0xF00D;
		SendMessage(wndpos,WM_COPYDATA,NULL,(LPARAM)&cds);
		if (SendMessage(wndpos,WM_USER_PROFILECHECK,0,0)) {
			WINDOWPLACEMENT wp={sizeof(wp),};
			GetWindowPlacement(wndpos,&wp);
			if (wp.showCmd == SW_SHOWMINIMIZED)  ShowWindow(wndpos,SW_RESTORE);
			else ShowWindow(wndpos,SW_SHOW);
			SetForegroundWindow(wndpos);

			if (wasteOpen) {
				cds.cbData=strlen(wasteOpen)+1;
				cds.lpData=(PVOID)wasteOpen;
				cds.dwData=0xB3EF;
				SendMessage(wndpos,WM_COPYDATA,NULL,(LPARAM)&cds);
				free(wasteOpen);
			};

			return 1;
		};
	};

	char tmp[1024+8];

	sprintf(tmp,"WASTEINSTANCE_%s",g_profile_name);
	tmp[MAX_PATH-1]=0;
	CreateSemaphore(NULL,0,1,tmp);
	if (GetLastError() == ERROR_ALREADY_EXISTS) return 1;

	//PIN

	UnifiedReadConfig();

	InitialLoadDb();

	if (g_log_level>0) {
		log_UpdatePath(g_config->ReadString(CONFIG_logpath,""));
	};

	PrepareDownloadDirectory();

	int need_setup=g_config->ReadInt(CONFIG_valid,0)<5;
	if (need_setup) {
		if (!DialogBox(hInstance,MAKEINTRESOURCE(IDD_SETUPWIZ),NULL,SetupWizProc)) {
			delete g_config;
			memset(&g_key,0,sizeof(g_key));
			return 1;
		};
		g_config->WriteInt(CONFIG_valid,5);
		g_config->Flush();
	};

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
		memset(&g_key,0,sizeof(g_key));
		MessageBox(NULL,"Error initializing Winsock\n",APP_NAME " Error",0);
		return 1;
	};

	if (!g_key.bits) {
		reloadKey(
			g_config->ReadInt(CONFIG_storepass,CONFIG_storepass_DEFAULT)?
			g_config->ReadString(CONFIG_keypass,CONFIG_keypass_DEFAULT):
			NULL,
			GetDesktopWindow()
			);
	};

	InitializeNetworkparts();

	//oops gotta do this here.
	for (x = 0; x < SEARCHCACHE_NUMITEMS; x ++) g_searchcache[x]=new SearchCacheItem;

	g_context_menus=LoadMenu(g_hInst,MAKEINTRESOURCE(IDR_CONTEXTMENUS));
	HACCEL hAccel=LoadAccelerators(g_hInst,MAKEINTRESOURCE(IDR_ACCEL1));

	WM_TASKBARCREATED = RegisterWindowMessage("TaskbarCreated");

	CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_MAIN),GetDesktopWindow(),Main_DlgProc);

	if (wasteOpen) {
		handleWasteURL(wasteOpen);
		free(wasteOpen);
	};

	RunPerforms();

	MSG msg;
	while (GetMessage(&msg,NULL,0,0)) {
		if (!TranslateAccelerator(g_mainwnd,hAccel,&msg) &&
			!IsDialogMessage(g_mainwnd,&msg) &&
			!IsDialogMessage(g_netstatus_wnd,&msg) &&
			(!g_search_wnd || !IsDialogMessage(g_search_wnd,&msg)) &&
			!IsDialogMessage(g_xferwnd,&msg) &&
			(!g_xfer_subwnd_active || !IsDialogMessage(g_xfer_subwnd_active,&msg)) &&
			(!prefs_hwnd || !IsDialogMessage(prefs_hwnd,&msg)) &&
			(!prefs_cur_wnd || !IsDialogMessage(prefs_cur_wnd,&msg)) &&
			!IsChatRoomDlgMessage(&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		};
	};

	delete g_listen;
	delete g_dns;
	if (g_newdatabase != g_database) delete g_newdatabase;
	delete g_database;

	for (x = 0; x < g_recvs.GetSize(); x ++) delete g_recvs.Get(x);
	for (x = 0; x < g_sends.GetSize(); x ++) delete g_sends.Get(x);
	for (x = 0; x < g_new_net.GetSize(); x ++) delete g_new_net.Get(x);
	for (x = 0; x < g_uploads.GetSize(); x ++) free(g_uploads.Get(x));
	if (g_aclist) free(g_aclist);
	KillPkList();
	for (x = 0; x < SEARCHCACHE_NUMITEMS; x ++) delete g_searchcache[x];
	KillDirgetlist();
	KillSearchhistory();

	CloseAllChatWindows();

	delete g_mql;

	delete g_config;
	memset(&g_key,0,sizeof(g_key));

	DestroyMenu(g_context_menus);
	DestroyIcon(g_hSmallIcon);
	WSACleanup();
	log_UpdatePath(NULL);
	return 0;
}
