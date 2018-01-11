#ifndef __REDIS_DEF_H
#define __REDIS_DEF_H

//注册相关
#define REG_MOD_COUNT               100
#define ACCOUNT_MOD_COUNT           100
#define PRE_NIM_ACCOUNT_VERITY_CODE "nim_account_verify_code_"
#define PRE_NIM_ACCOUNT_USER_HASH   "nim_account_user_hash_"
#define NIM_GEN_USER_POOL           "nim_gen_user_pool"
#define PRE_NIM_REG_PWD_HASH        "nim_reg_pwd_hash_"

//登录相关
#define LOGIN_MOD_COUNT             100
#define LOGIN_DETAIL_MOD_COUNT      10
#define PRE_NIM_LOGIN_HASH			"nim_login_hash_"
#define LUA_LOGIN_SERVER        	"config/lua/get_login_server.lua"
#define PRE_NIM_LOGIN_DETAIL_LIST	"nim_ld_list_"

//用户
#define USERSERVER_MOD_COUNT        100
#define PRE_USERINFO_LST			"nim_userinfo_lst_"
#define DEVICE_TOKEN_MAP		    "nim_dt_map"
#define DEVICE_TOKEN_REVERT_MAP		"nim_revert_map"
#define PRE_TOKEN_LST				"nim_token_lst_"
#define PRE_COMPLAINT_LST			"nim_complaint_lst_"

#define LUA_USER_MANAGER			"config/lua/user_manager.lua"
#define LUA_USER_INFO				"config/lua/get_userinfo.lua"
#define LUA_USER_LST				"config/lua/user_lst.lua"
#define LUA_USER_SET				"config/lua/user_set.lua"
#define LUA_USER_LST_BYMOBILE		"config/lua/get_userlst_bymobile.lua"
#define LUA_CHANGE_MAIL				"config/lua/user_change_mail.lua"
#define LUA_CHANGE_MOBILE			"config/lua/user_change_mobile.lua"

// 单聊相关
#define SC_CHAT_TO_BACK_COUNT       16
#define SC_CHAT_INFO                "nim_sc_chat_"
#define SC_CHAT_LIST_MSG            "nim_chat_msg_id_list_"
#define SC_CHAT_TO_DB_BACK          "nim_sc_chat_to_db_back_"
#define SC_CHAT_SINGLE_STATUS       "nim_sc_single_status_"
#define SC_CHAT_WEB_LIST_MSG        "nim_chat_web_msg_id_list_"

#define EC_CHAT_TO_DB_BACK          "nim_ec_chat_to_db_back_"
#define EC_CHAT_TO_BACK_COUNT       16


// 群聊相关
#define GC_CHAT_TO_BACK_COUNT       16
#define LOGOUT_MOD_COUNT            100
#define SC_GROUP_CHAT_INFO          "nim_group_chat_"
#define GC_GROUP_USER_ID_SET        "nim_group_"
#define GC_GROUP_LAST_TIME          "nim_group_last_time_"
#define GC_CHAT_TO_DB_BACK          "nim_gc_chat_to_db_back_"
#define GC_GROUP_MSG_ID             "nim_group_msg_id_"

//推送相关

//商家服务器
#define LUA_EC_SETLASTWID           "config/lua/trade_setlastwid.lua"
#define EC_RECENT_USER_KEY          "nim_ec_rc_"
#define EC_LASE_WAIT_KEY            "nim_ec_lw_"
#define EC_CHAT_MUTEX               "nim_ec_mutex_"
#define EC_CHAT_INFO                "nim_ec_chat_"
#define EC_CHAT_LIST_MSG            "nim_ec_msg_id_list_"
#define EC_BUSINESS_INFO            "nim_ec_binfo"
#define EC_BUSINESS_PROMPTS         "nim_ec_prompts"

//公众号相关
#define SC_OFFCIAL_MOD_COUNT          16
#define SC_OFFCIAL_CHAT_INFO          "nim_offcial_chat_"
#define SC_FANS_CHAT_INFO			  "nim_fans_chat_"
#define SC_OFFCIAL_SYS_INFO			  "nim_offcial_sys"
#define SC_FANS_SYS_INFO			  "nim_fans_sys_"
#define SC_OFFCIAL_PRIVATECHAT_INFO   "nim_offcial_privatechat_"
#define SC_OFFCIAL_CHAT_INFO_DB       "nim_offcial_chat_db_"
#define SC_FANS_CHAT_INFO_DB		  "nim_fans_chat_db_"

//friend
#define FD_RELEATION_MSG_KYE          "nim_fd_msg_"
#define FD_CONFIRM_EXPIR_KEY          "nim_fd_expirttime_"

#endif
