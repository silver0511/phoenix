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



// --------------------------------------------------------
class CUser:public CRefObjectforCache
{
public:
	CUser();
	~CUser();
	void init();
	void clear();

private:
	void init_impl();
public:
	BOOL		mbIsinMgr;
	SOCKET		mhSocket;				//socket
	void*		mpSocketInfo;			//底层通信模块socket指针
	SESSIONID  	miCometID;				//服务器会话ID
	USERID 		miUserID;
	IPTYPE		miIPAddr;				//IP
	uint16      miIPPort;				//端口


	CCriticalSection moCriticalSection;

};

#endif //__USER_H