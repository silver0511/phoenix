#ifndef __ECRECENTMSGTASK_H__
#define __ECRECENTMSGTASK_H__
#include "Task.h"

class CEcRecentMsgTask : public CTask
{

public:
	enum
	{
		MAC_CNT = 50,
	};

public:
		CEcRecentMsgTask():CTask()
	   {	  
		   m_cid = 0;
		   m_bid = 0;
		   m_msgid = 0;
		   m_maxcnt = 0;
		}
		
		CEcRecentMsgTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
		{
		   m_cid = 0; 
		   m_bid = 0;
		   m_msgid = 0;
		   m_maxcnt = 0;
		}
		virtual ~CEcRecentMsgTask()
		{
			
		}
	
		CEcRecentMsgTask(const CEcRecentMsgTask& rhs):CTask(rhs)
		{
			m_cid = rhs.m_cid;
			m_bid = rhs.m_bid;
	        m_msgid = rhs.m_msgid;
		    m_maxcnt = rhs.m_maxcnt;
		}
	
		CEcRecentMsgTask& operator=(const CEcRecentMsgTask& rhs)
		{
			if ( this == &rhs)
			{
				return *this;
			}
	
			CTask::operator=(rhs);
			
			this->m_bid = rhs.m_bid;
			this->m_cid = rhs.m_cid;
		    m_msgid = rhs.m_msgid;
		    m_maxcnt = rhs.m_maxcnt;
			return *this;
		}
		
		virtual void run();
		void DoForwardPackageFail();
		void SetBId(USERID aibid){m_bid = aibid;}
		void SetCId(USERID aicid){m_cid = aicid;}
		void SetMsgId(UINT64 msgid){m_msgid = msgid;}
		void SetMaxCnt(int maxcnt)
		{
			m_maxcnt = maxcnt > MAC_CNT? MAC_CNT:maxcnt;
		}
		
private:
		USERID m_cid;
		USERID m_bid;
		UINT64 m_msgid;
		int m_maxcnt;
};
#endif

