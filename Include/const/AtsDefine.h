
// $_FILEHEADER_BEGIN ****************************
// 文件名称: USCommonDefine.h
// 创 建 人: 史云杰
// 文件说明: 
// $_FILEHEADER_END ******************************
#ifndef __USCOMMONDEFINE_H
#define __USCOMMONDEFINE_H
#define DEF_US_CLIENT_VERSION ("1.60.000")			//Client 当前版本
#define DEF_US_LOWEST_CLIENT_VERSION ("1.20.000")	//Client 最低版本
#define DEF_US_SERVER_VERSION ("1.20.001")			//USS Server 当前版本
#define DEF_US_LOWEST_SERVER_VERSION ("1.00.007")	//USS Server 最低版本

#ifndef _LONGMASTER_US_COMMON_DEFINE_H
#define _LONGMASTER_US_COMMON_DEFINE_H

#define DEF_UVS_COUNTRY_NAME_LEN			(20)
#define DEF_UVS_PROVINCE_NAME_LEN			(20)
#define DEF_UVS_CITY_NAME_LEN				(20)
#define DEF_UVS_PWD_LEN						(40)

#define DEF_UVS_TYPE_NAME_LEN				(40)
#define DEF_UVS_DESC_LEN				    (200)

#define DEF_UVS_NAME_LEN					(40)		//MCU名称的长度
#define DEF_ENCRYPT_PWD_LEN					(40)		//加密后密码的长度
#define DEF_NOTIFY_INFO_LEN					(512)		//通告信息的长度 
#define DEF_MCU_NAME_LEN					(40)		//MCU名称的长度
#define DEF_MCU_DESCRIPTION_LEN				(512)		//MCU描述的长度 
#define DEF_US_VERSION_LEN					(10)		//客户端版本的长度
#define DEF_UVS_URL_LEN						(256)		//UVS的URL路径长度

#define DEF_CREATOR_NAME_LEN				(20)	
#define DEF_MAC_ADDRESS_LEN					(12)
#define DEF_MACHINE_NAME_LEN				(20)
#define DEF_HOT_WORD_LEN					(20)

#define DEF_CREATOR_NAME_LEN				(20)		//UVS创建者名称的长度


#define DEF_IP_ADDR_LEN						(20)

#define DEF_DB_NICK_NAME_LEN				(20)
#define DEF_DB_US_VERSION_LEN				(10)

#define DEF_DB_TIME_LEN						(20)
#define DEF_CREATOR_NAME_LEN				(20)

#define DEF_GK_SERVER						(0)
#define DEF_RPC_SERVER						(1)
#define DEF_UVS_SERVER						(2)


#define DEF_ADMIN_UPDATE					(0)
#define DEF_UVS_UPDATE						(1)

//用户uvs登录方式
#define DEF_LOGINDLG_LOGIN					(0) //登录界面登录
#define DEF_FINDDLG_LOGIN					(1) //查找窗口登录
#define DEF_QUICKDLG_LOGIN					(2) //快速登录窗口登录

//用户登录连接字符串
#define DEF_LOGINSTRING_ID					(0) //ID登录
#define DEF_LOGINSTRING_IP					(1) //IP登录
#define DEF_LOGINSTRING_URL					(2) //URL登录
#endif //_LONGMASTER_US_COMMON_DEFINE_H

//客户端和GK的查询类型
#define DEF_CL_GK_FIND_RANDOM			(0)//0 按随机查询
#define DEF_CL_GK_FIND_ID				(1)//1 按id查询
#define DEF_CL_GK_FIND_IP				(2)//2 按IP查询
#define DEF_CL_GK_FIND_CONDITION		(3)//3 按条件查询
#define DEF_CL_GK_FIND_HOT				(4)//4 热门查询
#define DEF_CL_GK_FIND_ADVANCE_RANDOM	(5)//5 按高级随机查询
#define DEF_CL_GK_FIND_HOT2				(6)//6 热门查询2

//通告类型
#define DEF_NOTIFY_INFO_TEXT_TYPE		(0) //文本类型
#define DEF_NOTIFY_INFO_HTML_TYPE		(1) //html类型
#define DEF_NOTIFY_INFO_FLASH_TYPE		(2) //falsh类型


// RPC 验证返回值
#define DEF_RPC_CHECK_SUCCEED_				(1)		// 验证成功，帐号正确
#define DEF_RPC_CHECK_TIME_OUT_				(-1)		// 验证帐号超时
#define DEF_RPC_CHECK_PWD_ERROR_			(0)		// 密码错误
#define DEF_RPC_CHECK_UID_ERROR_			(101)		// 用户不存在
#define DEF_RPC_CHECK_SYS_ERROR_			(-200)	// RPC服务器出现错误

// US最多创建UVS数量
#define DEF_US_MAX_UVS_COUNT			(32)	// US最多创建UVS数量

#define DEF_UVS_MAX_MCU_COUNT			(200)						//UVS中最大的MCU个数

#define DEF_UVS_MAX_PARENT_MCU_COUNT	(DEF_UVS_MAX_MCU_COUNT-32)	//UVS中最大的父MCU个数
#define DEF_UVS_MAX_SUB_MCU_COUNT		(24)						//UVS中最大的子MCU个数
#define DEF_UVS_MAX_FIX_MCU_COUNT		(DEF_UVS_MAX_MCU_COUNT-56)	//UVS中最大的固定MCU个数

//UVS 资源
struct STRU_UVS_RESOURCE
{
	int		miServerType;//
	WORD	mwRecvThreadCount;//=4接收线程数
	WORD	mwDealThreadCount;//=4处理线程数
	WORD	mwSendThreadCount;	//=4发送线程数	
	int		mlMaxMCUCount;//DEF_UVS_MAX_MCU_COUNT 最大mcu数
	WORD	mwMixThreadCount;//=4混音线程池中的线程数
	WORD	mwMixThreadMixerCount;//=4一个混音线程的混音器数
	int		miFixMCUTimeoutDay;// 固定频道失效时间
	IPTYPE	miAudioIP;//语音IP
	WORD	miAudioPort;//语音端口
};

#define DEF_SOFTWARE_NAME_LEN	20
#define DEF_SOFTWARE_VER_LEN	20
#define DEF_SOFTWARE_DL_URL_LEN	300

#endif //__USCOMMONDEFINE_H