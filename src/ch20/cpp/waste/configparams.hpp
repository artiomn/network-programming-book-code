/*
WASTE - xferwnd.hpp (File transfer dialogs)
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

#ifndef	_CONFIGPARAMS_H_
#define	_CONFIGPARAMS_H_

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define	CHATMENU_HIDEALL	4000
#define	CHATMENU_SHOWALL	4001
#define	CHATMENU_BASE		4002
#define	CHATMENU_MAX		5000
#define	CHATEDIT_HISTSIZE	64
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define	CONFIG_LOGLEVEL							"loglevel"
#define	CONFIG_LOGLEVEL_DEFAULT					ds_Critical
#define	CONFIG_LOGLEVEL_MIN						ds_Console
#ifdef _DEBUG
	#define	CONFIG_LOGLEVEL_MAX						ds_Debug
#else
	#define	CONFIG_LOGLEVEL_MAX						ds_Informational
#endif
#define	CONFIG_LOG_FLUSH_AUTO					"logflushauto"
#define	CONFIG_LOG_FLUSH_AUTO_DEFAULT			0

#define	CONFIG_logpath							"logpath"

#define	CONFIG_ac_cnt							"ac_cnt"
#define	CONFIG_ac_cnt_DEFAULT					0

#define	CONFIG_ac_use							"ac_use"
#define	CONFIG_ac_use_DEFAULT					0

#define	CONFIG_accept_uploads					"accept_uploads"
#define	CONFIG_accept_uploads_DEFAULT			1

#define	CONFIG_advertise_listen					"advertise_listen"
#define	CONFIG_advertise_listen_DEFAULT			1

#define	CONFIG_allowpriv						"allowpriv"
#define	CONFIG_allowpriv_DEFAULT				1

#define	CONFIG_allowbcast						"allowbcast"
#define	CONFIG_allowbcast_DEFAULT				1

#define	CONFIG_aorecv							"aorecv"
#define	CONFIG_aorecv_DEFAULT					1

#define	CONFIG_aorecv_btf						"aorecv_btf"
#define	CONFIG_aorecv_btf_DEFAULT				0

#define	CONFIG_aot								"aot"
#define	CONFIG_aot_DEFAULT						0

#define	CONFIG_aotransfer						"aotransfer"
#define	CONFIG_aotransfer_DEFAULT				1

#define	CONFIG_aotransfer_btf					"aotransfer_btf"
#define	CONFIG_aotransfer_btf_DEFAULT			1

#define	CONFIG_aoupload							"aoupload"
#define	CONFIG_aoupload_DEFAULT					1

#define	CONFIG_aoupload_btf						"aoupload_btf"
#define	CONFIG_aoupload_btf_DEFAULT				1

#define	CONFIG_appendpt							"appendpt"
#define	CONFIG_appendpt_DEFAULT					0

#define	CONFIG_bcastkey							"bcastkey"
#define	CONFIG_bcastkey_DEFAULT					1

#define	CONFIG_cfont_bgc						"cfont_bgc"
#define	CONFIG_cfont_bgc_DEFAULT				0xFFFFFF

#define	CONFIG_cfont_color						"cfont_color"
#define	CONFIG_cfont_color_DEFAULT				0

#define	CONFIG_cfont_others_color				"cfont_others_col"
#define	CONFIG_cfont_others_color_DEFAULT		0xFF0000

#define	CONFIG_cfont_face						"cfont_face"
#define	CONFIG_cfont_face_DEFAULT				"Fixedsys"

#define	CONFIG_cfont_fx							"cfont_fx"
#define	CONFIG_cfont_fx_DEFAULT					0

#define	CONFIG_cfont_yh							"cfont_yh"
#define	CONFIG_cfont_yh_DEFAULT					180

#define	CONFIG_chat_divpos						"chat_divpos"
#define	CONFIG_chat_divpos_DEFAULT				70

#define	CONFIG_chat_timestamp					"chat_timestamp"
#define	CONFIG_chat_timestamp_DEFAULT			0

#define	CONFIG_DOWNLOAD_ONCE					"download_only_on"
#define	CONFIG_DOWNLOAD_ONCE_DEFAULT			1

#define	CONFIG_chatlog							"chatlog"
#define	CONFIG_chatlog_DEFAULT					0 //1 private 2 room 4 bcast

#define	CONFIG_chatlogpath						"chatlogpath"
#define	CONFIG_chatlogpath_DEFAULT				""

#define	CONFIG_chatsnd							"chatsnd"
#define	CONFIG_chatsnd_DEFAULT					0

#define	CONFIG_chatsndfn						"chatsndfn"
#define	CONFIG_chatsndfn_DEFAULT				""

#define	CONFIG_chatchan_h						"chatchan_h"
#define	CONFIG_chatchan_h_DEFAULT				300

#define	CONFIG_chatchan_w						"chatchan_w"
#define	CONFIG_chatchan_w_DEFAULT				500

#define	CONFIG_chatuser_h						"chatuser_h"
#define	CONFIG_chatuser_h_DEFAULT				300

#define	CONFIG_chatuser_w						"chatuser_w"
#define	CONFIG_chatuser_w_DEFAULT				300

#define	CONFIG_clientid128						"clientid128"

#define	CONFIG_concb_pos						"concb_pos"
#define	CONFIG_concb_pos_DEFAULT				0

#define	CONFIG_confirmquit						"confirmquit"
#define	CONFIG_confirmquit_DEFAULT				1

#define	CONFIG_conspeed							"conspeed"

#define	CONFIG_cwhmin							"cwhmin"
#define	CONFIG_cwhmin_DEFAULT					0

#define	CONFIG_databasepath						"databasepath"
#define	CONFIG_databasepath_DEFAULT				""

#define	CONFIG_db_save							"db_save"
#define	CONFIG_db_save_DEFAULT					1

#define	CONFIG_directxfers						"directxfers"
#define	CONFIG_directxfers_DEFAULT				1

#define	CONFIG_directxfers_connect				"directxfers_conn"
#define	CONFIG_directxfers_connect_DEFAULT		1

#define	CONFIG_dlppath							"dlppath"
#define	CONFIG_dlppath_DEFAULT					(1|32|4|16)

#define	CONFIG_dorefresh						"dorefresh"
#define	CONFIG_dorefresh_DEFAULT				0

#define	CONFIG_downloadflags					"downloadflags"
#define	CONFIG_downloadflags_DEFAULT			7

#define	CONFIG_downloadpath						"downloadpath"

#define	CONFIG_extlist							"extlist"
#define	CONFIG_EXTENSIONS_LIST_DEFAULT			"ppt;doc;xls;txt;zip;"

#define	CONFIG_extrainf							"extrainf"
#define	CONFIG_extrainf_DEFAULT					0

#define	CONFIG_forceip_dynip_mode				"forceip_dynip_mo"
#define	CONFIG_forceip_dynip_mode_DEFAULT		2

#define	CONFIG_forceip_name						"forceip_name"
#define	CONFIG_forceip_name_DEFAULT				""

#define	CONFIG_gayflash							"gayflash"
#define	CONFIG_gayflash_DEFAULT					(16<<2)+(1<<1)+1

#define	CONFIG_gayflashp						"gayflashp"
#define	CONFIG_gayflashp_DEFAULT				(16<<2)+(1<<1)+1

#define	CONFIG_gayflashb						"gayflashb"
#define	CONFIG_gayflashb_DEFAULT				(16<<2)+(1<<1)+1

#define	CONFIG_hideallonmin						"hideallonmin"
#define	CONFIG_hideallonmin_DEFAULT				1

#define	CONFIG_keepupnet						"keepupnet"
#define	CONFIG_keepupnet_DEFAULT				0

#define	CONFIG_keydistflags						"keydistflags"
#define	CONFIG_keydistflags_DEFAULT				4|2 // no 1 -> don't autoallow

#define	CONFIG_storepass						"storepass"
#define	CONFIG_keypass							"keypass"
#define	CONFIG_storepass_DEFAULT				0
#define	CONFIG_keypass_DEFAULT					NULL

#define	CONFIG_limit_uls						"limit_uls"
#define	CONFIG_limit_uls_DEFAULT				1

#define	CONFIG_limitchat						"limitchat"
#define	CONFIG_limitchat_DEFAULT				1

#define	CONFIG_limitchatn						"limitchatn"
#define	CONFIG_limitchatn_DEFAULT				64

#define	CONFIG_limitInCons						"limit_in_cons"
#define	CONFIG_limitInCons_DEFAULT				64

#define	CONFIG_limitInConsPHost					"limit_in_cons_ho"
#define	CONFIG_limitInConsPHost_DEFAULT			1

#define	CONFIG_listen							"listen"
#define	CONFIG_listen_DEFAULT					1

#define	CONFIG_main_divpos						"main_divpos"
#define	CONFIG_main_divpos_DEFAULT				55

#define	CONFIG_maxsizesha						"maxsizesha"
#define	CONFIG_maxsizesha_DEFAULT				32

#define	CONFIG_mul_bgc							"mul_bgc"
#define	CONFIG_mul_bgc_DEFAULT					0xFFFFFF

#define	CONFIG_mul_color						"mul_color"
#define	CONFIG_mul_color_DEFAULT				0

#define	CONFIG_net_col1							"net_col1"
#define	CONFIG_net_col2							"net_col2"
#define	CONFIG_net_col3							"net_col3"
#define	CONFIG_net_col4							"net_col4"
#define	CONFIG_net_col1_DEFAULT					126
#define	CONFIG_net_col2_DEFAULT					45
#define	CONFIG_net_col3_DEFAULT					133
#define	CONFIG_net_col4_DEFAULT					108

#define	CONFIG_net_maximized					"net_maximized"
#define	CONFIG_net_maximized_DEFAULT			0

#define	CONFIG_net_vis							"net_vis"
#define	CONFIG_net_vis_DEFAULT					1

#define	CONFIG_networkname						"networkname"
#define	CONFIG_networkname_DEFAULT				""

#define	CONFIG_USE_PSK							"presharedkey"
#define	CONFIG_USE_PSK_DEFAULT					0

#define	CONFIG_nick								"nick"

#define	CONFIG_nickonxfers						"nickonxfers"
#define	CONFIG_nickonxfers_DEFAULT				1

#define	CONFIG_performs							"performs"
#define	CONFIG_performs_DEFAULT					""

#define	CONFIG_port								"port"
#define	CONFIG_port_DEFAULT						1337

#define	CONFIG_prefslp							"prefslp"
#define	CONFIG_prefslp_DEFAULT					12

#define	CONFIG_recv_autoclear					"recv_autoclear"
#define	CONFIG_recv_autoclear_DEFAULT			0

#define	CONFIG_recv_autores						"recv_autores"
#define	CONFIG_recv_autores_DEFAULT				1

#define	CONFIG_recv_col1						"recv_col1"
#define	CONFIG_recv_col2						"recv_col2"
#define	CONFIG_recv_col3						"recv_col3"
#define	CONFIG_recv_col4						"recv_col4"
#define	CONFIG_recv_col5						"recv_col5"
#define	CONFIG_recv_col1_DEFAULT				282
#define	CONFIG_recv_col2_DEFAULT				80
#define	CONFIG_recv_col3_DEFAULT				101
#define	CONFIG_recv_col4_DEFAULT				30
#define	CONFIG_recv_col5_DEFAULT				50

#define	CONFIG_recv_maxdl						"recv_maxdl"
#define	CONFIG_recv_maxdl_DEFAULT				4

#define	CONFIG_recv_maxdl_host					"recv_maxdl_host"
#define	CONFIG_recv_maxdl_host_DEFAULT			1

#define	CONFIG_recv_qcol1						"recv_qcol1"
#define	CONFIG_recv_qcol2						"recv_qcol2"
#define	CONFIG_recv_qcol3						"recv_qcol3"
#define	CONFIG_recv_qcol4						"recv_qcol4"
#define	CONFIG_recv_qcol1_DEFAULT				339
#define	CONFIG_recv_qcol2_DEFAULT				112
#define	CONFIG_recv_qcol3_DEFAULT				30
#define	CONFIG_recv_qcol4_DEFAULT				50

#define	CONFIG_refreshint						"refreshint"

#define	CONFIG_route							"route"

#define	CONFIG_scanonstartup					"scanonstartup"
#define	CONFIG_scanonstartup_DEFAULT			1

#define	CONFIG_search_col1						"search_col1"
#define	CONFIG_search_col2						"search_col2"
#define	CONFIG_search_col3						"search_col3"
#define	CONFIG_search_col4						"search_col4"
#define	CONFIG_search_col5						"search_col5"
#define	CONFIG_search_col6						"search_col6"
#define	CONFIG_search_col1_DEFAULT				321
#define	CONFIG_search_col2_DEFAULT				82
#define	CONFIG_search_col3_DEFAULT				84
#define	CONFIG_search_col4_DEFAULT				53
#define	CONFIG_search_col5_DEFAULT				0
#define	CONFIG_search_col6_DEFAULT				84

#define	CONFIG_search_maximized					"search_maximized"
#define	CONFIG_search_maximized_DEFAULT			0

#define	CONFIG_search_minspeed					"search_minspeed"
#define	CONFIG_search_minspeed_DEFAULT			0

#define	CONFIG_search_showfull					"search_showfull"
#define	CONFIG_search_showfull_DEFAULT			1

#define	CONFIG_search_showfullb					"search_showfullb"
#define	CONFIG_search_showfullb_DEFAULT			0

#define	CONFIG_search_sortcol					"search_sortcol"
#define	CONFIG_search_sortcol_DEFAULT			0

#define	CONFIG_search_sortdir					"search_sortdir"
#define	CONFIG_search_sortdir_DEFAULT			1

#define	CONFIG_search_vis						"search_vis"
#define	CONFIG_search_vis_DEFAULT				0

#define	CONFIG_send_autoclear					"send_autoclear"
#define	CONFIG_send_autoclear_DEFAULT			0

#define	CONFIG_send_col1						"send_col1"
#define	CONFIG_send_col2						"send_col2"
#define	CONFIG_send_col3						"send_col3"
#define	CONFIG_send_col4						"send_col4"
#define	CONFIG_send_col1_DEFAULT				260
#define	CONFIG_send_col2_DEFAULT				50
#define	CONFIG_send_col3_DEFAULT				72
#define	CONFIG_send_col4_DEFAULT				88

#define	CONFIG_shafiles							"shafiles"
#define	CONFIG_shafiles_DEFAULT					1

#define	CONFIG_srchcb_use						"srchcb_use"
#define	CONFIG_srchcb_use_DEFAULT				0

#define	CONFIG_systray							"systray"
#define	CONFIG_systray_DEFAULT					1

#define	CONFIG_systray_hide						"systray_hide"
#define	CONFIG_systray_hide_DEFAULT				1

#define	CONFIG_throttleflag						"throttleflag"
#define	CONFIG_throttlerecv						"throttlerecv"
#define	CONFIG_throttlesend						"throttlesend"
#define	CONFIG_throttleflag_DEFAULT				0
#define	CONFIG_throttlerecv_DEFAULT				12
#define	CONFIG_throttlesend_DEFAULT				12

#define	CONFIG_toolwindow						"toolwindow"
#define	CONFIG_toolwindow_DEFAULT				0

#define	CONFIG_ul_limit							"ul_limit"
#define	CONFIG_ul_limit_DEFAULT					16

#define	CONFIG_ulfullpaths						"ulfullpaths"
#define	CONFIG_ulfullpaths_DEFAULT				0

#define	CONFIG_use_extlist						"use_extlist"
#define	CONFIG_use_extlist_DEFAULT				0

#define	CONFIG_userinfo							"userinfo"
#define	CONFIG_userinfo_DEFAULT					APP_NAME " User"

#define	CONFIG_valid							"valid"

#define	CONFIG_xfer_sel							"xfer_sel"
#define	CONFIG_xfer_sel_DEFAULT					0

#define	CONFIG_xfers_maximized					"xfers_maximized"
#define	CONFIG_xfers_maximized_DEFAULT			0

#define	CONFIG_xfers_vis						"xfers_vis"
#define	CONFIG_xfers_vis_DEFAULT				0

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#endif//_CONFIGPARAMS_H_

