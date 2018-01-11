#ifndef __FRIENDCLIENTADDTASK_H__
#define __FRIENDCLIENTADDTASK_H__
#include "Task.h"

class CFriendClientAddTask : public CTask
{

public:
	CFriendClientAddTask():CTask()
   { 
	   m_nicknmae = ""; 
	   m_opmsg = "";
	   m_friendid = 0; 
	   m_sourcetype = 0;
	}
	
	CFriendClientAddTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
	:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{
	   m_nicknmae = ""; 
	   m_opmsg = "";
	   m_friendid = 0; 
	   m_sourcetype = 0;
	}
	virtual ~CFriendClientAddTask()
    {
    	
    }

	CFriendClientAddTask(const CFriendClientAddTask& rhs):CTask(rhs)
	{
		m_friendid = rhs.m_friendid;
	    m_sourcetype = rhs.m_sourcetype;
		m_nicknmae = rhs.m_nicknmae;
		m_opmsg = rhs.m_opmsg;
	}

	CFriendClientAddTask& operator=(const CFriendClientAddTask& rhs)
	{
		if ( this == &rhs)
		{
			return *this;
		}

		CTask::operator=(rhs);
		
		this->m_friendid = rhs.m_friendid;
	    this->m_sourcetype = rhs.m_sourcetype;
		this->m_nicknmae = rhs.m_nicknmae;
		this->m_opmsg =  rhs.m_opmsg;
		return *this;
	}
	
	virtual void run();
	void DoForwardPackageFail();
	void SetFriendId(USERID aifriendid){m_friendid = aifriendid;}
	void SetNickName(const std::string& sznickname) {m_nicknmae = sznickname;}
	void SetSourceType(int aisourcetype){m_sourcetype = aisourcetype;}
	void SetOpMsg(const std::string& szopmsg){m_opmsg = szopmsg;}
	
private:
	USERID m_friendid;
	int m_sourcetype;
	std::string m_nicknmae;
	std::string m_opmsg;
	
};
#endif

