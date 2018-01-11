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
	INT miDiscussGroupMaxCount;

};


#endif //__CTRACKERCONFIG_H_

