#ifndef __CONSUME_DEALMSG_THREAD_H__
#define __CONSUME_DEALMSG_THREAD_H__

#include "common/BaseThread.h"
#include "ConsumeThreadNotify.h"

#define NEEDAPNS "0"
#define NOTNEEDAPNS  "1"

class CServerMgr;

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
	bool DealMsg(BYTE*  data, uint32 uldatalen, UINT64 ulgroupid,bool IsNeedApns);	
	void Send2Comet(uint16 uimsgtype,
 				 	std::set<SZSERVERID> serverids,
 					std::multimap<SZSERVERID, USERID> serverid_userid_map, 
 					BYTE * pdata,
 					uint32 uidatalen);

	uint32 DivisionUserid(std::vector<USERID> & userids, 
                        std::multimap<SZSERVERID,USERID>& szappserverid_userid_map,
                        std::set<SZSERVERID>& szappserverids,
                        uint16 uimsgtype,
                        BYTE * pdata,
                        uint32 uidatalen,
                        bool IsNeedApns);

	void DirectDealMsg(StrConsumeMsg* pmsg, UINT64 groupid);
	
	void CacheDealMsg(StrConsumeMsg* pmsg,
		                       const std::string& szgroupid, 
		                       const std::string& szmsgtype, 
		                       const std::string& szpageindex);

	void OnDeaNeedApnsCacheTime();
	void OnDealNoNeedApnsCacheTime();	

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
	
	std::map<std::string, StrConsumeMsg*>m_group_msg_needapns_map;
	std::map<std::string, StrConsumeMsg*>m_group_msg_notneedapns_map;
};

#endif

