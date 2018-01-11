#ifndef __FRIENDBLACKLISTTASK_H__
#define __FRIENDBLACKLISTTASK_H__
#include "Task.h"

enum enBlackOpType
{
	ACTIVECLEAR = 0,
    POSITIVECLEAR = 1,
    ACTIVESET = 2,
    POSITIVESET = 3
};
class CFriendBlackListTask : public CTask
{
public:
	CFriendBlackListTask():CTask()
   {
      m_type = 0;
	  m_friendid = 0;
    }
	
	CFriendBlackListTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
   {
      m_type = 0; 
	  m_friendid = 0;
    }
	
	virtual ~CFriendBlackListTask()
	{
		
	}

	CFriendBlackListTask(const CFriendBlackListTask& rhs):CTask(rhs)
	{
		m_friendid = rhs.m_friendid;
	    m_type = rhs.m_type;
	}

	CFriendBlackListTask& operator=(const CFriendBlackListTask& rhs)
	{
		if ( this == &rhs)
		{
			return *this;
		}

		CTask::operator=(rhs);
		
		this->m_friendid = rhs.m_friendid;
	    this->m_type = rhs.m_type;
		return *this;
	}
	virtual void run();
	void DoForwardPackageFail();

	void SetFriendId(USERID aifriendid){m_friendid = aifriendid;}
	void SetType(int aitype){m_type = aitype;}

	int GetIncBlackType(bool IsActive);
	
private:
	USERID m_friendid;
	int m_type;
};
#endif


