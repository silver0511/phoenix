#include "stdafx.h"
#include "WSMgr.h"
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
CWSMgr::CWSMgr()
{
	mpImp = NULL;
}


//
// CTCPMgr析构
//
CWSMgr::~CWSMgr()
{
	if(NULL !=mpImp)
	{
		mpImp->Release();
		mpImp = NULL;
	}
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

INT CWSMgr::Init(ITransMgrCallBack *apInterface,
		               STRU_OPTION_BASE *apstruOption)
{
	if (!apstruOption || !apInterface)
	{
		return RET_ERROR;
	}

	apstruOption->mbIsServer = true;
	WebSocketMgr *lpWsMgr =  new WebSocketMgr();
	mpImp = lpWsMgr;

	
	U9_ASSERT(mpImp);
	return mpImp->Init(apInterface, apstruOption);
}
void CWSMgr::InitEncrypt(int8 aiEncryptType)
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
INT CWSMgr::Open()
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
INT CWSMgr::Close()
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
VOID CWSMgr::Release()
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
INT CWSMgr::SendData(STRU_SENDDATA_BASE *apstruSendData)
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
INT CWSMgr::SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData)
{
	U9_ASSERT(mpImp);
	return mpImp->SendData(apSocketInfo,apstruSendData);
}

INT CWSMgr::SendRawData(STRU_SENDDATA_BASE *apstruSendData)
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
INT CWSMgr::ConnectTo(const SESSIONID& aiPeerID, IPTYPE aiIPAddr, WORD awPort)
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
INT CWSMgr::DisConnectTo(SOCKET ahSocket)
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
INT CWSMgr::GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo)
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
VOID CWSMgr::SetBindIP(IPTYPE aiIP)
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
VOID CWSMgr::SetBindPort(WORD awPort)
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
IPTYPE CWSMgr::GetBindIP()
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
WORD CWSMgr::GetBindPort()
{
	U9_ASSERT(mpImp);
	return mpImp->GetBindPort();
}
//处理函数
void CWSMgr::Processs()
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
bool CWSMgr::AddRelationPtr(void*apSocketInfo,void* apRelationPtr)
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
bool CWSMgr::DelRelationPtr(void*apSocketInfo,void* apRelationPtr)
{
	U9_ASSERT(mpImp);
	return mpImp->DelRelationPtr(apSocketInfo,apRelationPtr);
}

