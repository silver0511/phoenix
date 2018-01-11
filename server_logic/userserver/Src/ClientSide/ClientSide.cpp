#include <ServerUtilMgr.h>
#include "stdafx.h"
#include "const/ErrorCodeDef.h"

#include "ClientSide.h"
#include "../ServerMgr.h"
#include "network/packet/client/CommonPackDef.h"
#include "redisclient/RedisUtil.h"
#include "rjson/JsonParser.h"

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

	if(!ClientPack::GetInstance().Open(this, lstruTcpOpt, ENUM_CONNECT_MODE_TCP))
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," Initialize tcp error.");
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
	CUser *lpUser = mpServerMgr->moUserConMgr.MallocRefObj();

	lpUser->mhSocket = ahSocket;
	lpUser->miIPAddr = aiIP;
	lpUser->miIPPort = awPort;
	lpUser->mpSocketInfo = apSocketInfo;
	lpUser->IncreaseRef();

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Connected "
			<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
			<<" Socket="<< ahSocket
			<<" SocketInfoPtr="<< apSocketInfo
			<<" UserRefCount="<< lpUser->GetRefCount()
	);
	ClientPack::GetInstance().AddRelationPtr(apSocketInfo,(void*)lpUser);
}

void CClientSide::OnSessionConnectFailure(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Connected " 
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
	CUser *lpUser = (CUser*)apRelationPtr;
	mpServerMgr->moUserConMgr.delUser(lpUser);
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Disconnected  delete user"
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

INT CClientSide::ServerDisconID(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	STRU_TRANS_BODY_PACKAGE trans_body;
	trans_body.mpBuffer = apData;
	trans_body.mwLen = awLen;

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "user kick process");
	CUser* lpUser = mpServerMgr->moUserConMgr.getUser(aiCometID, aiUserID);
	//fix bug当连接服务器停掉了，而redis中依旧存在这台连接服务器的用户信息时
	if(!lpUser)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "user kick failed lpUser is null user_id = " << aiUserID);
		return 1;
	}

	Send2Comet(STRU_SERVER_DISCON_ID::ENUM_TYPE ,trans_body, aiUserID, lpUser);
	return 1;
}

INT CClientSide::GetMeInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GET_ME_INFO_RQ, aiUserID, STRU_GET_ME_INFO_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << t_fb_data->s_rq_head()->user_id());

	char verification[50]={0};
	INT64 tt = GetRand();
	int platform = tt % USERSERVER_MOD_COUNT;
	sprintf (verification, "%ld",tt); 
	std::string verification_st(verification);
	std::string strUsr = std::to_string(aiUserID);
	mpServerMgr->mpRedisUtil->HSetTokenInfo(strUsr.c_str(),verification_st.c_str(),platform);

	std::string str = "";
	str = mpServerMgr->mpRedisUtil->LuaHGetUserInfo(strUsr.c_str());
	if (str == "")
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "user_id does not exist aiUserID = " << aiUserID);

		STRU_GET_ME_INFO_RS loSendPack;
		commonpack::S_RS_HEAD s_rs_head_result(t_fb_data->s_rq_head()->user_id(),
			t_fb_data->s_rq_head()->pack_session_id(),
			MAKE_ERROR_RESULT(RET_USERINFO_BASE), 
			t_fb_data->s_rq_head()->platform());
		
		loSendPack.fbbuf = userpack::CreateT_GET_ME_INFO_RSDirect(loSendPack.fbbuilder,
			&s_rs_head_result,
			nullptr,
			verification_st.c_str());
		Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
		return 0;
	}
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "---LuaHGetUserInfo return str = " << str.c_str());

	RJ_CREATE_EMPTY_DOCUMENT(doc)
	JsonParse::parse(doc, str.c_str());
	std::string  birthday;
	JsonParse::get(doc, "birthday", birthday);
	uint64_t birthdaynumber = std::atol(birthday.c_str());

	std::string  userid;
	JsonParse::get(doc, "user_id", userid);
	uint64_t useridnumber = std::atol(userid.c_str());

	std::string  sex;
	JsonParse::get(doc, "sex", sex);
	byte sexnumber = std::atoi(sex.c_str());

	auto messageid1 = t_fb_data->token();
	std::string messageid2;
	JsonParse::get(doc, "token", messageid2);
	std::string username;
	JsonParse::get(doc, "user_name", username);
	std::string city;
	JsonParse::get(doc, "city", city);
	std::string province;
	JsonParse::get(doc, "province", province);
	std::string signature;
	JsonParse::get(doc, "signature", signature);
	std::string str_mobile;
	JsonParse::get(doc, "mobile", str_mobile);
	uint64_t mobile = std::atol(str_mobile.c_str());
	std::string nickname;
	JsonParse::get(doc, "nick_name", nickname);
	std::string mail;
	JsonParse::get(doc, "mail", mail);

	LOG_TRACE(LOG_INFO, false, __FUNCTION__, " ---RS, user_id = " << t_fb_data->s_rq_head()->user_id()
		<< " birthday = " << birthday.c_str()
		<< " sex = " << sex.c_str()
		<< " token = " << messageid2.c_str()
		<< " username = " << username.c_str()
		<< " city = " << city.c_str()
		<< " province = " << province.c_str()
		<< " signature = " << signature.c_str()
		<< " mobile = " << str_mobile.c_str()
		<< " nickname = " << nickname.c_str()
		<< " mail = " << mail.c_str());
	if ( t_fb_data->token() != NULL && messageid2 != "" && messageid2.compare(messageid1->c_str()) == 0 )
	{
		STRU_GET_ME_INFO_RS loSendPack;
		commonpack::S_RS_HEAD s_rs_head_result(t_fb_data->s_rq_head()->user_id(),
			t_fb_data->s_rq_head()->pack_session_id(),
			MAKE_ERROR_RESULT(RET_GETUSERINFO_BASE), 
			t_fb_data->s_rq_head()->platform());

		loSendPack.fbbuf = userpack::CreateT_GET_ME_INFO_RSDirect(loSendPack.fbbuilder,
			&s_rs_head_result,
			messageid2.c_str(),
			verification_st.c_str());
		Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
		return 0;
	}
	else
	{
		STRU_GET_ME_INFO_RS loSendPack;
		loSendPack.fbbuf = userpack::CreateT_GET_ME_INFO_RSDirect(loSendPack.fbbuilder,
			&s_rs_head,
			messageid2.c_str(),
			verification_st.c_str(),
			useridnumber,
			username.c_str(),
			birthdaynumber,
			city.c_str(),
			province.c_str(),
			signature.c_str(),
			mobile,
			nickname.c_str(),
			sexnumber,
			mail.c_str());
		Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	}

    return 1;
}


INT CClientSide::GetUserInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GET_USER_INFO_RQ, aiUserID, STRU_GET_USER_INFO_RS::ENUM_TYPE)
		GENERAL_PACK_SAME_BEGIN()

		LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << t_fb_data->s_rq_head()->user_id());

	const std::string &str_user_msg = t_fb_data->user_msg()->str();
	std::string str = "";
	str = mpServerMgr->mpRedisUtil->LuaHGetUserInfo(t_fb_data->user_msg()->str());
	if (str == "")
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "user_id does not exist user_msg = " << t_fb_data->user_msg());

		STRU_GET_USER_INFO_RS loSendPack;
		commonpack::S_RS_HEAD s_rs_head_result(t_fb_data->s_rq_head()->user_id(),
			t_fb_data->s_rq_head()->pack_session_id(),
			MAKE_ERROR_RESULT(RET_USERINFO_BASE), 
			t_fb_data->s_rq_head()->platform());

		loSendPack.fbbuf = userpack::CreateT_GET_USER_INFO_RSDirect(loSendPack.fbbuilder,
			&s_rs_head_result,
			str_user_msg.c_str());
		Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
		return 0;
	}

	RJ_CREATE_EMPTY_DOCUMENT(doc)
		JsonParse::parse(doc, str.c_str());
	std::string  birthday;
	JsonParse::get(doc, "birthday", birthday);
	uint64_t birthdaynumber = std::atol(birthday.c_str());

	std::string  userid;
	JsonParse::get(doc, "user_id", userid);
	uint64_t useridnumber = std::atol(userid.c_str());

	std::string  sex;
	JsonParse::get(doc, "sex", sex);
	byte sexnumber = std::atoi(sex.c_str());

	std::string username;
	JsonParse::get(doc, "user_name", username);
	std::string city;
	JsonParse::get(doc, "city", city);
	std::string province;
	JsonParse::get(doc, "province", province);
	std::string signature;
	JsonParse::get(doc, "signature", signature);
	std::string str_mobile;
	JsonParse::get(doc, "mobile", str_mobile);
	uint64_t mobile = std::atol(str_mobile.c_str());
	std::string nickname;
	JsonParse::get(doc, "nick_name", nickname);
	std::string mail;
	JsonParse::get(doc, "mail", mail);

	LOG_TRACE(LOG_INFO, false, __FUNCTION__, " ---RS, user_id = " << t_fb_data->s_rq_head()->user_id()
		<< " t_fb_data->user_msg() = " << str_user_msg.c_str()
		<< " birthday = " << birthday.c_str()
		<< " sex = " << sex.c_str()
		<< " username = " << username.c_str()
		<< " city = " << city.c_str()
		<< " province = " << province.c_str()
		<< " signature = " << signature.c_str()
		<< " mobile = " << str_mobile.c_str()
		<< " nickname = " << nickname.c_str()
		<< " mail = " << mail.c_str());

	STRU_GET_USER_INFO_RS loSendPack;
	loSendPack.fbbuf = userpack::CreateT_GET_USER_INFO_RSDirect(loSendPack.fbbuilder,
		&s_rs_head,
		str_user_msg.c_str(),
		useridnumber,
		username.c_str(),
		birthdaynumber,
		city.c_str(),
		province.c_str(),
		signature.c_str(),
		mobile,
		nickname.c_str(),
		sexnumber,
		mail.c_str());
	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	return 1;
}

INT CClientSide::GetUserlstInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GET_USERLST_INFO_RQ, aiUserID, STRU_GET_USERLST_INFO_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << t_fb_data->s_rq_head()->user_id());
	STRU_GET_USERLST_INFO_RS loSendPack;
	std::vector<flatbuffers::Offset<userpack::T_GET_USERINFO> > vector_userlst;
	std::vector<std::string> vector_user_lst_info;
	std::vector<uint64_t> usernolst;
	if (t_fb_data->userLst() != NULL && t_fb_data->userLst()->size() > 0)
	{
		std::vector<std::string> userlst;
		for (int i = 0;i<t_fb_data->userLst()->size() && i < 50;i++)
		{
			auto userid = t_fb_data->userLst()->Get(i);
			userlst.push_back(std::to_string(userid));
		}
		
		int qrr = mpServerMgr->mpRedisUtil->LuaHGetUserLst(userlst, &vector_user_lst_info);
		if (qrr < 0)
		{
			auto userid = t_fb_data->userLst()->Get(0);
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "user does not exist user_id = " << userid);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_GET_USERLST_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_USERINFO_BASE));
			return 0;
		}

	} 
	else if (t_fb_data->mobileLst() != NULL && t_fb_data->mobileLst()->size() > 0)
	{
		std::vector<std::string> mobilelst;
		for (int i = 0;i<t_fb_data->mobileLst()->size() && i < 50;i++)
		{
			auto mobile = t_fb_data->mobileLst()->Get(i);
			mobilelst.push_back(std::to_string(mobile));
			LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---RQ,  mobile->c_str()=" << mobile);
		}
		
		int qrr = mpServerMgr->mpRedisUtil->LuaHGetUserLstByMobile(mobilelst, &vector_user_lst_info);
		if (qrr < 0)
		{
			auto mobile = t_fb_data->mobileLst()->Get(0);
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "user does not exist mobile = " << mobile);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_GET_USERLST_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_USERINFO_BASE));
			return 0;
		}

	} 
	else
	{
		LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---RQ,  no userLst or mobileLst " );
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GET_USERLST_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_USERINFO_BASE));
		return 0;
	}

	for (int i = 0;i<vector_user_lst_info.size() && i < 50;i++)
	{
		if (vector_user_lst_info[i].empty())
		{

			if (t_fb_data->userLst() != NULL && t_fb_data->userLst()->size() > 0)
			{
				auto userid = t_fb_data->userLst()->Get(i);
				usernolst.push_back(userid);
			} 
			else if (t_fb_data->mobileLst() != NULL && t_fb_data->mobileLst()->size() > 0)
			{
				auto mobile = t_fb_data->mobileLst()->Get(i);
				usernolst.push_back(mobile);
			}
			continue;
		}

		RJ_CREATE_EMPTY_DOCUMENT(doc)
			JsonParse::parse(doc, vector_user_lst_info[i].c_str());
		std::string username;
		JsonParse::get(doc, "user_name", username);
		std::string nickname;
		JsonParse::get(doc, "nick_name", nickname);
		std::string str_mobile;
		JsonParse::get(doc, "mobile", str_mobile);
		uint64_t mobile = std::atol(str_mobile.c_str());
		std::string  userid;
		JsonParse::get(doc, "user_id", userid);
		uint64_t useridnumber = std::atol(userid.c_str());
		flatbuffers::Offset<userpack::T_GET_USERINFO> user_info = userpack::CreateT_GET_USERINFODirect(loSendPack.fbbuilder,
			useridnumber,
			username.c_str(),
			nickname.c_str(),
			mobile);
		vector_userlst.push_back(user_info);

		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << t_fb_data->s_rq_head()->user_id()
			<< " username = " << username.c_str()
			<< " nickname = " << nickname.c_str());
	}
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  aiUserID=" << t_fb_data->s_rq_head()->user_id());
	auto vector_list_builder = loSendPack.fbbuilder.CreateVector(vector_userlst);
	if (usernolst.size() > 0)
	{
		auto vector_usernolst_builder = loSendPack.fbbuilder.CreateVector(usernolst);
		loSendPack.fbbuf = userpack::CreateT_GET_USERLST_INFO_RS(loSendPack.fbbuilder,
			&s_rs_head,
			vector_list_builder,
			vector_usernolst_builder);
	} 
	else
	{
		loSendPack.fbbuf = userpack::CreateT_GET_USERLST_INFO_RS(loSendPack.fbbuilder,
			&s_rs_head,
			vector_list_builder,
			0);
	}

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	return 1;
}

INT CClientSide::UpdateUserInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_UPDATE_USER_INFO_RQ, aiUserID, STRU_T_UPDATE_USER_INFO_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << t_fb_data->s_rq_head()->user_id());

	std::string str_user_id = std::to_string(t_fb_data->s_rq_head()->user_id());
	int namenumber = 0;
	for(int i=0;i <= str_user_id.size();i++)
	{
		namenumber += int(str_user_id[i]);
	}
	int platform = namenumber % USERSERVER_MOD_COUNT;

	std::string str = mpServerMgr->mpRedisUtil->HGetUserInfo(t_fb_data->s_rq_head()->user_id(),platform);
	TEXTMSGID message_id = CSystem::GetMicroSystemTime();
	
	if (str == "")
	{
		RJ_CREATE_O_DOCUMENT(doc, l_allocator)

		JsonParse::add(doc, "user_id", std::to_string( t_fb_data->s_rq_head()->user_id()), l_allocator);
		JsonParse::add(doc, "token", std::to_string(message_id), l_allocator);

		std::string mobile = "0";
		std::string user_name = "0";
		std::string mail = "0";
		bool uinfo[9]={false};
		for (int i = 0; i < t_fb_data->key_lst_info()->size();i++)
		{
			auto keydata = t_fb_data->key_lst_info()->Get(i);
			LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---addUserInfo,  keydata->key_value()=" << keydata->key_value()->c_str());
			switch(keydata->key_name())
			{
			case key_user_name:
				{
					if (keydata->key_value() != NULL && !keydata->key_value()->str().empty() && !uinfo[0])
					{
						JsonParse::add(doc, "user_name", keydata->key_value()->str(), l_allocator);
						user_name = keydata->key_value()->c_str();
						uinfo[0]= true;
					}
				}
				break;
			case key_birthday:
				{
					if (keydata->key_value() != NULL && !keydata->key_value()->str().empty() && !uinfo[1])
					{
						JsonParse::add(doc, "birthday", keydata->key_value()->str(), l_allocator);
						uinfo[1]= true;
					}
				}
				break;
			case key_city:
				{
					if (keydata->key_value() != NULL && !keydata->key_value()->str().empty() && !uinfo[2])
					{
						JsonParse::add(doc, "city", keydata->key_value()->str(), l_allocator);
						uinfo[2]= true;
					}
				}
				break;
			case key_signature:
				{
					if (keydata->key_value() != NULL && !keydata->key_value()->str().empty() && !uinfo[3])
					{
						JsonParse::add(doc, "signature", keydata->key_value()->str(), l_allocator);
						uinfo[3]= true;
					}
				}
				break;
			case key_province:
				{
					if (keydata->key_value() != NULL && !keydata->key_value()->str().empty() && !uinfo[4])
					{
						JsonParse::add(doc, "province", keydata->key_value()->str(), l_allocator);
						uinfo[4]= true;
					}
				}
				break;
			case key_mobile:
				{
					if (keydata->key_value() != NULL && !keydata->key_value()->str().empty() && !uinfo[5])
					{
						JsonParse::add(doc, "mobile", keydata->key_value()->str(), l_allocator);
						mobile = keydata->key_value()->c_str();
						uinfo[5]= true;
					}
				}
				break;
			case key_nick_name:
				{
					if (keydata->key_value() != NULL && !keydata->key_value()->str().empty() && !uinfo[6])
					{
						JsonParse::add(doc, "nick_name", keydata->key_value()->str(), l_allocator);
						uinfo[6]= true;
					}
				}
				break;
			case key_sex:
				{
					if (keydata->key_value() != NULL && !keydata->key_value()->str().empty() && !uinfo[7])
					{
						JsonParse::add(doc, "sex", keydata->key_value()->str(), l_allocator);
						uinfo[7]= true;
					}
				}
				break;
			case key_mail:
				{
					if (keydata->key_value() != NULL && !keydata->key_value()->str().empty() && !uinfo[8])
					{
						JsonParse::add(doc, "mail", keydata->key_value()->str(), l_allocator);
						mail = keydata->key_value()->str();
						uinfo[8]= true;
					}
				}
				break;
			default:
				LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "key does not exist key_name = " << keydata->key_name());
				TEMPLATE_GENERAL_FAIL_RESULT(STRU_T_UPDATE_USER_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_UPDATEUSERINFO_BASE));
				break;
			}

		}
		if (user_name == "0")
		{
			LOG_TRACE(LOG_ERR,true,__FUNCTION__,"no user_name .");
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_T_UPDATE_USER_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_NOATTRIBUTE_BASE));
			return 0;
		}
		str = mpServerMgr->mpRedisUtil->LuaCheckUserMsg(mobile,user_name,mail);
		if (str == "1")
		{
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "mobile is already  exist user_id = " << t_fb_data->s_rq_head()->user_id());
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_T_UPDATE_USER_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_MOBILE_BASE))
			return 0;
		}
		else if (str == "2")
		{
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "user_name is already exist user_id = " << t_fb_data->s_rq_head()->user_id());
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_T_UPDATE_USER_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_USERNAME_BASE))
			return 0;
		}
		else if (str == "3")
		{
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "mail is already  exist user_id = " << t_fb_data->s_rq_head()->user_id());
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_T_UPDATE_USER_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_MAIL_BASE))
			return 0;
		}
		std::string out_json = "";
		JsonParse::to_string(doc, out_json);
		LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---addUserInfo,  out_json=" << out_json.c_str());

		int rtn = mpServerMgr->mpRedisUtil->LuaHAddUserInfo(t_fb_data->s_rq_head()->user_id(), out_json,mobile, user_name,mail);
		if (rtn < 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "LuaHSetUserInfo error rtn= " << rtn
				<< " out_json= " << out_json.c_str());
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_T_UPDATE_USER_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_REQ_REDIS_ERROR));
			return 0;
		}
	} 
	else
	{
		RJ_CREATE_EMPTY_DOCUMENT(doc)
		JsonParse::parse(doc, str.c_str());
		RJAllocType& l_allocator = doc.GetAllocator();
		std::string token = std::to_string(message_id);
		JsonParse::add(doc, "token", token, l_allocator);
		for (int i = 0; i < t_fb_data->key_lst_info()->size();i++)
		{
			auto keydata = t_fb_data->key_lst_info()->Get(i);
			LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---updataUserInfo,  keydata->key_value()=" << keydata->key_value()->c_str());
			switch(keydata->key_name())
			{
			case key_user_name:
				break;
			case key_birthday:
				if (keydata->key_value() != NULL && !keydata->key_value()->str().empty())
				{
					std::string str1 = keydata->key_value()->str();
					JsonParse::add(doc, "birthday", str1, l_allocator);
				}
				break;
			case key_city:
				if (keydata->key_value() != NULL && !keydata->key_value()->str().empty())
				{
					JsonParse::add(doc, "city", keydata->key_value()->str(), l_allocator);
				}
				break;
			case key_mobile:
				break;
			case key_nick_name:
				if (keydata->key_value() != NULL && !keydata->key_value()->str().empty())
				{
					JsonParse::add(doc, "nick_name", keydata->key_value()->str(), l_allocator);
				}
				break;
			case key_signature:
				if (keydata->key_value() != NULL && !keydata->key_value()->str().empty())
				{
					JsonParse::add(doc, "signature", keydata->key_value()->str(), l_allocator);
				}
				break;
			case key_province:
				if (keydata->key_value() != NULL && !keydata->key_value()->str().empty())
				{
					JsonParse::add(doc, "province", keydata->key_value()->str(), l_allocator);
				}
				break;
			case key_sex:
				if (keydata->key_value() != NULL && !keydata->key_value()->str().empty())
				{
					JsonParse::add(doc, "sex", keydata->key_value()->str(), l_allocator);
				}
				break;
			case key_mail:
				//if (keydata->key_value() != NULL && !keydata->key_value()->str().empty())
				//{
				//	JsonParse::add(doc, "mail", keydata->key_value()->str(), l_allocator);
				//}
				break;
			default:
				LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "key does not exist key_name = " << keydata->key_name());
				TEMPLATE_GENERAL_FAIL_RESULT(STRU_T_UPDATE_USER_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_UPDATEUSERINFO_BASE));
				return 0;
				break;
			}

		}
		std::string out_json = "";
		JsonParse::to_string(doc, out_json);
		int rtn = mpServerMgr->mpRedisUtil->HSetUserInfo(t_fb_data->s_rq_head()->user_id(),platform,out_json);
		if (rtn < 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "LuaHSetUserInfo error rtn= " << rtn
				<< " out_json= " << out_json.c_str());
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_T_UPDATE_USER_INFO_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_REQ_REDIS_ERROR));
			return 0;
		}
	}
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  aiUserID=" << t_fb_data->s_rq_head()->user_id());
	STRU_T_UPDATE_USER_INFO_RS loSendPack;
	std::vector<flatbuffers::Offset<userpack::T_KEYINFO> > vectorKeyInfo;
	for (int i = 0; i < t_fb_data->key_lst_info()->size();i++)
	{
		auto keysinfo = t_fb_data->key_lst_info()->Get(i);
		flatbuffers::Offset<userpack::T_KEYINFO> key_info = userpack::CreateT_KEYINFODirect(loSendPack.fbbuilder,
			keysinfo->key_name(),
			keysinfo->key_value()->c_str());
		vectorKeyInfo.push_back(key_info);
	}

	auto vector_list_builder = loSendPack.fbbuilder.CreateVector(vectorKeyInfo);
	loSendPack.fbbuf = userpack::CreateT_UPDATE_USER_INFO_RS(loSendPack.fbbuilder,
		&s_rs_head,
		vector_list_builder);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}

INT CClientSide::UserComplaintRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_USER_COMPLAINT_RQ, aiUserID, STRU_USER_COMPLAINT_RS::ENUM_TYPE)
		GENERAL_PACK_SAME_BEGIN()

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << t_fb_data->s_rq_head()->user_id());
	if (t_fb_data->type() < regulations || t_fb_data->type() > others || t_fb_data->user_id() == t_fb_data->s_rq_head()->user_id())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," UserComplaintRQ t_fb_data->type() error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_USER_COMPLAINT_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_COMPLAINTTYPE_BASE));
		return 0;
	}
	TEXTMSGID message_id = CSystem::GetMicroSystemTime();
	std::string out_json = "";
	int platform = t_fb_data->user_id() % USERSERVER_MOD_COUNT;
	std::string str = mpServerMgr->mpRedisUtil->HGetUserComplaintInfo(t_fb_data->user_id(),platform);
	if (str == "")
	{
		RJ_CREATE_O_DOCUMENT(doc, l_allocator)
			RJ_CREATE_ARRAY(child_array)
			RJ_CREATE_OBJECT(child_obj)
		JsonParse::add(child_obj, "user_id", std::to_string( t_fb_data->s_rq_head()->user_id()), l_allocator);
		JsonParse::add(child_obj, "time", std::to_string(message_id), l_allocator);
		if (t_fb_data->reason() != NULL && !t_fb_data->reason()->str().empty() && t_fb_data->type() == 5)
		{
			JsonParse::add(child_obj, "reason", t_fb_data->reason()->str(), l_allocator);
		}
		JsonParse::push(child_array, child_obj, l_allocator);
		std::string strType = std::to_string(t_fb_data->type());
		JsonParse::add(doc, strType.c_str(), child_array, l_allocator);
		
		JsonParse::to_string(doc, out_json);
	}
	else
	{
		RJ_CREATE_EMPTY_DOCUMENT(doc)
		JsonParse::parse(doc, str.c_str());
		RJAllocType& l_allocator = doc.GetAllocator();
		RJ_CREATE_ARRAY(child_array)
		RJ_CREATE_OBJECT(child_obj)
		JsonParse::add(child_obj, "user_id", std::to_string( t_fb_data->s_rq_head()->user_id()), l_allocator);
		JsonParse::add(child_obj, "time", std::to_string(message_id), l_allocator);
		if (t_fb_data->reason() != NULL && !t_fb_data->reason()->str().empty() && t_fb_data->type() == 5)
		{
			JsonParse::add(child_obj, "reason", t_fb_data->reason()->str(), l_allocator);
		}

		std::string strType = std::to_string(t_fb_data->type());
		RJsonValue &child_object = JsonParse::get(doc, strType.c_str());
		if(!child_object.IsNull())
		{
			JsonParse::push(child_object, child_obj, l_allocator);
		}
		else
		{
			JsonParse::push(child_array, child_obj, l_allocator);
			JsonParse::add(doc, strType.c_str(), child_array, l_allocator);
		}

		JsonParse::to_string(doc, out_json);
	}
	int rtn = mpServerMgr->mpRedisUtil->HSetUserComplaintInfo(t_fb_data->user_id(),platform,out_json);
	if (rtn < 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "LuaHSetUserInfo error rtn= " << rtn
			<< " out_json= " << out_json.c_str());
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_USER_COMPLAINT_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_REQ_REDIS_ERROR));
		return 0;
	}
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  aiUserID=" << t_fb_data->s_rq_head()->user_id());
	
	STRU_USER_COMPLAINT_RS loSendPack;
	if (t_fb_data->reason() != NULL && !t_fb_data->reason()->str().empty() && t_fb_data->type() == 5)
	{
		const std::string &str_reason = t_fb_data->reason()->str();
		loSendPack.fbbuf = userpack::CreateT_USER_COMPLAINT_RSDirect(loSendPack.fbbuilder,
			&s_rs_head,
			t_fb_data->user_id(),
			t_fb_data->type(),
			str_reason.c_str());
	}
	else
	{
		loSendPack.fbbuf = userpack::CreateT_USER_COMPLAINT_RSDirect(loSendPack.fbbuilder,
			&s_rs_head,
			t_fb_data->user_id(),
			t_fb_data->type());
	}

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

}

INT CClientSide::ChangeMobileRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_CHANGE_MOBILE_RQ, aiUserID, STRU_CHANGE_MOBILE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	std::string str_user_id = std::to_string(t_fb_data->s_rq_head()->user_id());
	int namenumber = 0;
	for(int i=0;i <= str_user_id.size();i++)
	{
		namenumber += int(str_user_id[i]);
	}
	int platform = namenumber % USERSERVER_MOD_COUNT;

	std::string str = mpServerMgr->mpRedisUtil->HGetUserInfo(t_fb_data->s_rq_head()->user_id(),platform);
	TEXTMSGID message_id = CSystem::GetMicroSystemTime();

	if (str == "")
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "user does not exist user_id = " << str_user_id);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CHANGE_MOBILE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_USERINFO_BASE));
		return 0;
	}

	RJ_CREATE_EMPTY_DOCUMENT(doc)
		JsonParse::parse(doc, str.c_str());
	std::string  str_mobile;
	JsonParse::get(doc, "mobile", str_mobile);
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "--- mobile=" << str_mobile);
	if (str_mobile != "")
	{
		if (str_mobile.compare(to_string(t_fb_data->new_moblie())) == 0 || t_fb_data->new_moblie() == t_fb_data->old_moblie())
		{
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "新手机号码不能与旧手机号码相同 user_id = " << str_user_id);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_CHANGE_MOBILE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_NEW_MOBILE_BASE));
			return 0;
		}

		if (str_mobile.compare(to_string(t_fb_data->old_moblie())) != 0)
		{
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "旧手机号码验证失败 user_id = " << str_user_id);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_CHANGE_MOBILE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_CHECK_MOBILE_BASE));
			return 0;
		}
	}

	RJAllocType& l_allocator = doc.GetAllocator();
	std::string token = std::to_string(message_id);
	JsonParse::add(doc, "token", token, l_allocator);
	std::string new_moblie = std::to_string(t_fb_data->new_moblie());
	JsonParse::add(doc, "mobile", new_moblie, l_allocator);
	std::string old_moblie = std::to_string(t_fb_data->old_moblie());

	if(new_moblie == "")
	{
			new_moblie = "0";
	}
	if(old_moblie == "")
	{
			old_moblie = "0";
	}
	std::string out_json = "";
	JsonParse::to_string(doc, out_json);
	//int rtn = mpServerMgr->mpRedisUtil->HSetUserInfo(t_fb_data->s_rq_head()->user_id(),platform,out_json);
	int rtn = mpServerMgr->mpRedisUtil->LuaChangeMobile(t_fb_data->s_rq_head()->user_id(), out_json,new_moblie,old_moblie);
	if (rtn < 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "--- error rtn= " << rtn
			<< " out_json= " << out_json.c_str());
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CHANGE_MOBILE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_REQ_REDIS_ERROR));
		return 0;
	}

	STRU_CHANGE_MOBILE_RS loSendPack;

	loSendPack.fbbuf = userpack::CreateT_CHANGE_MOBILE_RS(loSendPack.fbbuilder,
		&s_rs_head,
		t_fb_data->old_moblie(),
		t_fb_data->new_moblie());

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  aiUserID=" << aiUserID);
}

INT CClientSide::ChangeMailRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_CHANGE_MAIL_RQ, aiUserID, STRU_CHANGE_MAIL_RS::ENUM_TYPE)
		GENERAL_PACK_SAME_BEGIN()

	std::string str_user_id = std::to_string(t_fb_data->s_rq_head()->user_id());
	int namenumber = 0;
	for(int i=0;i <= str_user_id.size();i++)
	{
		namenumber += int(str_user_id[i]);
	}
	int platform = namenumber % USERSERVER_MOD_COUNT;

	std::string str = mpServerMgr->mpRedisUtil->HGetUserInfo(t_fb_data->s_rq_head()->user_id(),platform);
	TEXTMSGID message_id = CSystem::GetMicroSystemTime();

	if (str == "")
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "user does not exist user_id = " << str_user_id);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CHANGE_MAIL_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_USERINFO_BASE));
		return 0;
	}

	RJ_CREATE_EMPTY_DOCUMENT(doc)
		JsonParse::parse(doc, str.c_str());
	std::string  str_mail;
	JsonParse::get(doc, "mail", str_mail);
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "--- mail=" << str_mail);
	std::string new_mail = t_fb_data->new_mail()->c_str();
	std::string old_mail = t_fb_data->old_mail()->c_str();

	if (str_mail != "")
	{
		if (str_mail.compare(new_mail) == 0 || (new_mail.compare(old_mail) == 0))
		{
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "新邮箱不能与旧邮箱相同 user_id = " << str_user_id);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_CHANGE_MAIL_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_NEW_MAIL_BASE));
			return 0;
		}

		if (str_mail.compare(old_mail) != 0)
		{
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "旧邮箱验证失败 user_id = " << str_user_id);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_CHANGE_MOBILE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_CHECK_MAIL_BASE));
			return 0;
		}
	}

	RJAllocType& l_allocator = doc.GetAllocator();
	std::string token = std::to_string(message_id);
	JsonParse::add(doc, "token", token, l_allocator);
	JsonParse::add(doc, "mail", new_mail, l_allocator);

	std::string out_json = "";
	JsonParse::to_string(doc, out_json);
	if(new_mail == "")
	{
			new_mail = "0";
	}
	if(old_mail == "")
	{
			old_mail = "0";
	}
	//int rtn = mpServerMgr->mpRedisUtil->HSetUserInfo(t_fb_data->s_rq_head()->user_id(),platform,out_json);
	int rtn = mpServerMgr->mpRedisUtil->LuaChangeMail(t_fb_data->s_rq_head()->user_id(), out_json,new_mail,old_mail);
	if (rtn < 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "--- error rtn= " << rtn
			<< " out_json= " << out_json.c_str());
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CHANGE_MAIL_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_REQ_REDIS_ERROR));
		return 0;
	}

	STRU_CHANGE_MAIL_RS loSendPack;
	loSendPack.fbbuf = userpack::CreateT_CHANGE_MAIL_RSDirect(loSendPack.fbbuilder,
		&s_rs_head,
		t_fb_data->old_mail()->c_str(),
		t_fb_data->new_mail()->c_str());

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  aiUserID=" << aiUserID);
}

INT CClientSide::RegisterApnsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_REGISTER_APNS_RQ, aiUserID, STRU_REGISTER_APNS_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	const std::string &device_token = t_fb_data->device_token()->str();
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, " user_id=" << t_fb_data->s_rq_head()->user_id() << " device_token=" << device_token);

	if(device_token == "<0000000000000000000000000000000000000000000000000000000000000000>" ||
			device_token == "0000000000000000000000000000000000000000000000000000000000000000")
	{
        std::string l_origin_device_token = mpServerMgr->mpRedisUtil->HGetDeviceToken(aiUserID);
		mpServerMgr->mpRedisUtil->HDelDeviceToken(aiUserID);
        mpServerMgr->mpRedisUtil->HDelDeviceUser(l_origin_device_token);
	}
	else
	{
        std::string s_origin_user_id = mpServerMgr->mpRedisUtil->HGetDeviceUser(device_token);
        USERID l_origin_user_id = ServerUtilMgr::STR2UINT64(s_origin_user_id);
        if(l_origin_user_id != aiUserID)
        {
            mpServerMgr->mpRedisUtil->HDelDeviceToken(l_origin_user_id);
        }
        mpServerMgr->mpRedisUtil->HSetDeviceUser(device_token, aiUserID);
        mpServerMgr->mpRedisUtil->HSetDeviceToken(aiUserID, device_token);
	}

	STRU_REGISTER_APNS_RS loSendPack;
	loSendPack.fbbuf = userpack::CreateT_REGISTER_APNS_RSDirect(loSendPack.fbbuilder, &s_rs_head, device_token.c_str());

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	return 1;
}
