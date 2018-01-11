// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: BaseTCPMgr.cpp
// 创 建 人: 史云杰
// 文件说明: TCP连接池管理
// $_FILEHEADER_END *****************************

#include "stdafx.h"
#include "TCPConnPoolMgr.h"

CTCPConnPoolMgr::CTCPConnPoolMgr()
{
	mpInterface = NULL;
}

CTCPConnPoolMgr::~CTCPConnPoolMgr()
{
}
INT CTCPConnPoolMgr::Close()
{
	moTCPConnectPool.Close();
	return 1;
}
VOID CTCPConnPoolMgr::Release()
{
	delete this;
}
INT CTCPConnPoolMgr::Open(ITCPConnPoolMgrCallback *apInterface, 
					 const STRU_TCP_POOL_OPT &aoOpt)
{
	mpInterface = apInterface;

	if(moTCPConnectPool.Open(this,aoOpt))
		return true;
	else
		return false;
}
INT CTCPConnPoolMgr::SendDataBySession(const SESSIONID & aiUserID,STRU_SENDDATA_BASE *apstruSendData)
{
	int32 liIndex = aiUserID %  moTCPConnectPool.miConnectPoolNum;
	STRU_SENDDATA_TCP * lpSendData = static_cast< STRU_SENDDATA_TCP * >(apstruSendData);
	
	lpSendData->mhSocket = moTCPConnectPool.GetSocket(liIndex);
	if(INVALID_SOCKET ==lpSendData->mhSocket)
		return 0;
	
	return moTCPConnectPool.mpTCPMgr->SendData(apstruSendData);
}

INT CTCPConnPoolMgr::SendDataBySocket(SOCKET ahSoket,STRU_SENDDATA_BASE *apstruSendData)
{
	STRU_SENDDATA_TCP * lpSendData = static_cast< STRU_SENDDATA_TCP * >(apstruSendData);

	lpSendData->mhSocket = ahSoket;
	if(INVALID_SOCKET ==lpSendData->mhSocket)
		return 0;

	return moTCPConnectPool.mpTCPMgr->SendData(apstruSendData);
}

INT CTCPConnPoolMgr::SendData(void* apSocketInfo,const SESSIONID & aiUserID,STRU_SENDDATA_BASE *apstruSendData)
{
	int32 liIndex = aiUserID %  moTCPConnectPool.miConnectPoolNum;
	STRU_SENDDATA_TCP * lpSendData = static_cast< STRU_SENDDATA_TCP * >(apstruSendData);
	
	lpSendData->mhSocket = moTCPConnectPool.GetSocket(liIndex);
	if(INVALID_SOCKET ==lpSendData->mhSocket)
		return 0;
	
	return moTCPConnectPool.mpTCPMgr->SendData(apSocketInfo,apstruSendData);
}


INT CTCPConnPoolMgr::GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo)
{
	return moTCPConnectPool.GetNetStatisticsInfo(astruInfo);
};
// $_FUNCTION_BEGIN ******************************
// 函数名称： OnSendDataError
// 函数参数：  char* apData 数据
//			 WORD awLen,apData长度
//			,IPTYPE aiIP,对方地址
//			WORD awPort 对方端口
// 返 回 值： 1是成功， -1是失败
// 函数说明： 从发送该数据失败
// $_FUNCTION_END ********************************
INT CTCPConnPoolMgr::OnSendDataError(IPTYPE aiIP,
							WORD awPort, BYTE* apData,WORD awLen)
{
	return 0;
}


// $_FUNCTION_BEGIN ******************************
// 函数名称： OnError
// 函数参数：INT aiProtocolType:0是uc 1是us
//			UINT aiErrType 错误类型
// 返 回 值： 1是成功 -1是失败
// 函数说明： 出错处理
// $_FUNCTION_END ********************************
INT CTCPConnPoolMgr::OnError(UINT aiErrType)
{
	return 0;
}


// $_FUNCTION_BEGIN ******************************
// 函数名称： CheckSessionEvent
// 函数参数：bool abIsNormal 是否到正常检测时间
// 返 回 值： 无
// 函数说明： 检查处理事件
// $_FUNCTION_END ********************************
VOID CTCPConnPoolMgr::CheckEvent(bool abIsNormal)
{
	//moTCPConnectPool.ReConnectCheck();
	return ;
}


// $_FUNCTION_BEGIN ******************************
// 函数名称： OnSessionClose
// 函数参数：session连接关闭
// 返 回 值： 无
// 函数说明： 连接关闭通知
// $_FUNCTION_END ********************************
void CTCPConnPoolMgr::OnSessionClose(const SESSIONID & aiPeerID,SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	mpInterface->OnTCPPoolSessionClose(aiPeerID,ahSocket,aiIP,awPort);
	moTCPConnectPool.Disconnect(ahSocket);
}


// $_FUNCTION_BEGIN ******************************
// 函数名称： OnSessionClose
// 函数参数：session连接关闭
// 返 回 值： 无
// 函数说明： 连接关闭通知
// $_FUNCTION_END ********************************
void CTCPConnPoolMgr::OnSessionCloseEx(const SESSIONID & aiPeerID,void* apSocketInfo,SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	mpInterface->OnTCPPoolSessionCloseEx(aiPeerID,apSocketInfo,ahSocket,aiIP,awPort);
	moTCPConnectPool.Disconnect(ahSocket);
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： OnSessionConnected
// 函数参数：session连接成功
// 返 回 值： 无
// 函数说明： 新连接通知
// $_FUNCTION_END ********************************
void CTCPConnPoolMgr::OnSessionConnected(const SESSIONID & aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	mpInterface->OnTCPPoolSessionConnected(aiPeerID,ahSocket,aiIP,awPort);
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： OnSessionConnectedEx
// 函数参数：session连接成功
// 返 回 值： 无
// 函数说明：  新连接通知
// $_FUNCTION_END ********************************
void CTCPConnPoolMgr::OnSessionConnectedEx(const SESSIONID & aiPeerID,void* apSocketInfo , SOCKET ahSocket,IPTYPE aiIP,WORD awPort)	
{
	mpInterface->OnTCPPoolSessionConnectedEx(aiPeerID,apSocketInfo,ahSocket,aiIP,awPort);
}

void CTCPConnPoolMgr::OnSessionConnectFailure(const SESSIONID & aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	//mpInterface->OnSessionConnectFailure(aiPeerID,ahSocket,aiIP,awPort);
}

INT CTCPConnPoolMgr::OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
{
	if(NULL != mpInterface)
		return mpInterface->OnTCPPoolTransRecvData(ahSocket,aiIP,awPort,apData,awLen);
	return 1;
}

INT CTCPConnPoolMgr::OnNewRecvData(SOCKET ahSocket, void* apapRelationPtr,IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
{
	if(NULL != mpInterface)
		return mpInterface->OnTCPPoolTransNewRecvData(ahSocket,apapRelationPtr,aiIP,awPort,apData,awLen);
	return 1;
}

bool CTCPConnPoolMgr::JudgeIsConnectServer(IPTYPE aiIP, WORD awPort)
{
	return moTCPConnectPool.JudgeIsConnectServer(aiIP, awPort);
}