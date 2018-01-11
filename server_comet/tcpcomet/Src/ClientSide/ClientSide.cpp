#include "stdafx.h"
#include "const/ErrorCodeDef.h"

#include "ClientSide.h"
#include "../ServerMgr.h"
#include "rjson/JsonParser.h"
#include "encrypt/aes.h"
#include "../../../../server_logic/ecserver/Src/ServerMgr.h"
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

	lstruTcpOpt.muiKeepAlive = 5 * 60;

	if(!mpServerMgr->moConfig.mstrWithOutCheckIP.empty())
	{
		strncpy(lstruTcpOpt.mszWithOutCheckIP, mpServerMgr->moConfig.mstrWithOutCheckIP.c_str(), sizeof(lstruTcpOpt.mszWithOutCheckIP));
	}
	if(!ClientPack::GetInstance().Open(this, lstruTcpOpt, ENUM_CONNECT_MODE_TCP))
	{
		LOG_TRACE(LOG_CRIT,true,__FUNCTION__," Initialize tcp error.");
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
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}

//TCP连接建立
void CClientSide::OnSessionConnectedEx(const SESSIONID& aiPeerID,void* apSocketInfo, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Connected "
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
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
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}
//客户端关闭连接
void CClientSide::OnSessionClose(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Disconnected "
		<<" Errcode="<<aiPeerID
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<< " Socket="<< ahSocket
		);
}
void CClientSide::OnSessionCloseEx(const SESSIONID & aiPeerID,void*apRelationPtr, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Disconnected "
		<<" Errcode="<<aiPeerID
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<< " Socket="<< ahSocket
		);
	CUser *lpUser = (CUser*)apRelationPtr;
    //添加用户活跃信息
    UINT64 login_time = lpUser->miServerTime / 1000000;
    UINT64 logout_time = CSystem::GetTime();
	if(lpUser->miUserID > 0 && logout_time - login_time > 30)
	{
		RJ_CREATE_O_DOCUMENT(l_document, l_allocator)
		JsonParse::add<int>(l_document, "user_id", lpUser->miUserID, l_allocator);
		JsonParse::add<int>(l_document, "ap_id", lpUser->miAppID, l_allocator);
		JsonParse::add(l_document, "client_version", lpUser->msClientVersion, l_allocator);
		JsonParse::add<int>(l_document, "platform", lpUser->miPlatform, l_allocator);
		JsonParse::add(l_document, "device_code", lpUser->msDeviceCode, l_allocator);
		JsonParse::add(l_document, "os_type", lpUser->msOsType, l_allocator);
		JsonParse::add<int>(l_document, "net_type", lpUser->miNetType, l_allocator);
		JsonParse::add<uint64_t>(l_document, "login_time", login_time, l_allocator);
		JsonParse::add<uint64_t>(l_document, "logout_time", logout_time, l_allocator);

		std::string out_json;
		JsonParse::to_string(l_document, out_json);

		mpServerMgr->mpRedisUtil->RPushLoginDetail(lpUser->miUserID, out_json);
	}

	CAutoReleaseRefObjMgrTemplate2<PHONEID,CUser>  loUser(mpServerMgr->moUserMgr, lpUser);
	{
		CRITICAL_SECTION_HELPER(lpUser->GetCriticalSection());
        ClientPack::GetInstance().DelRelationPtr(lpUser->mpSocketInfo,lpUser);
		lpUser->mpSocketInfo =  NULL;
	}

	if (RET_SUCCESS != RouteSyncID(lpUser->miUserID, LOG_OUT_TYPE))
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "route sync false "
				<<" user_id="<< lpUser->miUserID
				<<" platform="<< lpUser->miPlatform
		);
	}

	if (RET_SUCCESS != UpdateTimeID(lpUser->miUserID))
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "update logout time false "
						<<" user_id="<< lpUser->miUserID
						<<" platform="<< lpUser->miPlatform
						);
	}
	
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, " delete user"
            << ",ahSocket=" << ahSocket
            <<" ,IP="<< GetIPString(aiIP)
            <<":"<< ntohs(awPort)
            <<" SocketInfoPtr="<< lpUser->mpSocketInfo
            <<" lpUserPtr="<< (void*)(lpUser)
            <<" UserID="<< lpUser->miUserID
            <<" RefCount="<< lpUser->GetRefCount()
            << "LoginCount=" << lpUser->miLoginCount
		);

    //删除注册用户
    mpServerMgr->moRegUserMgr.delUser(lpUser->miTempUserID);

	uint8 liLoginCount = lpUser->miLoginCount;
	if(TRUE == mpServerMgr->moUserMgr.delUser(lpUser))
	{
		//多台设备登陆的情况下只会删除自己的session
		if(liLoginCount > 1)
		{
			std::string s_session = mpServerMgr->mpRedisUtil->HGetLoginInfo(lpUser->miUserID, lpUser->miPlatform);
			if(s_session != "" && atol(s_session.c_str()) == goLocalServerInfo.mlServerID)
			{
                LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, " delete user not asss"
                        <<" UserID="<< lpUser->miUserID);
				mpServerMgr->mpRedisUtil->HDelLoginInfo(lpUser->miUserID, lpUser->miPlatform);
			}
		}
        else
        {
            mpServerMgr->mpRedisUtil->HDelLoginInfo(lpUser->miUserID, lpUser->miPlatform);
        }
	}
    else
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, " delete user false"
                <<" UserID="<< lpUser->miUserID);
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
	unsigned char key[] = "qianwang2017body";
	AES aes1(key);
	aes1.InvCipher(apData, awLen);

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
	lpUser->msClientVersion = t_fb_data->client_version() ? t_fb_data->client_version()->str() : "";
	lpUser->miPlatform = (ENUM_PLATFORM)t_fb_data->platform();
	lpUser->msDeviceCode = t_fb_data->device_code() ? t_fb_data->device_code()->str() : "";
	lpUser->msOsType = t_fb_data->os_type() ? t_fb_data->os_type()->str() : "";
	lpUser->miNetType = t_fb_data->net_type();
	lpUser->miServerTime = CSystem::GetMicroSystemTime();

	//平台验证(如果不对，踢掉)
	if(lpUser->miPlatform != PLATFORM_APP)
	{
		LOG_TRACE(LOG_WARNING, true, __FUNCTION__, "platform not app"
				<<" user="<< lpUser->miUserID);
		//踢掉用户
		KickUserID(lpUser, 0, false, RET_PLATFORM_ERROR);
		return 1;
	}

	unsigned char key2[] = "qianwang2017cook";
	AES aes2(key2);

	char *c_data = (char *)t_fb_data->cookie()->data();
	aes2.InvCipher((unsigned char *)c_data, t_fb_data->cookie()->Length());
	lpUser->msCookie = string(c_data, t_fb_data->cookie()->Length());
	string target_cookie = to_string(lpUser->miUserID) + lpUser->msTgt + lpUser->msDeviceCode + to_string(t_fb_data->client_time());
	if(lpUser->msCookie != target_cookie)
	{
		LOG_TRACE(LOG_WARNING, true, __FUNCTION__, "cookie invalid"
				<<" src_cookie:" << lpUser->msCookie
				<<" src_c_l:" << lpUser->msCookie.length()
				<<" origin_l:" << t_fb_data->cookie()->Length()
				<<" target_cookie:" << target_cookie
				<<" t_c_l:" << target_cookie.length()
				<<" user="<< lpUser->miUserID);
		//踢掉用户
		KickUserID(lpUser, 0, false, RET_USERINFO_BASE);
		return 1;
	}

	//验证密码
//    string password_info = mpServerMgr->mpRedisUtil->HGetRegPassWord(lpUser->miUserID);
//    if(password_info.empty())
//    {
//        LOG_TRACE(LOG_WARNING, true, __FUNCTION__, "user not register"
//                <<" src_cookie:" << lpUser->msCookie
//                <<" src_c_l:" << lpUser->msCookie.length()
//                <<" origin_l:" << t_fb_data->cookie()->Length()
//                <<" target_cookie:" << target_cookie
//                <<" t_c_l:" << target_cookie.length()
//                <<" user="<< lpUser->miUserID);
//        //踢掉用户
//        KickUserID(lpUser, 0, false, RET_USERINFO_BASE);
//        return 1;
//    }
//
//    RJ_CREATE_EMPTY_DOCUMENT(l_pwd_doc)
//    JsonParse::parse(l_pwd_doc, password_info.c_str());
//    uint64_t pwd_end_time;
//    JsonParse::get<uint64_t>(l_pwd_doc, "end_time", pwd_end_time);
//    if((lpUser->miServerTime/1000000) > pwd_end_time)
//    {
//
//    }

	//查看用户是不是已经登录了
	std::string new_s_session = std::to_string(goLocalServerInfo.mlServerID);
    std::string s_session = mpServerMgr->mpRedisUtil->HGetLoginInfo(lpUser->miUserID, lpUser->miPlatform);
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

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "recv "
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

	if (RET_SUCCESS != RouteSyncID(lpUser->miUserID, LOG_IN_TYPE))
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "route sync false "
				<<" user_id="<< lpUser->miUserID
				<<" platform="<< lpUser->miPlatform
		);
	}
	return 1;
}


INT CClientSide::onRecvTimeSyncRQ(WORD awPackType, SOCKET ahSocket,void* apRelationPtr,IPTYPE aiIP, WORD awPort, BYTE * apData, WORD awLen)
{
	GENERAL_UNPACK_BODY(STRU_TIME_SYNC_RQ, TRUE)
	int liResult = MAKE_SUCCESS_RESULT();

	auto t_fb_data = loUserLoad.t_fb_data;
	STRU_TIME_SYNC_RS loSendPack;
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(),
									t_fb_data->s_rq_head()->pack_session_id(),
									liResult, t_fb_data->s_rq_head()->platform());
	loSendPack.fbbuf = syspack::CreateT_TIME_SYNC_RS(loSendPack.fbbuilder, &s_rs_head, CSystem::GetMicroSystemTime());
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

	INT result = ServerPack::GetInstance().Send2LogicServer(awPackType, trans_body, goLocalServerInfo.mlServerID, lpUser->miUserID);
	if(result <= 0 && result != RET_SUCCESS)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "result = " << result << " awPackType = " << awPackType);
		KickUserID(lpUser, 0, false, RET_SYS_PACK_TYPE_INVALID);
		return result;
	}

	return result;
}

INT CClientSide::onRecvTransmitRQ2(WORD awPackType, SOCKET ahSocket,void* apRelationPtr,IPTYPE aiIP, WORD awPort, BYTE * apData, WORD awLen)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "recv "
			<< " onRecvTransmitRQ2 enter" << " awPackType = "
			<< awPackType << " apRelationPtr = " << apRelationPtr);

	CUser* lpUser = (CUser*)apRelationPtr;
	USERID temp_reg_uid = CSystem::GetMicroSystemTime();
    lpUser->miTempUserID = temp_reg_uid;
	mpServerMgr->moRegUserMgr.addUser(lpUser);
	STRU_TRANS_BODY_PACKAGE trans_body;
	trans_body.mpBuffer = apData;
	trans_body.mwLen = awLen;

	INT result = ServerPack::GetInstance().Send2LogicServer(awPackType, trans_body, goLocalServerInfo.mlServerID, temp_reg_uid);
	if(result <= 0 && result != RET_SUCCESS)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "result = " << result << " awPackType = " << awPackType);
		KickUserID(lpUser, 0, false, RET_SYS_PACK_TYPE_INVALID);
		return result;
	}

	return result;
}

INT CClientSide::KickUserID(CUser* client_user, const SESSIONID& comet_id, BOOL is_back, uint32 error_code)
{
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "kick user========"
			<<" user_id="<< client_user->miUserID);
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

INT CClientSide::UpdateTimeID(USERID user_id)
{
	if(user_id <= 0)
	{
		return RET_SUCCESS;
	}

	STRU_GROUP_UPDATE_LOGOUT_ID update_time_pack;
	update_time_pack.miUpdateTime = CSystem::GetMicroSystemTime();
	return ServerPack::GetInstance().Send2LogicServer(STRU_GROUP_UPDATE_LOGOUT_ID::ENUM_TYPE, update_time_pack,
													  goLocalServerInfo.mlServerID, user_id);
	
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
    return RET_SUCCESS;
}
