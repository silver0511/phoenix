#ifndef __FRIENDUPDATETASK_H__
#define __FRIENDUPDATETASK_H__
#include "Task.h"

class CFriendUpdateTask : public CTask
{
public:
	CFriendUpdateTask():CTask()
   {
	  m_friendid = 0;
    }
	
	CFriendUpdateTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
   {
	  m_friendid = 0;
    }
	
	virtual ~CFriendUpdateTask()
	{
		
	}

	CFriendUpdateTask(const CFriendUpdateTask& rhs):CTask(rhs)
	{
		m_friendid = rhs.m_friendid;
	}

	CFriendUpdateTask& operator=(const CFriendUpdateTask& rhs)
	{
		if ( this == &rhs)
		{
			return *this;
		}

		CTask::operator=(rhs);
		
		this->m_friendid = rhs.m_friendid;
		return *this;
	}
	virtual void run();
	void DoForwardPackageFail();	

	void SetFriendId(USERID aifriendid){m_friendid = aifriendid;}

private:
	USERID m_friendid;
};
#endif

