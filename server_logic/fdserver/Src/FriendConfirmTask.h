#ifndef __FRIENDCONFIRMTASK_H__
#define __FRIENDCONFIRMTASK_H__
#include "Task.h"

class CFriendConfirmTask : public CTask
{

public:
	CFriendConfirmTask():CTask()
   { 
       m_remarkname = ""; 
	   m_nicknmae = ""; 
	   m_friendid = 0; 
	   m_sourcetype = 0;
	   m_result = 0;
	}
	
	CFriendConfirmTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform)
	:CTask(aiCometID, aiUserID, aiPackSessionID, aiPlatform)
	{
	   m_remarkname = ""; 
	   m_nicknmae = ""; 
	   m_friendid = 0; 
	   m_sourcetype = 0;
	   m_result = 0;
	}
	virtual ~CFriendConfirmTask()
    {
    	
    }

	CFriendConfirmTask(const CFriendConfirmTask& rhs):CTask(rhs)
	{
		m_friendid = rhs.m_friendid;
	    m_sourcetype = rhs.m_sourcetype;
	    m_remarkname = rhs.m_remarkname;
		m_nicknmae = rhs.m_nicknmae;
		m_result = rhs.m_result;
	}

	CFriendConfirmTask& operator=(const CFriendConfirmTask& rhs)
	{
		if ( this == &rhs)
		{
			return *this;
		}

		CTask::operator=(rhs);
		
		this->m_friendid = rhs.m_friendid;
	    this->m_sourcetype = rhs.m_sourcetype;
	    this->m_remarkname = rhs.m_remarkname;
		this->m_nicknmae = rhs.m_nicknmae;
		this->m_result = rhs.m_result;
		return *this;
	}
	
	virtual void run();
	void DoForwardPackageFail();	
	void SetFriendId(USERID aifriendid){m_friendid = aifriendid;}
	void SetRemarkName(const std::string& szremark){m_remarkname = szremark;}
	void SetNickName(const std::string& sznickname) {m_nicknmae = sznickname;}
	void SetSourceType(int aisourcetype){m_sourcetype = aisourcetype;}
	void SetResult(int airesult){m_result = airesult;}
	void DealAgree();
	void DealRefuse();
	
private:
	USERID m_friendid;
	int m_sourcetype;
	std::string m_remarkname;
	std::string m_nicknmae;
	int m_result;
	
};
#endif

