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
	mhSocket= INVALID_SOCKET;
	mpSocketInfo= NULL;

	mbIsinMgr = FALSE;
	miIPAddr=0;
	miIPPort=0;

	miCometID = 0;
	miUserID = 0;
}


