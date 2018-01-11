#ifndef __CCONFIG_H
#define __CCONFIG_H

#include "common/BaseConfig.h"

struct GroupTypeInfo {
	int group_type;
	int group_max_count;
	int group_is_show;
	int group_add_max_count;
};

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
	int m_nGroupTypes;
	std::map<INT, GroupTypeInfo> mapGroupTypeInfo;
};


#endif //__CTRACKERCONFIG_H_

