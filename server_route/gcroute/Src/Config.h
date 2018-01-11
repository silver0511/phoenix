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
	std::string m_sapptype;
	int m_statinterval;
	int m_kafkathreadcnt;
	int m_consumethreadcnt;
	int m_grouppushinterval;
};
#endif //__CTRACKERCONFIG_H_

