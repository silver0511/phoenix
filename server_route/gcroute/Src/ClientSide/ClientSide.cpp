#include "stdafx.h"
#include "const/ErrorCodeDef.h"
#include "GlobalResource.h"
#include "network/packet/client/SysPackDef.h"
#include "network/packet/client/UserPackDef.h"
#include "Config.h"
#include "ClientSide.h"
#include "../ServerMgr.h"

CClientSide::CClientSide(CServerMgr *apoServerMgr)
{
	mpServerMgr = apoServerMgr;
	mbOpen = false;
}

CClientSide::~CClientSide()
{
	mbOpen = false;
}

bool CClientSide::open()
{
	printf("CClientSide::open begin\n");
	
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
	lstruTcpOpt.mbUseRelationPtr = false;
	lstruTcpOpt.mbyEpollCount = mpServerMgr->moConfig.miEpollCount;
	lstruTcpOpt.muiKeepAlive = 0;

	if(!ClientPack::GetInstance().Open(this, lstruTcpOpt, ENUM_CONNECT_MODE_TCP))
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," Initialize tcp error.");
		return false;
	}

	printf("CClientSide::open end ip:%s, port:%d\n", GetIPString(lstruTcpOpt.mulIPAddr).c_str(), ntohs(lstruTcpOpt.mwPort));
	
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
 
VOID CClientSide::CheckEvent(bool abIsNormal)
{
	if(abIsNormal != true)
	{
		return ;
	}

	if(NULL != mpServerMgr)
	{
		mpServerMgr->OnTimeCheck();
	}

	static uint32 siSecCount = 0;

	if (0 != mpServerMgr->moConfig.m_statinterval)
	{
		if (siSecCount > mpServerMgr->moConfig.m_statinterval)
		{
			Statistics();
			siSecCount = 0;
		}
		else
		{
			siSecCount++;
		}
	}

	ClientPack::GetInstance().DisplayStatisticsInfo();
}

VOID CClientSide::Statistics()
{
	if (!mpServerMgr)
	{
		return;
	}

	LOG_TRACE(LOG_CRIT, true,  "",  " queuecnt " << mpServerMgr->GetQueueMsgCnt()
						 << " kafakasuccnt " << mpServerMgr->GetKafkaConsumeSucCnt()
						 << " kafakafailcnt " << mpServerMgr->GetKafkaConsumeFailCnt()
                         << " onlinecnt" << mpServerMgr->GetDealConsumeOnlineCnt()
                         << " apnscnt" << mpServerMgr->GetDealConsumeApnsCnt());
	
	return;
}

//TCP连接建立
void CClientSide::OnSessionConnected(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Connected " 
                                   << " IP=" << GetIPString(aiIP)
                                   << ":" << ntohs(awPort)
	                               << " Socket=" << ahSocket);

}

//TCP连接建立
void CClientSide::OnSessionConnectedEx(const SESSIONID& aiPeerID,void* apSocketInfo, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	return;	
}

void CClientSide::OnSessionConnectFailure(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Connected " 
		                               <<" IP=" << GetIPString(aiIP)
                                       << ":"<< ntohs(awPort)
		                               <<" Socket="<< ahSocket);
}

//客户端关闭连接
void CClientSide::OnSessionClose(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Disconnected "
		                               <<" Errcode="<<aiPeerID
		                               <<" IP=" << GetIPString(aiIP)
                                       << ":"<< ntohs(awPort)
		                               << " Socket="<< ahSocket);

	

	moRefCriticalSocketVec.Enter();
    for (auto it1 =  m_server_socket_map.begin(); it1 != m_server_socket_map.end(); it1++)
    {
        for (auto it2 = (it1->second).begin(); it2 != (it1->second).end(); it2++)
        {
            if ( *it2 == ahSocket)
            {
                (it1->second).erase(it2);
                break;
            }
        }

        if ( 0 == (it1->second).size() )
        {
            m_server_socket_map.erase(it1);
            break;
        }
    }	
	moRefCriticalSocketVec.Leave();
}
void CClientSide::OnSessionCloseEx(const SESSIONID & aiPeerID,void*apRelationPtr, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	return;	
}

INT CClientSide::OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
{

	COMET_2_BACK_UNPACK_HEAD()

	if (loHeadPackage.mwPackType == NEW_DEF_ACK_ID)
	{
		LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Login" 
										   << " IP=" << GetIPString(aiIP)
										   << ":" << ntohs(awPort)
										   << " ServerId=" << loHeadPackage.miCometID);

        std::string szserverid = std::to_string(loHeadPackage.miCometID);

        moRefCriticalSocketVec.Enter();
        if (0 == m_server_socket_map.count(szserverid))
        {
            std::vector<SOCKET> socketvec;
            socketvec.push_back(ahSocket);
            m_server_socket_map[szserverid] = socketvec;
        }
        else
        {
            m_server_socket_map[szserverid].push_back(ahSocket);
        }
        moRefCriticalSocketVec.Leave();	
	}	
}

INT CClientSide::OnNewRecvData(SOCKET ahSocket,  void* apRelationPtr, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
{
	//consumerserver 不需要关联其他指针
	
	LOG_TRACE(LOG_ERR, false, __FUNCTION__, "OnNewRecvData failed");		
	return -1;
}

bool CClientSide::Send2Comet(uint32 threadid, const std::string & szserverid , BYTE * pdata,uint32 uidatalen)
{	
	SOCKET socket = FindRemoteSocketIdbyServerId(threadid, szserverid);

	if (0xFFFFFFFFFFFFFFFF == socket)
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "serverid not match serverid:" << szserverid);
		return false;
	}		

	//send to remote server
	if (RET_SUCCESS != ClientPack::GetInstance().Send2Comet(socket, pdata, uidatalen))
	{
        return false;
	}	
	return true;
}

SOCKET CClientSide::FindRemoteSocketIdbyServerId(uint32 threadid, const std::string & serverid)
{
    SOCKET dstsocket = 0xFFFFFFFFFFFFFFFF;
    if ( 0 == m_server_socket_map.count(serverid))
    {
        return 0xFFFFFFFFFFFFFFFF;
    }

	moRefCriticalSocketVec.Enter();    
    std::vector<SOCKET> sockets = m_server_socket_map[serverid];

    if ( 0 == sockets.size())
    {
        return 0xFFFFFFFFFFFFFFFF;
    }

    uint32 index = 0;

    if (threadid > sockets.size())
    {
        threadid = sockets.size();
    }

    if ( 0 != threadid )
    {
        index = sockets.size() % threadid;
    }

    dstsocket =  sockets[index];
	moRefCriticalSocketVec.Leave();

    return dstsocket;
}


