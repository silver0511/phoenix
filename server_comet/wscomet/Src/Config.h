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

private:
	void ReadServerConfigByType(EnumServerLogicType enum_type);
public:
	BOOL 	mbTls;						//是否TLS
};


#endif //__CTRACKERCONFIG_H_

