#pragma once

#ifndef __SERVERMGR_H
#define __SERVERMGR_H

#include <map>
#include "stdafx.h"
#include "common/CriticalSection.h"
#include "common/RefObjMgrTemplate.h"
#include "network/ITransProtocolMgr.h"
#include "network/ITCPPoolTransMgr.h"
#include "Config.h"
#include "redisclient/RedisUtil.h"


using namespace std;
using namespace U9;

class CServerMgr;
class CClientSide;

extern volatile bool gbRunning;

class CServerMgr 
{
public:
	CServerMgr();
	~CServerMgr();
public:	
	bool DealMsg(uint16 uimsgtype, 
						USERID uiuserid,
						BYTE * pdata, 
						uint32 uidatalen);
	INT Open(int port);
	INT Close();
	void clear();
	
private:
	friend class CClientSide;
	
public:
	void OnTimeCheck();	
private:
	CConfig moConfig;

	CClientSide*		mpClientSide;
	RedisUtil *mpRedisUtil;
};

#endif //__SERVERMGR_H

