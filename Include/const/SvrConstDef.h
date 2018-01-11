// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：SvrConstDef.h
// 创建人： 史云杰
// 文件说明：服务器常量定义
// $_FILEHEADER_END ******************************
	
#ifndef __SRVCONSTDEF_H_
#define __SRVCONSTDEF_H_

#include "base0/platform.h"
#include "constdef.h"


#define DEF_TYPE_VERSION_V1		(1)		//版本1
#define DEF_ENCRYPT_VERSION_V0	(0)		//加密版本0
#define DEF_ENCRYPT_VERSION_V1	(1)		//版本1		//控制信息


#define THREAD_RAW_MAX_PACKAGE_LEN 64 * 100
#define THREAD_DEAL_MAX_PACKAGE_LEN 64 * 100
#define CON_MAX_PACKAGE_LEN 40960
/************************************************************************/
/*  数据包分割																*/
/************************************************************************/

enum ENUM_PLATFORM
{
	PLATFORM_INVALID = 0,
	PLATFORM_APP	 = 0x01,
	PLATFROM_WEB 	 = 0x10,
	PLATFORM_HTTP 	 = 0x100
};

enum TIME_TYPE
{
	LOG_OUT_TYPE = 0,
	LOG_IN_TYPE =1,
	LOG_IN_BUSY = 2,

	ERROR_TYPE
};

enum ENUM_DIRECT 
{
	DIRECT_FRONT = 1,
	DIRECT_BACK = 2
};

enum GROUP_CHAT_OFFLINE_TYPE
{
	GROUP_OFFLINE_CHAT_NORMAL = 1,
	GROUP_OFFLINE_CHAT_ADD_USER = 2,
	GROUP_OFFLINE_CHAT_KICK_USER = 3,
	GROUP_OFFLINE_CHAT_LEADER_CHANGE = 4,
	GROUP_OFFLINE_CHAT_ENTER_AGREE = 5, 				// 邀请需要群主统一
	GROUP_OFFLINE_CHAT_ENTER_DEFAULT = 6, 				// 默认方式
	GROUP_OFFLINE_CHAT_ADD_USER_AGREE = 7, 				// 邀请成员但是需要同意
	GROUP_OFFLINE_CHAT_AGREE = 8,						// 群主同意
	GROUP_OFFLINE_CHAT_SCANNING = 9,					// 通过扫二维码自己进入
	GROUP_OFFLINE_CREATE_GROUP = 10,					// 建群
	GROUP_OFFLINE_CHAT_MODIFY_GROUP_NAME = 11, 			// 修改群名称
	GROUP_OFFLINE_CHAT_MODIFY_GROUP_REMARK = 12, 		// 修改群公告
	GROUP_OFFLINE_CHAT_MODIFY_GROUP_USER_NAME = 13,		// 修改群用户备注 自己修改自己的
	GROUP_OFFLINE_CHAT_SCAN_ADD_USER = 14				// 扫描进群
};

enum GROUP_IS_ENTER_AGREE
{
	GROUP_ENTER_AGREE_DEFAULT = 0,
	GROUP_ENTER_AGREE_USER = 1
};

enum GROUP_MESSAGE_STATUS 
{
	GROUP_MESSAGE_STATUS_NORMAL = 0, 
	GROUP_MESSAGE_STATUS_NO_HIT = 1, 				// 收消息不提醒				 不用apns
	GROUP_MESSAGE_IN_HELP_NO_HIT = 2 				// 收入群助手不提醒 			不用apns
};

enum E_M_TYPE
{
	E_M_TYPE_TEXT = 1,
	E_M_TYPE_HTML,
	E_M_TYPE_IMAGE,
	E_M_TYPE_VOICE,
	E_M_TYPE_SMILEY,
	E_M_TYPE_JSON,
	E_M_TYPE_MAP
};

enum USER_IS_REINSTALL_CLIENT 
{
	USER_NORMAL_LOGIN = 1,
	USER_REINSTALL_CLIENT = 2
};

enum GROUP_OFFLINE_MSG_IS_FINISH
{
	GROUP_OFFLINE_MSG_NO_FINISH = 1,
	GROUP_OFFLINE_MSG_FINISH = 2
};

enum OFFCIAL_MSG_IS_FINISH
{
	OFFCIAL_MSG_NO_FINISH = 1,
	OFFCIAL_MSG_FINISH = 2
};

#endif //__SRVCONSTDEF_H_
