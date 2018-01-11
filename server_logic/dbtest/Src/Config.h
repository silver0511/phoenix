#ifndef __CCONFIG_H
#define __CCONFIG_H

#include "common/BaseConfig.h"
#include "struct/SvrBaseStruct.h"
#include "MySQLDriver/MySQLDriver.h"


class CConfig:public CServerConfigExt
{

public:
	CConfig();
	~CConfig();

	virtual BOOL Read();
	virtual BOOL DynamicRead();

private:
	bool GetConnectionInfo(string lstrName,STRU_MYSQL_CONNECT_INFO *apPlayerDBList,INT aiCount);
public:

	INT     m_statinterval;
	INT		miContactDBCount;
	STRU_MYSQL_CONNECT_INFO * mpContactDBList;
};
#endif //__CTRACKERCONFIG_H_

