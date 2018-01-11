// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: ITCPNode.h
// 创 建 人: 史云杰
// 文件说明: ITCPNode传输节点接口
// $_FILEHEADER_END *****************************


#ifndef __ITCPNODE_H
#define __ITCPNODE_H

DLL_EXPORT int __stdcall CreateNetSpeedProxy(const char* aszClassName,void** appInterface);

struct STRU_PROXY_CONNECT_INFO;
class ITCPNode
{
public:
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Release
	// 函数参数：
	// 返 回 值： VOID
	// 函数说明： 释放对象
	// $_FUNCTION_END ********************************
	virtual void _stdcall Release()=0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： AsyncConnect
	// 函数参数： SOCKET ahSocket
	//			  IPTYPE aiIPAddr, WORD awPort	
	// 返 回 值： 
	// 函数说明： 异步连接
	// $_FUNCTION_END ********************************
	virtual int __stdcall Connect(void *apOldWSPProcTable,
		int* apiErrno,
		SOCKET ahSocket,
		STRU_PROXY_CONNECT_INFO* apConnectInfo) = 0;

	//// $_FUNCTION_BEGIN ******************************
	//// 函数名称： SyncConnect
	//// 函数参数： SOCKET ahSocket
	////			  IPTYPE aiIPAddr, WORD awPort	
	//// 返 回 值： 
	//// 函数说明： 同步连接
	//// $_FUNCTION_END ********************************
	//virtual int __stdcall SyncConnect(void *apOldWSPProcTable,
	//	int* apiErrno,
	//	SOCKET ahSocket,
	//	STRU_PROXY_CONNECT_INFO* apConnectInfo) = 0;
};

#endif //__ITCPPROXYMGR_H