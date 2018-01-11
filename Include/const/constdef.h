// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：constdef.h
// 创建人： 史云杰
// 文件说明：常量定义
// $_FILEHEADER_END ******************************

#ifndef __CONSDEF_H
#define __CONSDEF_H

//////////////////////////////////////////////////////////////////////////
//网络处理包
#define DEF_MAX_GENERAL_PACK_LEN		(4000)	//通用包长度	
#define DEF_MAX_UDPPACK_LEN				(4056)	//udp网络最大包 4k-16(bytes)-24(bytes)
#define DEF_MAX_TCP_PACKET_LEN			(8190)  //tcp网络最大包 4k-8(bytes)


#define DEF_TCP_PACKHEAD_LEN			(2)		//2字节
#define DEF_COMET_2_BACK_HEAD_LEN		(DEF_TCP_PACKHEAD_LEN + 16)
#define DEF_LOGIC_2_DB_HEAD_LEN			(DEF_TCP_PACKHEAD_LEN + 16 + 8)
#define DEF_BACK_2_COMET_HEAD_MAX_LEN	(8192)
#define DEF_PACKET_LEN              	(DEF_MAX_TCP_PACKET_LEN + DEF_TCP_PACKHEAD_LEN)//TCP网络最大包

#define DEF_PACK_PASSWORD			"x1!P"
#define DEF_PACK_PASSWORD_LEN		(4)

#define DEF_RECV_PACK_BUFF_LEN (DEF_PACKET_LEN*2) //TCP接受包缓存
//////////////////////////////////////////////////////////////////////////
//包定义
#define DEF_DEFAULT_CLIENT_PACKET_BASE				(0)		//普通包基地址
#define DEF_DEFAULT_SERVER_PACKET_BASE				(20000)	//服务器端包基地址
#define DEF_DEFAULT_LOG_PACKET_BASE					(40000)	//服务器端包基地址
#define DEF_DEFAULT_MONINTER_PACKET_BASE			(42000) //监控系统包基地址
//服务器地址不要超过0xB000(45000)
#define DEF_SVR_MSG_BASE							(45000)	//服务器消息0xB000


#define MAPPING_PACKTYPE(PACK_TYPE) \
	enum ENUM_HOST_TYPE {\
	ENUM_TYPE = PACK_TYPE\
};

//////////////////////////////////////////////////////////////////////////
//ODBC连接所用常量
#define	DEF_DATABASE_LEN			(20)		//数据库名长度
#define	ODBCTIMEOUT					(10)		//ODBC超时时间(秒)
#define GETODBCTIMEOUT				(300000)	//6分钟
#define CRITICAL_ERROR				(-2)		//严重错误
#define NORMAL_ERROR				(-1)		//一般错误
#define SQL_MAX_RET_ROW				(64)		//需要返回多条记录时每次返回的数量
#define SQL_MAX_RET_ROW_EX			(200)		//需要返回多条记录时每次返回的数量
#define DEF_SQLSTRING_SIZE			(1024)		//sql语句长度

#define DEF_MAX_APP_NAME			(24)	//应用名

///////////////////////////////////////////////////////////////////////////
//设备信息
#define DEF_DEVICE_CODE_LEN         (128)   //机器码最大长度
#define DEF_DEVICE_TYPE_LEN         (64)	//机器类型最大长度
#define DEF_DEVICE_OS_INFO			(64)	//机器os信息
#define MAX_DEVICE_TOKEN_LEN		(32)
#define DEF_DEVICE_DETIAL_INFO_LEN	(512)	//机器信息最大长度
#define DEF_NET_TYPE_LEN				(32)	

#define DEF_PHONE_INTERNAL_ZONE_NUMBER_LEN 	(4)	//国际区号
#define DEF_PHONE_NUMBER_LEN				(15)//电话号码
#define DEF_TOTAL_PHONE_NUMBER_LEN			(DEF_PHONE_INTERNAL_ZONE_NUMBER_LEN+DEF_PHONE_NUMBER_LEN+1)//完整电话号码
//////////////////////////////////////////////////////////////////////////
//用户信息
#define DEF_URL_LEN					 (256)		// url	
#define DEF_MD5_LEN					 (32)		// md5值
#define	DEF_PASSPORT_NAME_LEN		 (50)		// 帐号名
#define DEF_PASSPORT_PWD_LEN		 DEF_MD5_LEN// 帐号md5密码
#define DEF_VERSION_LEN              (10)		// 版本信息最大长度
#define DEF_EXT_VERSION_LEN          (16)		// 版本信息最大长度
#define DEF_MIN_VERSION_LEN			 (7)		// 版本信息最小长度
#define DEF_MAJOR_VERSION_LEN		 (2)		// 主版本长度
#define DEF_MINOR_VERSION_LEN		 (2)		// 副版本长度
#define DEF_MINI_VERSION_LEN		 (3)		// 子版本长度长度
#define DEF_VERSION_DESC_LEN		 (200)		// 版本描述信息长度


#define DEF_MAC_ADDR_STR_LEN			(17)	// MAC地址长度
#define	DEF_IP_ADDR_STR_LEN				(24)	// IP地址长度
#define	DEF_USER_NICK_NAME_LEN			(32)	// 玩家昵称
#define DEF_NICK_NAME_LEN_ALLOW_INPUT	(16)	// 允许玩家输入的昵称长度
#define DEF_PERSONAL_DESC_LEN			(50)    // 个人描述
#define DEF_TEXT_LEN					(256)	// 文本长度
#define DEF_EMOTION_LEN					(64)	// emotion

#define DEF_SUPPORT_VOICE_MSG_PROTOCOL (10)

#if !defined(MAKE_WORD)
#define MAKE_WORD(high, low) \
    ((WORD)(((BYTE)(low)) | ((WORD)((BYTE)(high))) << 8))
#endif

#if !defined(GET_LOW)
#define GET_LOW(word)   ((BYTE)word)
#endif

#if !defined(GET_HIGH)
#define GET_HIGH(word)   ((((WORD)word) >> 8) & 0x00FF)
#endif

#if !defined(MAKE_DWORD)
#define MAKE_DWORD(high, low) \
	((DWORD)(((WORD)(low)) | ((DWORD)((WORD)(high))) << 16))
#endif

#if !defined(GET_DLOW)
#define GET_DLOW(dword)   ((WORD)dword)
#endif

#if !defined(GET_DHIGH)
#define GET_DHIGH(dword)   ((((DWORD)dword) >> 16) & 0x0000FFFF)
#endif

//当前数据的状态
enum ENUM_DATA_STATE
{
	DATA_STATE_INIT = -1,  // 初始态
	DATA_STATE_LOCAL,      // 本地文件
	DATA_STATE_DB,         // 数据库
	DATA_STATE_UNDB        // 尚未入库
};
enum ENUM_PHONE_TYPE
{
	PHONE_TYPE_UNKNOWN=0,

	PHONE_TYPE_IPHONE=1,
	PHONE_TYPE_IPAD,

	PHONE_TYPE_ANDROID=20,
	PHONE_TYPE_WINDOWS_PHONE=50,
	PHONE_TYPE_SYMBIAN=100,
};

enum ENUM_FRIEND_ONLINE_STATUS
{
	OSTATUS_OFFLINE = 0,//离线
	OSTATUS_ONLINE,		//在线
	OSTATUS_LEAVE,		//离开
	OSTATUS_HIDE,		//隐藏
	OSTATUS_BUSY,		//忙
	OSTATUS_ES_DICONNECT,//与服务器连接断开
	OSTATUS_NOFRIEND,
	OSTATUS_SUM
};

#define IS_OFFLINE_STATUS(uStatus)	(uStatus != OSTATUS_ONLINE)

enum ENU_PROTOCOL_TYPE
{
	ENU_PROTOCOL_UNKNOWN = 0,		//未知类型
	ENU_PROTOCOL_00001 = 0x1,		//0x1协议  加入通知包
	
	ENU_PROTOCOL_TYPE_MAX,			//协议版本最大值
};


enum ENUM_NETWORK_TYPE
{
	NET_UNKNOWN=0,
	NET_TYPE_WIFI=0,
	NET_TYPE_3G,
	NET_TYPE_GPRS,
	NET_TYPE_LTE,

	NET_TYPE_SUM
};

enum ENUM_OS_TYPE
{
	OS_TYPE_UNKNOWN =0,	//未知
	OS_TYPE_IOS,		//苹果iOS
	OS_TYPE_ANDROID,	//谷歌android
	OS_TYPE_WP,			//微软windows phone
	OS_TYPE_SYMBIAN,	//诺基亚symbian
	OS_TYPE_WINDOWS,	//微软 windows pc系列
	OS_TYPE_MACOS,		//苹果 mac系列
	OS_TYPE_LINUX,		//linux系列

	OS_TYPE_SUM

};


//测试用通道ID定义
#define DEF_DEFAULT_A_USER_CHANNEL_ID				(20121230)			//回环测试通道ID
#define DEF_DEFAULT_B_USER_CHANNEL_ID				(20121231)			//双人通话测试，A用户通道ID
#define DEF_DEFAULT_C_USER_CHANNEL_ID				(20121232)			//双人通话测试，B用户通道ID
#define DEF_DEFAULT_D_USER_CHANNEL_ID				(20121233)
#define DEF_DEFAULT_E_USER_CHANNEL_ID				(20121234)


//好友操作状态
enum ENUM_CONTACTS_UPDATE_TYPE
{
	ENUM_CONTACTS_UPDATE_TYPE_NORMAL= 0,
	ENUM_CONTACTS_UPDATE_TYPE_ADD = 1,
	ENUM_CONTACTS_UPDATE_TYPE_DEL = 2,
};

enum ENUM_CALL_ROLE_TYPE
{
	ENUM_CALL_ROLE_TYPE_CALLER = 0,
	ENUM_CALL_ROLE_TYPE_RECEIVER
};

enum ENUM_TALK_TYPE
{
	ENUM_TALK_TYPE_NOT_TALKING = 0,
	ENUM_TALK_TYPE_TALKING = 1,
	ENUM_TALK_TYPE_WAKE_UP = 2,
};

//#define IS_VALID_PHONEID(aiPhoneID) 	((aiPhoneID  > 0) && (aiPhoneID) != 18616966460)

#define IS_VALID_PHONEID(aiPhoneID) 	((aiPhoneID  > 0))

//是否是合法同步操作
#define IS_VALID_UPDATE_OP(uOpt)	((uOpt == ENUM_CONTACTS_UPDATE_TYPE_ADD) || (uOpt == ENUM_CONTACTS_UPDATE_TYPE_DEL))

typedef int ITFriendNetWorkType;
typedef int ITFriendOSType;

//频道状态
enum ENU_CHANNEL_STATE
{
	ENU_CHANNEL_STATE_IDLE = 0,					//初始状态
	ENU_CHANNEL_STATE_CHECK_USER,				//检测被叫用户
	ENU_CHANNEL_STATE_CREATE_CHANNEL,			//向ATS申请创建频道
	ENU_CHANNEL_STATE_INVITE,					//向被叫方发出邀请
	ENU_CHANNEL_STATE_RING,						//被叫方振铃
	ENU_CHANNEL_STATE_ACCEPT,					//被叫方接受
	ENU_CHANNEL_STATE_TALK,						//通话
	ENU_CHANNEL_STATE_OFFLINE_TALK,				//通话中其中一方掉线
	ENU_CHANNEL_STATE_BOTH_OFFLINE_TALK,		//通话中两方都掉线
	ENU_CHANNEL_STATE_PASUE_TALK,				//暂停通话
	ENU_CHANNEL_STATE_EXIT,						//结束状态

	ENU_CHANNEL_STATE_INVAILD,					//无效状态
	ENU_CHANNEL_STATE_SUM,						//频道状态总数
};


//呼叫操作
enum ENU_CALL_OP
{
	ENU_CALL_OP_CALL = 0,						//发起呼叫
	ENU_CALL_OP_OTHER_SIDE_ONLINE,				//对方在线			
	ENU_CALL_OP_OTHER_SIDE_OFFLINE,				//对方不在线
	ENU_CALL_OP_CHANNEL_CREATE_SUCCESS,			//创建频道成功
	ENU_CALL_OP_CHANNEL_CREATE_FAIL,			//创建频道失败
	ENU_CALL_OP_OTHER_SIDE_RING,				//对方开始振铃
	ENU_CALL_OP_OTHER_SIDE_BUSY,				//对方忙
	ENU_CALL_OP_OTHER_SIDE_ACCEPT,				//对方接受通话邀请
	ENU_CALL_OP_OTHER_SIDE_REFUSE,				//对方拒绝通话邀请
	ENU_CALL_OP_TALK_BEGIN,						//通话开始	
	ENU_CALL_OP_TALK_END,						//通话结束
	ENU_CALL_OP_CALLER_HANG_UP,					//主叫挂断
	ENU_CALL_OP_OTHER_SIDE_HANG_UP,				//被叫挂断	
	ENU_CALL_OP_CALLER_LEAVE,					//主叫掉线
	ENU_CALL_OP_OTHER_SIDE_LEAVE,				//被叫掉线
	ENU_CALL_OP_CALLER_INTO_AGAIN,				//主叫重新上线，在通话中
	ENU_CALL_OP_OTHER_SIDE_INTO_AGAIN,			//被叫重新上线，在通话中
	ENU_CALL_OP_CALLER_PASUE,					//主叫暂停通话
	ENU_CALL_OP_OTHER_SIDE_PASUE,				//被叫暂停通话
	ENU_CALL_OP_STATE_TIME_OUT,					//状态超时
	ENU_CALL_OP_CALLER_INTO_AGAIN_EX,				//主叫重新上线,不在通话中
	ENU_CALL_OP_OTHER_SIDE_INTO_AGAIN_EX,			//被叫重新上线，不在通话中

	ENU_CALL_OP_SUM,							//频道状态总数
};



//频道状态
enum ENU_APNS_TYPE
{
	ENU_APNS_TYPE_CALL = 0,
	ENU_APNS_TYPE_MSG,	
	ENU_APNS_TYPE_TEST,	
	ENU_APNS_TYPE_SYS_MSG,	
};


//连接类型
enum ENUM_CONNECT_MODE
{
	ENUM_CONNECT_MODE_TCP	 = 0x01,
	ENUM_CONNECT_MODE_WS 	 = 0x10,
	ENUM_CONNECT_MODE_HTTP 	 = 0x100
};

//账号类型
enum E_ACCOUNT_TYPE
{
	E_ACCOUNT_MOBILE = 0x01,
	E_ACCOUNT_EMAIL = 0x10,
	E_ACCOUNT_WCHAT = 0x100,
	E_ACCOUNT_QQ = 0x1000,
	E_ACCOUNT_WEIBO = 0x10000
};

const string TT_SMS_APP_ID = "1400047542";
const string TT_SMS_APP_KEY = "76e077adb8d3e9d7d853ddb0af24a521";
const int TT_SMS_APP_T_ID = 53250;


#define SMS_SEND_INTERVAL   55
#define SMS_VERITY_VALID_TIME 3*60
#define REG_PASSWORD_VALID_TIME 24*60*60*7
#endif //__CONSDEF_H
