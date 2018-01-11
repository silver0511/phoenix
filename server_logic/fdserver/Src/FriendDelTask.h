#ifndef __FRIENDDELTASK_H__
#define __FRIENDDELTASK_H__
#include "Task.h"

class CFriendDelTask : public CTask
{
public:
	CFriendDelTask():CTask()
	{
	   m_friendid = 0;
	   m_nickname = "";
	}
	
	CFriendDelTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
		:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
   {
      m_friendid = 0;
	  m_nickname = "";
   }

	virtual ~CFriendDelTask()
    {
    
    }

	CFriendDelTask(const CFriendDelTask& rhs):CTask(rhs)
	{
		m_friendid = rhs.m_friendid;
		m_nickname = rhs.m_nickname;
	}

	CFriendDelTask& operator=(const CFriendDelTask& rhs)
	{
		if ( this == &rhs)
		{
			return *this;
		}

		CTask::operator=(rhs);
		
		this->m_friendid = rhs.m_friendid;	
		this->m_nickname = rhs.m_nickname;
		return *this;
	}
	
	virtual void run();
	void DoForwardPackageFail();	
	void SetFriendId(USERID aifriendid){m_friendid = aifriendid;}
	void SetNickName(const std::string& nickname){m_nickname=nickname;}

private:
	USERID m_friendid;
	std::string m_nickname;
};
#endif

