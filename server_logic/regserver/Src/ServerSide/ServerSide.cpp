// $_FILEHEADER_BEGIN ***************************
// 版权声明: xxxxx版权所有
//			  
// 文件名称: ServerSide.cpp
// 创 建 人: 史云杰
// 文件说明: 处理客户端链接处理器
// $_FILEHEADER_END *****************************

#include "stdafx.h"

#include "network/packet/PackTypeDef.h"
#include "ServerSide.h"
#include "../ServerMgr.h"
#include "network/packet/client/SysPackDef.h"

// --------------------------------------------------------

CServerSide::CServerSide(CServerMgr* apoServerMgr)
{
	U9_ASSERT(apoServerMgr);
	mpServerMgr = apoServerMgr;

	InitCallBack();
}


CServerSide::~CServerSide()
{
}

void CServerSide::InitCallBack()
{
}

bool CServerSide::open()
{
	return true;
}


INT CServerSide::OnTCPPoolTransRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort, 
						   BYTE* apData,WORD awLen)
{
	return OnRecvData(ahSocket,aiIP,awPort,apData,awLen);
}

INT CServerSide::OnTCPPoolTransNewRecvData(SOCKET ahSocket, void *apRelationPtr,IPTYPE aiIP,WORD awPort, 
						   BYTE* apData,WORD awLen)
{
	return OnRecvData(ahSocket,aiIP,awPort,apData,awLen);
}

void CServerSide::OnTCPPoolSessionClose(const SESSIONID& aiPeerID,SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{ 
	LOG_TRACE(LOG_DEBUG, 0, __FUNCTION__, "Error TCP distconnect"
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}
void CServerSide::OnTCPPoolSessionCloseEx(const SESSIONID & aiPeerID,void* apSocketInfo,SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, 0, __FUNCTION__, "Error TCP distconnect"
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}
void CServerSide::OnTCPPoolSessionConnected(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, 0, __FUNCTION__, "TCP connect"
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}

void CServerSide::OnTCPPoolSessionConnectedEx(const SESSIONID& aiPeerID, void* apSocketInfo,SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, 0, __FUNCTION__, "TCP connect"
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}