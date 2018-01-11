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
#include "CKafkaConsumeThread.h"
#include "ConsumeDealMsgThread.h"
#include "kafka/KafkaMgr.h"


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
	INT Open(int port);
	INT Close();
	void clear();
	void AddTailMsg(StrConsumeMsg* msg);
	bool Send2Comet(uint32 threadid, const std::string & szserverid, BYTE * pdata,uint32 uidatalen);

private:
	friend class CClientSide;
	friend class CKafkaConsumeThread;
	friend class CConsumeDealMsgThread;
	
public:
	void OnTimeCheck();
	UINT64 GetKafkaConsumeSucCnt();
	UINT64 GetKafkaConsumeFailCnt();
	UINT64 GetDealConsumeSucCnt();
	UINT64 GetDealConsumeFailCnt();
	UINT64 GetDealConsumeApnsCnt();
	UINT64 GetDealConsumeOnlineCnt();
	UINT64 GetQueueMsgCnt();
	
private:
	CConfig moConfig;

	CClientSide*		mpClientSide;
	RedisUtil *mpRedisUtil;
	std::vector<CKafkaConsumeThread *> mpCkafkaConsumeThreadlist;
	std::vector<CConsumeDealMsgThread*> mpCConsumeDealMsgThreadlist;	
	uint32 miCurInex;
};

#endif //__SERVERMGR_H

