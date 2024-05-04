/*
WASTE - config.cpp (Configuration file class)
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
#include "config.hpp"

void C_Config::Flush()
{
	if (!m_dirty) return;
	FILE *fp=fopen(m_inifile,"wt");
	if (!fp) return;

	fprintf(fp,"[profile]\n");
	int x;
	if (m_strs) {
		for (x = 0; x < m_num_strs; x ++) {
			char name[17];
			memcpy(name,m_strs[x].name,16);
			name[16]=0;
			if (m_strs[x].value) fprintf(fp,"%s=%s\n",name,m_strs[x].value);
		};
	};
	fclose(fp);
	m_dirty=0;
}

C_Config::~C_Config()
{
	int x;
	Flush();
	if (m_strs) for (x = 0; x < m_num_strs; x ++) free(m_strs[x].value);
	free(m_strs);
	free(m_inifile);
}

C_Config::C_Config(char *ini)
{
	m_dirty=0;
	m_inifile=strdup(ini);
	m_strs=NULL;
	m_num_strs=m_num_strs_alloc=0;

	//read config
	FILE *fp=fopen(m_inifile,"rt");
	if (!fp) return;

	for (;;) {
		// TODO: make this shit more generic
		//downloadpath!=?4096 ....
		char buf[4096];
		buf[0]=0;
		fgets(buf,sizeof(buf),fp);
		if (!buf[0] || feof(fp)) break;
		for (;;) {
			int l=strlen(buf);
			if (l > 0 && (buf[l-1] == '\n' || buf[l-1] == '\r')) buf[l-1]=0;
			else break;
		};
		if (buf[0] != '[') {
			char *p=strstr(buf,"=");
			if (p) {
				*p++=0;
				WriteString(buf,p);
			};
		};
	};
	m_dirty=0;
	fclose(fp);
}

void C_Config::WriteInt(char *name, int value)
{
	char buf[32];
	sprintf(buf,"%d",value);
	WriteString(name,buf);
}

int C_Config::ReadInt(char *name, int defvalue)
{
	const char *t=ReadString(name,"");
	if (*t) return atoi(t);
	return defvalue;
}

const char *C_Config::WriteString(char *name, const char *string)
{
	int x;
	m_dirty=1;
	for (x = 0; x < m_num_strs; x ++) {
		if (m_strs[x].value && !strncmp(name,m_strs[x].name,16)) {
			unsigned int l=(strlen(m_strs[x].value)+16)&~15;
			if (strlen(string)<l) {
				strcpy(m_strs[x].value,string);
			}
			else {
				free(m_strs[x].value);
				m_strs[x].value = (char *)malloc((strlen(string)+16)&~15);
				strcpy(m_strs[x].value,string);
			};
			return m_strs[x].value;
		};
	};

	//not already in there
	if (m_num_strs >= m_num_strs_alloc || !m_strs) {
		m_num_strs_alloc=m_num_strs*3/2+8;
		m_strs=(strType*)::realloc(m_strs,sizeof(strType)*m_num_strs_alloc);
	};
	strncpy(m_strs[m_num_strs].name,name,16);
	m_strs[m_num_strs].value = (char *)malloc((strlen(string)+16)&~15);
	strcpy(m_strs[m_num_strs].value,string);
	return m_strs[m_num_strs++].value;
}

const char *C_Config::ReadString(char *name, const char *defstr)
{
	int x;
	for (x = 0; x < m_num_strs; x ++) {
		if (m_strs[x].value && !::strncmp(name,m_strs[x].name,16)) {
			return m_strs[x].value;
		};
	};
	return defstr;
}
