#include "stdafx.h"
#include "const/ErrorCodeDef.h"

#include "ClientSide.h"
#include "../ServerMgr.h"
// -----------------------------

CClientSide::CClientSide(CServerMgr *apoServerMgr)
{
	assert(apoServerMgr);
	IntiFuncMap();
	mpServerMgr = apoServerMgr;
	mbOpen = false;
}

CClientSide::~CClientSide()
{
	mbOpen = false;
}

bool CClientSide::open()
{
	STRU_OPTION_TCP lstruTcpOpt;
	strncpy(lstruTcpOpt.mszAppName, g_server_name.c_str(),DEF_MAX_APP_NAME);
	lstruTcpOpt.miLogLevel = mpServerMgr->moConfig.miTCPLogLevel;
	lstruTcpOpt.mbIsServer = TRUE;
	lstruTcpOpt.muiMaxConnCount =   mpServerMgr->moConfig.miMaxConnCount;
	lstruTcpOpt.mulIPAddr = goLocalServerInfo.mlServerIP;
	lstruTcpOpt.mwPort = goLocalServerInfo.mwServerPort;
	lstruTcpOpt.mbyDealThreadCount =  mpServerMgr->moConfig.miDealThreadCount;
	lstruTcpOpt.mbyRecvThreadCount = mpServerMgr->moConfig.miRecvThreadCount;

	lstruTcpOpt.miMaxRecvPackageCount = mpServerMgr->moConfig.miMaxRecvPackageCount;
	lstruTcpOpt.miMaxSendPackageCount = mpServerMgr->moConfig.miMaxSendPackageCount;
	lstruTcpOpt.mbUseRelationPtr = true;
    lstruTcpOpt.mbTls = mpServerMgr->moConfig.mbTls;
	
	//lstruTcpOpt.muiKeepAlive = 0;
	lstruTcpOpt.muiKeepAlive = 5 * 5;//60;
	//lstruTcpOpt.muiKeepAlive = mpServerMgr->moConfig.miLoginTimeout;
	if(!ClientPack::GetInstance().Open(this, lstruTcpOpt, ENUM_CONNECT_MODE_HTTP))
	{
		LOG_TRACE(LOG_CRIT,true,__FUNCTION__," Initialize tcp error.");
		return false;
	}

	printf("CClientSide::open end ip:%s, port:%d\n", GetIPString(lstruTcpOpt.mulIPAddr).c_str(), lstruTcpOpt.mwPort);
	mbOpen = true;
	return true;
}


INT CClientSide::OnSendDataError(IPTYPE aiIP,WORD awPort, BYTE* apData,WORD awLen)
{
	return 0;
}

INT CClientSide::OnError(UINT aiErrType)
{
	return 0;
}

//
VOID CClientSide::CheckEvent(bool abIsNormal)
{
	//LOG_TRACE(4, true, __FUNCTION__, " time"
	//	<<" abIsNormal="<<abIsNormal);

	if(abIsNormal != true)
		return ;
	if(mpServerMgr == NULL)
		return;
	mpServerMgr->OnTimeCheck();

	ClientPack::GetInstance().DisplayStatisticsInfo();
}


//TCP连接建立
void CClientSide::OnSessionConnected(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Connected "
		<<" IP=" << GetIPString(aiIP)<< ":"<< awPort);
}

//TCP连接建立
void CClientSide::OnSessionConnectedEx(const SESSIONID& aiPeerID,void* apSocketInfo, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Connected "
		<<" IP=" << GetIPString(aiIP)<< ":"<< awPort
		<<" SocketInfoPtr="<< apSocketInfo
		<<" aiPeerID = " << (SESSIONID)aiPeerID
		);

	CUser *lpUser = mpServerMgr->moUserMgr.MallocRefObj();

	lpUser->mhSocket = ahSocket;
	lpUser->miIPAddr = aiIP;
	lpUser->miIPPort = awPort;
	lpUser->mpSocketInfo = apSocketInfo;
	lpUser->miUserID = aiPeerID;
	if(!mpServerMgr->moUserMgr.addUser(lpUser))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "add user failed ip = " << GetIPString(aiIP)
					<< " port = " << awPort);
		return ;
	}
		
	lpUser->IncreaseRef();
    ClientPack::GetInstance().AddRelationPtr(apSocketInfo,(void*)lpUser);
}

void CClientSide::OnSessionConnectFailure(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_ERR, true, __FUNCTION__, "TCP Connected "
		<<" IP=" << GetIPString(aiIP)<< ":"<< awPort);
}
//客户端关闭连接
void CClientSide::OnSessionClose(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Disconnected "
		<<" Errcode="<<aiPeerID
		<<" IP=" << GetIPString(aiIP)<< ":"<< awPort);
}
void CClientSide::OnSessionCloseEx(const SESSIONID & aiPeerID,void*apRelationPtr, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Disconnected " << " apRelationPtr = " << apRelationPtr);
	CUser *lpUser = (CUser*)apRelationPtr;
	CAutoReleaseRefObjMgrTemplate2<PHONEID,CUser>  loUser(mpServerMgr->moUserMgr, lpUser);
	{
		CRITICAL_SECTION_HELPER(lpUser->GetCriticalSection());
        ClientPack::GetInstance().DelRelationPtr(lpUser->mpSocketInfo,lpUser);
		lpUser->mpSocketInfo =  NULL;
	}
	
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, " delete user"
            << ",ahSocket=" << ahSocket
            <<" ,IP="<< GetIPString(aiIP)
            <<":"<< awPort
            <<" SocketInfoPtr="<< lpUser->mpSocketInfo
            <<" lpUserPtr="<< (void*)(lpUser)
            <<" UserID="<< lpUser->miUserID
            <<" RefCount="<< lpUser->GetRefCount()
            << "LoginCount=" << lpUser->miLoginCount
		);
	
	mpServerMgr->moUserMgr.delUser(lpUser);
}

// 包关联
void CClientSide::IntiFuncMap()
{
	//U9_NET_PACKET_INIT();
}

INT CClientSide::onRecvTransmitRQ(WORD awPackType, SOCKET ahSocket,void* apRelationPtr,IPTYPE aiIP, WORD awPort, BYTE * apData, WORD awLen)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "recv "
		<< " onRecvTransmitRQ enter" << " awPackType = " 
		<< awPackType << " apRelationPtr = " << apRelationPtr);

	CLIENT_CHECK_LOGIN();


	STRU_TRANS_BODY_PACKAGE trans_body;
	trans_body.mpBuffer = apData;
	trans_body.mwLen = awLen;
	return ServerPack::GetInstance().Send2LogicServer(awPackType, trans_body, goLocalServerInfo.mlServerID, lpUser->miUserID);
}