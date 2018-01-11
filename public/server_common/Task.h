#ifndef __TASK_H_
#define __TASK_H_
#include "base0/platform.h"
#include "common/System.h"
#include "const/NetAddrDef.h"

class CClientSide;

class CTask
{

public:
	CTask()
	{
		m_parm = NULL;
		m_pclientside = NULL;
		m_data = "";
		m_starttime = CSystem::GetMicroSystemTime();
		m_aiCometID = 0;
		m_aiUserID = 0;
		m_aiPackSessionID = 0;
		m_aiPlatform = 0;		
	}

	CTask(SESSIONID aiCometID, USERID aiUserID, INT aiPackSessionID, BYTE aiPlatform):
	m_aiCometID(aiCometID),m_aiUserID(aiUserID),m_aiPackSessionID(aiPackSessionID),m_aiPlatform(aiPlatform)
	{
		m_parm = NULL;
		m_pclientside = NULL;
		m_data = "";
		m_starttime = CSystem::GetMicroSystemTime();
	}
	
	virtual ~CTask()
	{
		
	}
	
	CTask(const CTask& task)
	{
		m_parm = task.m_parm;
		m_pclientside = task.m_pclientside;
		m_data = task.m_data;
        m_starttime = task.m_starttime;
		m_aiCometID = task.m_aiCometID;
		m_aiUserID = task.m_aiUserID;
		m_aiPackSessionID = task.m_aiPackSessionID;
		m_aiPlatform = task.m_aiPlatform;
	}

	CTask& operator=(const CTask& rhs)
	{
		if ( this == &rhs)
		{
			return *this;
		}

		this->m_parm = rhs.m_parm;
		this->m_pclientside = rhs.m_pclientside;
		this->m_data = rhs.m_data;
        this->m_starttime = rhs.m_starttime;
		this->m_aiCometID =  rhs.m_aiCometID;
		this->m_aiUserID =  rhs.m_aiUserID;
		this->m_aiPackSessionID =  rhs.m_aiPackSessionID;
		this->m_aiPlatform =  rhs.m_aiPlatform;
		return *this;
	}
	
	virtual void run() = 0;
	void SetParamter(void * parm){ m_parm = parm;}	
	void SetData(const std::string& szdata) {m_data = szdata;}
	void SetClientSide(CClientSide* pclientside){m_pclientside = pclientside;}
	uint64 GetInterValTime(){return CSystem::GetMicroSystemTime() - m_starttime;}

protected:
	std::string m_data;
	void* m_parm;
	CClientSide* m_pclientside;
    uint64 m_starttime;

	SESSIONID m_aiCometID;
	USERID m_aiUserID;
	INT m_aiPackSessionID;
	BYTE m_aiPlatform;
};

#endif
