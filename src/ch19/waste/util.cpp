/*
WASTE - util.cpp (General utility code)
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
#include "netkern.hpp"
#include "util.hpp"
#include "rsa/md5.hpp"
#include "rsa/r_random.hpp"

#if (defined(_WIN32) && defined(_CHECK_RSA_BLINDING))
	#include "rsa/nn.hpp"
	#include "rsa/rsa.hpp"
#endif

#ifdef _DEFINE_SRV
	#include "resourcesrv.hpp"
#else
	#include "resource.hpp"
#endif

//global strings

extern const char szDotWastestate[]=".WASTESTATE";
extern const char szWastestate[]   ="WASTESTATE";

//////guid

void CreateID128(T_GUID *id)
{
	R_GenerateBytes((unsigned char *)id->idc, 16, &g_random);
}

void MakeID128Str(T_GUID *id, char *str)
{
	Bin2Hex(str,id->idc,sizeof(id->idc));
}

#ifdef _DEBUG
	char* dbgstrdup(const char*str,const char *file, unsigned int line)
	{
		if (!str) return NULL;
		int len=strlen(str);
		char *dest=(char*)_malloc_dbg(len+1, _NORMAL_BLOCK, file, line);
		safe_strncpy(dest,str,len+1);
		return dest;
	}
#endif

int MakeID128FromStr(const char *str, T_GUID *id)
{
	int x;
	const char *p=str;
	for (x = 0; x < 16; x ++) {
		int h=*p++;
		int l=*p++;
		if (l >= '0' && l <= '9') l-='0';
		else if (l >= 'A' && l <= 'F') l -= 'A'-10;
		else return 1;
		if (h >= '0' && h <= '9') h-='0';
		else if (h >= 'A' && h <= 'F') h -= 'A'-10;
		else return 1;
		id->idc[x]=(unsigned char)((l|(h<<4))&0xff);
	};
	return 0;
}

inline void Bin2Hex_Single(char* buf,char inp)
{
	char c1,c2;
	c1=((inp>>0)&0xf)+'0';if (c1>'9') c1=c1-'9'-1+'A';
	c2=((inp>>4)&0xf)+'0';if (c2>'9') c2=c2-'9'-1+'A';
	buf[1]=c1;buf[0]=c2;
}

char* Bin2Hex(char* output, unsigned char* input, int len)
{
	while (len>0) {
		Bin2Hex_Single(output,*input);
		input+=1;
		output+=2;
		len--;
	};
	*output=0;
	return output;
}

char* Bin2Hex_Lf(char* output, unsigned char* input, int len, int &perline, int maxperline, bool wantCrLf)
{
	if (maxperline==0) maxperline=1;
	while (len>0) {
		if (perline>=maxperline) {
			perline=0;
			#ifdef _WIN32
			if (wantCrLf) *output++='\r';
			#endif
			*output++='\n';
		};
		Bin2Hex_Single(output,*input);
		input+=1;
		output+=2;
		len--;
		perline++;
	};
	*output=0;
	return output;
}

///rng

R_RANDOM_STRUCT g_random;

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
static WNDPROC rng_oldWndProc;
static unsigned int rng_movebuf[7];
static int rng_movebuf_cnt;

static BOOL CALLBACK rng_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_MOUSEMOVE) {
		rng_movebuf[rng_movebuf_cnt%7]+=lParam;
		rng_movebuf[(rng_movebuf_cnt+1)%7]+=GetTickCount();
		rng_movebuf[(rng_movebuf_cnt+2)%7]+=GetMessageTime()+GetMessagePos();
		if (++rng_movebuf_cnt >= 53) {
			rng_movebuf_cnt=0;
			R_RandomUpdate(&g_random,(unsigned char *)rng_movebuf,sizeof(rng_movebuf));

			unsigned int bytesNeeded;
			R_GetRandomBytesNeeded(&bytesNeeded, &g_random);
			SendDlgItemMessage(hwndDlg,IDC_PROGRESS_RNG,PBM_SETPOS,(WPARAM)(64-bytesNeeded/4),0);
			if (bytesNeeded<1) EndDialog(hwndDlg,1);
		};
	};
	return CallWindowProc(rng_oldWndProc,hwndDlg,uMsg,wParam,lParam);
}

static BOOL WINAPI RndProc(HWND hwndDlg, UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (uMsg == WM_INITDIALOG) {
		SetWindowText(hwndDlg,APP_NAME " Random Number Generator Initialization");
		ShowWindow(GetDlgItem(hwndDlg,IDC_PROGRESS_RNG),SW_SHOWNA);
		SendDlgItemMessage(hwndDlg,IDC_PROGRESS_RNG,PBM_SETRANGE,0,MAKELPARAM(0,64));
		rng_oldWndProc=(WNDPROC) SetWindowLong(hwndDlg,GWL_WNDPROC,(LONG)rng_newWndProc);
	};
	return 0;
}

#endif//WIN32

void MYSRANDUPDATE(unsigned char *buf, int bufl)
{
	static int last_srtime;
	int t=GetTickCount()-last_srtime;
	last_srtime=GetTickCount();

	if (buf&&bufl) R_RandomUpdate(&g_random,buf,bufl);
	R_RandomUpdate(&g_random, (unsigned char*)&t, sizeof(t));

	//ADDED Md5Chap removed that nonworking code
}

void MYSRAND()
{
	R_RandomInit(&g_random);
	#ifdef _WIN32
		if (!GetPrivateProfileStruct("config","rngseedn",g_random.state,16,g_config_mainini)) {
			#ifndef _DEFINE_SRV
				DialogBox(g_hInst,MAKEINTRESOURCE(IDD_RNGINIT),NULL,RndProc);
				rng_movebuf_cnt=0;
				memset(rng_movebuf,0,sizeof(rng_movebuf));
			#else
				log_printf(ds_Console,"CRITICAL WARNING! You need to run wastesrv configs at least once with WASTE. \"%s\" needs rngseedn value for security reasons!!! Exiting!",g_config_mainini);
				g_exit=1;
				return;
			#endif
		}
		else {
			g_random.outputAvailable=0;
			g_random.bytesNeeded=0;
		};
	#else
		log_printf(ds_Console,"initializing RNG");
		FILE *fp=fopen("/dev/urandom","rb");
		if (fp) {
			fread(g_random.state,32,1,fp);
			fclose(fp);
		};
		g_random.outputAvailable=0;
		g_random.bytesNeeded=0;
		log_printf(ds_Console,"done");
	#endif

	MYSRANDUPDATE();

	unsigned char tmp[16];

	R_GenerateBytes(tmp,sizeof(tmp), &g_random);

	#ifdef _WIN32
		WritePrivateProfileStruct("config","rngseedn",tmp,16,g_config_mainini);
	#endif
}

////misc string shit

char *extension(char *fn)
{
	char *s = fn+strlen(fn);
	while (s > fn && *s != '.'
#ifdef _WIN32
		&& *s != '\\'
#endif
		&& *s != '/') s=CharPrev(fn,s);
	if (s == fn || *s != '.') return "";
	return (s+1);
}

void removeInvalidFNChars(char *filename)
{
	char *p=filename;
	while (*p) {
		if (*p == '?' || *p == '*' || *p == '>' || *p == '<' || *p == '|' || *p == ':') *p='_';
		p=CharNext(p);
	};
}

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
////windows shit

int toolwnd_state;
int systray_state;

BOOL systray_add(HWND hwnd, HICON icon)
{
	NOTIFYICONDATA tnid;
	systray_state=1;
	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = hwnd;
	tnid.uID = 232;
	tnid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	tnid.uCallbackMessage = WM_USER_SYSTRAY;
	tnid.hIcon = icon;
	GetWindowText(g_mainwnd,tnid.szTip,sizeof(tnid.szTip));
	return (Shell_NotifyIcon(NIM_ADD, &tnid));
}

BOOL systray_del(HWND hwnd)
{
	NOTIFYICONDATA tnid;
	systray_state=0;
	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = hwnd;
	tnid.uID = 232;
	return(Shell_NotifyIcon(NIM_DELETE, &tnid));
}

/* BSC Comment for compat
struct FLASHWINFO
{
	UINT  cbSize;
	HWND  hwnd;
	DWORD dwFlags;
	UINT  uCount;
	DWORD dwTimeout;
}
typedef FLASHWINFO *PFLASHWINFO;
*/
//#define FLASHW_TRAY 2
//#define FLASHW_TIMERNOFG 12

static int (__stdcall *fflashWindowEx)(FLASHWINFO *);

void DoFlashWindow(HWND hwndParent, int timeoutval)
{
	static HMODULE hUser32=NULL;
	if (!hUser32) {
		hUser32=LoadLibrary("USER32.DLL");
		*((void**)&fflashWindowEx)=GetProcAddress(hUser32,"FlashWindowEx");
	};

	if (fflashWindowEx && GetForegroundWindow() != hwndParent && GetParent(GetForegroundWindow()) != hwndParent) {
		FLASHWINFO fi;
		fi.cbSize=sizeof(fi);
		fi.hwnd=hwndParent;
		fi.dwFlags=FLASHW_TRAY|FLASHW_TIMERNOFG;
		fi.uCount=timeoutval;
		fi.dwTimeout=0;
		fflashWindowEx(&fi);
	};
}

HWND CreateTooltip(HWND hWnd, LPSTR strTT)
{
	HWND hWndTT;                 //handle to the ToolTip control
	TOOLINFO ti;
	unsigned int uid = 0;       //for ti initialization
	LPTSTR lptstr = strTT;
	RECT rect;                  //for client area coordinates

	/* CREATE A TOOLTIP WINDOW */
	hWndTT = CreateWindowEx(
		NULL,
		TOOLTIPS_CLASS,
		NULL,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		hWnd,
		NULL,
		g_hInst,
		NULL
		);

	SetWindowPos(
		hWndTT,
		HWND_TOPMOST,
		0,
		0,
		0,
		0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	/* GET COORDINATES OF THE MAIN CLIENT AREA */
	GetClientRect(hWnd, &rect);

	/* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hWnd;
	ti.hinst = g_hInst;
	ti.uId = uid;
	ti.lpszText = lptstr;
	//ToolTip control will cover the whole window
	ti.rect.left = rect.left;
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	/* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
	if(!SendMessage(hWndTT, TTM_ADDTOOL, 0, (LPARAM)&ti)){
		#ifdef _DEBUG
			MessageBox(0,"DEBUG: Couldn't create the ToolTip control.","Error",MB_OK);
			return NULL;
		#endif
	};
	return hWndTT;
}

void toolWindowSet(int twstate)
{
	if (!toolwnd_state != !twstate) {
		toolwnd_state=twstate;
		int a=0;
		if (IsWindowVisible(g_mainwnd)) {
			ShowWindow(g_mainwnd,SW_HIDE);
			a++;
		};
		if (toolwnd_state) {
			SetWindowLong(g_mainwnd,GWL_EXSTYLE,(GetWindowLong(g_mainwnd,GWL_EXSTYLE)|WS_EX_TOOLWINDOW)&~WS_EX_APPWINDOW);
			//SetWindowLong(g_mainwnd,GWL_STYLE,GetWindowLong(g_mainwnd,GWL_STYLE)(WS_MINIMIZEBOX|WS_SYSMENU));
		}
		else {
			SetWindowLong(g_mainwnd,GWL_EXSTYLE,GetWindowLong(g_mainwnd,GWL_EXSTYLE)&~WS_EX_TOOLWINDOW);
			//SetWindowLong(g_mainwnd,GWL_STYLE,GetWindowLong(g_mainwnd,GWL_STYLE)|(WS_MINIMIZEBOX|WS_SYSMENU));
		};

		if (a) {
			ShowWindow(g_mainwnd,SW_SHOWNA);
			SetWindowPos(g_mainwnd,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DRAWFRAME);
		};
	};
}
#endif //WIN32
///access control shit

ACitem *g_aclist=0;
int g_aclist_size;

bool allowIP(unsigned long addr)
{
	if (!g_use_accesslist) return true;

	ACitem *p=g_aclist;
	int x;
	if (!p) return 1;
	for (x = 0; x < g_aclist_size; x ++) {
		if (IPv4TestIpInMask(addr,htonl(p->ip),IPv4NetMask(p->maskbits))) {
			return (p->allow!=0);
		};
		p++;
	};
	return true;
}

int ACStringToStruct(const char *t, ACitem *i)
{
	char buf[64];
	safe_strncpy(buf,t,64);
	t=buf;
	if (*t == 'A') i->allow=1;
	else if (*t == 'D') i->allow=0;
	else return 0;
	t++;
	char *p=strstr(t,"/");
	if (!p || !p[1]) return 0;
	i->maskbits=(char)(atoi(++p)&0xff);
	if (*p < '0' || *p > '9') return 0;

	p[-1]=0;
	i->ip=inet_addr(t);
	i->ip=ntohl(i->ip);
	#if 0
		dbg_printf(ds_Debug,"converted %s to %d:%08X/%d",t-1,i->allow,i->ip,i->maskbits);
	#endif
	return 1;
}

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	void updateACList(W_ListView *lv)
#else
	void updateACList(void *lv)
#endif
{
	free(g_aclist);
	if (lv) {
		#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
			g_aclist_size=lv->GetCount();
			g_aclist=(ACitem *)malloc(sizeof(ACitem)*g_aclist_size);
			g_config->WriteInt(CONFIG_ac_cnt,g_aclist_size);
			int x;
			int a=0;
			for (x = 0; x < g_aclist_size; x ++) {
				char b[512];
				lv->GetText(x,0,b,sizeof(b));
				lv->GetText(x,1,b+1,sizeof(b)-1);
				char nstr[32];
				sprintf(nstr,"ac_%d",x);
				g_config->WriteString(nstr,b);
				if (ACStringToStruct(b,g_aclist+a)) a++;
			};
			g_aclist_size=a;
		#endif//WIN32
	}
	else //read it from config
	{
		g_aclist_size=g_config->ReadInt(CONFIG_ac_cnt,CONFIG_ac_cnt_DEFAULT);
		g_aclist=(ACitem *)malloc(sizeof(ACitem)*g_aclist_size);
		int x;
		int a=0;
		for (x = 0; x < g_aclist_size; x ++) {
			char buf[64];
			sprintf(buf,"ac_%d",x);
			const char *t=g_config->ReadString(buf,"");
			if (ACStringToStruct(t,g_aclist+a)) a++;
		};
		g_aclist_size=a;
	};
}

////key shit

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))

static char tmp_passbuf[256];

BOOL WINAPI PassWordProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowText(hwndDlg,APP_NAME " : password");
			memset(tmp_passbuf,0,256);
			if (g_config->ReadInt(CONFIG_storepass,CONFIG_storepass_DEFAULT)) {
				if (g_config->ReadString(CONFIG_keypass,CONFIG_keypass_DEFAULT)) {
					safe_strncpy(tmp_passbuf,g_config->ReadString(CONFIG_keypass,CONFIG_keypass_DEFAULT),256);
				};
				CheckDlgButton(hwndDlg,IDC_CHECK_SAVE_PASSWD,BST_CHECKED);
			};
			SetDlgItemText(hwndDlg,IDC_EDIT_PASSWORD,tmp_passbuf);
			memset(tmp_passbuf,0,256);
			return 0;
		};
	case WM_CLOSE:
		{
			wParam=IDOK;
		};
	case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK) {
				GetDlgItemText(hwndDlg,IDC_EDIT_PASSWORD,tmp_passbuf,255);
				tmp_passbuf[255]=0;
				if (IsDlgButtonChecked(hwndDlg,IDC_CHECK_SAVE_PASSWD)) {
					g_config->WriteInt(CONFIG_storepass,1);
					g_config->WriteString(CONFIG_keypass,tmp_passbuf);
				}
				else {
					g_config->WriteInt(CONFIG_storepass,0);
					g_config->WriteString(CONFIG_keypass,"");
				};
				EndDialog(hwndDlg,0);
			};
			if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hwndDlg,1);
			};
			return 0;
		};
	};
	return 0;
}

#endif

static int readEncodedChar(FILE *in) //returns -1 on error
{
	char buf[2];
	int ret;
	do buf[0]=(char)(fgetc(in)&0xff);
	while (buf[0] == '\n' ||
		buf[0] == '\r' ||
		buf[0] == '\t' ||
		buf[0] == ' ' ||
		buf[0] == '-' ||
		buf[0] == '_' ||
		buf[0] == '>' ||
		buf[0] == '<' ||
		buf[0] == ':' ||
		buf[0] == '|'
		);
	buf[1]=(char)(fgetc(in)&0xff);

	if (buf[0] >= '0' && buf[0] <= '9') ret=(buf[0]-'0')<<4;
	else if (buf[0] >= 'A' && buf[0] <= 'F') ret=(buf[0]-'A'+10)<<4;
	else return -1;

	if (buf[1] >= '0' && buf[1] <= '9') ret+=(buf[1]-'0');
	else if (buf[1] >= 'A' && buf[1] <= 'F') ret+=(buf[1]-'A'+10);
	else return -1;

	return ret;
}

static int readBFdata(FILE *in, CBlowfish *bl, void *data, unsigned int len)
{
	unsigned int x;
	for (x = 0; x < len; x++) {
		int c=readEncodedChar(in);
		if (c<0) return 1;
		((unsigned char *)data)[x]=(unsigned char)c;
	};
	bl->DecryptCBC(data, len);
	return 0;
}

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
int doLoadKey(HWND hwndParent, const char *pstr, const char *keyfn, R_RSA_PRIVATE_KEY *key)
#else
int doLoadKey(const char *pstr, const char *keyfn, R_RSA_PRIVATE_KEY *key)
#endif
{
	int goagain=0;
	SHAify m;
	m.add((unsigned char *)pstr,strlen(pstr));
	unsigned char tmp[SHA_OUTSIZE];
	m.final(tmp);

	CBlowfish bl;
	//BLOWFISH_CTX bf;
	bl.Init(tmp,SHA_OUTSIZE);

	FILE *fp;
	if ((fp=fopen(keyfn,"rt"))==NULL) {
		key->bits=0;
#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		MessageBox(hwndParent,"Error loading private key",APP_NAME " Error",MB_OK|MB_ICONSTOP);
#else
		log_printf(ds_Console,APP_NAME " Error: Error loading private key");
#endif
		return 1;
	};

	char *err=NULL;
	char linebuf[1024];
	while (!err) {
		fgets(linebuf,1023,fp);
		linebuf[1023]=0;
		if (feof(fp) || !linebuf[0]) err="No private key found in file";
		if (!strncmp(linebuf,"WASTE_PRIVATE_KEY ",strlen("WASTE_PRIVATE_KEY "))) break;
		if (!strncmp(linebuf,"JSM_PRIVATE_KEY ",strlen("JSM_PRIVATE_KEY "))) break;
	};

	unsigned char tl[8]={0,};
	if (!err) {
		char *p=strstr(linebuf," ");
		while (p && *p == ' ') p++;
		if (p && atoi(p) >= 10 && atoi(p) < 20) {
			p=strstr(p," ");
			while (p && *p == ' ') p++;
			if (p && (key->bits=atoi(p)) <= MAX_RSA_MODULUS_BITS && key->bits >= MIN_RSA_MODULUS_BITS) {
				int x;
				for (x = 0; x < 8 && !err; x ++) {
					int c=readEncodedChar(fp);
					if (c < 0) err="Private key corrupt";
					else {
						tl[x]=(unsigned char)(c&0xff);
						//tl[x/4]|=c<<((x&3)*8);
					};
				};
				if (!err) {
					char buf[8];
					bl.SetIV(CBlowfish::IV_BOTH,(unsigned long*)tl);
					if (readBFdata(fp,&bl,buf,8)) {
						err="Private key corrupt";
					}
					else if (memcmp(buf,"PASSWORD",8)) {
						goagain++;
						err="Invalid password for private key";
					}
					else {
						int a=0;
						//HACK md5chap g_key->key
						#define WPK(x) if (!readBFdata(fp,&bl,key->x,sizeof(key->x)))
						WPK(modulus)
						WPK(publicExponent)
						WPK(exponent)
						WPK(prime)
						WPK(primeExponent)
						WPK(coefficient)
						a++;
						#undef WPK
						if (!a) err="Private key corrupt";
					};
				};
				//read key now
			}
			else
				err="Private key found but size incorrect";
		}
		else err="Private key found but version incorrect";
	};
	fclose(fp);

	if (err) {
		key->bits=0;
		#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
			MessageBox(hwndParent,err,APP_NAME " Error",MB_OK|MB_ICONSTOP);
		#else
			log_printf(ds_Console,APP_NAME " Error: %s",err);
		#endif
		return goagain?2:1;
	};
	return 0;
}

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	void reloadKey(const char *passstr, HWND hwndParent)
#else
	void reloadKey(const char *passstr)
#endif
{
	int retry=10;
	#if defined(_DEFINE_SRV)||(!defined(_WIN32))
		char tmp_passbuf[256];
	#endif

	char keyfn[1024];
	sprintf(keyfn,"%s.pr4",g_config_prefix);

	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		if (!passstr) {
	giveitanothergo:
			if (DialogBox(g_hInst,MAKEINTRESOURCE(IDD_PASSWD),hwndParent,PassWordProc)) {
				g_key.bits=0;
				memset(g_pubkeyhash,0,sizeof(g_pubkeyhash));
				return;
			};
			passstr=tmp_passbuf;
		};
	#else
		if (!passstr) {
	giveitanothergo:
			log_printf(ds_Console,"\nPassword: ");
			fgets(tmp_passbuf,sizeof(tmp_passbuf),stdin);
			if (tmp_passbuf[0] && tmp_passbuf[strlen(tmp_passbuf)-1] == '\n') {
				tmp_passbuf[strlen(tmp_passbuf)-1]=0;
			};
			passstr=tmp_passbuf;
		};
	#endif

	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		int ret=doLoadKey(hwndParent,passstr,keyfn,&g_key);
	#else
		int ret=doLoadKey(passstr,keyfn,&g_key);
	#endif
	if (ret) {
		if (ret == 2) {
			if (retry>0) {
				retry--;
				goto giveitanothergo;
			}
			else {
				g_key.bits=0;
				memset(g_pubkeyhash,0,sizeof(g_pubkeyhash));
				return;
			};
		};
		return;
	};

	SHAify m;

	m.add((unsigned char *)g_key.modulus,MAX_RSA_MODULUS_LEN);
	m.add((unsigned char *)g_key.publicExponent,MAX_RSA_MODULUS_LEN);
	m.final(g_pubkeyhash);

	#if 0
		int x;
		for (x=0; x < g_new_net.GetSize(); x ++) {
			C_Connection *cc=g_new_net.Get(x);
			cc->close(1);
		};
		if (g_mql) for (x = 0; x < g_mql->GetNumQueues(); x ++) {
			g_mql->GetQueue(x)->get_con()->close(1);
		};
	#endif
}

C_ItemList<PKitem> g_pklist,g_pklist_pending;

void KillPkList()
{
	int x=g_pklist.GetSize()-1;
	while (x >= 0) {
		free(g_pklist.Get(x));
		g_pklist.Del(x);
		x--;
	};
	x=g_pklist_pending.GetSize()-1;
	while (x >= 0) {
		free(g_pklist_pending.Get(x));
		g_pklist_pending.Del(x);
		x--;
	};
}

int loadPKList(char *fn)
{
	int err=0;
	int cnt=0;
	char str[1024+8];
	if (!fn) sprintf(str,"%s.pr3",g_config_prefix);

	FILE *fp=fopen(fn?fn:str,"rt");

	if (fp) while (!err) {
		char linebuf[1024];
		char *lineptr=NULL;
		PKitem item;
		memset(&item,0,sizeof(PKitem));

		int pending=0;
		while (!err) {
			lineptr=linebuf;
			fgets(lineptr,1023,fp);
			lineptr[1023]=0;
			if (feof(fp) || !lineptr[0]) err++;
			if (!strncmp(lineptr,"JSM_PENDING ",12) || !strncmp(lineptr,"WASTE_PENDING ",14)) {
				pending=1;
				lineptr+=(*lineptr == 'W')?14:12;
			}
			else pending=0;

			if (!strncmp(lineptr,"JSM_PUBLIC_KEY ",strlen("JSM_PUBLIC_KEY "))) break;
			if (!strncmp(lineptr,"WASTE_PUBLIC_KEY ",strlen("WASTE_PUBLIC_KEY "))) break;
		};

		if (!err) {
			char *p=strstr(lineptr," ");
			while (p && *p == ' ') p++;
			if (p && atoi(p) >= 10 && atoi(p) <= 20) {
				int newkeyfmt=0;
				if (atoi(p) >= 20) newkeyfmt++;
				p=strstr(p," ");
				while (p && *p == ' ') p++;
				if (p && (item.pk.bits=atoi(p)) <= MAX_RSA_MODULUS_BITS && item.pk.bits >= MIN_RSA_MODULUS_BITS) {
					while (*p && *p != ' ') p++;
					while (*p == ' ') p++;
					//p is name\n now :)
					while (*p && (p[strlen(p)-1] == '\n' || p[strlen(p)-1] == '\r' ||
						p[strlen(p)-1] == '\t' || p[strlen(p)-1] == ' ')) p[strlen(p)-1]=0;

					safe_strncpy(item.name,p,sizeof(item.name));

					int x;
					int ks=(item.pk.bits+7)/8;
					for (x = 0; x < ks && !err; x ++) {
						int c=readEncodedChar(fp);
						if (c<0) err++;
						item.pk.modulus[MAX_RSA_MODULUS_LEN-ks+x]=(unsigned char)(c&0xff);
					};
					if (newkeyfmt) {
						int a=readEncodedChar(fp);
						if (a<0) err++;
						else {
							int b=readEncodedChar(fp);
							if (b < 0) err++;
							else {
								int expsize=(a<<8)|b;
								if (expsize < 1 || expsize > ks) err++;
								else ks=expsize;
							};
						};
					};
					for (x = 0; x < ks && !err; x ++) {
						int c=readEncodedChar(fp);
						if (c<0) err++;
						item.pk.exponent[MAX_RSA_MODULUS_LEN-ks+x]=(unsigned char)(c&0xff);
					};
				}
				else err++;
			}
			else err++;
		};

		if (err) break;

		SHAify m;
		m.add((unsigned char *)item.pk.modulus,MAX_RSA_MODULUS_LEN);
		m.add((unsigned char *)item.pk.exponent,MAX_RSA_MODULUS_LEN);
		m.final(item.hash);

		PKitem *p=(PKitem *)malloc(sizeof(PKitem));
		memcpy(p,&item,sizeof(item));
		if (!pending) g_pklist.Add(p);
		else g_pklist_pending.Add(p);

		char buf[SHA_OUTSIZE*2+1];
		Bin2Hex(buf,p->hash,SHA_OUTSIZE);
		log_printf(ds_Informational,"PkLoad: %04i P%i %s %s",cnt+1,pending?1:0,buf,p->name);

		cnt++;
	};
	if (fp) fclose(fp);
	return cnt;
}

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
void copyMyPubKey(HWND hwndOwner)
{
	char buf[(MAX_RSA_MODULUS_LEN/8)*2*2+4096];
	char *buf2;

	buf[0]=0;buf2=buf;
	int ks=(g_key.bits+7)/8;
	if (!ks) return;

	sprintf(buf2,"WASTE_PUBLIC_KEY 20 %d %s\r\n",g_key.bits,g_regnick[0]?g_regnick:"unknown");
	buf2+=strlen(buf);

	int lc=0, lcm=30;

	buf2=Bin2Hex_Lf(buf2,&g_key.modulus[MAX_RSA_MODULUS_LEN-ks],ks,lc,lcm,true);

	int zeroes;
	for (zeroes = MAX_RSA_MODULUS_LEN-ks; zeroes < MAX_RSA_MODULUS_LEN && !g_key.publicExponent[zeroes]; zeroes ++);

	ks=MAX_RSA_MODULUS_LEN-zeroes;

	unsigned char x;
	x=(unsigned char)((ks>>8)&0xff);buf2=Bin2Hex_Lf(buf2,&x,1,lc,lcm,true);
	x=(unsigned char)((ks>>0)&0xff);buf2=Bin2Hex_Lf(buf2,&x,1,lc,lcm,true);

	buf2=Bin2Hex_Lf(buf2,&g_key.publicExponent[MAX_RSA_MODULUS_LEN-ks],ks,lc,lcm,true);

	sprintf(buf2,"\r\nWASTE_PUBLIC_KEY_END\r\n");

	HANDLE h=GlobalAlloc(GMEM_MOVEABLE,strlen(buf)+1);
	void *t=GlobalLock(h);
	memcpy(t,buf,strlen(buf)+1);
	GlobalUnlock(h);
	OpenClipboard(hwndOwner);
	EmptyClipboard();
	SetClipboardData(CF_TEXT,h);
	CloseClipboard();
}
#endif

void savePKList()
{
	char str[1024+8];
	char buf[(MAX_RSA_MODULUS_LEN/8)*2*2+4096];
	char *buf2;
	sprintf(str,"%s.pr3",g_config_prefix);
	FILE *fp=fopen(str,"wt");
	if (fp) {
		int x;
		int total=g_pklist.GetSize()+g_pklist_pending.GetSize();
		for (x = 0; x < total; x ++) {
			PKitem *t;
			bool pending=(x >= g_pklist.GetSize());
			if (pending) t=g_pklist_pending.Get(x-g_pklist.GetSize());
			else t=g_pklist.Get(x);

			buf[0]=0;buf2=buf;
			int ks=(t->pk.bits+7)/8;
			if (!ks) continue;

			sprintf(buf2,"%sWASTE_PUBLIC_KEY 20 %d %s\n",pending?"WASTE_PENDING ":"",t->pk.bits,t->name[0]?t->name:"unknown");
			buf2+=strlen(buf);
			int lc=0,lcm=30;

			buf2=Bin2Hex_Lf(buf2,&t->pk.modulus[MAX_RSA_MODULUS_LEN-ks],ks,lc,lcm,false);

			int zeroes;
			for (zeroes = MAX_RSA_MODULUS_LEN-ks; zeroes < MAX_RSA_MODULUS_LEN && !t->pk.exponent[zeroes]; zeroes ++);
			ks=MAX_RSA_MODULUS_LEN-zeroes;

			unsigned char x;
			x=(unsigned char)((ks>>8)&0xff);buf2=Bin2Hex_Lf(buf2,&x,1,lc,lcm,false);
			x=(unsigned char)((ks>>0)&0xff);buf2=Bin2Hex_Lf(buf2,&x,1,lc,lcm,false);

			buf2=Bin2Hex_Lf(buf2,&t->pk.exponent[MAX_RSA_MODULUS_LEN-ks],ks,lc,lcm,false);
			sprintf(buf2,"\nWASTE_PUBLIC_KEY_END\n\n");
			buf2+=strlen(buf);

			fprintf(fp,"%s",buf);
		};
		fclose(fp);
	};
}

int findPublicKeyFromKey(R_RSA_PUBLIC_KEY *key) // 1 on found, searches pending too!
{
	int x;
	for (x = 0; x < g_pklist.GetSize(); x ++) {
		PKitem *pi=g_pklist.Get(x);
		R_RSA_PUBLIC_KEY *t=&pi->pk;
		if (key->bits == t->bits &&
			!memcmp(key->exponent,t->exponent,MAX_RSA_MODULUS_LEN) &&
			!memcmp(key->modulus,t->modulus,MAX_RSA_MODULUS_LEN))
			return 1;
	};
	for (x = 0; x < g_pklist_pending.GetSize(); x ++) {
		PKitem *pi=g_pklist_pending.Get(x);
		R_RSA_PUBLIC_KEY *t=&pi->pk;
		if (key->bits == t->bits &&
			!memcmp(key->exponent,t->exponent,MAX_RSA_MODULUS_LEN) &&
			!memcmp(key->modulus,t->modulus,MAX_RSA_MODULUS_LEN))
			return 1;
	};
	if (key->bits == g_key.bits &&
		!memcmp(key->exponent,g_key.publicExponent,MAX_RSA_MODULUS_LEN) &&
		!memcmp(key->modulus,g_key.modulus,MAX_RSA_MODULUS_LEN))
		return 1;

	return 0; //no key found
}

char *findPublicKey(unsigned char *hash, R_RSA_PUBLIC_KEY *out)
{
	int x;
	for (x = 0; x < g_pklist.GetSize(); x ++) {
		PKitem *pi=g_pklist.Get(x);
		if (!memcmp(hash,pi->hash,SHA_OUTSIZE)) {
			if (out) memcpy(out,&pi->pk,sizeof(R_RSA_PUBLIC_KEY));
			return pi->name;
		};
	};
	if (!memcmp(hash,g_pubkeyhash,SHA_OUTSIZE)) {
		if (out) {
			out->bits=g_key.bits;
			memcpy(out->exponent,g_key.publicExponent,MAX_RSA_MODULUS_LEN);
			memcpy(out->modulus,g_key.modulus,MAX_RSA_MODULUS_LEN);
		};
		return "(local)";
	};

	return NULL; //no key found
}

char *conspeed_strs[5]={"Modem","ISDN","Slow DSL/Cable","T1/Fast DSL/Cable","T3/LAN"};
int conspeed_speeds[5]={32,64,384,1600,20000};
int get_speedstr(int kbps, char *str)
{
	int x;
	for (x = 0; x < 5; x ++)
		if (kbps <= conspeed_speeds[x]) break;
	if (x == 5) x--;
	if (str) strcpy(str,conspeed_strs[x]);
	return x;
}

bool IPv4TestIpInMask(unsigned long addr,unsigned long subnet,unsigned long mask)
{
	subnet&=mask;
	addr&=mask;
	return addr==subnet;
}

unsigned long IPv4Addr(unsigned char i1,unsigned char i2,unsigned char i3,unsigned char i4)
{
	unsigned long t;
	t=htonl(
		(((unsigned long)i1)<<24)|
		(((unsigned long)i2)<<16)|
		(((unsigned long)i3)<< 8)|
		(((unsigned long)i4)<< 0)
		);
	return t;
}

unsigned long IPv4NetMask(unsigned int networkbits)
{
	unsigned long i;
	if (!networkbits) return 0;
	networkbits&=0x1f;
	i=(unsigned long)-1;
	i=i<<(32-(networkbits&31));
	return htonl(i);
}

bool IPv4IsLoopback(unsigned long addr)
{
	return IPv4TestIpInMask(addr,IPv4Addr(127,  0,  0,  0),IPv4NetMask( 8));
}

bool IPv4IsPrivateNet(unsigned long addr)
{
	bool t;
	t=IPv4TestIpInMask(addr,IPv4Addr( 10,  0,  0,  0),IPv4NetMask( 8));if (t) return true;
	t=IPv4TestIpInMask(addr,IPv4Addr(172, 16,  0,  0),IPv4NetMask(12));if (t) return true;
	t=IPv4TestIpInMask(addr,IPv4Addr(192,168,  0,  0),IPv4NetMask(16));if (t) return true;
	return false;
}

#ifdef _WIN32
	bool GetInterfaceInfoOnAddr(unsigned long addr,unsigned long &localaddr,unsigned long &localmask)
	{
		SOCKET s;
		s=socket(AF_INET,SOCK_DGRAM,0);
		if (s==INVALID_SOCKET){
			log_printf(ds_Error,"Call to socket() failed with error!!");
			localaddr=localmask=0;
			return false;
		};

		INTERFACE_INFO ii[64];
		DWORD len;
		if (WSAIoctl(s,SIO_GET_INTERFACE_LIST,NULL,0,&ii,sizeof(ii),&len,NULL,NULL)==SOCKET_ERROR) {
			log_printf(ds_Error,"Call to WSAIoctl() failed with error!!");
			localaddr=localmask=0;
			closesocket(s);
			return false;
		};

		len=len/sizeof(ii[0]);
		unsigned long laddr=0;
		unsigned long lmask=0;
		bool ret=false;
		DWORD i;
		for (i=0;i<len;i++) {
			if ((ii[i].iiFlags & IFF_UP)!=0) {
				if (ii[i].iiAddress.Address.sa_family==AF_INET) {
					laddr=ii[i].iiAddress.AddressIn.sin_addr.s_addr;
					lmask=ii[i].iiNetmask.AddressIn.sin_addr.s_addr;
					if (IPv4TestIpInMask(addr,laddr,lmask)) {
						ret=true;
						break;
					};
				};
			};
		};
		if (!ret) {
			laddr=lmask=0;
		};
		localaddr=laddr;
		localmask=lmask;
		closesocket(s);
		return false;
	}
#else
	bool GetInterfaceInfoOnAddr(unsigned long addr,unsigned long &localaddr,unsigned long &localmask)
	{
		struct ifaddrs *ifaces, *ifa;

		if (getifaddrs(&ifaces) < 0)
		{
			if (errno != ENOSYS)
			{
				log_printf(ds_Error,"Call to getifaddrs() failed with error!!");
				localaddr=localmask=0;
				return false;
			}
			else {
				log_printf(ds_Error,"Call to getifaddrs() not implemented!! Huh?");
				localaddr=localmask=0;
				return false;
			};
		};

		unsigned long laddr=0;
		unsigned long lmask=0;
		bool ret=false;
		for (ifa = ifaces; ifa != NULL; ifa = ifa->ifa_next)
		{
			if ((ifa->ifa_flags & IFF_UP)!=0) {
				if (ifa->ifa_addr->sa_family==AF_INET) {
					sockaddr_in *in;
					in=(sockaddr_in*)(ifa->ifa_addr);
					unsigned long laddr=in->sin_addr.s_addr;
					in=(sockaddr_in*)(ifa->ifa_netmask);
					unsigned long lmask=in->sin_addr.s_addr;
					if (IPv4TestIpInMask(addr,laddr,lmask)) {
						ret=true;
						break;
					};
				};
			};
		};
		if (!ret) {
			laddr=lmask=0;
		};
		localaddr=laddr;
		localmask=lmask;
		freeifaddrs(ifaces);
		return false;
	}
#endif

bool is_accessable_addr(unsigned long addr)
{
	if (g_use_accesslist) return true;

	if (addr==0) return false;
	if (addr==INADDR_NONE) return false;
	if (IPv4IsLoopback(addr)) return false;
	if (!IPv4IsPrivateNet(addr)) return true;

	//if (!g_mql->GetNumQueues()) return true;

	unsigned long localaddr;
	unsigned long localmask;
	if (GetInterfaceInfoOnAddr(addr,localaddr,localmask)) {
		if (addr!=localaddr) {
			return true;
		};
	};
	return false;
}

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
void SetWndTitle(HWND hwnd, char *titlebase)
{
	char buf[1024];
	if (g_appendprofiletitles && g_profile_name[0] && strlen(titlebase)+strlen(g_profile_name)+3 < sizeof(buf)) {
		sprintf(buf,"%s/%s",titlebase,g_profile_name);
		SetWindowText(hwnd,buf);
	}
	else
		SetWindowText(hwnd,titlebase);
}
#endif

static const char* dSevStArr[]=
{
	"Console ", //ds_Console
	"Critical", //ds_Critical
	"Error   ", //ds_Error
	"Warning ", //ds_Warning
	"Inform. ", //ds_Informational
	"Debug   "  //ds_Debug
};

const char* sevString(dSeverity sev)
{
	static const char dummy[]="unknown!";
	if ((sev>=1)&&(sev<=sizeof(dSevStArr))) {
		return dSevStArr[sev-1];
	}
	else {
		return dummy;
	};
};

void CLogfile::operator()(dSeverity sev,const char *text,...) const
{
	if ((g_log_level>0)&&(sev<=g_log_level)) {
		static int logidx;
		static const int buflen=2048;
		char str[buflen],*strc;
		int len2;
		time_t t;struct tm *tm;
		t = time(NULL);
		tm = localtime(&t);

		va_list list;
		va_start(list,text);

		strc=str;strc[0]=0;
		sprintf(strc,"(%i)",(int)sev);strc+=strlen(strc);
		strcpy(strc,sevString(sev));strc+=strlen(strc);
		strcpy(strc,":");strc+=1;
		if (tm) sprintf(strc,"<%02d/%02d/%02d %02d:%02d:%02d:%d> ",tm->tm_year%100, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, logidx++);
		else strcpy(strc,"<????> ");

		#ifdef _DEBUG
			strc+=strlen(strc);
			strcpy(strc,"{");strc+=1;
			strcpy(strc,m_szFilename);strc+=strlen(strc);
			strcpy(strc,"(");strc+=1;
			sprintf(strc,"%u",m_line);strc+=strlen(strc);
			strcpy(strc,")");strc+=1;
			strcpy(strc,"}");strc+=1;
			strcpy(strc," ");
		#endif

		len2=strlen(str);
		strc=str+len2;
		if (len2<buflen) {
			_vsnprintf(strc,buflen-len2,text,list);
			strc+=strlen(strc);
		};
		va_end(list);

		if (_logfile) {
			fprintf(_logfile,"%s\n",str);
			if (g_log_flush_auto) fflush(_logfile);
			#if defined(_WIN32)&&(defined(_DEBUG))
				static const char* _ret="\n";
				len2=strc-str+strlen(_ret);
				if (len2<buflen) {
					strcpy(strc,_ret);strc+=strlen(strc);
					OutputDebugString(str);
				};
			#endif
		};
	};
}

bool log_UpdatePath(const char *logpath, bool bIsFilename)
{
	if (_logfile) {
		if((_logfile!=stderr)&&(_logfile!=stdout)) {
			fflush(_logfile);
			fclose(_logfile);
		};
		_logfile=NULL;
	};
	if ((!logpath)||(!logpath[0])) return true;

	static const int buflen=2048;
	static const char slfn[]="Wastelog-";
	static const char slext[]=".log";
	char np[buflen];np[0]=0;
	char *npc=np;
	int len=buflen-1;
	int tl;
	FILE* tf;

	tl=strlen(logpath);
	if (tl>len) return false;
	strcat(npc,logpath);len-=tl;
	npc+=tl;

	if (!bIsFilename) {
		tl=1;
		if (tl>len) return false;
		*npc++=DIRCHAR;*npc=0;len-=tl;

		tl=strlen(slfn);
		if (tl>len) return false;
		strcat(npc,slfn);len-=tl;
		npc+=tl;

		tl=strlen(g_profile_name);
		if (tl>len) return false;
		strcat(npc,g_profile_name);len-=tl;
		npc+=tl;

		tl=strlen(slext);
		if (tl>len) return false;
		strcat(npc,slext);len-=tl;
		npc+=tl;
	};

	tf=fopen(np,"a");
	if (!tf) return false;
	_logfile=tf;
	return true;
}

void update_forceip_dns_resolution()
{
	if (g_forceip_dynip_mode==1) {
		unsigned long ipold=g_forceip_dynip_addr;
		unsigned long ip=inet_addr(g_forceip_name);
		if (ip==INADDR_NONE) {
			hostent* ent = gethostbyname(g_forceip_name);
			if (ent) {
				g_forceip_dynip_addr = *(long*)(ent->h_addr_list[0]);
			}
			else {
				g_forceip_dynip_addr=INADDR_NONE;
			};
		}
		else {
			if (ip==0) ip=INADDR_NONE;
			g_forceip_dynip_addr=ip;
		};
		if (ipold!=g_forceip_dynip_addr) {
			char *ad1,ad2[64];
			struct in_addr in;
			in.s_addr=g_forceip_dynip_addr;
			ad1=inet_ntoa(in);
			safe_strncpy(ad2,ad1,64);
			in.s_addr=ipold;
			ad1=inet_ntoa(in);
			log_printf(ds_Informational,"Ip Mode(force): Resolved new ip. Old Ip: %s -> New Ip: %s",ad1,ad2);
		};
	};
}

//ADDED Md5Chap Moved from srchwnd coz need in server for dbg
void FormatSizeStr64(char *out, unsigned int low, unsigned int high)
{
	if (high) {
		sprintf(out,"%u GB",high*4 + (low>>30));
	}
	else {
		if (low < 1024*1024)  sprintf(out,"%u.%u KB",low>>10,((low*10)>>10)%10);
		else if (low < 256*1024*1024)  sprintf(out,"%u.%u MB",low>>20,((low*10)>>20)%10);
		else if (low < 1024*1024*1024) sprintf(out,"%u MB",low>>20);
		else {
			low>>=20;
			sprintf(out,"%u.%u GB",low>>10,((low%1024)*10)/1024);
		};
	};
}

// return is position of (c) or NULL if not found
// maxlen is bufferlen
const char* GetNextOf(const char* buf,char c,int maxlen)
{
	int tl=maxlen;
	for (;;) {
		if (tl<=0) {
			return NULL;
		};
		if (*buf==c) {
			return buf;
		};
		buf++;tl--;
	};
}

// return value is position of (stopchar)+1 or NULL if not found
// len are bufferlen
// autolen is decremented by return-source
const char* CopySingleToken(char* dest,const char* source,char stopchar,int destLen,int sourceLen, int* autolen)
{
	int maxlen=min(destLen,sourceLen);

	if (maxlen<=0) {
		if (destLen>0) dest[0]=0;
		return NULL;
	};

	const char *t=GetNextOf(source,stopchar,sourceLen);
	if (t) {
		int clen=t-source+1;
		if (clen > destLen) clen=destLen;
		memcpy(dest,source,clen-1);
		dest[clen-1]=0;
		if (autolen) *autolen=*autolen-clen;
		return t+1;
	}
	else {
		memcpy(dest,source,maxlen-1);
		dest[maxlen-1]=0;
		if (autolen) *autolen=0;
		return NULL;
	};
}

//len must be integral
void GenerateRandomSeed(void* target,unsigned int len,R_RANDOM_STRUCT *random)
{
	//according to X9.17, afaik.
	//ADDED md5chap using BFCBC for Ivector
	//x9.17 uses 3des originally

	CBlowfish bl;

	unsigned char tmpkey[8];

	R_GenerateBytes((unsigned char *)target,len,random);
	R_GenerateBytes(tmpkey,sizeof(tmpkey),random);

	bl.Init(tmpkey, sizeof(tmpkey));
	memset(&tmpkey,0,sizeof(tmpkey));

	unsigned long tm[2]={time(NULL),GetTickCount()};
	bl.EncryptECB(tm,sizeof(tm));

	bl.SetIV(CBlowfish::IV_ENC,tm);
	memset(&tm,0,sizeof(tmpkey));

	bl.EncryptCBC(target, len);

	bl.Final();
}

// you need double size array if converting to crlf
// buflen Specifies the target size!
bool str_return_unpack(char *dst,const char* src,unsigned int dstbuflen,const char returnChar)
{
	if (dstbuflen<=0) return false;
	const char *p1;
	char *p2;
	p1=src;
	p2=dst;
	while (*p1) {
		if (((unsigned int)(p2-dst))>=(dstbuflen-1)) {
			*p2=0;
			return false;
		};
		if (*p1==returnChar) {
			*p2++=0x0d;*p2++=0x0a;
			p1++;
		}
		else {
			*p2++=*p1++;
		};
	};
	*p2=0;
	return true;
};

// you need double size array if converting to crlf
// buflen Specifies the target size!
// automatic removal of leading/tailing returns
bool str_return_pack(char *dst,const char* src,unsigned int dstbuflen,const char returnChar)
{
	if (dstbuflen<=0) return false;
	const char *p1;
	char *p2, *p3;
	bool rin=true;
	p1=src;
	p3=p2=dst;
	while (*p1) {
		if (((unsigned int)(p2-dst))>=(dstbuflen-1)) {
			*p2=0;
			return false;
		};
		if (*p1==0x0d) {
			p1++;
		}
		else if (*p1==0x0a){
			if (!rin) *p2++=returnChar;
			p1++;
		}
		else {
			*p2++=*p1++;
			p3=p2;
			rin=false;
		};
	};
	*p3=0;
	return true;
};

void RandomizePadding(void* buf,unsigned int bufsize,unsigned int datasize)
{
	if (!VERIFY(bufsize>=datasize)) return;
	unsigned int len=bufsize-datasize;
	if (len==0) return;
	unsigned char* cbuf=(unsigned char* )buf;
	cbuf+=datasize;
	R_GenerateBytes(cbuf, len, &g_random);
}

void RelpaceCr(char* st)
{
	while (*st) {
		if (*st=='\r') *st=' ';
		st++;
	};
}

//ADDED Md5Chap
//Hint: has is mandatory, strings optional, length=16+40+32=88
void MakeUserStringFromHash(unsigned char *hash,char* longstring, char*shortstring)
{
	char *t;
	bool nouser=false;
	if (((t=findPublicKey(hash,NULL))!=0) && *t) {
		if (shortstring) safe_strncpy(shortstring,t,16);
		if (longstring) safe_strncpy(longstring,t,16);
	}
	else {
		if (shortstring) shortstring[0]=0;
		if (longstring) longstring[0]=0;
		nouser=true;
	};
	if (!nouser) {
		if (shortstring) strcat(shortstring," (");
		if (longstring) strcat(longstring," (");
	};
	if (longstring) Bin2Hex(longstring+strlen(longstring), hash, SHA_OUTSIZE);
	if (shortstring) {
		Bin2Hex(shortstring+strlen(shortstring), hash, 4 );
		strcat(shortstring,"...");
	};
	if (!nouser) {
		if (shortstring) strcat(shortstring,")");
		if (longstring) strcat(longstring,")");
	};
};

//ADDED Md5Chap
#if (defined(_WIN32) && defined(_CHECK_RSA_BLINDING))
	//----------------------------------------------------------------------------------
	#define N 60
	#define _UQ0 1
	#define _UQ1 1
	#define _UN1 1
	#define _BP0 1
	#define _BP1 1
	#define _BN1 1
	//----------------------------------------------------------------------------------
	static unsigned long CheckRsa_Single(int enable,int n,unsigned char *inp,bool blind)
	{
		unsigned char out[4096/8];
		unsigned int outlen;
		int ret;
		if (enable) {
			Sleep(0);
			DWORD tick=GetTickCount();
			int i;
			for (i=0;i<n;i++) {
				outlen=0;
				ret=RSAPrivateDecrypt(out, &outlen, inp, (g_key.bits + 7) / 8, &g_key, blind?&g_random:NULL);
			};
			tick=GetTickCount()-tick;
			return tick;
		};
		return 0;
	}

	void CheckRsaBlinding()
	{
		NN_DIGIT q_0[MAX_NN_DIGITS];
		NN_DIGIT q_1[MAX_NN_DIGITS];
		NN_DIGIT n_1[MAX_NN_DIGITS];
		NN_DIGIT one[MAX_NN_DIGITS];

		unsigned char cq0[4096/8];
		unsigned char cq1[4096/8];
		unsigned char cn1[4096/8];
		char buf[4096];
		int pDigits;
		int nDigits;

		if (g_key.bits>0) {
			NN_Decode(q_0, MAX_NN_DIGITS, g_key.prime[1], MAX_RSA_PRIME_LEN);
			pDigits=NN_Digits(q_0,MAX_NN_DIGITS);
			NN_Decode(n_1, MAX_NN_DIGITS, g_key.modulus, MAX_RSA_MODULUS_LEN);
			nDigits=NN_Digits(n_1,MAX_NN_DIGITS);

			NN_ASSIGN_DIGIT(one, 1, MAX_NN_DIGITS);

			NN_Sub(q_1,q_0,one,MAX_NN_DIGITS);
			NN_Add(q_0,q_0,one,MAX_NN_DIGITS);
			NN_Sub(n_1,n_1,one,MAX_NN_DIGITS);

			NN_Encode(cq0, (g_key.bits + 7) / 8, q_0, pDigits);
			NN_Encode(cq1, (g_key.bits + 7) / 8, q_1, pDigits);
			NN_Encode(cn1, (g_key.bits + 7) / 8, n_1, nDigits);

			//------------------------------
			int n=N;
			unsigned long tick1u=CheckRsa_Single(_UQ0,n,cq0,false);
			unsigned long tick2u=CheckRsa_Single(_UQ1,n,cq1,false);
			unsigned long tick3u=CheckRsa_Single(_UN1,n,cn1,false);
			unsigned long tick1b=CheckRsa_Single(_BP0,n,cq0,true);
			unsigned long tick2b=CheckRsa_Single(_BP1,n,cq1,true);
			unsigned long tick3b=CheckRsa_Single(_BN1,n,cn1,true);
			//------------------------------
			double dperitem1u=(double)tick1u/(double)n;
			double dperitem2u=(double)tick2u/(double)n;
			double dperitem3u=(double)tick3u/(double)n;
			double dperitemavgu=((tick1u>0)?1:0)+((tick2u>0)?1:0)+((tick3u>0)?1:0);
			double dfactoru=tick1u;
			if (dperitemavgu>0) dperitemavgu=(dperitem1u+dperitem2u+dperitem3u)/dperitemavgu;
			if (dfactoru!=0) dfactoru=(double)tick2u/dfactoru;
			//------------------------------
			double dperitem1b=(double)tick1b/(double)n;
			double dperitem2b=(double)tick2b/(double)n;
			double dperitem3b=(double)tick3b/(double)n;
			double dperitemavgb=((tick1b>0)?1:0)+((tick2b>0)?1:0)+((tick3b>0)?1:0);
			double dfactorb=tick1b;
			if (dperitemavgb>0) dperitemavgb=(dperitem1b+dperitem2b+dperitem3b)/dperitemavgb;
			if (dfactorb!=0) dfactorb=(double)tick2b/dfactorb;
			//------------------------------
			#pragma push_macro("sprintf")
			#undef sprintf
			sprintf(buf,
				"Rounds n: %i\n"
				"-------------------\n"
				"Normal:\n"
				"TickCount All    : %10i\n"
				"TickCount Avg    : %10.4f\n"
				#if _UQ0
					"TickCount(q+1)   : %10i\n"
					"TickCount(q+1)/n : %10.4f\n"
				#endif
				#if _UQ1
					"TickCount(q-1)   : %10i\n"
					"TickCount(q-1)/n : %10.4f\n"
				#endif
				#if _UN1
					"TickCount(n-1)   : %10i\n"
					"TickCount(n-1)/n : %10.4f\n"
				#endif
				"Tick(q-1)/Tick(q): %10.4f\n"
				"-------------------\n"
				"Blinded:\n"
				"TickCount All    : %10i\n"
				"TickCount Avg    : %10.4f\n"
				#if _BP0
					"TickCount(q+1)   : %10i\n"
					"TickCount(q+1)/n : %10.4f\n"
				#endif
				#if _BP1
					"TickCount(q-1)   : %10i\n"
					"TickCount(q-1)/n : %10.4f\n"
				#endif
				#if _BN1
					"TickCount(n-1)   : %10i\n"
					"TickCount(n-1)/n : %10.4f\n"
				#endif
				"Tick(q-1)/Tick(q): %10.4f\n",
				n,
				tick1u+tick2u+tick3u,
				dperitemavgu,
				#if _UQ0
					tick1u,
					dperitem1u,
				#endif
				#if _UQ1
					tick2u,
					dperitem2u,
				#endif
				#if _UN1
					tick3u,
					dperitem3u,
				#endif
					dfactoru,
					tick1b+tick2b+tick3b,
					dperitemavgb,
				#if _BP0
					tick1b,
					dperitem1b,
				#endif
				#if _BP1
					tick2b,
					dperitem2b,
				#endif
				#if _BN1
					tick3b,
					dperitem3b,
				#endif
				dfactorb
				);
			#pragma pop_macro("sprintf")
			MessageBox(0,buf,"Info",0);
		};
	}
#endif
