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
	lstruTcpOpt.muiKeepAlive = 5 * 60;
	//lstruTcpOpt.muiKeepAlive = mpServerMgr->moConfig.miLoginTimeout;
	if(!ClientPack::GetInstance().Open(this, lstruTcpOpt, ENUM_CONNECT_MODE_WS))
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
		);

	CUser *lpUser = mpServerMgr->moUserMgr.MallocRefObj();

	lpUser->mhSocket = ahSocket;
	lpUser->miIPAddr = aiIP;
	lpUser->miIPPort = awPort;
	lpUser->mpSocketInfo = apSocketInfo;
	//if(!mpServerMgr->moUserMgr.addUser(lpUser))
	//	return ;
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
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Disconnected "
		<<" Errcode="<<aiPeerID
		<<" IP=" << GetIPString(aiIP)<< ":"<< awPort);
	CUser *lpUser = (CUser*)apRelationPtr;
	CAutoReleaseRefObjMgrTemplate2<PHONEID,CUser>  loUser(mpServerMgr->moUserMgr, lpUser);
	{
		CRITICAL_SECTION_HELPER(lpUser->GetCriticalSection());
        ClientPack::GetInstance().DelRelationPtr(lpUser->mpSocketInfo,lpUser);
		lpUser->mpSocketInfo =  NULL;
	}

	UpdateTimeID(lpUser->miUserID, LOG_OUT_TYPE);
	RouteSyncID(lpUser->miUserID, LOG_OUT_TYPE);
	
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
	uint8 liLoginCount = lpUser->miLoginCount;
	if(TRUE == mpServerMgr->moUserMgr.delUser(lpUser))
	{
		//多台设备登陆的情况下只会删除自己的session
		if(liLoginCount > 1)
		{
			std::string s_session = mpServerMgr->mpRedisUtil->HGetLoginInfo(lpUser->miUserID, lpUser->miPlatform);
			if(s_session != "" && atol(s_session.c_str()) == goLocalServerInfo.mlServerID)
			{
				mpServerMgr->mpRedisUtil->HDelLoginInfo(lpUser->miUserID, lpUser->miPlatform);
			}
		}
        else
        {
            mpServerMgr->mpRedisUtil->HDelLoginInfo(lpUser->miUserID, lpUser->miPlatform);
        }
	}
}

// 包关联
void CClientSide::IntiFuncMap()
{
	//U9_NET_PACKET_INIT();
}

INT CClientSide::onRecvHeartRQ(WORD awPackType, SOCKET ahSocket,void* apRelationPtr,IPTYPE aiIP, WORD awPort, BYTE * apData, WORD awLen)
{
	CLIENT_CHECK_LOGIN();

	STRU_HEART_RS loSendPack;
	Send2Client(loSendPack, lpUser);
}

INT CClientSide::onRecvLoginRQ(WORD awPackType, SOCKET ahSocket,void* apRelationPtr,IPTYPE aiIP, WORD awPort, BYTE * apData, WORD awLen)
{
	GENERAL_UNPACK_BODY(STRU_LOGIN_RQ, TRUE)

	int liResult = MAKE_SUCCESS_RESULT();//MAKE_ERROR_RESULT(RET_ERR_INVALID_PARAM)
	auto t_fb_data = loUserLoad.t_fb_data;
	//用户已登录
	if(lpUser->miUserID > 0)
	{
		liResult = MAKE_ERROR_RESULT(RET_SYS_REPEAT_LOGINED);
		STRU_LOGIN_RS loSendPack;
		commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(),
										t_fb_data->s_rq_head()->pack_session_id(),
										liResult, t_fb_data->s_rq_head()->platform());
		loSendPack.fbbuf = syspack::CreateT_LOGIN_RS(loSendPack.fbbuilder,
													 &s_rs_head,
													 lpUser->miServerTime);
		Send2Client(loSendPack, lpUser);
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "user repeat login"
                <<" old user_id = " << lpUser->miUserID
                <<" new user_id = " << t_fb_data->s_rq_head()->user_id());
		return 1;
	}
	//生成新用户信息
	lpUser->miUserID = t_fb_data->s_rq_head()->user_id();
	lpUser->miAppID = t_fb_data->ap_id();
	lpUser->msTgt = t_fb_data->tgt() ? t_fb_data->tgt()->str() : "";
	lpUser->msCookie = string((char *)t_fb_data->cookie()->data(), t_fb_data->cookie()->Length());
	lpUser->msClientVersion = t_fb_data->client_version() ? t_fb_data->client_version()->str() : "";
	lpUser->miPlatform = (ENUM_PLATFORM)t_fb_data->platform();
	lpUser->msDeviceCode = t_fb_data->device_code() ? t_fb_data->device_code()->str() : "";
	lpUser->msOsType = t_fb_data->os_type() ? t_fb_data->os_type()->str() : "";
	lpUser->miNetType = t_fb_data->net_type();
	lpUser->miServerTime = CSystem::GetMicroSystemTime();

    //平台验证(如果不对，踢掉)
    if(lpUser->miPlatform != PLATFROM_WEB)
    {
        LOG_TRACE(LOG_WARNING, true, __FUNCTION__, "platform not web"
                <<" user="<< lpUser->miUserID);
		//踢掉用户
		KickUserID(lpUser, 0, false, RET_PLATFORM_ERROR);
		return 1;
	}

	//查看用户是不是已经登录了
	std::string new_s_session = std::to_string(goLocalServerInfo.mlServerID);
    std::string s_session = mpServerMgr->mpRedisUtil->HGetLoginInfo(lpUser->miUserID, lpUser->miPlatform);
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "debug log11111 "
			<<" s_session="<< s_session
			<<" new_s_session=" << new_s_session
			<< " apRelationPtr = " << apRelationPtr);
    //用户如果已登录踢掉老用户
    CAutoReleaseRefObjMgrTemplate2<PHONEID,CUser>  loUser(mpServerMgr->moUserMgr, lpUser->miUserID);
    CUser* lpOldUser = loUser.GetPtr();
	if(s_session != "")
	{
		//老用户和新用户在同一台连接服务器
		if(atol(s_session.c_str()) == goLocalServerInfo.mlServerID)
		{
			if(lpOldUser)
			{
				//先删掉老用户
				mpServerMgr->moUserMgr.delUser(lpOldUser);
				LOG_TRACE(LOG_ERR, true, __FUNCTION__, "debug log22222 "
						<<" is in mgr="<< lpOldUser->mbIsinMgr
						<< " apRelationPtr = " << apRelationPtr);
				//踢掉老用户
				KickUserID(lpOldUser, 0, false, RET_SYS_DISCON_USER_KICKED);
			}
			else
			{
				//错误
				LOG_TRACE(LOG_WARNING, true, __FUNCTION__, "old user not exsist "
						<<" user_id="<< lpUser->miUserID
						<<" pack_session_id="<< t_fb_data->s_rq_head()->pack_session_id()
						<< " apRelationPtr = " << apRelationPtr);
			}
		}
		else
		{
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "kick user in other comet"
                    <<" user_id="<< lpUser->miUserID
                    <<" pack_session_id="<< t_fb_data->s_rq_head()->pack_session_id()
                    << " apRelationPtr = " << apRelationPtr);
			//发送后端服务器踢人
			lpUser->miLoginCount++;
			KickUserID(lpUser, atol(s_session.c_str()), true, RET_SYS_DISCON_USER_KICKED);
		}
	}
    else
    {
        //先删掉有问题的用户
		if(lpOldUser)			
		{
			mpServerMgr->moUserMgr.delUser(lpOldUser);
        	LOG_TRACE(LOG_ERR, true, __FUNCTION__, "error user in memory "
                <<" is in mgr="<< lpOldUser->mbIsinMgr
                <<" user_id="<< lpOldUser->miUserID
                << " apRelationPtr = " << apRelationPtr);
		}
    }

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "recv "
		<<" user_id="<< lpUser->miUserID
		<<" pack_session_id="<< t_fb_data->s_rq_head()->pack_session_id()
		<<" tgt=" << lpUser->msTgt
		<<" cookie=" << lpUser->msCookie
		<<" client_version=" << lpUser->msClientVersion
		<<" platform=" << lpUser->miPlatform
		<<" device_code=" << lpUser->msDeviceCode
		<<" os_type=" << lpUser->msOsType
		<<" net_type=" << lpUser->miNetType
		<<" server_time=" << lpUser->miServerTime
		<< " apRelationPtr = " << apRelationPtr);

	//添加新用户
	mpServerMgr->moUserMgr.addUser(lpUser);

	mpServerMgr->mpRedisUtil->HSetLoginInfo(lpUser->miUserID, lpUser->miPlatform, new_s_session);

	STRU_LOGIN_RS loSendPack;
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(), 
 		t_fb_data->s_rq_head()->pack_session_id(),
 		liResult, t_fb_data->s_rq_head()->platform());
	loSendPack.fbbuf = syspack::CreateT_LOGIN_RS(loSendPack.fbbuilder,
													   &s_rs_head,
													   lpUser->miServerTime);
	Send2Client(loSendPack, lpUser);

	UpdateTimeID(lpUser->miUserID, LOG_IN_TYPE);
	RouteSyncID(lpUser->miUserID, LOG_IN_TYPE);
	return 1;
}

INT CClientSide::onRecvBusinessSetLoginStatusRQ(WORD awPackType, SOCKET ahSocket,void* apRelationPtr,IPTYPE aiIP, WORD awPort, BYTE * apData, WORD awLen)
{
	GENERAL_UNPACK_BODY(STRU_EC_SETLOGINSTATUS_RQ, FALSE)

	int liResult = MAKE_SUCCESS_RESULT();//MAKE_ERROR_RESULT(RET_ERR_INVALID_PARAM)
	//生成新用户信息
	auto t_fb_data = loUserLoad.t_fb_data;

	UpdateTimeID(lpUser->miUserID, t_fb_data->status());

	STRU_PACKAGE_RS<T_EC_SET_LOGINSTATUS_RS> loSendPack(NEW_DEF_BUSINESS_SETLOGINSTATUS_RS);
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(), t_fb_data->s_rq_head()->pack_session_id(),
									liResult, t_fb_data->s_rq_head()->platform());
	ecpack::T_EC_SET_LOGINSTATUS_RSBuilder c1 = ecpack::T_EC_SET_LOGINSTATUS_RSBuilder(loSendPack.fbbuilder);
	c1.add_s_rs_head(&s_rs_head);
	loSendPack.fbbuf = c1.Finish();
	Send2Client(loSendPack, lpUser);

	return 1;
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

INT CClientSide::KickUserID(CUser* client_user, const SESSIONID& comet_id, BOOL is_back, uint32 error_code)
{
	STRU_SERVER_DISCON_ID kick_pack;
	kick_pack.fbbuf = syspack::CreateT_SERVER_DISCON_ID(kick_pack.fbbuilder,
														 MAKE_ERROR_RESULT(error_code));
	if(is_back)
	{
		ServerPack::GetInstance().Send2LogicServer(STRU_SERVER_DISCON_ID::ENUM_TYPE,
												   kick_pack,
												   comet_id,
												   client_user->miUserID);
	}
	else
	{
		Send2Client(kick_pack, client_user, true);
	}

	return 1;
}

INT CClientSide::UpdateTimeID(USERID user_id, BYTE time_type)
{
	if(user_id <= 0)
	{
		return RET_SUCCESS;
	}

	STRU_BUSINESS_UPDATE_LOGIN_ID update_login_id;
	update_login_id.mbType = time_type;
	update_login_id.miTime = CSystem::GetMicroSystemTime();
	ServerPack::GetInstance().Send2AllLogicServer(STRU_BUSINESS_UPDATE_LOGIN_ID::ENUM_TYPE, update_login_id,
													  goLocalServerInfo.mlServerID, user_id);
	LOG_TRACE(LOG_ERR, true, __FUNCTION__, " "
			<<" user_id="<< user_id
			<<" time_type="<< time_type
	);
	return RET_SUCCESS;
}

INT CClientSide::RouteSyncID(USERID user_id, BYTE b_type)
{
	if(user_id <= 0)
	{
		return RET_SUCCESS;
	}

	STRU_ROUTE_SYNC_ID route_sync_pack;
	route_sync_pack.mbType = b_type;
	ServerPack::GetInstance().Send2RouteServer(STRU_ROUTE_SYNC_ID::ENUM_TYPE, PC_ROUTE, route_sync_pack,
											   goLocalServerInfo.mlServerID, user_id);
	LOG_TRACE(LOG_ERR, true, __FUNCTION__, " "
			<<" user_id="<< user_id
			<<" b_type="<< b_type
	);
	return RET_SUCCESS;
}