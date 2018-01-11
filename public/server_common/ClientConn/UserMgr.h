// $_FILEHEADER_BEGIN ***************************
// 版权声明: xxxxx版权所有
//			  
// 文件名称: UserMgr.h
// 创 建 人: 史云杰
// 文件说明: 用户结构管理器
// $_FILEHEADER_END *****************************

#ifndef __USERMGR_H
#define __USERMGR_H

#include "User.h"

//////////////////////////////////////////////////////////////////////////
//玩家列表管理类
class CUserMgr:public CRefObjMgrTemplate2<PHONEID,CUser>
{
public:
	CUserMgr();
	~CUserMgr();
	void init(uint32 aiCacheSize=64);
	virtual void clear();
	//加入登录信息
	BOOL addUser(CUser *apUser);
	BOOL delUser(CUser *apUser);
public:
	friend class CServerMgr;
};



#endif //#ifndef __USERMGR_H