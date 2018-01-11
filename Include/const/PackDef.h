// $_FILEHEADER_BEGIN ****************************
//版权声明： xxxxx版权所有
//             
// 文件名称：PackDef.h
// 创建人： 史云杰
// 文件说明：包定义
// $_FILEHEADER_END ******************************

#ifndef __GAME_DEFINES_H
#define __GAME_DEFINES_H
//-----------------------------------------------------------------------------
#include "base0/platform.h"
//-----------------------------------------------------------------------------

// 无效ID定义
#define INVALID_TOKENID		(-1)
#define INVALID_ITEMTYPEID	(-1)
#define INVALID_PETTYPEID	(-1)
//////////////////////////////////////////////////////////////////////////
//玩家网络
#define DEF_PLAYER_LIVE_TIME			(60)	//(1 * 60)	// 存活包发送时间

#define DEF_CLIENT_DISCONNECT_OVERTIME	(5)		// 客户端主动断开连接超时

#define DEF_PACK_OFFLINE_SPAN_NUMBER    (50)    // 离线信息一次请求个数
#define DEF_PACK_OFFLINE_CONTENT_SIZE   (6144)  // 离线包体内容最大长度
#define DEF_PACK_GROUP_SPAN_NUMBER      (50)    // 离线群一次请求个数
#define DEF_GROUP_TO_KAFKA_NUMBER       (500)    // 发送到kafka的群修改包
#define DEF_GROUP_USER_INFO_LIST        (30)    // 群组成员一次同步多少个
#define DEF_GROUP_LIST_PAGE_NUMBER      (30)    // 群组一次同步多少个
#define DEF_GROUP_LIST_IDS_PAGE_NUMBER  (360)   // 一次同步多少个群ids
#define DEF_GROUP_OFFLINE_MAX_COUNT     (10)    //  一次批量获取群离线上限个数
#define DEF_GROUP_BATCH_MAX_COUNT       (20)    // 批量获取群信息最大个数
#define DEF_SC_CHAT_MAX_CONTENT         (1000)   // 聊天最大字节数
#define DEF_SYS_OFFCIAL_ID				(1000)   //系统公众号id
#define DEF_OFFCIAL_TO_KAFKA_NUMBER		(200)   //公众号消息一次最多发送粉丝个数
#define DEF_ENTERPRISE_TO_KAFKA_KEY		"10"	//企业商家分区标志
#define DEF_OFFCIAL_TO_KAFKA_KEY		"0"		//非企业商家分区标志
#define DEF_MAX_SINGLE_TEXT_COUNT       (1000)   // 单聊最大个数
#define DEF_MAX_BATCH_DEL               (500)   // 单聊批量删除个数

#ifdef __SERVER
#define __SYSTEMNEW
#define DECLARE_FIXEDSIZE_ALLOCATOR(t) 
#define DEFINE_FIXEDSIZE_ALLOCATOR(t,t1,t3)
#endif //_SERVER
//////////////////////////////////////////////////////////////////////////
#endif //__GAME_DEFINES_H