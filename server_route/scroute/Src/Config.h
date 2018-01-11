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

	std::string m_sgroupid;
	int m_statinterval;
	int m_kafkathreadcnt;
	int m_consumethreadcnt;
};
#endif //__CTRACKERCONFIG_H_

