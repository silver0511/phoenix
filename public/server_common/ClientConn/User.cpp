// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//			 
// 文件名称: User.cpp
// 创 建 人: 史云杰
// 文件说明: 保存用户的信息
// $_FILEHEADER_END *****************************
#include "stdafx.h"
#include "User.h"


CUser::CUser()
{
}
CUser::~CUser()
{

}

void CUser::clear()
{
	LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " User Memory manange "
		<<" UserPtr="<<(void*)this
		<<" Socket="<<mhSocket
		<< " IP="<< GetIPString(miIPAddr)
		<< ":"<< ntohs(miIPPort)
		<< "UserID:" << miUserID
		);
	init_impl();
}
void CUser::init()
{
	LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " User Memory manange "
		<<" UserPtr="<<(void*)this
	);
	//必须执行
	InitRefObjectforCache();
	init_impl();
}

void CUser::init_impl()
{
	mhSocket= INVALID_SOCKET;				//socket
	mpSocketInfo= NULL;						//底层通信模块socket指针

	mbIsinMgr = FALSE;
	miIPAddr=0;				//IP
	miIPPort=0;				//端口

	miUserID = 0;
	miTempUserID = 0;
	miAppID = 0;
    msTgt = "";
    msCookie = "";
	msClientVersion = "";
	miPlatform = PLATFORM_APP;
	msDeviceCode = "";
	msOsType = "";
	miNetType = 0;
	miServerTime = 0;
	miLoginCount = 1;
	miRecvCount = 0;
	miRecvTime = CSystem::GetSystemTime();
}


bool CUser::check()
{
	miRecvCount++;
	if(miRecvCount < USER_MAX_REQ_P_SEC)
	{
		return true;
	}

	INT64 cur_time = CSystem::GetSystemTime();
	INT64 delta_time = cur_time - miRecvTime;

	miRecvCount = 0;
	miRecvTime = cur_time;


	if(delta_time > 1000)
	{
		return true;
	}

	LOG_TRACE(LOG_NOTICE,true, __FUNCTION__, " User req fast"
			<<" UserPtr="<<(void*)this
			<<" Socket="<<mhSocket
			<< " IP="<< GetIPString(miIPAddr)
			<< ":"<< ntohs(miIPPort)
			<< "UserID:" << miUserID
	);
	return false;
}
