#ifndef __FRIENDREMARKTASK_H__
#define __FRIENDREMARKTASK_H__
#include "Task.h"

class CFriendRemarkTask : public CTask
{
public:
	CFriendRemarkTask():CTask()
   {
      m_remarkname = "";  
	  m_friendid = 0;
    }
	
	CFriendRemarkTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
   {
      m_remarkname = ""; 
	  m_friendid = 0;
    }
	
	virtual ~CFriendRemarkTask()
	{
		
	}

	CFriendRemarkTask(const CFriendRemarkTask& rhs):CTask(rhs)
	{
		m_friendid = rhs.m_friendid;
	    m_remarkname = rhs.m_remarkname;
	}

	CFriendRemarkTask& operator=(const CFriendRemarkTask& rhs)
	{
		if ( this == &rhs)
		{
			return *this;
		}

		CTask::operator=(rhs);
		
		this->m_friendid = rhs.m_friendid;
	    this->m_remarkname = rhs.m_remarkname;
		return *this;
	}
	virtual void run();
	void DoForwardPackageFail();	

	void SetFriendId(USERID aifriendid){m_friendid = aifriendid;}
	void SetRemarkName(const std::string& szremark){m_remarkname = szremark;}

private:
	USERID m_friendid;
	std::string m_remarkname;
};
#endif

