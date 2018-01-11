// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: DLLExport.cpp
// 创 建 人: 史云杰
// 文件说明: 模块接口管理类实现文件
// $_FILEHEADER_END *****************************
#ifdef WIN32
#include <winsock2.h>
#endif /* WIN32 */

#include "stdafx.h"
#include "../src/WSMgr.h"
#include "../src/TCPMgr.h"

// $_FUNCTION_BEGIN ******************************
// 函数名称：CreateNetTCP
// 函数参数：
// 返 回 值：long UVS对象句柄
// 函数说明：CreateNetTCP的创建函数
// $_FUNCTION_END ********************************
///////////////////////////////////////////////////////////////////////
// 导出函数
DLL_EXPORT INT CreateNetTCP(const char* aszClassName,
											void** appInterface)
{
	if(NULL == aszClassName)
		return RET_ERROR;

	if(NULL == appInterface )
		return RET_ERROR;

	ITransProtocolMgr *lpProtoMgr = NULL;

	if (strcmp(aszClassName, "NetTCP") == 0)
	{
		lpProtoMgr = new CTCPMgr();
		*appInterface = lpProtoMgr;
	}
	else if (strcmp(aszClassName, "NETWS") == 0)
	{
		lpProtoMgr = new CWSMgr();
		*appInterface = lpProtoMgr;
	}
	else
	{
		return RET_ERROR;
	}
	return RET_SUCCESS;
}

