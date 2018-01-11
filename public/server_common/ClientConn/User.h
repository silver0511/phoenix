// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//			 
// 文件名称: User.h
// 创 建 人: 史云杰
// 文件说明: 保存用户的信息
// $_FILEHEADER_END *****************************
#ifndef __USER_H
#define __USER_H

#include "common/RefObjMgrTemplate.h"
#include "common/Queue2.h"
#include "struct/SvrBaseStruct.h"

using namespace U9;
//单个用户每秒请求最大数
#define USER_MAX_REQ_P_SEC	100

// --------------------------------------------------------
class CUser:public CRefObjectforCache
{
public:
	CUser();
	~CUser();
	void init();
	void clear();
	bool check();
private:
	void init_impl();
public:
	BOOL		mbIsinMgr;
	SOCKET		mhSocket;				//socket
	void*		mpSocketInfo;			//底层通信模块socket指针

	IPTYPE		miIPAddr;				//IP
	uint16      miIPPort;				//端口
	
	USERID			miUserID;			//UserID
	USERID			miTempUserID;		//临时用户ID【注册,游客】
	int32			miAppID;
	std::string 	msTgt;
	std::string 	msCookie;
	std::string 	msClientVersion;
	ENUM_PLATFORM	miPlatform;
	std::string 	msDeviceCode;
	std::string 	msOsType;
	int8			miNetType;
	UINT64			miServerTime;
	uint8 			miLoginCount;

	int 			miRecvCount;
	INT64 			miRecvTime;
	CCriticalSection moCriticalSection;

};

#endif //__USER_H