#ifndef __ECRECENTORDERSTASK_H__
#define __ECRECENTORDERSTASK_H__
#include "Task.h"

class CEcRecentOrdersTask : public CTask
{
public:
		CEcRecentOrdersTask():CTask()
	   {	  
		   m_buyerid = 0;
		   m_sellerid = 0;
		}
		
		CEcRecentOrdersTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
		{
		   m_buyerid = 0; 
		   m_sellerid = 0;
		}
		virtual ~CEcRecentOrdersTask()
		{
			
		}
	
		CEcRecentOrdersTask(const CEcRecentOrdersTask& rhs):CTask(rhs)
		{
			m_buyerid = rhs.m_buyerid;
			m_sellerid = rhs.m_sellerid;
		}
	
		CEcRecentOrdersTask& operator=(const CEcRecentOrdersTask& rhs)
		{
			if ( this == &rhs)
			{
				return *this;
			}
	
			CTask::operator=(rhs);
			
			this->m_sellerid = rhs.m_sellerid;
			this->m_buyerid = rhs.m_buyerid;
			return *this;
		}
		
		virtual void run();
		void DoForwardPackageFail();
		void SetSellerId(USERID aisellerid){m_sellerid = aisellerid;}
		void SetBuyerId(USERID aibuyerid){m_buyerid = aibuyerid;}		
private:
		USERID m_sellerid;
		USERID m_buyerid;
};
#endif


