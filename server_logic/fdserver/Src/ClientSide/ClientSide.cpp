#include "stdafx.h"
#include "const/ErrorCodeDef.h"
#include <map>
#include "ServerUtilMgr.h"
#include "ClientSide.h"
#include "../ServerMgr.h"
#include "ServerToKafka.h"
#include "network/packet/client/CommonPackDef.h"
#include "network/packet/client/FdServerPackDef.h"
#include "network/packet/client/ScServerPackDef.h"
#include "FriendListTask.h"
#include "FriendRemarkTask.h"
#include "FriendDelTask.h"
#include "FriendConfirmTask.h"
#include "FriendBlackListTask.h"
#include "FriendUpdateTask.h"
#include "FriendClientAddTask.h"

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

INT CClientSide::FriendListRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	STRU_PACKAGE_RS<T_FRIEND_LIST_RS> loSendPack(NEW_DEF_FRIEND_LIST_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_FRIEND_LIST_RQ, aiUserID, loSendPack.GetPackType())
	auto t_fb_data = loUserLoad.t_fb_data;	

    LOG_TRACE(LOG_INFO, true, __FUNCTION__, " userid="<< aiUserID << " token = " << t_fb_data->token());
    if (t_fb_data->token() == 0)
    {
        CFriendListTask *pfriendlisttask = new CFriendListTask(aiCometID, 
                                                                 aiUserID, 
                                                                 t_fb_data->s_rq_head()->pack_session_id(), 
                                                                 t_fb_data->s_rq_head()->platform());
        if (NULL == pfriendlisttask)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new pfriendlisttask failed!");
            return -1;
        }
        
        std::string szdata = "user_id=" + std::to_string(aiUserID) +
                              "&offset="   + std::to_string(t_fb_data->offset()) +
                              "&max_cnt="   + std::to_string(MAX_FD_LIST_CNT);

        pfriendlisttask->SetData(szdata);
        pfriendlisttask->SetClientSide(this);
        m_taskthreadpoll.AddTask(pfriendlisttask);       
    }
    else
    {
        int liResult = MAKE_SUCCESS_RESULT();
        UINT64 ultoken = t_fb_data->token();
        commonpack::S_RS_HEAD s_rs_head(aiUserID, t_fb_data->s_rq_head()->pack_session_id(),liResult,t_fb_data->s_rq_head()->platform());

        std::map<std::string, std::string> friendinfo_map;
        mpServerMgr->mpRedisUtil->ZRrangeFdMsgByScore(aiUserID,t_fb_data->token()+1,MAX_FD_LIST_CNT, &friendinfo_map);    
        
        std::vector< flatbuffers::Offset<friendpack::T_FREIND_INFO> > std_friend_info_vec;
        for (auto iter = friendinfo_map.begin(); iter != friendinfo_map.end(); iter++)
        {
            friendpack::T_FREIND_INFO *t_friend_info;
	        t_friend_info = (friendpack::T_FREIND_INFO*)friendpack::GetT_FREIND_INFO(iter->second.c_str());
            
            std::string szremarkname("");
            std::string szopmsg("");
        
            if (t_friend_info->remark_name())
            {
                szremarkname = t_friend_info->remark_name()->c_str();
            }

            if (t_friend_info->op_msg())
            {
                szopmsg = t_friend_info->op_msg()->c_str();
            }

            auto fbsremarkname = loSendPack.fbbuilder.CreateString(szremarkname.c_str()); 
            auto fbsopmsg = loSendPack.fbbuilder.CreateString(szopmsg.c_str()); 

            friendpack::T_FREIND_INFOBuilder fbsfriendinfo = friendpack::T_FREIND_INFOBuilder(loSendPack.fbbuilder);
            fbsfriendinfo.add_remark_name(fbsremarkname);
            fbsfriendinfo.add_op_msg(fbsopmsg);
            fbsfriendinfo.add_user_id(t_friend_info->user_id());           
            fbsfriendinfo.add_source_type(t_friend_info->source_type());
            fbsfriendinfo.add_opt_type(t_friend_info->opt_type());
            fbsfriendinfo.add_black_type(t_friend_info->black_type());
            fbsfriendinfo.add_op_time(t_friend_info->op_time());
            std_friend_info_vec.push_back(fbsfriendinfo.Finish());
            ultoken = ServerUtilMgr::STR2UINT64(iter->first);
        }

        auto fbs_friend_info_vec = loSendPack.fbbuilder.CreateVector(std_friend_info_vec);
        friendpack::T_FRIEND_LIST_RSBuilder c1 = friendpack::T_FRIEND_LIST_RSBuilder(loSendPack.fbbuilder);
        c1.add_s_rs_head(&s_rs_head);
        c1.add_friend_list(fbs_friend_info_vec);
        c1.add_token(ultoken);
        c1.add_msg_source(INCGET);
        loSendPack.fbbuf = c1.Finish();

        Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
        
        long ulmsgcnt = 0;
        mpServerMgr->mpRedisUtil->ZcountFdMsg(aiUserID,ulmsgcnt);
        
        if (ulmsgcnt > MAX_FD_MSG_CNT)
        {
            mpServerMgr->mpRedisUtil->ZremrangebyrankFdMsg(aiUserID, 0, MAX_FD_LIST_CNT);
        }
    }

    return 1;	
}

INT CClientSide::FriendRemarkRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	STRU_PACKAGE_RS<T_FRIEND_REMARK_RS> loSendPack(NEW_DEF_FRIEND_REMARK_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_FRIEND_REMARK_RQ, aiUserID, loSendPack.GetPackType())

	auto t_fb_data = loUserLoad.t_fb_data;   

    CFriendRemarkTask *pfriendremarktask = new CFriendRemarkTask(aiCometID, 
                                                                 aiUserID, 
                                                                 t_fb_data->s_rq_head()->pack_session_id(), 
                                                                 t_fb_data->s_rq_head()->platform());
    if (NULL == pfriendremarktask)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new pfriendremarktask failed!");
        return -1;
    }
    pfriendremarktask->SetFriendId(t_fb_data->peer_user_id());

    std::string szdata = "user_id=" + std::to_string(aiUserID) +
                         "&friend_id="   + std::to_string(t_fb_data->peer_user_id());
    
	if (t_fb_data->peer_remark_name())
	{
		std::string szmsgcontent =  t_fb_data->peer_remark_name()->c_str();
        szdata += "&remark_name=" + ServerUtilMgr::UrlEncode(szmsgcontent);
        pfriendremarktask->SetRemarkName(szmsgcontent);
	}

    pfriendremarktask->SetData(szdata);
    pfriendremarktask->SetClientSide(this);
    m_taskthreadpoll.AddTask(pfriendremarktask);

    return 1;
}


INT CClientSide::FriendDelRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	STRU_PACKAGE_RS<T_FRIEND_DEL_RS> loSendPack(NEW_DEF_FRIEND_DEL_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_FRIEND_DEL_RQ, aiUserID, loSendPack.GetPackType())	

	auto t_fb_data = loUserLoad.t_fb_data;

    CFriendDelTask *pfrienddeltask = new CFriendDelTask(aiCometID, 
                                                         aiUserID,
                                                         t_fb_data->s_rq_head()->pack_session_id(), 
                                                         t_fb_data->s_rq_head()->platform());
    if (NULL == pfrienddeltask)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new pfrienddeltask failed!");
        return -1;
    }

    if (t_fb_data->own_nickname())
    {
        std::string nickname = t_fb_data->own_nickname()->c_str();
        pfrienddeltask->SetNickName(nickname);
    }

    pfrienddeltask->SetFriendId(t_fb_data->peer_user_id());
    pfrienddeltask->SetClientSide(this);  
    m_taskthreadpoll.AddTask(pfrienddeltask);
    return 1;
}

INT CClientSide::FriendClientAddRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	STRU_PACKAGE_RS<T_FRIEND_CLIENT_ADD_RS> loSendPack(NEW_DEF_CLIENT_FRIEND_ADD_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_FRIEND_CLIENT_ADD_RQ, aiUserID, loSendPack.GetPackType())

	auto t_fb_data = loUserLoad.t_fb_data; 
    
    CFriendClientAddTask *pfriendclientaddtask = new CFriendClientAddTask(aiCometID, 
                                                                aiUserID, 
                                                                t_fb_data->s_rq_head()->pack_session_id(), 
                                                                t_fb_data->s_rq_head()->platform());

    if (NULL == pfriendclientaddtask)
    {
       LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new pfriendclientaddtask failed!");
       return -1;
    }

    pfriendclientaddtask->SetFriendId(t_fb_data->peer_user_id());  
    pfriendclientaddtask->SetSourceType(t_fb_data->source_type());

    std::string szdata = "user_id=" + std::to_string(aiUserID) +
                     "&friend_id="   + std::to_string(t_fb_data->peer_user_id()) +
                     "&source_type=" + std::to_string(t_fb_data->source_type());    
                      	
    
    if (t_fb_data->own_nickname())
    {
        std::string sznickname = t_fb_data->own_nickname()->c_str();
        pfriendclientaddtask->SetNickName(sznickname);
    }

    std::string szopmsg_tmp("");
    if (t_fb_data->op_msg())
    {
        std::string szmsgcontent =  t_fb_data->op_msg()->c_str();
        szdata += "&friend_msg=" + ServerUtilMgr::UrlEncode(szmsgcontent);
        pfriendclientaddtask->SetOpMsg(szmsgcontent);
    }

    pfriendclientaddtask->SetData(szdata);
    pfriendclientaddtask->SetClientSide(this);
    m_taskthreadpoll.AddTask(pfriendclientaddtask);
    return 1;
}

INT CClientSide::FriendClientCONFIRMRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	STRU_PACKAGE_RS<T_FRIEND_CLIENT_CONFIRM_RS> loSendPack(NEW_DEF_CLINET_FRIEND_CONFIRM_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_FRIEND_CLIENT_CONFIRM_RQ, aiUserID, loSendPack.GetPackType())

	auto t_fb_data = loUserLoad.t_fb_data;
    
    std::string fdrequesttime;
    mpServerMgr->mpRedisUtil->GetFdRequestTime(t_fb_data->s_rq_head()->user_id(), t_fb_data->peer_user_id(),fdrequesttime);
    if (!fdrequesttime.empty())
    {
        UINT64 curltime = time(NULL);
        UINT64 ulrequesttime = ServerUtilMgr::STR2UINT64(fdrequesttime);
        if (curltime > (ulrequesttime + GetFriendExpiretime()))
        {
            //好友确认超时
            int liResult = RET_FRIEND_CONFIRM_TIMEOUT_ERROR;
            commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(), t_fb_data->s_rq_head()->pack_session_id(),
            					liResult, t_fb_data->s_rq_head()->platform());
            friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder c1 = friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder(loSendPack.fbbuilder);
            c1.add_s_rs_head(&s_rs_head);
            c1.add_peer_user_id(t_fb_data->peer_user_id());
            c1.add_result(0);
            loSendPack.fbbuf = c1.Finish();	
            Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

            
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "friend confirm timeout token=" << t_fb_data->token());
            return -1;
        }
    }

	//send to db flatbuf to json
    CFriendConfirmTask *pfriendconfirmtask = new CFriendConfirmTask(aiCometID, 
                                                                     aiUserID, 
                                                                     t_fb_data->s_rq_head()->pack_session_id(), 
                                                                     t_fb_data->s_rq_head()->platform());
    if (NULL == pfriendconfirmtask)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new pfriendconfirmtask failed!");
        return -1;
    }
    

    pfriendconfirmtask->SetFriendId(t_fb_data->peer_user_id());  
    pfriendconfirmtask->SetSourceType(t_fb_data->source_type());

    std::string szdata = "user_id=" + std::to_string(aiUserID) +
                         "&friend_id="   + std::to_string(t_fb_data->peer_user_id()) +
                         "&source_type=" + std::to_string(t_fb_data->source_type()); 
    		
    if (t_fb_data->peer_remark())
    {
    	std::string szmsgcontent =  t_fb_data->peer_remark()->c_str();
        szdata += "&remark_name=" + ServerUtilMgr::UrlEncode(szmsgcontent);
        pfriendconfirmtask->SetRemarkName(szmsgcontent);
    }

    if (t_fb_data->own_nickname())
    {
        std::string sznickname = t_fb_data->own_nickname()->c_str();
        pfriendconfirmtask->SetNickName(sznickname);
    }
    
    pfriendconfirmtask->SetResult(t_fb_data->result());
    pfriendconfirmtask->SetData(szdata);
    pfriendconfirmtask->SetClientSide(this);
    m_taskthreadpoll.AddTask(pfriendconfirmtask);  
		
	return 1;
}

INT CClientSide::FriendClientBlackListRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	STRU_PACKAGE_RS<T_CLINET_FRIEND_BLACKLIST_RS> loSendPack(NEW_DEF_FRIEND_UPDATE_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_CLIENT_FRIEND_BLACKLIST_RQ, aiUserID, loSendPack.GetPackType())

	auto t_fb_data = loUserLoad.t_fb_data;   

    CFriendBlackListTask *pfriendblacklisttask = new CFriendBlackListTask(aiCometID, 
                                                                 aiUserID, 
                                                                 t_fb_data->s_rq_head()->pack_session_id(), 
                                                                 t_fb_data->s_rq_head()->platform());

    if (NULL == pfriendblacklisttask)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new pfriendblacklisttask failed!");
        return -1;
    }

    pfriendblacklisttask->SetFriendId(t_fb_data->peer_user_id());
    pfriendblacklisttask->SetType(t_fb_data->type());

    std::string szdata = "user_id=" + std::to_string(aiUserID) +
                         "&friend_id="   + std::to_string(t_fb_data->peer_user_id())+
                         "&black_type=" + std::to_string(t_fb_data->type());

    pfriendblacklisttask->SetData(szdata);
    pfriendblacklisttask->SetClientSide(this);
    m_taskthreadpoll.AddTask(pfriendblacklisttask);

    return 1;
}

INT CClientSide::FriendUpdateRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
    STRU_PACKAGE_RS<T_FRIEND_UPDATE_RS> loSendPack(NEW_DEF_FRIEND_UPDATE_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_FRIEND_UPDATE_RQ, aiUserID, loSendPack.GetPackType())

	auto t_fb_data = loUserLoad.t_fb_data;

    CFriendUpdateTask *pfrienupdatetask = new CFriendUpdateTask(aiCometID, 
                                                                 aiUserID, 
                                                                 t_fb_data->s_rq_head()->pack_session_id(), 
                                                                 t_fb_data->s_rq_head()->platform());

    if (NULL == pfrienupdatetask)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "new pfrienupdatetask failed!");
        return -1;
    }

    pfrienupdatetask->SetFriendId(t_fb_data->peer_user_id());

    std::string szdata = "user_id=" + std::to_string(aiUserID) +
                         "&friend_id="   + std::to_string(t_fb_data->peer_user_id());

    pfrienupdatetask->SetData(szdata);
    pfrienupdatetask->SetClientSide(this);
    m_taskthreadpoll.AddTask(pfrienupdatetask);
}

INT CClientSide::GetFriendExpiretime()
{
    #define MONTHTOSECS  2592000

    if (NULL == mpServerMgr)
    {
        return MONTHTOSECS;
    }

    return mpServerMgr->moConfig.m_fdexpire_seconds;
}



