// $_FILEHEADER_BEGIN ***************************
// 版权声明: xxxxx版权所有
//			  
// 文件名称: UserMgr.h
// 创 建 人: 史云杰
// 文件说明: 用户结构管理器
// $_FILEHEADER_END *****************************

#include "stdafx.h"
#include "UserMgr.h"

CUserMgr::CUserMgr()
{

}
CUserMgr::~CUserMgr()
{
	clear();
}
void CUserMgr::init(uint32 aiCacheSize)
{
	Initialize(aiCacheSize);
}

void CUserMgr::clear()
{
	CRITICAL_SECTION_HELPER(moCriticalSection);
	CRefObjMgrTemplate2<PHONEID,CUser>::clear();
}

BOOL CUserMgr::addUser(CUser *apUser)
{
	U9_ASSERT(NULL != apUser);
	CUser *lpOldUser= AddRefObj(apUser->miUserID,apUser);
	if(lpOldUser != apUser)
	{//有老用户
		LOG_TRACE(LOG_ERR,true, __FUNCTION__, " change user"
			<<" UserPtr="<<(void*)apUser
			<<" Socket="<<apUser->mhSocket
			<<" UserID="<<apUser->miUserID
			<< " IP="<< GetIPString(apUser->miIPAddr)
			<< ":"<< ntohs(apUser->miIPPort)
			<<" RefCount="<<apUser->GetRefCount()
			<<" Old UserPtr="<<(void*)lpOldUser
			<<" Old Socket="<<lpOldUser->mhSocket
			<<" Old IP="<< GetIPString(lpOldUser->miIPAddr)
			<< ":"<< ntohs(lpOldUser->miIPPort)
			<<" Old RefCount="<<lpOldUser->GetRefCount()
			);
		return FALSE;
	}
	
	LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " add user"
		<<" UserPtr="<<(void*)apUser
		<<" Socket="<<apUser->mhSocket
		<<" UserID="<<apUser->miUserID
		<< " IP="<< GetIPString(apUser->miIPAddr)
		<< ":"<< ntohs(apUser->miIPPort)
		<<" RefCount="<<apUser->GetRefCount()
		);
	apUser->mbIsinMgr = TRUE;
	apUser->IncreaseRef();

	return TRUE;
}

BOOL CUserMgr::delUser(CUser *apUser)
{
	if(!apUser->mbIsinMgr)
		return FALSE;
	LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, " delete user"
		<<" UserPtr="<<(void*)apUser
		<<" Socket="<<apUser->mhSocket
		<<" UserID="<<apUser->miUserID
		<< " IP="<< GetIPString(apUser->miIPAddr)
		<< ":"<< ntohs(apUser->miIPPort)
		<<" RefCount="<<apUser->GetRefCount()
	);
	apUser->mbIsinMgr = FALSE;
	apUser->DecreaseRef();
	DeleteRefObj(apUser->miUserID);

	return TRUE;
}
