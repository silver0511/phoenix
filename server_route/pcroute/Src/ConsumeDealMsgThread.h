#ifndef __CONSUME_DEALMSG_THREAD_H__
#define __CONSUME_DEALMSG_THREAD_H__

#include "common/BaseThread.h"
#include "ConsumeThreadNotify.h"

class CServerMgr;

#define NEEDAPNS "0"
#define NOTNEEDAPNS  "1"
#define ONLYWEBNEED  "10"


class CConsumeDealMsgThread:public CBaseThread
{
public:
	CConsumeDealMsgThread();
	CConsumeDealMsgThread(CServerMgr* pServerMgr, uint32 threadid,std::string& szapptype);
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
	inline UINT64 GetOnlineCnt(){return mulonlinecnt;}
	inline UINT64 GetConsumeQueueCnt(){return m_msgcnt;}

private:
	bool DealMsg(BYTE*  data, uint32 uldatalen);	
	void Send2Comet(uint16 uimsgtype,
 				 	std::set<SERVERID> serverids,
 					std::multimap<SERVERID, USERID> serverid_userid_map, 
 					BYTE * pdata,
 					uint32 uidatalen);

	uint32 DivisionUserid(std::vector<USERID> & userids, 
                        std::multimap<SERVERID,USERID>& szappserverid_userid_map,
                        std::set<SERVERID>& szappserverids,
                        uint16 uimsgtype,
                        BYTE * pdata,
                        uint32 uidatalen);
	

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

