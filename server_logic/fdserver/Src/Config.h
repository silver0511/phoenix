#ifndef __CCONFIG_H
#define __CCONFIG_H

#include "common/BaseConfig.h"

class CConfig:public CServerConfigExt
{

public:
	CConfig();
	~CConfig();

	virtual BOOL Read();
	virtual BOOL DynamicRead();
	
public:
	unsigned int m_taskthreadcnt;
	int m_statinterval;
	int m_fdexpire_seconds;
};


#endif //__CTRACKERCONFIG_H_

