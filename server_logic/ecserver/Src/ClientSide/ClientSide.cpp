#include "stdafx.h"
#include "const/ErrorCodeDef.h"
#include "ClientSide.h"
#include "../ServerMgr.h"
#include "network/packet/client/CommonPackDef.h"
#include "EcRecentMsgTask.h"
#include "EcLastMsgTask.h"
#include "EcRecentOrdersTask.h"
#include "ServerUtilMgr.h"


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
	lstruTcpOpt.muiKeepAlive = 0;
    
    LOG_TRACE(LOG_ALERT,true,__FUNCTION__," init task cnt=" << mpServerMgr->moConfig.m_taskthreadcnt);
    if (!m_taskthreadpoll.Init(mpServerMgr->moConfig.m_taskthreadcnt))
    {
        LOG_TRACE(LOG_ERR,false,__FUNCTION__," Initialize taskthreadpoll error.");
        return false;
    }

	if(!ClientPack::GetInstance().Open(this, lstruTcpOpt, ENUM_CONNECT_MODE_TCP))
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," Initialize tcp error.");
		return false;
	}

	LOG_TRACE(LOG_NOTICE,true,__FUNCTION__," clientside init ok");

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
    
    static uint32 siSecCount = 0;
    
    if (0 != mpServerMgr->moConfig.m_statinterval)
    {
        if (siSecCount > mpServerMgr->moConfig.m_statinterval)
        {
             for (int i = 0; i < mpServerMgr->moConfig.m_taskthreadcnt; i++)
             {
                    if (m_taskthreadpoll.GetThreadQueueSize(i) > 10000)
                    {
                        LOG_TRACE(LOG_CRIT, true, __FUNCTION__, "task"<< i <<  "queuecnt"  <<  m_taskthreadpoll.GetThreadQueueSize(i));        
                     } 
             }
             siSecCount = 0;
        }
        else
        {
            siSecCount++;
        }
    }   
}


//TCP连接建立
void CClientSide::OnSessionConnected(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Connected " 
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}

//TCP连接建立
void CClientSide::OnSessionConnectedEx(const SESSIONID& aiPeerID,void* apSocketInfo, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	CUser *lpUser = mpServerMgr->moUserConMgr.MallocRefObj();

	lpUser->mhSocket = ahSocket;
	lpUser->miIPAddr = aiIP;
	lpUser->miIPPort = awPort;
	lpUser->mpSocketInfo = apSocketInfo;
	lpUser->IncreaseRef();

	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Connected "
			<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
			<<" Socket="<< ahSocket
			<<" SocketInfoPtr="<< apSocketInfo
			<<" UserRefCount="<< lpUser->GetRefCount()
	);
	ClientPack::GetInstance().AddRelationPtr(apSocketInfo,(void*)lpUser);
}

void CClientSide::OnSessionConnectFailure(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Connected " 
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}
//客户端关闭连接
void CClientSide::OnSessionClose(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Disconnected "
		<<" Errcode="<<aiPeerID
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<< " Socket="<< ahSocket
		);
}
void CClientSide::OnSessionCloseEx(const SESSIONID & aiPeerID,void*apRelationPtr, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	CUser *lpUser = (CUser*)apRelationPtr;
	mpServerMgr->moUserConMgr.delUser(lpUser);
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "TCP Disconnected  delete user"
			<< "ahSocket=" << ahSocket
			<<" ,IP="<< GetIPString(aiIP)
			<<":"<< ntohs(awPort)
			<<" SocketInfoPtr="<< lpUser->mpSocketInfo
			<<" lpUserPtr="<< (void*)(lpUser)
	);
}

INT CClientSide::OnNewRecvData(SOCKET ahSocket, void* apRelationPtr,IPTYPE aiIP,WORD awPort, BYTE* apData,WORD awLen)
{
	CHECK_INVALID()
	COMET_2_BACK_UNPACK_HEAD()

    if(loHeadPackage.mwPackType == NEW_DEF_ACK_ID)
	{
		lpUser->miCometID = loHeadPackage.miCometID;
		mpServerMgr->moUserConMgr.addUser(lpUser);
		return 1;
	}

	TCP_PACKET_DISTRIBUTE(loHeadPackage.mwPackType, loHeadPackage.miUserID, loHeadPackage.miCometID)
}

// 包关联
void CClientSide::IntiFuncMap()
{
    TCP_PACKET_INIT()
}

INT CClientSide::GetFreeWaiterRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
    typedef std::multimap<MOD, USERID>::iterator multiMapItor;
	STRU_PACKAGE_RS<T_EC_GETFREEWAITER_RS> loSendPack(NEW_DEF_BUSINESS_GETFREEWAITER_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_GETFREEWAITER_RQ, aiUserID, loSendPack.GetPackType())
	auto t_fb_data = loUserLoad.t_fb_data;
  
    USERID ulbid = t_fb_data->b_id();
    USERID ulcid = t_fb_data->s_rq_head()->user_id();
    USERID uldstwid = ulbid;
    USERID ullastwid = 0;
    std::string szlastwid("");

    std::vector<USERID> ulwidlist;
    std::vector<USERID> ulloginwidlist;  

     
    std::string srcwidlist("[");
    const flatbuffers::Vector<uint64_t> * s_widlist =  t_fb_data->w_id_list();
    for (flatbuffers::uoffset_t i = 0; i < s_widlist->size(); i++)
    {
        USERID ulwid = (USERID) s_widlist->Get(i);
        if (ulwid != ulcid && ulwid != ulbid)
        {
            ulwidlist.push_back(ulwid);
            srcwidlist += std::to_string(ulwid) + ";";
        }
    }
    srcwidlist += "]";

    if (0 == ulwidlist.size())
    {
        goto endprocess;
    }

    LOG_TRACE(LOG_INFO, true, __FUNCTION__, "widlist is " << srcwidlist);

    GetLoginWaiter(ulwidlist, ulloginwidlist);

    if (0 == ulloginwidlist.size())
    {
        LOG_TRACE(LOG_INFO, true, __FUNCTION__,"login widlist is null");
        goto endprocess;
    }    

    mpServerMgr->mpRedisUtil->GetTradeLastWid(ulcid, ulbid, szlastwid);
    ullastwid = ServerUtilMgr::STR2UINT64(szlastwid);

    if (find(ulloginwidlist.begin(), ulloginwidlist.end(), ullastwid) != ulloginwidlist.end())
    {
        LOG_TRACE(LOG_INFO, true, __FUNCTION__,"lastwid is selected");
        uldstwid = ullastwid;
    }
    else
    {
        //need redistribution
        INT index = GetRandom() % ulloginwidlist.size();
        uldstwid = ulloginwidlist[index];  
    }        


endprocess:
    std::string szsessionid("");
    mpServerMgr->mpRedisUtil->GetTradeLastSessionId(ulcid, ulbid, szsessionid);
    
    int ilsessionid = 0;
    if (!szsessionid.empty())
    {
        ilsessionid = ServerUtilMgr::STR2UINT64(szsessionid);
    }
    else
    {
        ilsessionid = time(NULL);
    }

    std::string szpromptsmsg("");
    if (IsUserLogin(ullastwid))
    {
        mpServerMgr->mpRedisUtil->GetBusinessPrompts(ulbid, LOG_IN_TYPE, szpromptsmsg);
    }
    else
    {
        mpServerMgr->mpRedisUtil->GetBusinessPrompts(ulbid, LOG_OUT_TYPE, szpromptsmsg);
    }   

    LOG_TRACE(LOG_INFO, true, __FUNCTION__, "bid="<< ulbid << " cid=" << ulcid << " wid=" << uldstwid << " sessionid=" << ilsessionid << "promptsmsg=" << szpromptsmsg);
    mpServerMgr->mpRedisUtil->SetTradeLastWid(ulcid, ulbid, uldstwid, ilsessionid);
    int liResult = MAKE_SUCCESS_RESULT();
    commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(), t_fb_data->s_rq_head()->pack_session_id(),
								liResult, t_fb_data->s_rq_head()->platform());

    auto fbspromptsmsg = loSendPack.fbbuilder.CreateString(szpromptsmsg.c_str()); 
    ecpack::T_EC_GETFREEWAITER_RSBuilder c1 = ecpack::T_EC_GETFREEWAITER_RSBuilder(loSendPack.fbbuilder);
	c1.add_s_rs_head(&s_rs_head);
    c1.add_b_id(ulbid);
    c1.add_w_id(uldstwid);
    c1.add_session_id(ilsessionid);
    c1.add_promtpsmsg(fbspromptsmsg);
	loSendPack.fbbuf = c1.Finish();

	return Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);	
}

INT CClientSide::GetRecentMsgsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	STRU_PACKAGE_RS<T_EC_GETRECENTMSGS_RS> loSendPack(NEW_DEF_BUSINESS_GETRECENTMSGS_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_GETRECENTMSGS_RQ, aiUserID, loSendPack.GetPackType())

	auto t_fb_data = loUserLoad.t_fb_data;

     CEcRecentMsgTask *pecrecntmsgtask = new CEcRecentMsgTask(aiCometID, 
                                                                 aiUserID, 
                                                                 t_fb_data->s_rq_head()->pack_session_id(), 
                                                                 t_fb_data->s_rq_head()->platform());

    if (NULL == pecrecntmsgtask)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new pecrecntmsgtask failed!");
        return -1;
    }

    pecrecntmsgtask->SetBId(t_fb_data->b_id());
    pecrecntmsgtask->SetCId(t_fb_data->c_id());
    pecrecntmsgtask->SetMsgId(t_fb_data->message_id());
    pecrecntmsgtask->SetMaxCnt(t_fb_data->max_cnt());

    std::string szdata = "user_id=" + std::to_string(aiUserID) +                         
                          "&b_id="   + std::to_string(t_fb_data->b_id()) +
                          "&c_id="   + std::to_string(t_fb_data->c_id()) +
                          "&message_id=" + std::to_string(t_fb_data->message_id()) +
                          "&max_cnt=" + std::to_string(t_fb_data->max_cnt());
    
    pecrecntmsgtask->SetData(szdata);
    pecrecntmsgtask->SetClientSide(this);
    m_taskthreadpoll.AddTask(pecrecntmsgtask);

    return 1;	
}


INT CClientSide::GetLastMsgsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	STRU_PACKAGE_RS<T_EC_GETRECENTMSGS_RS> loSendPack(NEW_DEF_BUSINESS_GETLASTMSGS_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_GETLASTMSGS_RQ, aiUserID, loSendPack.GetPackType())

	auto t_fb_data = loUserLoad.t_fb_data;

     CEcLastMsgTask *peclastmsgtask = new CEcLastMsgTask(aiCometID, 
                                                                 aiUserID, 
                                                                 t_fb_data->s_rq_head()->pack_session_id(), 
                                                                 t_fb_data->s_rq_head()->platform());

    if (NULL == peclastmsgtask)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new peclastmsgtask failed!");
        return -1;
    }

    peclastmsgtask->SetBId(t_fb_data->b_id());
    peclastmsgtask->SetOffset(t_fb_data->offset());
    peclastmsgtask->SetMaxCnt(t_fb_data->max_cnt());

    std::string szdata = "user_id=" + std::to_string(aiUserID) +                         
                          "&b_id="   + std::to_string(t_fb_data->b_id()) + 
                          "&offset=" + std::to_string(t_fb_data->offset()) +
                          "&max_cnt=" + std::to_string(t_fb_data->max_cnt());
                          
    peclastmsgtask->SetData(szdata);
    peclastmsgtask->SetClientSide(this);
    m_taskthreadpoll.AddTask(peclastmsgtask);

    return 1;	
}

INT CClientSide::GetRecentOrdersRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
    STRU_PACKAGE_RS<T_EC_GETRECENTORDERS_RS> loSendPack(NEW_DEF_BUSINESS_GETRECENTORDERS_RS);
    TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_GETRECENTORDERS_RQ, aiUserID, loSendPack.GetPackType())

    auto t_fb_data = loUserLoad.t_fb_data;	

    CEcRecentOrdersTask *pecrecntorderstask = new CEcRecentOrdersTask(aiCometID, 
                                                                 aiUserID, 
                                                                 t_fb_data->s_rq_head()->pack_session_id(), 
                                                                 t_fb_data->s_rq_head()->platform());

    if (NULL == pecrecntorderstask)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new pecrecntmsgtask failed!");
        return -1;
    }

    pecrecntorderstask->SetSellerId(t_fb_data->sellerId());
    pecrecntorderstask->SetBuyerId(t_fb_data->buyerId());

    std::string szdata = "userid=" + std::to_string(aiUserID) +                         
                          "&sellerId="   + std::to_string(t_fb_data->sellerId()) +
                          "&buyerId="   + std::to_string(t_fb_data->buyerId());
  
    pecrecntorderstask->SetData(szdata);
    pecrecntorderstask->SetClientSide(this);
    m_taskthreadpoll.AddTask(pecrecntorderstask);
    
    return 1;
}

INT CClientSide::GetRecentUsersRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	STRU_PACKAGE_RS<T_EC_GETRECENTUSERS_RS> loSendPack(NEW_DEF_BUSINESS_GETRECENTUSERS_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_GETRECENTUSERS_RQ, aiUserID, loSendPack.GetPackType())	

	auto t_fb_data = loUserLoad.t_fb_data;

    std::vector<std::string> userlist;
    std::string szkey = std::to_string(t_fb_data->b_id());              
 
    int liResult = MAKE_SUCCESS_RESULT();
    commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(), t_fb_data->s_rq_head()->pack_session_id(),
								liResult, t_fb_data->s_rq_head()->platform());    
    
    std::vector< uint64_t > std_vec_cid_list;

    if (RC_SUCCESS == mpServerMgr->mpRedisUtil->GetRecentCidList(szkey, userlist))
    {
        for (auto iter = userlist.begin(); iter != userlist.end(); iter++)
        {   
            std_vec_cid_list.push_back(ServerUtilMgr::STR2UINT64(*iter));
        }
    }
    
    LOG_TRACE(LOG_INFO, true, __FUNCTION__, " get recent usrers key= " << szkey << " cidcnt=" << std_vec_cid_list.size());

    auto fbs_vec_cid_list = loSendPack.fbbuilder.CreateVector(std_vec_cid_list);

    ecpack::T_EC_GETRECENTUSERS_RSBuilder c1 = ecpack::T_EC_GETRECENTUSERS_RSBuilder(loSendPack.fbbuilder);
	c1.add_s_rs_head(&s_rs_head);
    c1.add_b_id(t_fb_data->b_id());
    c1.add_c_id_list(fbs_vec_cid_list);
	loSendPack.fbbuf = c1.Finish();

	return Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);	
}

INT CClientSide::GetIsUserBusyRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
    STRU_PACKAGE_RS<T_EC_ISUSERBUSY_RS> loSendPack(NEW_DEF_BUSINESS_ISUSERBUSY_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_ISUSERBUSY_RQ, aiUserID, loSendPack.GetPackType())	

	auto t_fb_data = loUserLoad.t_fb_data;

    int liResult = MAKE_SUCCESS_RESULT();
    commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(), t_fb_data->s_rq_head()->pack_session_id(),
								liResult, t_fb_data->s_rq_head()->platform());  

    USERID ulbid = t_fb_data->b_id();
    USERID ulcid = t_fb_data->c_id();
    USERID ulwid = t_fb_data->s_rq_head()->user_id();
    std::string szlastwid("");
    int istatus = 0;

    mpServerMgr->mpRedisUtil->GetTradeLastWid(ulcid, ulbid, szlastwid);

    if (szlastwid.empty())
    {
        int ilsessionid = time(NULL);
        mpServerMgr->mpRedisUtil->SetTradeLastWid(ulcid, ulbid, ulwid, ilsessionid);
    }
    else
    {
        if (szlastwid != std::to_string(ulwid))
        {
            USERID userid = ServerUtilMgr::STR2UINT64(szlastwid);
            bool bret = false;
            
            if (!IsUserLogin(userid))
            {
                int ilsessionid = time(NULL);
                mpServerMgr->mpRedisUtil->SetTradeLastWid(ulcid, ulbid, ulwid, ilsessionid);
            }
            else
            {
                istatus = 1;
            } 
        }
    }

    ecpack::T_EC_ISUSERBUSY_RSBuilder c1 = ecpack::T_EC_ISUSERBUSY_RSBuilder(loSendPack.fbbuilder);
	c1.add_s_rs_head(&s_rs_head);
    c1.add_b_id(t_fb_data->b_id());
    c1.add_c_id(ulcid);
    c1.add_status(istatus);
	loSendPack.fbbuf = c1.Finish();

	return Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

       
}

INT CClientSide::UpdateEcLoginRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
    GENERAL_UNPACK_BODY(STRU_BUSINESS_UPDATE_LOGIN_ID, FALSE) 
    TEcInof tecinfo;

    moRefCriticalecloginstat.Enter();
    if (loUserLoad.mbType == LOG_IN_TYPE)
    {
        if ( 0 == m_ecloginstat_map.count(aiUserID))
        {
            tecinfo.t_logincnt = 1;
            tecinfo.t_logintype = LOG_IN_TYPE;
        }
        else
        {
            tecinfo = m_ecloginstat_map[aiUserID];
            if ( LOG_IN_BUSY == tecinfo.t_logintype )
            {
                tecinfo.t_logintype = LOG_IN_TYPE;
            }
            else if ( LOG_IN_TYPE == tecinfo.t_logintype )
            {
                tecinfo.t_logincnt++;
                tecinfo.t_logintype = LOG_IN_TYPE;
            }
        }

        m_ecloginstat_map[aiUserID] = tecinfo;
        LOG_TRACE(LOG_INFO, true, __FUNCTION__, " login waiterid= " << aiUserID << " logintimes=" << tecinfo.t_logincnt);
    }
    else if (loUserLoad.mbType == LOG_OUT_TYPE)
    {
        auto iter = m_ecloginstat_map.find(aiUserID);
        if ( iter != m_ecloginstat_map.end())
        {
            tecinfo = m_ecloginstat_map[aiUserID];
            tecinfo.t_logincnt--;
            if (0 == tecinfo.t_logincnt)
            {
                m_ecloginstat_map.erase(iter);
            }
            else
            {
                m_ecloginstat_map[aiUserID] =  tecinfo;
            }

            LOG_TRACE(LOG_INFO, true, __FUNCTION__, " logout waiterid= " << aiUserID << " logintimes=" << tecinfo.t_logincnt);
        }
        else
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "logout error waiterid=" << aiUserID);
        }
        
    }
    else if (loUserLoad.mbType == LOG_IN_BUSY)
    {
        auto iter = m_ecloginstat_map.find(aiUserID);

        if ( iter != m_ecloginstat_map.end())
        {
            tecinfo = m_ecloginstat_map[aiUserID];

            if (tecinfo.t_logincnt == 1)
            {
                tecinfo.t_logintype = LOG_IN_BUSY;
                m_ecloginstat_map[aiUserID] = tecinfo;
            }            
            
            LOG_TRACE(LOG_INFO, true, __FUNCTION__, " logbusy waiterid= " << aiUserID << " logintimes= " << tecinfo.t_logincnt);
        }
        else
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, " logbusy error waiterid= " << aiUserID);
        }
        
    }
    else
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, " logintype is error:" << loUserLoad.mbType);
        return -1;
    }

    moRefCriticalecloginstat.Leave();
	return 1;  
}

INT CClientSide::SetBusinessRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{    
    STRU_PACKAGE_RS<T_EC_SETBUSINESS_RS> loSendPack(NEW_DEF_BUSINESS_SETINFO_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_SETBUSINESS_RQ, aiUserID, loSendPack.GetPackType())	

	auto t_fb_data = loUserLoad.t_fb_data;

    std::string sztmp;
    sztmp.assign((char*)apData,awLen);

    mpServerMgr->mpRedisUtil->SetBusinessInfo(t_fb_data->sellerId(), sztmp);

    std::string sellerAvatar;
    std::string sellerName;
    std::string sellerUrl;

    if (t_fb_data->sellerAvatar())
    {
        sellerAvatar = t_fb_data->sellerAvatar()->c_str();
    }

    if (t_fb_data->sellerName())
    {
        sellerName = t_fb_data->sellerName()->c_str();
    }

    if (t_fb_data->sellerUrl())
    {
        sellerUrl = t_fb_data->sellerUrl()->c_str();
    }

    LOG_TRACE(LOG_INFO, true, __FUNCTION__, "sellerId:" << t_fb_data->sellerId()
                                           <<"sellerType:" << t_fb_data->sellerType()
                                           <<"sellerAvatar:" << sellerAvatar
                                           <<"sellerName:" << sellerName
                                           <<"sellerUrl:" << sellerUrl);

    int liResult = MAKE_SUCCESS_RESULT();
    commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(), t_fb_data->s_rq_head()->pack_session_id(),
								liResult, t_fb_data->s_rq_head()->platform());

    ecpack::T_EC_SETBUSINESS_RSBuilder c1 = ecpack::T_EC_SETBUSINESS_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_sellerId(t_fb_data->sellerId());

    loSendPack.fbbuf = c1.Finish();
	return Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);   
}

INT CClientSide::GetBusinessRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
    STRU_PACKAGE_RS<T_EC_GETBUSINESS_RS> loSendPack(NEW_DEF_BUSINESS_GETINFO_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_GETBUSINESS_RQ, aiUserID, loSendPack.GetPackType())	

	auto t_fb_data = loUserLoad.t_fb_data;

    std::string sztmp;
    mpServerMgr->mpRedisUtil->GetBusinessInfo(t_fb_data->sellerId(), sztmp);

    STRU_GENERAL_BODY_PACKAGE<STRU_EC_SETBUSINESS_RQ> loCachePack;
    const STRU_EC_SETBUSINESS_RQ &loCacheUserLoad = loCachePack.moUserLoad;
    INT nCachePackResult = -2;
    
    if (!sztmp.empty())
    {
        nCachePackResult = loCachePack.UnPack((BYTE*)(sztmp.c_str()), sztmp.length());
    }

    int liResult = MAKE_SUCCESS_RESULT();
    std::string sellerAvatar;
    std::string sellerName;
    std::string sellerUrl;
    USERID sellerId = t_fb_data->sellerId();
    int sellerType = -1;
    
    if(-2 == nCachePackResult)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error unpack cache fault3.");
		
		liResult = RET_REQ_REDIS_ERROR;
	}
    else
    {
        auto t_fb_cache_data = loCacheUserLoad.t_fb_data;
        sellerType = t_fb_cache_data->sellerType();

        if (t_fb_cache_data->sellerAvatar())
        {
            sellerAvatar = t_fb_cache_data->sellerAvatar()->c_str();
        }

        if (t_fb_cache_data->sellerName())
        {
            sellerName = t_fb_cache_data->sellerName()->c_str();
        }

        if (t_fb_cache_data->sellerUrl())
        {
            sellerUrl = t_fb_cache_data->sellerUrl()->c_str();
        }

        LOG_TRACE(LOG_INFO, true, __FUNCTION__, "sellerId:" << t_fb_cache_data->sellerId()
                                   <<"sellerType:" << t_fb_cache_data->sellerType()
                                   <<"sellerAvatar:" << sellerAvatar
                                   <<"sellerName:" << sellerName
                                   <<"sellerUrl:" << sellerUrl);
    }
    
    commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(), t_fb_data->s_rq_head()->pack_session_id(),
								liResult, t_fb_data->s_rq_head()->platform());

    auto fbssellerAvatar = loSendPack.fbbuilder.CreateString(sellerAvatar.c_str()); 
    auto fbssellerName = loSendPack.fbbuilder.CreateString(sellerName.c_str());
    auto fbssellerUrl = loSendPack.fbbuilder.CreateString(sellerUrl.c_str());
    
    ecpack::T_EC_GETBUSINESS_RSBuilder c1 = ecpack::T_EC_GETBUSINESS_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_sellerId(t_fb_data->sellerId());
    c1.add_sellerType(sellerType);
    c1.add_sellerAvatar(fbssellerAvatar);
    c1.add_sellerName(fbssellerName);
    c1.add_sellerUrl(fbssellerUrl);

    loSendPack.fbbuf = c1.Finish();
	return Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);   
}

INT CClientSide::SetPromptsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{    
    STRU_PACKAGE_RS<T_EC_SETPROMPTS_RS> loSendPack(NEW_DEF_BUSINESS_SETPROMPTS_RS);
    TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_SETPROMPTS_RQ, aiUserID, loSendPack.GetPackType())  

    auto t_fb_data = loUserLoad.t_fb_data;

    int liResult = MAKE_SUCCESS_RESULT();
    const flatbuffers::Vector<flatbuffers::Offset<ecpack::T_PROMPTMSG_INFO>> * s_promptslist =  t_fb_data->promptsmsglist();
    std::map<std::string, std::string> filed_value_map;
    for (flatbuffers::uoffset_t i = 0; i < s_promptslist->size(); i++)
    {
        const ecpack::T_PROMPTMSG_INFO * s_prompts = s_promptslist->Get(i);
        USERID ulbid = s_prompts->b_id();
        int ipromptstype = s_prompts->prompts_type();
        std::string strfield = std::to_string(ulbid) + "_" + std::to_string(ipromptstype);
        
        std::string strpromptsmsg("nil");
        if ((s_prompts->prompts_msg()) && (std::string("") != s_prompts->prompts_msg()->c_str()))
        {
            strpromptsmsg = s_prompts->prompts_msg()->c_str();
        }        
        filed_value_map[strfield] = strpromptsmsg;

        LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "bid:" << ulbid
                                           <<"promptstype:" << ipromptstype
                                           <<"promptsmsg:" << strpromptsmsg);
    }

    if (0  != filed_value_map.size())
    {
        if (0 > mpServerMgr->mpRedisUtil->SetBusinessPrompts(filed_value_map))
        {
            liResult = RET_REQ_REDIS_ERROR;
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "set promptsmsg error");
        }
    }     
    
    commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(), t_fb_data->s_rq_head()->pack_session_id(),
                                    liResult, t_fb_data->s_rq_head()->platform());
    
    ecpack::T_EC_SETPROMPTS_RSBuilder c1 = ecpack::T_EC_SETPROMPTS_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
  
    loSendPack.fbbuf = c1.Finish();
	return Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);    
}

INT CClientSide::GetPromptsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{    
    STRU_PACKAGE_RS<T_EC_GETPROMPTS_RS> loSendPack(NEW_DEF_BUSINESS_GETPROMPTS_RS);
    TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_GETPROMPTS_RQ, aiUserID, loSendPack.GetPackType())  

    auto t_fb_data = loUserLoad.t_fb_data;

    int liResult = MAKE_SUCCESS_RESULT();
    const flatbuffers::Vector<uint64_t> * s_bidlist =  t_fb_data->b_id_list();
    std::map<std::string, std::string> filed_value_map;
    std::set<std::string> field_set;
    for (flatbuffers::uoffset_t i = 0; i < s_bidlist->size(); i++)
    {

        USERID ulbid = (USERID) s_bidlist->Get(i);
        std::string strfieldlogin = std::to_string(ulbid) + "_" + std::to_string(LOG_IN_TYPE);
        std::string strfieldlogout = std::to_string(ulbid) + "_"+ std::to_string(LOG_OUT_TYPE);
        field_set.insert(strfieldlogin);
        field_set.insert(strfieldlogout);
    }

    if (0  != field_set.size())
    {
        if (0 > mpServerMgr->mpRedisUtil->GetBusinessPrompts(field_set, filed_value_map))
        {
            liResult = RET_REQ_REDIS_ERROR;
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "get promptsmsg error");
        }
    }

    std::vector< flatbuffers::Offset<ecpack::T_PROMPTMSG_INFO> > std_promptmsg_vec;
    for (auto iter = filed_value_map.begin(); iter != filed_value_map.end(); iter++)
    {
        std::string strbid = iter->first.substr(0, iter->first.find_first_of('_'));
        USERID ulbid = ServerUtilMgr::STR2UINT64(strbid);
        std::string strprompttype = iter->first.substr(iter->first.find_first_of('_')+1);
        int prompttype = (int)(ServerUtilMgr::STR2UINT64(strprompttype));
        std::string stpromptmsg = iter->second;

        auto fbspromptmsg = loSendPack.fbbuilder.CreateString(stpromptmsg.c_str());
        ecpack::T_PROMPTMSG_INFOBuilder promptmsginfo = ecpack::T_PROMPTMSG_INFOBuilder(loSendPack.fbbuilder);
        promptmsginfo.add_b_id(ulbid);
        promptmsginfo.add_prompts_type(prompttype);
        promptmsginfo.add_prompts_msg(fbspromptmsg);
        std_promptmsg_vec.push_back(promptmsginfo.Finish());     
    }


    commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(), t_fb_data->s_rq_head()->pack_session_id(),
                                    liResult, t_fb_data->s_rq_head()->platform());
    

    auto fbs_prompts_info_vec = loSendPack.fbbuilder.CreateVector(std_promptmsg_vec);
    ecpack::T_EC_GETPROMPTS_RSBuilder c1 = ecpack::T_EC_GETPROMPTS_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_promptsmsglist(fbs_prompts_info_vec);
   
    loSendPack.fbbuf = c1.Finish();
	return Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);    
}


bool CClientSide::IsUserLogin(USERID userid)
{
    bool bret = false;
    moRefCriticalecloginstat.Enter();
    if (0 != m_ecloginstat_map.count(userid) && m_ecloginstat_map[userid].t_logintype == LOG_IN_TYPE)
    {
        bret = true;
    }
    moRefCriticalecloginstat.Leave();

    return bret;
}

void CClientSide::GetLoginWaiter(std::vector<USERID>& ulwidlist, std::vector<USERID>& ulloginwidlist)
{
    
    for (auto iter = ulwidlist.begin(); iter != ulwidlist.end(); iter++)
    {
        if (IsUserLogin(*iter))
        {
            ulloginwidlist.push_back(*iter);
        }
    }
    
    return;
}
INT CClientSide::GetRandom()
{    
    struct timeval tpstart;
    
    gettimeofday(&tpstart,NULL);
    
    srand(tpstart.tv_usec);
    
    return 1+(int) (100.0*rand()/(RAND_MAX+1.0));
}

