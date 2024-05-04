/*
WASTE - keygen.cpp (Keypair generation UI)
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

#ifdef _DEFINE_SRV
	#include "resourcesrv.hpp"
#else
	#include "resource.hpp"
#endif

#include "main.hpp"
#include "rsa/r_random.hpp"

static char *kg_privout;
static char kg_passbf[SHA_OUTSIZE];

static int kg_keysize;
static R_RANDOM_STRUCT kg_random;
static int kg_going;

static time_t kg_start_time;
static unsigned int kg_movebuf[7];
static int kg_movebuf_cnt;

static WNDPROC kg_oldWndProc;

static HANDLE kg_thread;

static R_RSA_PRIVATE_KEY kg_privKey;

static HWND kg_hwndDlg;

static void writeBFdata(
						FILE *out,
						CBlowfish *bl,
						void *data,
						unsigned int len,
						int *lc
						)
{
	unsigned int x;
	unsigned long *p=(unsigned long *)data;
	for (x = 0; x < len; x += 8) {
		unsigned long pp[2];
		pp[0]=*p++;
		pp[1]=*p++;

		bl->EncryptCBC(pp,8);
		int c;
		for (c = 0; c < 8; c ++) {
			fprintf(out,"%02X",((unsigned char *)pp)[c]);
			if (++*lc % 30 == 0) fprintf(out,"\n");
		};
	};
}

int kg_writePrivateKey(char *fn, R_RSA_PRIVATE_KEY *key, R_RANDOM_STRUCT *rnd, char *passhash)
{
	FILE *fp;
	//HACK init but unref
	//int ks=(key->bits+7)/8;
	int x;
	int lc=8;

	fp=fopen(fn,"wt");
	if (!fp) return 1;

	fprintf(fp,"WASTE_PRIVATE_KEY 10 %d\n",key->bits);

	unsigned long tl[2];
	R_GenerateBytes((unsigned char *)&tl,8,rnd);
	for (x = 0; x < 8; x ++) {
		fprintf(fp,"%02X",(tl[x/4]>>((x&3)*8))&0xff);
	};
	CBlowfish bl;
	bl.Init(passhash,SHA_OUTSIZE);
	bl.SetIV(CBlowfish::IV_BOTH,tl);
	char buf[]="PASSWORD";
	writeBFdata(fp,&bl,buf,8,&lc);

	#define WPK(x) writeBFdata(fp,&bl,key->x,sizeof(key->x),&lc);
		WPK(modulus);
		WPK(publicExponent);
		WPK(exponent);
		WPK(prime);
		WPK(primeExponent);
		WPK(coefficient);
	#undef WPK
	if (lc % 30) fprintf(fp,"\n");
	fprintf(fp,"WASTE_PRIVATE_KEY_END\n");
	fclose(fp);

	bl.Final();

	return 0;
}

static DWORD WINAPI keyThread(LPVOID /*p*/)
{
	kg_start_time=time(NULL);
	R_RSA_PROTO_KEY protoKey;
	protoKey.bits=kg_keysize;
	protoKey.useFermat4=1;
	R_RSA_PUBLIC_KEY kg_pubKey;
	memset(&kg_pubKey,0,sizeof(kg_pubKey));
	if (R_GeneratePEMKeys(&kg_pubKey,&kg_privKey,&protoKey,&kg_random) ||
		kg_writePrivateKey(kg_privout,&kg_privKey,&kg_random,kg_passbf))
	{
		SetDlgItemText(kg_hwndDlg,IDC_LABEL_LINE1,"Error creating keys");
		SetDlgItemText(kg_hwndDlg,IDC_LABEL_LINE2,"");
		MessageBox(kg_hwndDlg,"Error calling GeneratePEMKeys()",APP_NAME " Key Generation Error",MB_OK|MB_ICONSTOP);
	}
	else {
		kg_start_time=time(NULL)-kg_start_time;
		PostMessage(kg_hwndDlg,WM_USER+0x131,0,1);
	};
	memset(kg_passbf,0,SHA_OUTSIZE);
	memset(&kg_pubKey,0,sizeof(kg_pubKey));
	memset(&kg_random,0,sizeof(kg_random));

	memcpy(&g_key,&kg_privKey,sizeof(R_RSA_PRIVATE_KEY));
	SHAify m;
	m.add((unsigned char *)g_key.modulus,MAX_RSA_MODULUS_LEN);
	m.add((unsigned char *)g_key.publicExponent,MAX_RSA_MODULUS_LEN);
	m.final(g_pubkeyhash);

	memset(&kg_privKey,0,sizeof(R_RSA_PRIVATE_KEY));

	KillTimer(kg_hwndDlg,1);
	return 0;
}

static BOOL CALLBACK kg_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_MOUSEMOVE && kg_going==1) {
		kg_movebuf[kg_movebuf_cnt%7]+=lParam;
		kg_movebuf[(kg_movebuf_cnt+1)%7]+=GetTickCount();
		kg_movebuf[(kg_movebuf_cnt+2)%7]+=GetMessageTime()+GetMessagePos();
		if (++kg_movebuf_cnt >= 29) {
			kg_movebuf_cnt=0;
			R_RandomUpdate(&kg_random,(unsigned char *)kg_movebuf,sizeof(kg_movebuf));

			unsigned int bytesNeeded;
			R_GetRandomBytesNeeded(&bytesNeeded, &kg_random);
			SendDlgItemMessage(hwndDlg,IDC_PROGRESS_KEYGEN,PBM_SETPOS,(WPARAM)(64-bytesNeeded/4),0);
			if (!bytesNeeded) {
				kg_going=2;
				SendDlgItemMessage(hwndDlg,IDC_PROGRESS_KEYGEN,PBM_SETPOS,0,0);
				SetDlgItemText(hwndDlg,IDC_LABEL_LINE1,"Generating key pair... please wait");
				char buf[128];
				wsprintf(buf,"(this may take as long as %d minutes)",kg_keysize<=1024?1:kg_keysize<=2048?3:kg_keysize<=3072?10:20);
				SetDlgItemText(hwndDlg,IDC_LABEL_LINE2,buf);
				SetTimer(hwndDlg,1,250,NULL);
				DWORD id;
				kg_thread=CreateThread(NULL,0,keyThread,NULL,0,&id);
			};
		};
	};
	return CallWindowProc(kg_oldWndProc,hwndDlg,uMsg,wParam,lParam);
}

static BOOL WINAPI dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
	switch (uMsg)
	{
	case WM_USER+0x131: //success, finish writing out
		{
			SendDlgItemMessage(hwndDlg,IDC_PROGRESS_KEYGEN,PBM_SETPOS,(WPARAM)64,0);
			{
				char buf[123];
				wsprintf(buf,"Completed in %d minutes %d seconds",kg_start_time/60,kg_start_time%60);
				SetDlgItemText(hwndDlg,IDC_LABEL_LINE1,buf);
				SetDlgItemText(hwndDlg,IDC_LABEL_LINE2,"");
				MessageBox(hwndDlg,buf,APP_NAME " Key Generator - Completed",MB_OK|MB_ICONINFORMATION);
			};
			SetDlgItemText(hwndDlg,IDOK,"Close");
			EnableWindow(GetDlgItem(hwndDlg,IDOK),1);
			SetActiveWindow(GetDlgItem(hwndDlg,IDOK));

			EnableWindow(GetDlgItem(hwndDlg,IDCANCEL),0);
			if (kg_thread) {
				CloseHandle(kg_thread);
				kg_thread=0;
			};
			EndDialog(hwndDlg,1); //yay
			break;
		};
	case WM_INITDIALOG:
		{
			kg_hwndDlg=hwndDlg;
			SetWindowText(hwndDlg,APP_NAME " Key Generator");
			SendDlgItemMessage(hwndDlg,IDC_COMBO_KEYSIZE,CB_ADDSTRING,0,(LPARAM)"1024 bits (weak, not recommended)");
			SendDlgItemMessage(hwndDlg,IDC_COMBO_KEYSIZE,CB_ADDSTRING,0,(LPARAM)"1536 bits (recommended)");
			SendDlgItemMessage(hwndDlg,IDC_COMBO_KEYSIZE,CB_ADDSTRING,0,(LPARAM)"2048 bits (slower, recommended)");
			SendDlgItemMessage(hwndDlg,IDC_COMBO_KEYSIZE,CB_ADDSTRING,0,(LPARAM)"3072 bits (slow, not recommended)");
			SendDlgItemMessage(hwndDlg,IDC_COMBO_KEYSIZE,CB_ADDSTRING,0,(LPARAM)"4096 bits (very slow, not recommended)");
			SendDlgItemMessage(hwndDlg,IDC_COMBO_KEYSIZE,CB_SETCURSEL,1,0);
			break;
		};
	case WM_CLOSE: EndDialog(hwndDlg,0); break;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				{
					if (kg_thread) {
						if (WaitForSingleObject(kg_thread,0) == WAIT_TIMEOUT) TerminateThread(kg_thread,0);
						CloseHandle(kg_thread);
						kg_thread=0;
					};
					EndDialog(hwndDlg,0);
					break;
				};
			case IDOK:
				{
					if (kg_going == 2) {
						EndDialog(hwndDlg,1);
					};
					if (!kg_going) {
						int wndhide[]={
							IDC_EDIT_PASSWORD,IDC_EDIT_PASSWORD_AGAIN,
								IDC_LABEL_PASSWORD,IDC_LABEL_PASSWORD_AGAIN,IDC_LABEL_KEYSIZE,IDC_COMBO_KEYSIZE
						};
						switch (SendDlgItemMessage(hwndDlg,IDC_COMBO_KEYSIZE,CB_GETCURSEL,0,0))
						{
						case 0:
							kg_keysize=1024;
							break;
						case 1:
							kg_keysize=1536;
							break;
						case 2:
							kg_keysize=2048;
							break;
						case 3:
							kg_keysize=3072;
							break;
						case 4:
							kg_keysize=4096;
							break;
						default:
							kg_keysize=2048;
						};

						char pass1[1024],pass2[1024];

						GetDlgItemText(hwndDlg,IDC_EDIT_PASSWORD,pass1,sizeof(pass1));
						GetDlgItemText(hwndDlg,IDC_EDIT_PASSWORD_AGAIN,pass2,sizeof(pass2));
						if (strcmp(pass1,pass2)) {
							MessageBox(hwndDlg,"Passphrase mistyped",APP_NAME " Key Generator Error",MB_OK|MB_ICONSTOP);
							break;
						};

						SHAify c;
						c.add((unsigned char *)pass1,strlen(pass1));
						c.final((unsigned char *)kg_passbf);
						memset(pass1,0,sizeof(pass1));
						memset(pass2,0,sizeof(pass2));

						int x;
						for (x = 0; x < sizeof(wndhide)/sizeof(wndhide[0]); x ++)
							ShowWindow(GetDlgItem(hwndDlg,wndhide[x]),SW_HIDE);

						ShowWindow(GetDlgItem(hwndDlg,IDC_LABEL_LINE2),SW_SHOWNA);
						SetDlgItemText(hwndDlg,IDC_LABEL_LINE1,"Giving random number generator more randomness, please move");

						EnableWindow(GetDlgItem(hwndDlg,IDOK),0);

						ShowWindow(GetDlgItem(hwndDlg,IDC_PROGRESS_KEYGEN),SW_SHOWNA);
						SendDlgItemMessage(hwndDlg,IDC_PROGRESS_KEYGEN,PBM_SETRANGE,0,MAKELPARAM(0,64));
						kg_oldWndProc=(WNDPROC) SetWindowLong(hwndDlg,GWL_WNDPROC,(LONG)kg_newWndProc);

						kg_going=1;
						R_RandomInit(&kg_random);
						unsigned char buf[16];
						R_GenerateBytes(buf,sizeof(buf),&g_random);
						R_RandomUpdate(&kg_random,buf,sizeof(buf));
						memset(buf,0,sizeof(buf));
					};
					break;
				};
			};
			return 0;
		};
	case WM_DESTROY:
		{
			if (kg_oldWndProc)
				SetWindowLong(hwndDlg,GWL_WNDPROC,(LONG)kg_oldWndProc);
			kg_oldWndProc=0;
			return 0;
		};
	case WM_TIMER:
		{
			if (wParam == 1) {
				static int pos;
				if (++pos == 64) pos=0;
				SendDlgItemMessage(hwndDlg,IDC_PROGRESS_KEYGEN,PBM_SETPOS,(WPARAM)pos,0);
			};
			break;
		};
	};
	return 0;
}

static int running=0;

void RunKeyGen(HWND hwndParent, char *keyout)
{
	if (!running) {
		running=1;
		memset(&kg_privKey,0,sizeof(kg_privKey));
		kg_thread=0;
		kg_start_time=0;
		memset(kg_movebuf,0,sizeof(kg_movebuf));
		kg_movebuf_cnt=0;
		kg_oldWndProc=0;
		kg_going=0;
		kg_keysize=0;
		memset(&kg_random,0,sizeof(kg_random));
		kg_privout=keyout;
		DialogBox(g_hInst,MAKEINTRESOURCE(IDD_KEYGEN),hwndParent,dlgProc);
		running=0;
	};
}


