// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：DLLExport.cpp
// 创建人： 史云杰
// 文件说明：管理类实现文件
// $_FILEHEADER_END *****************************

#include "stdafx.h"
#include "const/ErrorCodeDef.h"
#include "../src/TCPConnPoolMgr.h"

// $_FUNCTION_BEGIN ******************************
// 函数名称：CreateNetTPool
// 函数参数：
// 返 回 值：long UVS对象句柄
// 函数说明：CUDPTransMgr的创建函数
// $_FUNCTION_END ********************************
DLL_EXPORT INT CreateNetTPool(const char* aszClassName, 
							void** appInterface)
{
	if(NULL == aszClassName)
		return RET_ERROR;

	if(NULL == appInterface )
		return RET_ERROR;
	
	if(0 == strcmp(aszClassName,"NetTPool"))
	{
		CTCPConnPoolMgr* lpMgr = NULL;
		lpMgr = new CTCPConnPoolMgr;
		if(lpMgr)
		{
			*appInterface=lpMgr;
			return RET_SUCCESS;
		}

		LOG_TRACE(LOG_ALERT,1,"NetTPool","Error Create NetTPool Fault");
		return RET_ERROR;
	}
	return RET_NOT_FOUND;
}

