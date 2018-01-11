#pragma once

#ifndef __SERVERMGR_H
#define __SERVERMGR_H

#include <map>
#include "common/CriticalSection.h"
#include "common/RefObjMgrTemplate.h"
#include "network/ITransProtocolMgr.h"
#include "network/ITCPPoolTransMgr.h"
#include "ServerConn/UserConMgr.h"
#include "Config.h"
#include "redisclient/RedisUtil.h"
#include "kafka/KafkaMgr.h"


using namespace std;
class CServerMgr;

class CServerSide;
class CClientSide;

extern volatile bool gbRunning;

class CServerMgr 
{
public:
	CServerMgr();
	~CServerMgr();
	
public:
	INT Open(int port);
	INT Close();
	void clear();
private:
	friend class CServerSide;
	friend class CClientSide;

public:
	void OnTimeCheck();

public:
	UserConMgr moUserConMgr;
	RedisUtil *mpRedisUtil;

private:
	CConfig moConfig;
	CClientSide*		mpClientSide;	
};

#endif //__SERVERMGR_H_
