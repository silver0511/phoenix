#include "stdafx.h"
#include "const/ErrorCodeDef.h"
#include "GlobalResource.h"
#include "network/packet/client/SysPackDef.h"
#include "network/packet/client/UserPackDef.h"
#include "Config.h"
#include "ClientSide.h"
#include "../ServerMgr.h"
#include "ServerUtilMgr.h"


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

    ULONG start_time = CSystem::GetSystemTime();
    int platform = mpServerMgr->GetApptype() == "app" ?  PLATFORM_APP : PLATFROM_WEB ;
    for (int i = 0; i < LOGIN_MOD_COUNT; i++)
    {
         std::string strCursor("0");
         std::map<std::string, std::string> userid_serverid_map;
         
        do
        {
            userid_serverid_map.clear();
            mpServerMgr->mpRedisUtil->HsanLogininfo(i, platform, strCursor,10000, &userid_serverid_map);
            
            for (auto iter = userid_serverid_map.begin();iter != userid_serverid_map.end(); iter++)
            {
                moRefCriticaluseridinfomap.Enter();
                USERID userid = ServerUtilMgr::ChangeStringToUll(iter->first);
                SESSIONID sessionid = ServerUtilMgr::ChangeStringToUll(iter->second);
                TLogInfo tloginfo;
                tloginfo.t_logincnt = 0;   //One type can only be one user
                tloginfo.t_serverid = sessionid;
                if (m_userid_loginfo.count(userid) == 0)
                {
                    m_userid_loginfo.insert(u9_hash_map<USERID, TLogInfo>::value_type(userid, tloginfo));
                }
                else
                {
                    //update session
                    m_userid_loginfo[userid] = tloginfo;
                }
                moRefCriticaluseridinfomap.Leave();
            }       
        }while (strCursor != "0");
        
    }   

    ULONG endtime = CSystem::GetSystemTime();
    LOG_TRACE(LOG_ALERT,true,__FUNCTION__,"load sessionid success time:" << endtime-start_time);
	
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
                         << "onlinecnt " << mpServerMgr->GetDealConsumeOnlineCnt()
                         << "apnscnt " << mpServerMgr->GetDealConsumeApnsCnt()
                         << " onlinesuer " << m_userid_loginfo.size());
	
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
    int iTimesLand = 0;

	if (loHeadPackage.mwPackType == NEW_DEF_ACK_ID)
	{
		LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Login" 
										   << " IP=" << GetIPString(aiIP)
										   << ":" << ntohs(awPort)
										   << " ServerId=" << loHeadPackage.miCometID);

        moRefCriticalSocketVec.Enter();
        if (0 == m_server_socket_map.count(loHeadPackage.miCometID))
        {
            std::vector<SOCKET> socketvec;
            socketvec.push_back(ahSocket);
            m_server_socket_map[loHeadPackage.miCometID] = socketvec;
        }
        else
        {
            m_server_socket_map[loHeadPackage.miCometID].push_back(ahSocket);
        }
        moRefCriticalSocketVec.Leave();	
	}	
    else if (loHeadPackage.mwPackType == NEW_DEF_ROUTE_SYNC_ID)
    {
        BYTE  bType = *((BYTE*)apData);        
        moRefCriticaluseridinfomap.Enter();
        if (bType == LOG_IN_TYPE)
        {
            TLogInfo tloginfo;
            if (m_userid_loginfo.count(loHeadPackage.miUserID) == 0)
            {
                
                tloginfo.t_logincnt = 1;
                tloginfo.t_serverid = loHeadPackage.miCometID;
                m_userid_loginfo.insert(u9_hash_map<USERID, TLogInfo>::value_type(loHeadPackage.miUserID, tloginfo));
            }
            else
            {
                tloginfo = m_userid_loginfo[loHeadPackage.miUserID];
                tloginfo.t_logincnt++;
                tloginfo.t_serverid = loHeadPackage.miCometID;
                m_userid_loginfo[loHeadPackage.miUserID] = tloginfo;
            }
            iTimesLand = tloginfo.t_logincnt;
        }
        else
        {
            auto iter = m_userid_loginfo.find(loHeadPackage.miUserID);
            TLogInfo tloginfo;
            if (iter != m_userid_loginfo.end())
            {
                tloginfo = m_userid_loginfo[loHeadPackage.miUserID];
                tloginfo.t_logincnt--;

                if (tloginfo.t_logincnt == 0)
                {
                    m_userid_loginfo.erase(iter);
                }
                else
                {
                    m_userid_loginfo[loHeadPackage.miUserID] = tloginfo;
                }
            }
            iTimesLand = tloginfo.t_logincnt;
        }
        moRefCriticaluseridinfomap.Leave();

        LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "User Login" 
    							   << " userid=" << loHeadPackage.miUserID										  
    							   << " ServerId=" << loHeadPackage.miCometID
                                   << " type=" << bType
                                   << "landtimes" << iTimesLand
                                   << " onlinecnt=" << m_userid_loginfo.size());
    }
}

INT CClientSide::OnNewRecvData(SOCKET ahSocket,  void* apRelationPtr, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
{
	//consumerserver 不需要关联其他指针
	
	LOG_TRACE(LOG_ERR, false, __FUNCTION__, "OnNewRecvData failed");		
	return -1;
}

bool CClientSide::Send2Comet(uint32 threadid, const SERVERID & serverid , BYTE * pdata,uint32 uidatalen)
{	
	SOCKET socket = FindRemoteSocketIdbyServerId(threadid, serverid);

	if (0xFFFFFFFFFFFFFFFF == socket)
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "serverid not match socketid, serverid:" << serverid);
		return false;
	}		

	//send to remote server
	if (RET_SUCCESS != ClientPack::GetInstance().Send2Comet(socket, pdata, uidatalen))
	{
        return false;
	}	
	return true;
}

SOCKET CClientSide::FindRemoteSocketIdbyServerId(uint32 threadid, const SERVERID & serverid)
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

void CClientSide::Getuseridlogins(std::vector<USERID> & userids, 
                             std::multimap<SERVERID,USERID>& szappserverid_userid_map,                                                  
                             std::set<SERVERID>& szappserverids,
                             USERID* pofflineuserlist,
                             uint16& offlineusercnt,
                             const std::string& apptype)
{
    if (userids.size() == 0)
    {
        return;
    }

    std::string onlineuser("[");
    std::string offlineuser("[");
    
    moRefCriticaluseridinfomap.Enter();
    for (auto iter1 = userids.begin(); iter1 != userids.end(); iter1++)
    {
        auto iter2 = m_userid_loginfo.find(*iter1);
        if(iter2 != m_userid_loginfo.end())
        {
            szappserverid_userid_map.insert(std::make_pair((iter2->second).t_serverid,*iter1));
            if (0 == szappserverids.count((iter2->second).t_serverid))
            {
            	szappserverids.insert((iter2->second).t_serverid);
            }
            onlineuser += std::to_string(*iter1) + ";";
        }
        else if (apptype == "app" )
        {
            pofflineuserlist[offlineusercnt] = *iter1;
            offlineusercnt++;
            offlineuser += std::to_string(*iter1) + ";";
        }
    }
    moRefCriticaluseridinfomap.Leave();

    onlineuser +="]";
    offlineuser += "]";

    
    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "onlineuser:" << onlineuser);
    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "offlineuser:" << offlineuser);
    return;    
}


