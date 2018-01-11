// $_FILEHEADER_BEGIN ****************************
// 版权声明: xxxxx版权所有
//			  
// 文件名称：GDBCSStruct.h
// 创建人： 史云杰
// 文件说明：GDBCS使用到结构
// $_FILEHEADER_END ******************************
#ifndef __GDBCSSTRUCT_H
#define __GDBCSSTRUCT_H

#include "struct/SvrBaseStruct.h"
struct STRU_GDBCS_SYS_TN_GROUP_NAME:public STRU_SYS_TN_GROUP_NAME
{
	BOOL mbValid;
	STRU_GDBCS_SYS_TN_GROUP_NAME()
	{
		mbValid=FALSE;
	}
	void initMember()
	{
		init();
		mbValid=FALSE;
	}
};

struct STRU_GDBCS_SYS_TN_GROUP_FULL_INFO:public STRU_GDBCS_SYS_TN_GROUP_NAME
{
	int32 miLastUpdateTime;
	STRU_GDBCS_SYS_TN_GROUP_FULL_INFO()
	{
		miLastUpdateTime = 0;
	}
	void initMember()
	{
		STRU_GDBCS_SYS_TN_GROUP_NAME::initMember();
		miLastUpdateTime = 0;
	}
};

struct STRU_GDBCS_SVR_SYS_TN_INFO:public STRU_SVR_SYS_TN_INFO
{
	BOOL mbValid;
	STRU_GDBCS_SVR_SYS_TN_INFO()
	{
		mbValid=FALSE;
	}
	void initMember()
	{
		init();
		mbValid=FALSE;
	}
};
#endif //__DBASSTRUCT_H_
