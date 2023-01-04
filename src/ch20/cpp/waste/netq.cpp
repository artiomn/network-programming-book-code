/*
WASTE - netq.cpp (Host list management)
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
#include "xferwnd.hpp"

#ifdef _DEFINE_SRV
	#include "resourcesrv.hpp"
#else
	#include "resource.hpp"
#endif

void add_to_netq(unsigned long ip, unsigned short port, int rating, int replace)
{
	if (rating > 100) rating=100;
	if (rating < 0) rating=0;

	struct in_addr in;
	in.s_addr=ip;
	char *t=inet_ntoa(in);
	if (!port) port=CONFIG_port_DEFAULT;
	if (t && ip != -1 && ip) {
		char host[256];
		safe_strncpy(host,t,sizeof(host));
		if (is_accessable_addr(ip) && allowIP(ip)) {
			sprintf(host+strlen(host),":%d",port);

			int x;
			int nh=g_lvnetcons.GetCount();
			int bestpos=-1;
			int lastrat=-1;

			for (x = 0; x < nh; x ++) {
				char text[256];
				text[0]=0;
				g_lvnetcons.GetText(x,2,text,sizeof(text));
				int rat=atoi(text);
				if (rating > rat && bestpos < 0) bestpos=x;

				text[0]=0;
				g_lvnetcons.GetText(x,1,text,sizeof(text));
				if (!stricmp(text,host)) {
					if (g_lvnetcons.GetParam(x)) return;
					char rat[32];
					g_lvnetcons.GetText(x,2,rat,sizeof(rat));
					int irat=atoi(rat);
					if (irat < rating) {
						if (irat < 50)
							rating = irat+1;
						else rating=irat;
					};
					if (!replace && rating < lastrat) {
						sprintf(rat,"%d",rating);
						g_lvnetcons.SetItemText(x,2,rat);
						return;
					};
					g_lvnetcons.DeleteItem(x--);
					nh--;
					break;
				};
				lastrat=rat;
			};

			if (bestpos < 0) bestpos=nh;
			char buf[32];
			sprintf(buf,"%d",rating);
			g_lvnetcons.InsertItem(bestpos,"",0);
			g_lvnetcons.SetItemText(bestpos,1,host);
			g_lvnetcons.SetItemText(bestpos,2,buf);
		};
	};
}

void LoadNetQ()
{
	char str[1024+8];
	sprintf(str,"%s.pr1",g_config_prefix);

	FILE *fp=fopen(str,"rb");
	if (fp) {
		for(;;) {
			char line[512];
			fgets(line,512,fp);
			if (feof(fp)) break;
			if (line[strlen(line)-1] == '\n') line[strlen(line)-1]=0;
			if (strlen(line)>1) {
				char *p=line;
				while (*p && *p != ':') p++;
				if (*p) *p++=0;
				char *ratingp=p;
				while (*ratingp && *ratingp != ':') ratingp++;
				if (*ratingp) *ratingp++=0;
				add_to_netq(inet_addr(line),(unsigned short)atoi(p),100-atoi(ratingp),1);
			}
			else break;
		};
		if (!feof(fp)) {
			for(;;) {
				char line[512];
				char line2[512];
				fgets(line,512,fp);
				if (feof(fp)) break;
				fgets(line2,512,fp);
				if (feof(fp)) break;
				if (line[strlen(line)-1] == '\n') line[strlen(line)-1]=0;
				if (line2[strlen(line2)-1] == '\n') line2[strlen(line2)-1]=0;
				if (strlen(line)>1 && strlen(line2)>1) {
					int p=g_lvrecvq.InsertItem(g_lvrecvq.GetCount(),line,0);
					char *ptr=strstr(line2,"<");

					if (ptr) {
						*ptr++=0;
						g_lvrecvq.SetItemText(p,1,ptr);
					}
					else g_lvrecvq.SetItemText(p,1,"?");

					g_lvrecvq.SetItemText(p,2,line2);
					g_files_in_download_queue++;
				};
			};
			RecvQ_UpdateStatusText();
		};
		fclose(fp);
	};
}

void SaveNetQ()
{
	char str[1024+8];
	sprintf(str,"%s.pr1",g_config_prefix);
	FILE *fp=fopen(str,"wb");
	if (fp) {
		int x;
		int n=g_lvnetcons.GetCount();
		for (x = 0; x < n; x ++) {
			char line[512];
			char rating[32];
			g_lvnetcons.GetText(x,1,line,512);
			g_lvnetcons.GetText(x,2,rating,32);
			if (strstr(line,":"))
				fprintf(fp,"%s:%d\n",line,100-atoi(rating));
		};

		fprintf(fp,"\n");

		for (x = 0; x < g_lvrecv.GetCount(); x ++) {
			char name[256];
			char loc[256];
			char status[256];
			g_lvrecv.GetText(x,2,status,sizeof(status));
			g_lvrecv.GetText(x,3,loc,sizeof(loc));
			if (loc[0] && strcmp(status,"Aborted by remote") && strcmp(status,"File not found")) {
				char size[64];
				g_lvrecv.GetText(x,0,name,sizeof(name));
				g_lvrecv.GetText(x,1,size,sizeof(size));
				fprintf(fp,"%s\n%s<%s\n",name,loc,size);
			};
		};
		for (x = 0; x < g_lvrecvq.GetCount(); x ++) {
			char name[256];
			char loc[256];
			char size[64];
			g_lvrecvq.GetText(x,0,name,sizeof(name));
			g_lvrecvq.GetText(x,2,loc,sizeof(loc));
			g_lvrecvq.GetText(x,1,size,sizeof(size));
			fprintf(fp,"%s\n%s<%s\n",name,loc,size);
		};
		fclose(fp);
	};
}
