// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：TCPMgr.cpp
// 创建人： 史云杰
// 文件说明：接口管理类实现文件
// $_FILEHEADER_END *****************************

#ifdef WIN32
#include <winsock2.h>
#endif /* WIN32 */

#include "stdafx.h"
#include "TCPMgr.h"
#include <assert.h>



///////////////////////////////////////////////////////////////////////////////////////////////////
// CTCPMgr 构造
CTCPMgr::CTCPMgr()
{
	mpImp = NULL;
}


//
// CTCPMgr析构
//
CTCPMgr::~CTCPMgr()
{
	if(NULL !=mpImp)
	{
		mpImp->Release();
		mpImp = NULL;
	}
	moSendCachePool.clear();
	moSocketInfoMgr.clear();
#ifdef WIN32
	WSACleanup();
#endif
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： Init
// 函数参数：	apInterface 事件通告接口
//		    apstruOption 设置属性
// 返 回 值： 
// 函数说明： 初始化接口及设置成员
//
// $_FUNCTION_END ********************************
//
// Winsock 版本定义 -- 2.0
//
#define DEF_WSA_HEIBYTE         2
#define DEF_WSA_LOWBYTE         0

INT CTCPMgr::Init(ITransMgrCallBack *apInterface, 
		               STRU_OPTION_BASE *apstruOption)
{
	if (!apstruOption || !apInterface)
	{
		return RET_ERROR;
	}

#if (defined WIN32)
	int     liErr;
	int     liVerRequested;
	WSADATA lwsadata;

	// 
	// 初始化WinSock库
	//
	liVerRequested = MAKEWORD(DEF_WSA_HEIBYTE, DEF_WSA_LOWBYTE);
	liErr = WSAStartup((WORD)liVerRequested, &lwsadata);
	if (liErr)
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "WSAStartup() failed code"<< GETSOCKET_ERRCODE());
		return RET_ERROR;
	}

	//
	// 判断WinSock的版本
	//

#endif

	if(apstruOption->mbIsServer)
	{
		
#ifdef WIN32
		if (LOBYTE(lwsadata.wVersion) != DEF_WSA_HEIBYTE ||
			HIBYTE(lwsadata.wVersion != DEF_WSA_LOWBYTE))
		{
			WSACleanup();

			LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "windowz socket version is too lower...\r\n");
			return RET_ERROR;
		}
		//Windows CompletionPort
		CIOCPMgr *lpIOCPMgr =  new CIOCPMgr();
		lpIOCPMgr->mpSendCachePool   =&moSendCachePool;
		lpIOCPMgr->mpSocketInfoMgr =  &moSocketInfoMgr;
		mpImp =lpIOCPMgr ;

#else
		CEpollBMgr *lpEpollMgr =  new CEpollBMgr();
		lpEpollMgr->mpSendCachePool   =&moSendCachePool;
		lpEpollMgr->mpSocketInfoMgr =  &moSocketInfoMgr;
		mpImp =lpEpollMgr ;

#endif

	
	}
	else
	{
		CSelectMgr *lpSelectMgr = new CSelectMgr();
		lpSelectMgr->mpSendCachePool   =&moSendCachePool;
		lpSelectMgr->mpSocketInfoMgr =  &moSocketInfoMgr;
		mpImp = lpSelectMgr;
	}

	
	U9_ASSERT(mpImp);
	return mpImp->Init(apInterface, apstruOption);
}
void CTCPMgr::InitEncrypt(int8 aiEncryptType)
{
	mpImp->InitEncrypt(aiEncryptType);
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： Open
// 函数参数： 
// 返 回 值： 1是成功 -1是失败，
// 函数说明： 打开TransmitMgr操作，实际是初始化整个接受
//			 网络处理模块
//
// $_FUNCTION_END ********************************
INT CTCPMgr::Open()
{
	U9_ASSERT(mpImp);
	return mpImp->Open();
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： Close
// 函数参数： 
// 返 回 值： 1是成功 -1是失败
// 函数说明： 关闭操作
//
// $_FUNCTION_END ********************************
INT CTCPMgr::Close()
{
	U9_ASSERT(mpImp);
	return mpImp->Close();
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： Release
// 函数参数：
// 返 回 值： VOID
// 函数说明： 释放对象
//
// $_FUNCTION_END ********************************
VOID CTCPMgr::Release()
{
	delete this;
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： SendData
// 函数参数： STRU_SENDDATA_BASE *apstruSendData 
//           数据发送设置
// 返 回 值： 1是成功 -1是失败
// 函数说明： 发送数据
//
// $_FUNCTION_END ********************************
INT CTCPMgr::SendData(STRU_SENDDATA_BASE *apstruSendData)
{
	U9_ASSERT(mpImp);
	return mpImp->SendData(apstruSendData);
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： SendData
// 函数参数： STRU_SENDDATA_BASE *apstruSendData 
//           数据发送设置
// 返 回 值： 1是成功 -1是失败
// 函数说明： 发送数据
//
// $_FUNCTION_END ********************************
INT CTCPMgr::SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData)
{
	U9_ASSERT(mpImp);
	return mpImp->SendData(apSocketInfo,apstruSendData);
}

INT CTCPMgr::SendRawData(STRU_SENDDATA_BASE *apstruSendData)
{
	return mpImp->SendRawData(apstruSendData);
}
// $_FUNCTION_BEGIN ******************************
//
// 函数名称： ConnectToSvr
// 函数参数： aiIPAddr 服务器IP， awPort 服务器的监听端口
// 返 回 值： 1是成功 -1是失败
// 函数说明： 发送数据
//
// $_FUNCTION_END ********************************
INT CTCPMgr::ConnectTo(const SESSIONID& aiPeerID, IPTYPE aiIPAddr, WORD awPort)
{
	U9_ASSERT(mpImp);
	return mpImp->ConnectTo(aiPeerID, aiIPAddr, awPort);
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： CloseTo
// 函数参数： ai64PeerID
// 返 回 值： 1是成功 -1是失败
// 函数说明： 发送数据
//
// $_FUNCTION_END ********************************
INT CTCPMgr::DisConnectTo(SOCKET ahSocket)
{
	U9_ASSERT(mpImp);
	return mpImp->DisConnectTo(ahSocket);
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetNetStatisticsInfo
// 函数参数： astruInfo
// 返 回 值： 
// 函数说明： 得到本地网络统计信息
//
// $_FUNCTION_END ********************************
INT CTCPMgr::GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo)
{
	U9_ASSERT(mpImp);
	return mpImp->GetNetStatisticsInfo(astruInfo);
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetBindIP
// 函数参数：
// 返 回 值： IPTYPE IP地址
// 函数说明： 设置本机绑定IP
//
// $_FUNCTION_END ********************************
VOID CTCPMgr::SetBindIP(IPTYPE aiIP)
{
	assert(mpImp);
	return mpImp->SetBindIP(aiIP);
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetBindPort
// 函数参数： 
// 返 回 值： 绑定端口
// 函数说明： 设置本机绑定端口
//
// $_FUNCTION_END ********************************
VOID CTCPMgr::SetBindPort(WORD awPort)
{
	U9_ASSERT(mpImp);
	return mpImp->SetBindPort(awPort);
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetBindIP
// 函数参数：
// 返 回 值： IPTYPE IP地址
// 函数说明： 得到本机绑定IP
//
// $_FUNCTION_END ********************************
IPTYPE CTCPMgr::GetBindIP()
{
	U9_ASSERT(mpImp);
	return mpImp->GetBindIP();
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetBindPort
// 函数参数： 
// 返 回 值： 绑定端口
// 函数说明： 得到本机绑定端口
//
// $_FUNCTION_END ********************************
WORD CTCPMgr::GetBindPort()
{
	U9_ASSERT(mpImp);
	return mpImp->GetBindPort();
}
//处理函数
void CTCPMgr::Processs()
{
	U9_ASSERT(mpImp);
	return mpImp->Processs();
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： AddRelationPtr
// 函数参数： 
// 返 回 值： 成功与否
// 函数说明： 设置关联指针
//
// $_FUNCTION_END ********************************
bool CTCPMgr::AddRelationPtr(void*apSocketInfo,void* apRelationPtr)
{
	U9_ASSERT(mpImp);
	return mpImp->AddRelationPtr(apSocketInfo,apRelationPtr);
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： DelRelationPtr
// 函数参数： 
// 返 回 值： 成功与否
// 函数说明： 设置关联指针
//
// $_FUNCTION_END ********************************
bool CTCPMgr::DelRelationPtr(void*apSocketInfo,void* apRelationPtr)
{
	U9_ASSERT(mpImp);
	return mpImp->DelRelationPtr(apSocketInfo,apRelationPtr);
}

