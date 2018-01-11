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
#include "kafka/KafkaMgr.h"
#include "redisclient/RedisUtil.h"
#include "SensitiveMgr.h"

using namespace std;
class CServerMgr;

class CClientSide;

extern volatile bool gbRunning;

class CServerMgr 
{
public:
	CServerMgr();
	~CServerMgr();
	
public:
public:
	INT Open(int port);
	INT Close();
	void clear();
private:
	friend class CServerSide;
	friend class CClientSide;

public:
	void OnTimeCheck();
private:
	CConfig moConfig;

	UserConMgr moUserConMgr;
	CClientSide*		mpClientSide;
	SensitiveMgr*		mpSensitiveMgr;
	RedisUtil *mpRedisUtil;

	CKafkaMgr* mpKafkaMgr;
};

#endif //__SERVERMGR_H_