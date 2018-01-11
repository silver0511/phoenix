#ifndef __CONSUME_DEALMSG_THREAD_H__
#define __CONSUME_DEALMSG_THREAD_H__

#include "common/BaseThread.h"
#include "ConsumeThreadNotify.h"

#define NEEDAPNS "0"
#define NOTNEEDAPNS  "1"
#define ONLYWEBNEED  "10"


class CServerMgr;

class CConsumeDealMsgThread:public CBaseThread
{
public:
	CConsumeDealMsgThread();
	CConsumeDealMsgThread(CServerMgr* pServerMgr, uint32 threadid, std::string& szapptype);
	virtual ~CConsumeDealMsgThread();

	bool Init();
	virtual void Run();
	virtual void Release();
	void Stop();
	void AddTailMsg(StrConsumeMsg* msg);
	StrConsumeMsg * GetandDelHead();
	
	inline UINT64 GetConsumeDealSucMsgCnt(){return m_ulDealSucMsgCnt;}
	inline UINT64 GetConsumeDealFailedMsgCnt(){return m_ulDealFailMsgCnt;}
	inline UINT64 GetApnsCnt(){return mulapnscnt;}
	inline UINT64 GetConsumeQueueCnt(){return m_msgcnt;}

protected:
	bool DealMsg(BYTE*  data, uint32 uldatalen,bool IsNeedApns);
	bool GenerateMsg(uint16 uimsgtype, USERID uiuserid,BYTE * pdata, uint32 uidatalen,bool IsNeedApns);
	bool SendPacketToServer(uint16 uimsgtype,USERID uiuserid, const std::string& szserverid,BYTE * pdata, uint32 uidatalen);

private:
	UINT64 m_ulDealSucMsgCnt;
	UINT64 m_ulDealFailMsgCnt;
	CServerMgr* m_pServerMgr;

	CQueue<StrConsumeMsg> m_msgqueue;
	CConsumeThreadNotify m_empty_notify;
	long m_msgcnt;
	uint32 mThreadID;
		
	UINT64 mulsendsuccnt;
	UINT64 mulsendfailcnt;
	UINT64 mulapnscnt;
	UINT64 mulonlinecnt;
	std::string m_apptye;
};

#endif

