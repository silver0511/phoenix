#ifndef __ECLASTMSGTASK_H__
#define __ECLASTMSGTASK_H__
#include "Task.h"

class CEcLastMsgTask : public CTask
{
public:
	enum
	{
		MAC_CNT = 50,
	};
		CEcLastMsgTask():CTask()
	   {	  
		   m_bid = 0;
           m_offset = 0;
		   m_maxcnt = 0;
		}
		
		CEcLastMsgTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
		{
		   m_bid = 0;
           m_offset = 0;
		   m_maxcnt = 0;
		}
		virtual ~CEcLastMsgTask()
		{
			
		}
	
		CEcLastMsgTask(const CEcLastMsgTask& rhs):CTask(rhs)
		{
			m_bid = rhs.m_bid;
		    m_offset = rhs.m_offset;
		    m_maxcnt = rhs.m_maxcnt;
		}
	
		CEcLastMsgTask& operator=(const CEcLastMsgTask& rhs)
		{
			if ( this == &rhs)
			{
				return *this;
			}
	
			CTask::operator=(rhs);
			
			this->m_bid = rhs.m_bid;
 			m_offset = rhs.m_offset;
		    m_maxcnt = rhs.m_maxcnt;
			return *this;
		}
		
		virtual void run();
		void DoForwardPackageFail();
		void SetBId(USERID aibid){m_bid = aibid;}
		void SetOffset(int offset){m_offset = offset;}
		void SetMaxCnt(int maxcnt)
		{
			m_maxcnt = maxcnt > MAC_CNT? MAC_CNT:maxcnt;
		}
		
private:
		USERID m_bid;
		int m_offset;
		int m_maxcnt;
};
#endif

