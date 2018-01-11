#include "stdafx.h"
#include "const/ErrorCodeDef.h"

#include "ClientSide.h"
#include "../ServerMgr.h"
#include "ServerToKafka.h"
#include "network/packet/client/CommonPackDef.h"
#include "network/packet/client/OffcialServerPackDef.h"
#include "const/PackDef.h"
#include "const/SvrConstDef.h"
#include "ServerUtilMgr.h"
#include "../OffciaToKafka/COffciaDistributeMgr.h"
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

INT CClientSide::OffcialSendSysMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	CUser* lpUser = (CUser*)apRelationPtr;
	STRU_GENERAL_BODY_PACKAGE<STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RQ> loRecvPack;
	const STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RQ &loUserLoad = loRecvPack.moUserLoad;
	INT nResult = loRecvPack.UnPack(apData, awLen);
	if (0 >= nResult)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error user_id is invalid user_id = "<< aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_UNPACK_FAILED_RESULT));
		return 0;
	}
	auto t_fb_data = loUserLoad.t_fb_data;
	int liResult = MAKE_SUCCESS_RESULT();
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(),
		t_fb_data->s_rq_head()->pack_session_id(),
		liResult, 
		t_fb_data->s_rq_head()->platform());
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);

	if (!t_fb_data->s_msg())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->s_msg() error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(!t_fb_data->s_msg()->msg_content() || !t_fb_data->s_msg()->msg_content()->c_str())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->s_msg()->msg_content() error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(!t_fb_data->s_msg()->send_user_name() || !t_fb_data->s_msg()->send_user_name()->c_str())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->s_msg()->send_user_name() error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	std::vector<uint64_t> vector_fans_lst;

	flatbuffers::FlatBufferBuilder fbbuilder_sys;
	flatbuffers::Offset<commonpack::S_MSG> s_server_sys_msg = commonpack::CreateS_MSGDirect(fbbuilder_sys,
		t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
		t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
		t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
		t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
		t_fb_data->s_msg()->send_user_name()->c_str());
	fbbuilder_sys.Finish(s_server_sys_msg);

	TEXTMSGID message_id = CSystem::GetSystemTime();
	message_id = message_id << 1;
	int offcial_id = DEF_SYS_OFFCIAL_ID;
	flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_sys_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
		fbbuilder_sys,
		message_id,
		offcial_id,
		s_server_sys_msg);
	fbbuilder_sys.Finish(msg_sys_info);
	BYTE*	lpData = fbbuilder_sys.GetBufferPointer();

	std::string strElement;
	strElement.assign((char*)lpData, fbbuilder_sys.GetSize());
	mpServerMgr->mpRedisUtil->ZaddOffcialSysChatInfo(message_id, strElement);


	RJ_CREATE_O_DOCUMENT(doc, l_allocator)
		JsonParse::add<unsigned>(doc, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
	JsonParse::add<unsigned>(doc, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
	JsonParse::add<unsigned>(doc, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
	JsonParse::add<unsigned>(doc, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
	JsonParse::add<unsigned>(doc, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
	JsonParse::add<unsigned>(doc, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
	JsonParse::add<uint64_t>(doc, "msg_time", (uint64_t)t_fb_data->s_msg()->msg_time(), l_allocator);
	string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
	JsonParse::add(doc, "send_user_name", send_user_name, l_allocator);
	string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
	JsonParse::add(doc, "msg_content", msg_content, l_allocator);
	JsonParse::add<uint64_t>(doc, "offcial_id", (uint64_t)offcial_id, l_allocator);
	JsonParse::add<uint64_t>(doc, "message_id", (uint64_t)message_id, l_allocator);
	JsonParse::add<uint64_t>(doc, "group_id", (uint64_t)0, l_allocator);
	std::string out_json = "";
	JsonParse::to_string(doc, out_json);

	LONG nRedisResult = 0;
	mpServerMgr->mpRedisUtil->LpushOffcialMsgdb(offcial_id,out_json,nRedisResult);
	if(nRedisResult <= 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " LpushOffcialMsgdb err. ");
	}

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  aiUserID=" << aiUserID);
	STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RS loSendPack;
	auto s_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder, t_fb_data->s_msg()->app_id(),
		t_fb_data->s_msg()->session_id(), t_fb_data->s_msg()->chat_type(),
		t_fb_data->s_msg()->m_type(), t_fb_data->s_msg()->s_type(),
		t_fb_data->s_msg()->ext_type(), t_fb_data->s_msg()->msg_content()->c_str(),
		t_fb_data->s_msg()->msg_time(), t_fb_data->s_msg()->send_user_name()->c_str());
	loSendPack.fbbuilder.Finish(s_msg);
	auto vector_list_builder = loSendPack.fbbuilder.CreateVector(vector_fans_lst);
	loSendPack.fbbuf = offcialpack::CreateT_CLIENT_OFFCIAL_SEND_SYS_MSG_RS(loSendPack.fbbuilder,
		&s_rs_head,s_msg);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
}

INT CClientSide::OffcialSendSomeSysMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	CUser* lpUser = (CUser*)apRelationPtr;
	STRU_GENERAL_BODY_PACKAGE<STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RQ> loRecvPack;
	const STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RQ &loUserLoad = loRecvPack.moUserLoad;
	INT nResult = loRecvPack.UnPack(apData, awLen);
	if (0 >= nResult)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error user_id is invalid user_id = "<< aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_UNPACK_FAILED_RESULT));
		return 0;
	}
	auto t_fb_data = loUserLoad.t_fb_data;
	int liResult = MAKE_SUCCESS_RESULT();
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(),
		t_fb_data->s_rq_head()->pack_session_id(),
		liResult, 
		t_fb_data->s_rq_head()->platform());
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);

	if (!t_fb_data->s_msg())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->s_msg error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(!t_fb_data->s_msg()->msg_content() || !t_fb_data->s_msg()->msg_content()->c_str())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->s_msg()->msg_content error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(!t_fb_data->s_msg()->send_user_name() || !t_fb_data->s_msg()->send_user_name()->c_str())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage send_user_name error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}


	if( NULL == t_fb_data->to_fans_lst())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->to_fans_lst error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(!t_fb_data->group_id())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->group_id error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	std::vector<uint64_t> vector_fans_lst;

	flatbuffers::FlatBufferBuilder fbbuilder_sys;
	flatbuffers::Offset<commonpack::S_MSG> s_server_sys_msg = commonpack::CreateS_MSGDirect(fbbuilder_sys,
		t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
		t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
		t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
		t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
		t_fb_data->s_msg()->send_user_name()->c_str());
	fbbuilder_sys.Finish(s_server_sys_msg);

	TEXTMSGID message_id = 0;

	if (t_fb_data->message_id() > 0)
	{
		std::map<TEXTMSGID, int>::iterator pos = sys_messageId_.find(t_fb_data->message_id());
		if (pos != sys_messageId_.end() && pos->second == 1)
		{
			message_id = t_fb_data->message_id();
		}
		else
		{
			LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---t_fb_data->message_id() error.");
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
				return 0;
		}
	}
	else
	{
		message_id = CSystem::GetSystemTime();
		message_id = message_id << 1;
		message_id = message_id | 1;
		if (sys_messageId_.size() > 1000)
		{
			std::map<TEXTMSGID, int>::iterator pos = sys_messageId_.begin();
			sys_messageId_.erase(pos);
		}
		sys_messageId_[message_id] = 1;
	}
	flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_sys_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
		fbbuilder_sys,
		message_id,
		DEF_SYS_OFFCIAL_ID,
		s_server_sys_msg);
	fbbuilder_sys.Finish(msg_sys_info);
	BYTE*	lpData = fbbuilder_sys.GetBufferPointer();

	std::string strElement;
	strElement.assign((char*)lpData, fbbuilder_sys.GetSize());
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "--- t_fb_data->to_fans_lst()->size()=" << t_fb_data->to_fans_lst()->size());
	if (NULL != t_fb_data->to_fans_lst() && t_fb_data->to_fans_lst()->size() > 0)
	{
		std::map<std::string, std::string> userlst;
		for (int i = 0;i<t_fb_data->to_fans_lst()->size() && i < 50;i++)
		{
			auto userid = t_fb_data->to_fans_lst()->Get(i);
			userlst[std::to_string(userid)] = "1";
			vector_fans_lst.push_back(userid);
		}
		mpServerMgr->mpRedisUtil->HSetFansLst(std::to_string(message_id), userlst);
		mpServerMgr->mpRedisUtil->ZaddOffcialSysChatInfo(message_id, strElement);


		int offcial_id=DEF_SYS_OFFCIAL_ID;
		RJ_CREATE_O_DOCUMENT(doc, l_allocator)
			JsonParse::add<unsigned>(doc, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
		JsonParse::add<unsigned>(doc, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
		JsonParse::add<unsigned>(doc, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
		JsonParse::add<uint64_t>(doc, "msg_time", (uint64_t)t_fb_data->s_msg()->msg_time(), l_allocator);
		string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
		JsonParse::add(doc, "send_user_name", send_user_name, l_allocator);
		string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
		JsonParse::add(doc, "msg_content", msg_content, l_allocator);
		JsonParse::add<uint64_t>(doc, "offcial_id", (uint64_t)offcial_id, l_allocator);
		JsonParse::add<uint64_t>(doc, "message_id", (uint64_t)message_id, l_allocator);
		JsonParse::add<uint64_t>(doc, "group_id", (uint64_t)t_fb_data->group_id(), l_allocator);

		std::string out_json = "";
		JsonParse::to_string(doc, out_json);

		LONG nRedisResult = 0;
		mpServerMgr->mpRedisUtil->LpushOffcialMsgdb(offcial_id,out_json,nRedisResult);
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " LpushOffcialMsgdb err. ");
		}

	}
	else
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---t_fb_data->to_fans_lst() error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  aiUserID=" << aiUserID);
	STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RS loSendPack;
	auto s_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder, t_fb_data->s_msg()->app_id(),
		t_fb_data->s_msg()->session_id(), t_fb_data->s_msg()->chat_type(),
		t_fb_data->s_msg()->m_type(), t_fb_data->s_msg()->s_type(),
		t_fb_data->s_msg()->ext_type(), t_fb_data->s_msg()->msg_content()->c_str(),
		t_fb_data->s_msg()->msg_time(), t_fb_data->s_msg()->send_user_name()->c_str());
	loSendPack.fbbuilder.Finish(s_msg);
	auto vector_list_builder = loSendPack.fbbuilder.CreateVector(vector_fans_lst);
	loSendPack.fbbuf = offcialpack::CreateT_CLIENT_OFFCIAL_SEND_SOME_SYS_MSG_RS(loSendPack.fbbuilder,
		&s_rs_head,
		message_id,
		vector_list_builder,
		t_fb_data->group_id(),
		s_msg);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
}

INT CClientSide::OffcialSendOneSysMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	CUser* lpUser = (CUser*)apRelationPtr;
	STRU_GENERAL_BODY_PACKAGE<STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RQ> loRecvPack;
	const STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RQ &loUserLoad = loRecvPack.moUserLoad;
	INT nResult = loRecvPack.UnPack(apData, awLen);
	if (0 >= nResult)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error user_id is invalid user_id = "<< aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_UNPACK_FAILED_RESULT));
		return 0;
	}
	auto t_fb_data = loUserLoad.t_fb_data;
	int liResult = MAKE_SUCCESS_RESULT();
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(),
		t_fb_data->s_rq_head()->pack_session_id(),
		liResult, 
		t_fb_data->s_rq_head()->platform());
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);

	if (!t_fb_data->s_msg())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->s_msg() error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(!t_fb_data->s_msg()->msg_content() || !t_fb_data->s_msg()->msg_content()->c_str())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage msg_content error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(!t_fb_data->s_msg()->send_user_name() || !t_fb_data->s_msg()->send_user_name()->c_str())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage send_user_name error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}


	if(NULL == t_fb_data->fans_id())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---t_fb_data->fans_id() error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	std::vector<uint64_t> vector_fans_lst;

	flatbuffers::FlatBufferBuilder fbbuilder_sys;
	flatbuffers::Offset<commonpack::S_MSG> s_server_sys_msg = commonpack::CreateS_MSGDirect(fbbuilder_sys,
		t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
		t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
		t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
		t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
		t_fb_data->s_msg()->send_user_name()->c_str());
	fbbuilder_sys.Finish(s_server_sys_msg);

	TEXTMSGID message_id = CSystem::GetSystemTime();
	message_id = message_id << 1;
	flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_sys_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
		fbbuilder_sys,
		message_id,
		DEF_SYS_OFFCIAL_ID,
		s_server_sys_msg);
	fbbuilder_sys.Finish(msg_sys_info);
	BYTE*	lpData = fbbuilder_sys.GetBufferPointer();

	std::string strElement;
	strElement.assign((char*)lpData, fbbuilder_sys.GetSize());
	if (t_fb_data->fans_id() > 0)
	{
		mpServerMgr->mpRedisUtil->ZaddFansSysMsgInfo(t_fb_data->fans_id(), message_id, strElement);


		int offcial_id=DEF_SYS_OFFCIAL_ID;
		RJ_CREATE_O_DOCUMENT(doc, l_allocator)
			JsonParse::add<unsigned>(doc, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
		JsonParse::add<unsigned>(doc, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
		JsonParse::add<unsigned>(doc, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
		JsonParse::add<uint64_t>(doc, "msg_time", (uint64_t)t_fb_data->s_msg()->msg_time(), l_allocator);
		string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
		JsonParse::add(doc, "send_user_name", send_user_name, l_allocator);
		string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
		JsonParse::add(doc, "msg_content", msg_content, l_allocator);
		JsonParse::add<uint64_t>(doc, "offcial_id", (uint64_t)offcial_id, l_allocator);
		JsonParse::add<uint64_t>(doc, "message_id", (uint64_t)message_id, l_allocator);
		JsonParse::add<uint64_t>(doc, "group_id", (uint64_t)0, l_allocator);

		std::string out_json = "";
		JsonParse::to_string(doc, out_json);

		LONG nRedisResult = 0;
		mpServerMgr->mpRedisUtil->LpushOffcialMsgdb(offcial_id,out_json,nRedisResult);
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " LpushOffcialMsgdb err. ");
		}

	} 
	else
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---t_fb_data->fans_id() error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  aiUserID=" << aiUserID);
	STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RS loSendPack;
	auto s_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder, t_fb_data->s_msg()->app_id(),
		t_fb_data->s_msg()->session_id(), t_fb_data->s_msg()->chat_type(),
		t_fb_data->s_msg()->m_type(), t_fb_data->s_msg()->s_type(),
		t_fb_data->s_msg()->ext_type(), t_fb_data->s_msg()->msg_content()->c_str(),
		t_fb_data->s_msg()->msg_time(), t_fb_data->s_msg()->send_user_name()->c_str());
	loSendPack.fbbuilder.Finish(s_msg);
	loSendPack.fbbuf = offcialpack::CreateT_CLIENT_OFFCIAL_SEND_ONE_SYS_MSG_RS(loSendPack.fbbuilder,
		&s_rs_head,
		t_fb_data->fans_id(),
		s_msg);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
}

INT CClientSide::FansGetSysMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_CLIENT_FANS_GET_SYS_MESSAGE_RQ, aiUserID, STRU_CLIENT_FANS_GET_SYS_MESSAGE_RS::ENUM_TYPE)
		GENERAL_PACK_SAME_BEGIN()
	
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);

	STRU_CLIENT_FANS_GET_SYS_MESSAGE_RS loSendPack;
	std::vector<std::string> vector_chat_info;
	std::vector<std::string> pmapVal;
	std::vector<flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> > vector_sys_msg;
	std::vector<flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> > vector_sys_private_msg;
	TEXTMSGID message_id1=t_fb_data->message_id();

	BYTE is_finish = OFFCIAL_MSG_FINISH;
	TEXTMSGID message_id2=0;
	do 
	{
		mpServerMgr->mpRedisUtil->ZRrangeOffcialSysMsgByScoreToMax(
			message_id1+10, DEF_PACK_GROUP_SPAN_NUMBER, 
			&pmapVal);
		if (pmapVal.size() >= DEF_PACK_GROUP_SPAN_NUMBER)
		{
			is_finish=OFFCIAL_MSG_NO_FINISH;
		}
		for(int index = 0; index < pmapVal.size(); index++)
		{
			auto poffcial=GetT_OFFCIAL_MESSAGE(pmapVal[index].c_str());
			TEXTMSGID message_id = poffcial->message_id();

			if (message_id & 1 == 1)
			{
				std::string str = "";
				str = mpServerMgr->mpRedisUtil->HCheckFansId(std::to_string(message_id),aiUserID);
				if (str != "1")
				{
					continue;
				}
			}
			flatbuffers::Offset<commonpack::S_MSG> s_sys_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder,
				poffcial->s_msg()->app_id(), poffcial->s_msg()->session_id(),
				poffcial->s_msg()->chat_type(), poffcial->s_msg()->m_type(),
				poffcial->s_msg()->s_type(), poffcial->s_msg()->ext_type(),
				poffcial->s_msg()->msg_content()->c_str(), poffcial->s_msg()->msg_time(),
				poffcial->s_msg()->send_user_name()->c_str());

			flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_sys_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
				loSendPack.fbbuilder,
				poffcial->message_id(),
				DEF_SYS_OFFCIAL_ID,
				s_sys_msg);

			vector_sys_msg.push_back(msg_sys_info);
			
		}
	} while (vector_sys_msg.size() == 0 && pmapVal.size() > 0);

	mpServerMgr->mpRedisUtil->ZRrangeOffcialOneSysMsgByScoreToMax(
		aiUserID,message_id2, DEF_PACK_GROUP_SPAN_NUMBER, 
		&vector_chat_info);
	if (pmapVal.size() >= DEF_PACK_GROUP_SPAN_NUMBER)
	{
		is_finish=OFFCIAL_MSG_NO_FINISH;
	}
	for(int index = 0; index < vector_chat_info.size(); index++)
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "--- vector_chat_info[index] size = " << vector_chat_info[index].size());
		auto poffcial=GetT_OFFCIAL_MESSAGE(vector_chat_info[index].c_str());

		flatbuffers::Offset<commonpack::S_MSG> s_sys_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder,
			poffcial->s_msg()->app_id(), poffcial->s_msg()->session_id(),
			poffcial->s_msg()->chat_type(), poffcial->s_msg()->m_type(),
			poffcial->s_msg()->s_type(), poffcial->s_msg()->ext_type(),
			poffcial->s_msg()->msg_content()->c_str(), poffcial->s_msg()->msg_time(),
			poffcial->s_msg()->send_user_name()->c_str());

		flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_sys_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
			loSendPack.fbbuilder,
			poffcial->message_id(),
			DEF_SYS_OFFCIAL_ID,
			s_sys_msg);
		message_id2 = poffcial->message_id();
		vector_sys_private_msg.push_back(msg_sys_info);
	}
	mpServerMgr->mpRedisUtil->ZremFansOneSysMsgbyscore(aiUserID, 0,message_id2);
	std::string table_name = std::string(SC_OFFCIAL_SYS_INFO);
	mpServerMgr->mpRedisUtil->ZremrangebyrankSysMsg(table_name,0, -1001);

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  aiUserID=" << aiUserID);

	auto vector_msg_builder = loSendPack.fbbuilder.CreateVector(vector_sys_msg);
	auto vector_private_msg_builder = loSendPack.fbbuilder.CreateVector(vector_sys_private_msg);
	loSendPack.fbbuf = offcialpack::CreateT_CLIENT_FANS_GET_SYS_MSG_RS(loSendPack.fbbuilder,
		&s_rs_head,
		is_finish,
		vector_msg_builder,
		vector_private_msg_builder);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
}

INT CClientSide::OffcialSendMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	CUser* lpUser = (CUser*)apRelationPtr;
	STRU_GENERAL_BODY_PACKAGE<STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RQ> loRecvPack;
	const STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RQ &loUserLoad = loRecvPack.moUserLoad;
	INT nResult = loRecvPack.UnPack(apData, awLen);
	if (0 >= nResult)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error user_id is invalid user_id = "<< aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_UNPACK_FAILED_RESULT));
		return 0;
	}
	auto t_fb_data = loUserLoad.t_fb_data;
	int liResult = MAKE_SUCCESS_RESULT();
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(),
		t_fb_data->s_rq_head()->pack_session_id(),
		liResult, 
		t_fb_data->s_rq_head()->platform());
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);

	if (!t_fb_data->s_msg())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," --- send_user_name error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALNAME_BASE))
			return 0;
	}

	if(!t_fb_data->s_msg()->msg_content() || !t_fb_data->s_msg()->msg_content()->c_str())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content is nil");
		return -1;
	}

	if(!t_fb_data->s_msg()->send_user_name() || !t_fb_data->s_msg()->send_user_name()->c_str())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage send_user_name error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
		return 0;
	}

	if(!t_fb_data->offcial_id())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->offcial_id error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(NULL == t_fb_data->to_user_lst())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->to_user_lst error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS loSendPack;
	auto s_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder, t_fb_data->s_msg()->app_id(),
		t_fb_data->s_msg()->session_id(), t_fb_data->s_msg()->chat_type(),
		t_fb_data->s_msg()->m_type(), t_fb_data->s_msg()->s_type(),
		t_fb_data->s_msg()->ext_type(), t_fb_data->s_msg()->msg_content()->c_str(),
		t_fb_data->s_msg()->msg_time(), t_fb_data->s_msg()->send_user_name()->c_str());
	loSendPack.fbbuilder.Finish(s_msg);

	TEXTMSGID message_id=0;
	if (t_fb_data->message_id() == 0)
	{
		message_id = CSystem::GetSystemTime();
		std::map<uint64, TEXTMSGID>::iterator pos = messageId_.find(t_fb_data->offcial_id());
		if (pos != messageId_.end())
		{
			TEXTMSGID next_message = pos->second;
			next_message = next_message >> 1;
			if (message_id - next_message < mpServerMgr->moConfig.m_lastmsgtime*1000)
			{
				LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage time error.");
				TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_BASE))
				return 0;
			}
		}
		message_id = message_id << 1;
		messageId_[t_fb_data->offcial_id()] = message_id;

		flatbuffers::FlatBufferBuilder fbbuilder_offline;
		flatbuffers::Offset<commonpack::S_MSG> s_server_offline_msg = commonpack::CreateS_MSGDirect(fbbuilder_offline,
			t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
			t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
			t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
			t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
			t_fb_data->s_msg()->send_user_name()->c_str());
		fbbuilder_offline.Finish(s_server_offline_msg);
		
		flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_offline_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
			fbbuilder_offline,
			message_id,
			t_fb_data->offcial_id(),
			s_server_offline_msg,
			t_fb_data->b_id(),
			t_fb_data->w_id(),
			t_fb_data->c_id());
		fbbuilder_offline.Finish(msg_offline_info);
		BYTE*	lpData = fbbuilder_offline.GetBufferPointer();

		std::string strElement;
		strElement.assign((char*)lpData, fbbuilder_offline.GetSize());
		mpServerMgr->mpRedisUtil->ZaddOffcialOffLineChatInfo(t_fb_data->offcial_id(), message_id, strElement);

		RJ_CREATE_O_DOCUMENT(doc, l_allocator)
			JsonParse::add<unsigned>(doc, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
		JsonParse::add<unsigned>(doc, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
		JsonParse::add<unsigned>(doc, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
		JsonParse::add<uint64_t>(doc, "msg_time", (uint64_t)t_fb_data->s_msg()->msg_time(), l_allocator);
		string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
		JsonParse::add(doc, "send_user_name", send_user_name, l_allocator);
		string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
		JsonParse::add(doc, "msg_content", msg_content, l_allocator);
		JsonParse::add<uint64_t>(doc, "offcial_id", (uint64_t)t_fb_data->offcial_id(), l_allocator);
		JsonParse::add<uint64_t>(doc, "message_id", (uint64_t)message_id, l_allocator);
		JsonParse::add<uint64_t>(doc, "group_id", (uint64_t)0, l_allocator);
		JsonParse::add<uint64_t>(doc, "b_id", (uint64_t)t_fb_data->b_id(), l_allocator);
		JsonParse::add<uint64_t>(doc, "w_id", (uint64_t)t_fb_data->w_id(), l_allocator);
		JsonParse::add<uint64_t>(doc, "c_id", (uint64_t)t_fb_data->c_id(), l_allocator);

		std::string out_json = "";
		JsonParse::to_string(doc, out_json);

		LONG nRedisResult = 0;
		mpServerMgr->mpRedisUtil->LpushOffcialMsgdb(t_fb_data->offcial_id(),out_json,nRedisResult);
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
				<< " redis fail");
		}
		if (t_fb_data->b_id() > 0)
		{
			RJ_CREATE_O_DOCUMENT(l_document, l_allocator)
				JsonParse::add(l_document, "userid", std::to_string(t_fb_data->offcial_id()), l_allocator);
			JsonParse::add<unsigned>(l_document, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
			JsonParse::add<unsigned>(l_document, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
			JsonParse::add<unsigned>(l_document, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
			JsonParse::add<unsigned>(l_document, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
			JsonParse::add<unsigned>(l_document, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
			JsonParse::add<unsigned>(l_document, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
			string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
			JsonParse::add(l_document, "msg_content", msg_content, l_allocator);
			JsonParse::add(l_document, "msg_time", to_string(t_fb_data->s_msg()->msg_time()), l_allocator);
			string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
			JsonParse::add(l_document, "send_user_name", send_user_name, l_allocator);
			JsonParse::add(l_document, "message_id", to_string(message_id), l_allocator);
			JsonParse::add(l_document, "op_user_id", to_string(0), l_allocator);
			JsonParse::add(l_document, "b_id", to_string(t_fb_data->b_id()), l_allocator);
			JsonParse::add(l_document, "c_id", to_string(t_fb_data->c_id()), l_allocator);
			JsonParse::add(l_document, "w_id", to_string(t_fb_data->w_id()), l_allocator);

			std::string out_json2 = "";
			JsonParse::to_string(l_document, out_json2);
			LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---JsonParse  out_json2=" << out_json2);

			nRedisResult=0;
			mpServerMgr->mpRedisUtil->RPushEcChatInfoToDbBack(t_fb_data->offcial_id() % EC_CHAT_TO_BACK_COUNT,out_json2,nRedisResult);
			if(nRedisResult <= 0)
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, " 2 t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
					<< " redis fail");
			}
		}
		
	}
	else
	{
		std::map<uint64, TEXTMSGID>::iterator pos = messageId_.find(t_fb_data->offcial_id());
		if (pos != messageId_.end())
		{
			message_id = pos->second;
		}
		else
		{
			message_id = CSystem::GetSystemTime();
			message_id = message_id << 1;
			messageId_[t_fb_data->offcial_id()] = message_id;

			flatbuffers::FlatBufferBuilder fbbuilder_offline;
			flatbuffers::Offset<commonpack::S_MSG> s_server_offline_msg = commonpack::CreateS_MSGDirect(fbbuilder_offline,
				t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
				t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
				t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
				t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
				t_fb_data->s_msg()->send_user_name()->c_str());
			fbbuilder_offline.Finish(s_server_offline_msg);

			flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_offline_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
				fbbuilder_offline,
				message_id,
				t_fb_data->offcial_id(),
				s_server_offline_msg,
				t_fb_data->b_id(),
				t_fb_data->w_id(),
				t_fb_data->c_id());
			fbbuilder_offline.Finish(msg_offline_info);
			BYTE*	lpData = fbbuilder_offline.GetBufferPointer();

			std::string strElement;
			strElement.assign((char*)lpData, fbbuilder_offline.GetSize());
			mpServerMgr->mpRedisUtil->ZaddOffcialOffLineChatInfo(t_fb_data->offcial_id(), message_id, strElement);

			RJ_CREATE_O_DOCUMENT(doc, l_allocator)
				JsonParse::add<unsigned>(doc, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
			JsonParse::add<unsigned>(doc, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
			JsonParse::add<unsigned>(doc, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
			JsonParse::add<unsigned>(doc, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
			JsonParse::add<unsigned>(doc, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
			JsonParse::add<unsigned>(doc, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
			JsonParse::add<uint64_t>(doc, "msg_time", (uint64_t)t_fb_data->s_msg()->msg_time(), l_allocator);
			string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
			JsonParse::add(doc, "send_user_name", send_user_name, l_allocator);
			string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
			JsonParse::add(doc, "msg_content", msg_content, l_allocator);
			JsonParse::add<uint64_t>(doc, "offcial_id", (uint64_t)t_fb_data->offcial_id(), l_allocator);
			JsonParse::add<uint64_t>(doc, "message_id", (uint64_t)message_id, l_allocator);
			JsonParse::add<uint64_t>(doc, "group_id", (uint64_t)0, l_allocator);
			JsonParse::add<uint64_t>(doc, "b_id", (uint64_t)t_fb_data->b_id(), l_allocator);
			JsonParse::add<uint64_t>(doc, "w_id", (uint64_t)t_fb_data->w_id(), l_allocator);
			JsonParse::add<uint64_t>(doc, "c_id", (uint64_t)t_fb_data->c_id(), l_allocator);

			std::string out_json = "";
			JsonParse::to_string(doc, out_json);

			LONG nRedisResult = 0;
			mpServerMgr->mpRedisUtil->LpushOffcialMsgdb(t_fb_data->offcial_id(),out_json,nRedisResult);
			if(nRedisResult <= 0)
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, " t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
					<< " redis fail");
			}
			if (t_fb_data->b_id() > 0)
			{
				RJ_CREATE_O_DOCUMENT(l_document, l_allocator)
					JsonParse::add(l_document, "userid", std::to_string(t_fb_data->offcial_id()), l_allocator);
				JsonParse::add<unsigned>(l_document, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
				JsonParse::add<unsigned>(l_document, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
				JsonParse::add<unsigned>(l_document, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
				JsonParse::add<unsigned>(l_document, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
				JsonParse::add<unsigned>(l_document, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
				JsonParse::add<unsigned>(l_document, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
				string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
				JsonParse::add(l_document, "msg_content", msg_content, l_allocator);
				JsonParse::add(l_document, "msg_time", to_string(t_fb_data->s_msg()->msg_time()), l_allocator);
				string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
				JsonParse::add(l_document, "send_user_name", send_user_name, l_allocator);
				JsonParse::add(l_document, "message_id", to_string(message_id), l_allocator);
				JsonParse::add(l_document, "op_user_id", to_string(0), l_allocator);
				JsonParse::add(l_document, "b_id", to_string(t_fb_data->b_id()), l_allocator);
				JsonParse::add(l_document, "c_id", to_string(t_fb_data->c_id()), l_allocator);
				JsonParse::add(l_document, "w_id", to_string(t_fb_data->w_id()), l_allocator);

				std::string out_json2 = "";
				JsonParse::to_string(l_document, out_json2);
				LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---JsonParse  out_json2=" << out_json2);

				nRedisResult=0;
				mpServerMgr->mpRedisUtil->RPushEcChatInfoToDbBack(t_fb_data->offcial_id() % EC_CHAT_TO_BACK_COUNT,out_json2,nRedisResult);
				if(nRedisResult <= 0)
				{
					LOG_TRACE(LOG_ERR, false, __FUNCTION__, " 2 t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
						<< " redis fail");
				}
			}
		}
	}
	std::vector<uint64_t> vectorGroupInfo;
	for(int i=0;i<t_fb_data->to_user_lst()->size() && i < DEF_OFFCIAL_TO_KAFKA_NUMBER;i++)
	{
		auto userid = t_fb_data->to_user_lst()->Get(i);
		vectorGroupInfo.push_back(userid);
	}
	auto vector_list_builder = loSendPack.fbbuilder.CreateVector(vectorGroupInfo);
	loSendPack.fbbuf = offcialpack::CreateT_CLIENT_OFFCIAL_SEND_MESSAGE_RS(loSendPack.fbbuilder,
		&s_rs_head, message_id,
		t_fb_data->offcial_id(), 
		vector_list_builder,
		s_msg,
		t_fb_data->b_id(),
		t_fb_data->w_id(),
		t_fb_data->c_id());

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---RS,  message_id=" << message_id);
	

	STRU_SERVER_OFFCIALR_MESSAGE_RQ loServerSendPack;
	commonpack::S_RQ_HEAD s_rq_head(0, t_fb_data->s_rq_head()->pack_session_id(),
		t_fb_data->s_rq_head()->platform());

	flatbuffers::Offset<commonpack::S_MSG> s_server_msg = commonpack::CreateS_MSGDirect(loServerSendPack.fbbuilder, 
		t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
		t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
		t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
		t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
		t_fb_data->s_msg()->send_user_name()->c_str());

	loServerSendPack.fbbuilder.Finish(s_server_msg);
	loServerSendPack.fbbuf = offcialpack::CreateT_SERVER_OFFCIAL_MESSAGE_RQ(loServerSendPack.fbbuilder,
		&s_rq_head, message_id,
		t_fb_data->offcial_id(), 
		s_server_msg,
		t_fb_data->b_id(),
		t_fb_data->w_id(),
		t_fb_data->c_id());

	int rand_partition_key = rand();
	COffcialDistributeMgr::GetInstance().DistributeToKafka(-1, vectorGroupInfo, 
		loServerSendPack, false, rand_partition_key);
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---RS,  rand key=" << rand_partition_key);

	return 1;
}

INT CClientSide::OffcialSendSomeMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	CUser* lpUser = (CUser*)apRelationPtr;
	STRU_GENERAL_BODY_PACKAGE<STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RQ> loRecvPack;
	const STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RQ &loUserLoad = loRecvPack.moUserLoad;
	INT nResult = loRecvPack.UnPack(apData, awLen);
	if (0 >= nResult)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error user_id is invalid user_id = "<< aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_UNPACK_FAILED_RESULT));
		return 0;
	}
	auto t_fb_data = loUserLoad.t_fb_data;
	int liResult = MAKE_SUCCESS_RESULT();
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(),
		t_fb_data->s_rq_head()->pack_session_id(),
		liResult, 
		t_fb_data->s_rq_head()->platform());
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);

	if (!t_fb_data->s_msg())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage send_user_name error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALNAME_BASE))
			return 0;
	}

	if(!t_fb_data->s_msg()->msg_content() || !t_fb_data->s_msg()->msg_content()->c_str())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content is nil");
		return -1;
	}

	if(!t_fb_data->s_msg()->send_user_name() || !t_fb_data->s_msg()->send_user_name()->c_str())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage send_user_name error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}
	if(!t_fb_data->group_id())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->group_id error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(!t_fb_data->offcial_id())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->offcial_id error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(NULL == t_fb_data->to_fans_lst())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->to_fans_lst error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	TEXTMSGID message_id=0;
	if (t_fb_data->message_id() == 0)
	{
		message_id = CSystem::GetSystemTime();

		std::map<uint64, TEXTMSGID>::iterator pos = messageId_.find(t_fb_data->offcial_id());
		if (pos != messageId_.end())
		{
			TEXTMSGID next_message = pos->second;
			next_message = next_message >> 1;
			if (message_id - next_message < mpServerMgr->moConfig.m_lastmsgtime*1000)
			{
				LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage time error.");
				TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_BASE))
					return 0;
			}
		}
		message_id = message_id << 1;
		message_id = message_id | 1;
		messageId_[t_fb_data->offcial_id()] = message_id;
		std::map<std::string, std::string> userlst;
		for (int i = 0;i<t_fb_data->to_fans_lst()->size() && i < 50;i++)
		{
			auto userid = t_fb_data->to_fans_lst()->Get(i);
			userlst[std::to_string(userid)] = "1";
			//vector_fans_lst.push_back(userid);
		}
		mpServerMgr->mpRedisUtil->HSetFansLst(aiUserID,std::to_string(message_id), userlst);
		
		flatbuffers::FlatBufferBuilder fbbuilder_offline;
		flatbuffers::Offset<commonpack::S_MSG> s_server_offline_msg = commonpack::CreateS_MSGDirect(fbbuilder_offline,
			t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
			t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
			t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
			t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
			t_fb_data->s_msg()->send_user_name()->c_str());
		fbbuilder_offline.Finish(s_server_offline_msg);

		flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_offline_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
			fbbuilder_offline,
			message_id,
			t_fb_data->offcial_id(),
			s_server_offline_msg,
			t_fb_data->b_id(),
			t_fb_data->w_id(),
			t_fb_data->c_id());
		fbbuilder_offline.Finish(msg_offline_info);
		BYTE*	lpData = fbbuilder_offline.GetBufferPointer();

		std::string strElement;
		strElement.assign((char*)lpData, fbbuilder_offline.GetSize());
		mpServerMgr->mpRedisUtil->ZaddOffcialOffLineChatInfo(t_fb_data->offcial_id(), message_id, strElement);

		RJ_CREATE_O_DOCUMENT(doc, l_allocator)
			JsonParse::add<unsigned>(doc, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
		JsonParse::add<unsigned>(doc, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
		JsonParse::add<unsigned>(doc, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
		JsonParse::add<unsigned>(doc, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
		JsonParse::add<uint64_t>(doc, "msg_time", (uint64_t)t_fb_data->s_msg()->msg_time(), l_allocator);
		string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
		JsonParse::add(doc, "send_user_name", send_user_name, l_allocator);
		string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
		JsonParse::add(doc, "msg_content", msg_content, l_allocator);
		JsonParse::add<uint64_t>(doc, "offcial_id", (uint64_t)t_fb_data->offcial_id(), l_allocator);
		JsonParse::add<uint64_t>(doc, "message_id", (uint64_t)message_id, l_allocator);
		JsonParse::add<uint64_t>(doc, "group_id", (uint64_t)t_fb_data->group_id(), l_allocator);
		JsonParse::add<uint64_t>(doc, "b_id", (uint64_t)t_fb_data->b_id(), l_allocator);
		JsonParse::add<uint64_t>(doc, "w_id", (uint64_t)t_fb_data->w_id(), l_allocator);
		JsonParse::add<uint64_t>(doc, "c_id", (uint64_t)t_fb_data->c_id(), l_allocator);

		std::string out_json = "";
		JsonParse::to_string(doc, out_json);

		LONG nRedisResult = 0;
		mpServerMgr->mpRedisUtil->LpushOffcialMsgdb(t_fb_data->offcial_id(),out_json,nRedisResult);
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
				<< " redis fail");
		}
		if (t_fb_data->b_id() > 0)
		{
			RJ_CREATE_O_DOCUMENT(l_document, l_allocator)
				JsonParse::add(l_document, "userid", std::to_string(t_fb_data->offcial_id()), l_allocator);
			JsonParse::add<unsigned>(l_document, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
			JsonParse::add<unsigned>(l_document, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
			JsonParse::add<unsigned>(l_document, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
			JsonParse::add<unsigned>(l_document, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
			JsonParse::add<unsigned>(l_document, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
			JsonParse::add<unsigned>(l_document, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
			string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
			JsonParse::add(l_document, "msg_content", msg_content, l_allocator);
			JsonParse::add(l_document, "msg_time", to_string(t_fb_data->s_msg()->msg_time()), l_allocator);
			string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
			JsonParse::add(l_document, "send_user_name", send_user_name, l_allocator);
			JsonParse::add(l_document, "message_id", to_string(message_id), l_allocator);
			JsonParse::add(l_document, "op_user_id", to_string(0), l_allocator);
			JsonParse::add(l_document, "b_id", to_string(t_fb_data->b_id()), l_allocator);
			JsonParse::add(l_document, "c_id", to_string(t_fb_data->c_id()), l_allocator);
			JsonParse::add(l_document, "w_id", to_string(t_fb_data->w_id()), l_allocator);

			std::string out_json2 = "";
			JsonParse::to_string(l_document, out_json2);
			LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---JsonParse  out_json2=" << out_json2);

			nRedisResult=0;
			mpServerMgr->mpRedisUtil->RPushEcChatInfoToDbBack(t_fb_data->offcial_id() % EC_CHAT_TO_BACK_COUNT,out_json2,nRedisResult);
			if(nRedisResult <= 0)
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, " 2 t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
					<< " redis fail");
			}
		}
	}
	else
	{
		std::map<uint64, TEXTMSGID>::iterator pos = messageId_.find(t_fb_data->offcial_id());
		if (pos != messageId_.end())
		{
			message_id = pos->second;
		}
		else
		{
			message_id = message_id << 1;
			message_id = message_id | 1;
			messageId_[t_fb_data->offcial_id()] = message_id;
			std::map<std::string, std::string> userlst;
			for (int i = 0;i<t_fb_data->to_fans_lst()->size() && i < 50;i++)
			{
				auto userid = t_fb_data->to_fans_lst()->Get(i);
				userlst[std::to_string(userid)] = "1";
				//vector_fans_lst.push_back(userid);
			}
			mpServerMgr->mpRedisUtil->HSetFansLst(aiUserID,std::to_string(message_id), userlst);

			flatbuffers::FlatBufferBuilder fbbuilder_offline;
			flatbuffers::Offset<commonpack::S_MSG> s_server_offline_msg = commonpack::CreateS_MSGDirect(fbbuilder_offline,
				t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
				t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
				t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
				t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
				t_fb_data->s_msg()->send_user_name()->c_str());
			fbbuilder_offline.Finish(s_server_offline_msg);

			flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_offline_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
				fbbuilder_offline,
				message_id,
				t_fb_data->offcial_id(),
				s_server_offline_msg,
				t_fb_data->b_id(),
				t_fb_data->w_id(),
				t_fb_data->c_id());
			fbbuilder_offline.Finish(msg_offline_info);
			BYTE*	lpData = fbbuilder_offline.GetBufferPointer();

			std::string strElement;
			strElement.assign((char*)lpData, fbbuilder_offline.GetSize());
			mpServerMgr->mpRedisUtil->ZaddOffcialOffLineChatInfo(t_fb_data->offcial_id(), message_id, strElement);

			RJ_CREATE_O_DOCUMENT(doc, l_allocator)
				JsonParse::add<unsigned>(doc, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
			JsonParse::add<unsigned>(doc, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
			JsonParse::add<unsigned>(doc, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
			JsonParse::add<unsigned>(doc, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
			JsonParse::add<unsigned>(doc, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
			JsonParse::add<unsigned>(doc, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
			JsonParse::add<uint64_t>(doc, "msg_time", (uint64_t)t_fb_data->s_msg()->msg_time(), l_allocator);
			string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
			JsonParse::add(doc, "send_user_name", send_user_name, l_allocator);
			string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
			JsonParse::add(doc, "msg_content", msg_content, l_allocator);
			JsonParse::add<uint64_t>(doc, "offcial_id", (uint64_t)t_fb_data->offcial_id(), l_allocator);
			JsonParse::add<uint64_t>(doc, "message_id", (uint64_t)message_id, l_allocator);
			JsonParse::add<uint64_t>(doc, "group_id", (uint64_t)t_fb_data->group_id(), l_allocator);
			JsonParse::add<uint64_t>(doc, "b_id", (uint64_t)t_fb_data->b_id(), l_allocator);
			JsonParse::add<uint64_t>(doc, "w_id", (uint64_t)t_fb_data->w_id(), l_allocator);
			JsonParse::add<uint64_t>(doc, "c_id", (uint64_t)t_fb_data->c_id(), l_allocator);

			std::string out_json = "";
			JsonParse::to_string(doc, out_json);

			LONG nRedisResult = 0;
			mpServerMgr->mpRedisUtil->LpushOffcialMsgdb(t_fb_data->offcial_id(),out_json,nRedisResult);
			if(nRedisResult <= 0)
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, " t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
					<< " redis fail");
			}
			if (t_fb_data->b_id() > 0)
			{
				RJ_CREATE_O_DOCUMENT(l_document, l_allocator)
					JsonParse::add(l_document, "userid", std::to_string(t_fb_data->offcial_id()), l_allocator);
				JsonParse::add<unsigned>(l_document, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
				JsonParse::add<unsigned>(l_document, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
				JsonParse::add<unsigned>(l_document, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
				JsonParse::add<unsigned>(l_document, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
				JsonParse::add<unsigned>(l_document, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
				JsonParse::add<unsigned>(l_document, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
				string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
				JsonParse::add(l_document, "msg_content", msg_content, l_allocator);
				JsonParse::add(l_document, "msg_time", to_string(t_fb_data->s_msg()->msg_time()), l_allocator);
				string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
				JsonParse::add(l_document, "send_user_name", send_user_name, l_allocator);
				JsonParse::add(l_document, "message_id", to_string(message_id), l_allocator);
				JsonParse::add(l_document, "op_user_id", to_string(0), l_allocator);
				JsonParse::add(l_document, "b_id", to_string(t_fb_data->b_id()), l_allocator);
				JsonParse::add(l_document, "c_id", to_string(t_fb_data->c_id()), l_allocator);
				JsonParse::add(l_document, "w_id", to_string(t_fb_data->w_id()), l_allocator);

				std::string out_json2 = "";
				JsonParse::to_string(l_document, out_json2);
				LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---JsonParse  out_json2=" << out_json2);

				nRedisResult=0;
				mpServerMgr->mpRedisUtil->RPushEcChatInfoToDbBack(t_fb_data->offcial_id() % EC_CHAT_TO_BACK_COUNT,out_json2,nRedisResult);
				if(nRedisResult <= 0)
				{
					LOG_TRACE(LOG_ERR, false, __FUNCTION__, " 2 t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
						<< " redis fail");
				}
			}
		}

	}

	STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RS loSendPack;
	auto s_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder, t_fb_data->s_msg()->app_id(),
		t_fb_data->s_msg()->session_id(), t_fb_data->s_msg()->chat_type(),
		t_fb_data->s_msg()->m_type(), t_fb_data->s_msg()->s_type(),
		t_fb_data->s_msg()->ext_type(), t_fb_data->s_msg()->msg_content()->c_str(),
		t_fb_data->s_msg()->msg_time(), t_fb_data->s_msg()->send_user_name()->c_str());
	loSendPack.fbbuilder.Finish(s_msg);

	std::vector<uint64_t> vectorGroupInfo;
	for(int i=0;i<t_fb_data->to_fans_lst()->size() && i < 200;i++)
	{
		auto userid = t_fb_data->to_fans_lst()->Get(i);
		vectorGroupInfo.push_back(userid);
	}
	auto vector_list_builder = loSendPack.fbbuilder.CreateVector(vectorGroupInfo);
	loSendPack.fbbuf = offcialpack::CreateT_CLIENT_OFFCIAL_SEND_SOME_MSG_RS(loSendPack.fbbuilder,
		&s_rs_head, t_fb_data->message_id(),
		t_fb_data->offcial_id(), 
		vector_list_builder,
		t_fb_data->group_id(),
		s_msg,
		t_fb_data->b_id(),
		t_fb_data->w_id(),
		t_fb_data->c_id());

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  t_fb_data->offcial_id()=" << t_fb_data->offcial_id());

	STRU_SERVER_OFFCIALR_MESSAGE_RQ loServerSendPack;
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---OffcialMessageRQ,  message_id=" << message_id);
	commonpack::S_RQ_HEAD s_rq_head(0, t_fb_data->s_rq_head()->pack_session_id(),
		t_fb_data->s_rq_head()->platform());

	flatbuffers::Offset<commonpack::S_MSG> s_server_msg = commonpack::CreateS_MSGDirect(loServerSendPack.fbbuilder, 
		t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
		t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
		t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
		t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
		t_fb_data->s_msg()->send_user_name()->c_str());

	loServerSendPack.fbbuilder.Finish(s_server_msg);
	loServerSendPack.fbbuf = offcialpack::CreateT_SERVER_OFFCIAL_MESSAGE_RQ(loServerSendPack.fbbuilder,
		&s_rq_head, message_id,
		t_fb_data->offcial_id(),
		s_server_msg,
		t_fb_data->b_id(),
		t_fb_data->w_id(),
		t_fb_data->c_id());
	int rand_patition_key = rand();
	COffcialDistributeMgr::GetInstance().DistributeToKafka(-1, vectorGroupInfo, 
		loServerSendPack, false, rand_patition_key);
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "rand key = "<< rand_patition_key);
	return 1;
}

INT CClientSide::OffcialSendOneMsgRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	CUser* lpUser = (CUser*)apRelationPtr;
	STRU_GENERAL_BODY_PACKAGE<STRU_CLIENT_OFFCIALR_SEND_ONE_MSG_RQ> loRecvPack;
	const STRU_CLIENT_OFFCIALR_SEND_ONE_MSG_RQ &loUserLoad = loRecvPack.moUserLoad;
	INT nResult = loRecvPack.UnPack(apData, awLen);
	if (0 >= nResult)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error user_id is invalid user_id = "<< aiUserID << " nResult= " << nResult );

		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_ONE_MSG_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_UNPACK_FAILED_RESULT));
		return 0;
	}
	auto t_fb_data = loUserLoad.t_fb_data;
	int liResult = MAKE_SUCCESS_RESULT();
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(),
		t_fb_data->s_rq_head()->pack_session_id(),
		liResult, 
		t_fb_data->s_rq_head()->platform());
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);

	if (!t_fb_data->s_msg())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," --- send_user_name error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_ONE_MSG_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALNAME_BASE))
			return 0;
	}

	if(!t_fb_data->s_msg()->msg_content() || !t_fb_data->s_msg()->msg_content()->c_str())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " msg_content is nil");
		return -1;
	}

	if(!t_fb_data->s_msg()->send_user_name() || !t_fb_data->s_msg()->send_user_name()->c_str())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," --- send_user_name error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_ONE_MSG_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(!t_fb_data->offcial_id())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->offcial_id error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	if(!t_fb_data->fans_id())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->fans_id error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}


	TEXTMSGID message_id = CSystem::GetSystemTime();
	message_id = message_id << 1;
	STRU_CLIENT_OFFCIALR_SEND_ONE_MSG_RS loSendPack;
	auto s_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder, t_fb_data->s_msg()->app_id(),
		t_fb_data->s_msg()->session_id(), t_fb_data->s_msg()->chat_type(),
		t_fb_data->s_msg()->m_type(), t_fb_data->s_msg()->s_type(),
		t_fb_data->s_msg()->ext_type(), t_fb_data->s_msg()->msg_content()->c_str(),
		t_fb_data->s_msg()->msg_time(), t_fb_data->s_msg()->send_user_name()->c_str());
	loSendPack.fbbuilder.Finish(s_msg);

	flatbuffers::FlatBufferBuilder fbbuilder_offline;
	flatbuffers::Offset<commonpack::S_MSG> s_server_offline_msg = commonpack::CreateS_MSGDirect(fbbuilder_offline,
		t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
		t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
		t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
		t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
		t_fb_data->s_msg()->send_user_name()->c_str());
	fbbuilder_offline.Finish(s_server_offline_msg);

	flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_offline_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
		fbbuilder_offline,
		message_id,
		t_fb_data->offcial_id(),
		s_server_offline_msg,
		t_fb_data->b_id(),
		t_fb_data->w_id(),
		t_fb_data->c_id());
	fbbuilder_offline.Finish(msg_offline_info);
	BYTE*	lpData = fbbuilder_offline.GetBufferPointer();

	std::string strElement;
	strElement.assign((char*)lpData, fbbuilder_offline.GetSize());
	mpServerMgr->mpRedisUtil->ZaddOffcialOffLinePrivateChatInfo(t_fb_data->fans_id(), message_id, strElement);


	RJ_CREATE_O_DOCUMENT(doc, l_allocator)
		JsonParse::add<unsigned>(doc, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
	JsonParse::add<unsigned>(doc, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
	JsonParse::add<unsigned>(doc, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
	JsonParse::add<unsigned>(doc, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
	JsonParse::add<unsigned>(doc, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
	JsonParse::add<unsigned>(doc, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
	JsonParse::add<uint64_t>(doc, "msg_time", (uint64_t)t_fb_data->s_msg()->msg_time(), l_allocator);
	string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
	JsonParse::add(doc, "send_user_name", send_user_name, l_allocator);
	string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
	JsonParse::add(doc, "msg_content", msg_content, l_allocator);
	JsonParse::add<uint64_t>(doc, "offcial_id", (uint64_t)t_fb_data->offcial_id(), l_allocator);
	JsonParse::add<uint64_t>(doc, "message_id", (uint64_t)message_id, l_allocator);
	JsonParse::add<uint64_t>(doc, "group_id", (uint64_t)0, l_allocator);
	JsonParse::add<uint64_t>(doc, "b_id", (uint64_t)t_fb_data->b_id(), l_allocator);
	JsonParse::add<uint64_t>(doc, "w_id", (uint64_t)t_fb_data->w_id(), l_allocator);
	JsonParse::add<uint64_t>(doc, "c_id", (uint64_t)t_fb_data->c_id(), l_allocator);

	std::string out_json = "";
	JsonParse::to_string(doc, out_json);

	LONG nRedisResult = 0;
	mpServerMgr->mpRedisUtil->LpushOffcialMsgdb(t_fb_data->offcial_id(),out_json,nRedisResult);
	if(nRedisResult <= 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
			<< " redis fail");
	}
	if (t_fb_data->b_id() > 0)
	{
		RJ_CREATE_O_DOCUMENT(l_document, l_allocator)
			JsonParse::add(l_document, "userid", std::to_string(t_fb_data->offcial_id()), l_allocator);
		JsonParse::add<unsigned>(l_document, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
		JsonParse::add<unsigned>(l_document, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
		JsonParse::add<unsigned>(l_document, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
		JsonParse::add<unsigned>(l_document, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
		JsonParse::add<unsigned>(l_document, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
		JsonParse::add<unsigned>(l_document, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
		string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
		JsonParse::add(l_document, "msg_content", msg_content, l_allocator);
		JsonParse::add(l_document, "msg_time", to_string(t_fb_data->s_msg()->msg_time()), l_allocator);
		string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
		JsonParse::add(l_document, "send_user_name", send_user_name, l_allocator);
		JsonParse::add(l_document, "message_id", to_string(message_id), l_allocator);
		JsonParse::add(l_document, "op_user_id", to_string(t_fb_data->fans_id()), l_allocator);
		JsonParse::add(l_document, "b_id", to_string(t_fb_data->b_id()), l_allocator);
		JsonParse::add(l_document, "c_id", to_string(t_fb_data->c_id()), l_allocator);
		JsonParse::add(l_document, "w_id", to_string(t_fb_data->w_id()), l_allocator);

		std::string out_json2 = "";
		JsonParse::to_string(l_document, out_json2);

		LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---JsonParse  out_json2=" << out_json2);

		nRedisResult=0;
		mpServerMgr->mpRedisUtil->RPushEcChatInfoToDbBack(t_fb_data->offcial_id() % EC_CHAT_TO_BACK_COUNT,out_json2,nRedisResult);
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " 2 t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
				<< " redis fail");
		}
	}

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  t_fb_data->offcial_id()=" << t_fb_data->offcial_id());
	loSendPack.fbbuf = offcialpack::CreateT_CLIENT_OFFCIAL_SEND_ONE_MSG_RS(loSendPack.fbbuilder,
		&s_rs_head, 0,
		t_fb_data->offcial_id(), 
		t_fb_data->fans_id(),
		s_msg,
		t_fb_data->b_id(),
		t_fb_data->w_id(),
		t_fb_data->c_id());

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	STRU_SERVER_OFFCIALR_PRIVATE_MESSAGE_RQ loServerSendPack;
	//LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---OffcialMessageRQ,  message_id=" << message_id);
	commonpack::S_RQ_HEAD s_rq_head(0, t_fb_data->s_rq_head()->pack_session_id(),
		t_fb_data->s_rq_head()->platform());

	flatbuffers::Offset<commonpack::S_MSG> s_server_msg = commonpack::CreateS_MSGDirect(loServerSendPack.fbbuilder, 
		t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
		t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
		t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
		t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
		t_fb_data->s_msg()->send_user_name()->c_str());

	loServerSendPack.fbbuilder.Finish(s_server_msg);
	loServerSendPack.fbbuf = offcialpack::CreateT_SERVER_OFFCIAL_PRIVATE_MESSAGE_RQ(loServerSendPack.fbbuilder,
		&s_rq_head, message_id,
		t_fb_data->offcial_id(), 
		s_server_msg,
		t_fb_data->b_id(),
		t_fb_data->w_id(),
		t_fb_data->c_id());

	std::vector<uint64_t> vectorFansInfo;
	vectorFansInfo.push_back(t_fb_data->fans_id());
	int rand_patition_key = rand();
	COffcialDistributeMgr::GetInstance().DistributeToKafka(-1, vectorFansInfo, 
		loServerSendPack, false, rand_patition_key);
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "rand key = "<< rand_patition_key);

	return 1;
}

INT CClientSide::FansPrivateMsgRS(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	GENERAL_UNPACK_BODY(STRU_SERVER_OFFCIALR_PRIVATE_MESSAGE_RS, FALSE)
		auto t_fb_data = loUserLoad.t_fb_data;
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, " ---RS "
		<<" package_session_id="<< t_fb_data->s_rs_head()->pack_session_id()
		<<" user_id=" << t_fb_data->s_rs_head()->user_id());


	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << t_fb_data->s_rs_head()->user_id()
		<< " pack_session_id = " << t_fb_data->s_rs_head()->pack_session_id()
		<< " app_id = " << t_fb_data->s_msg()->app_id()
		<< " session_id = " << t_fb_data->s_msg()->session_id()
		<< " chat_type = " << t_fb_data->s_msg()->chat_type()
		<< " m_type = " << t_fb_data->s_msg()->m_type()
		<< " s_type = " << t_fb_data->s_msg()->s_type() 
		<< " ext_type = " << t_fb_data->s_msg()->ext_type()
		<< " msg_content = " << t_fb_data->s_msg()->msg_content()->c_str()
		<< " msg_time = " << t_fb_data->s_msg()->msg_time()
		<< " message_id = " << t_fb_data->message_id()
		<< " offcial_id = " << t_fb_data->offcial_id());

	mpServerMgr->mpRedisUtil->ZremFansOffLinePrivateChatInfobyscore(t_fb_data->s_rs_head()->user_id(), 0,t_fb_data->message_id());

}

INT CClientSide::FansSendMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_CLIENT_FANS_SEND_MESSAGE_RQ, aiUserID, STRU_CLIENT_FANS_SEND_MESSAGE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);
	STRU_CLIENT_FANS_SEND_MESSAGE_RS loSendPack;
	auto s_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder, t_fb_data->s_msg()->app_id(),
		t_fb_data->s_msg()->session_id(), t_fb_data->s_msg()->chat_type(),
		t_fb_data->s_msg()->m_type(), t_fb_data->s_msg()->s_type(),
		t_fb_data->s_msg()->ext_type(), t_fb_data->s_msg()->msg_content()->c_str(),
		t_fb_data->s_msg()->msg_time(), t_fb_data->s_msg()->send_user_name()->c_str());
		loSendPack.fbbuilder.Finish(s_msg);

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << t_fb_data->s_rq_head()->user_id()
		<< " pack_session_id = " << t_fb_data->s_rq_head()->pack_session_id()
		<< " app_id = " << t_fb_data->s_msg()->app_id()
		<< " session_id = " << t_fb_data->s_msg()->session_id()
		<< " chat_type = " << t_fb_data->s_msg()->chat_type()
		<< " m_type = " << t_fb_data->s_msg()->m_type()
		<< " s_type = " << t_fb_data->s_msg()->s_type() 
		<< " ext_type = " << t_fb_data->s_msg()->ext_type()
		<< " msg_content = " << t_fb_data->s_msg()->msg_content()->c_str()
		<< " msg_time = " << t_fb_data->s_msg()->msg_time()
		<< " message_id = " << t_fb_data->message_id()
		<< " offcial_id = " << (TEXTMSGID)t_fb_data->offcial_id()
		<< " t_fb_data->w_id() = " << (TEXTMSGID)t_fb_data->w_id());

	loSendPack.fbbuf = offcialpack::CreateT_CLIENT_FANS_SEND_MESSAGE_RS(loSendPack.fbbuilder,
		&s_rs_head, t_fb_data->message_id(),
		t_fb_data->offcial_id(),
		s_msg,
		t_fb_data->b_id(),
		t_fb_data->w_id(),
		t_fb_data->c_id());

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---RS,  aiUserID=" << aiUserID);
	//STRU_SERVER_PACKAGE_RQ<T_FANS_MESSAGE_RQ> loServerSendPack(NEW_DEF_FAN_MESSAGE_RQ);
	STRU_SERVER_FANS_MESSAGE_RQ loServerSendPack;
	commonpack::S_RQ_HEAD s_rq_head(0, t_fb_data->s_rq_head()->pack_session_id(),
		t_fb_data->s_rq_head()->platform());
	auto ls_msg = commonpack::CreateS_MSGDirect(loServerSendPack.fbbuilder, t_fb_data->s_msg()->app_id(),
		t_fb_data->s_msg()->session_id(), t_fb_data->s_msg()->chat_type(),
		t_fb_data->s_msg()->m_type(), t_fb_data->s_msg()->s_type(),
		t_fb_data->s_msg()->ext_type(), t_fb_data->s_msg()->msg_content()->c_str(),
		t_fb_data->s_msg()->msg_time(), t_fb_data->s_msg()->send_user_name()->c_str());
	loServerSendPack.fbbuilder.Finish(ls_msg);
	loServerSendPack.fbbuf = offcialpack::CreateT_SERVER_FANS_MESSAGE_RQ(loServerSendPack.fbbuilder,
		&s_rq_head,
		t_fb_data->message_id(),
		aiUserID, 
		t_fb_data->offcial_id(),
		ls_msg,
		t_fb_data->b_id(),
		t_fb_data->w_id(),
		t_fb_data->c_id());
	//CServerToKafka::GetInstance().DistributeMsgToScRoute(loServerSendPack.GetPackType(), 
	//	t_fb_data->offcial_id(), 
	//	loServerSendPack, IM_SC_TOPIC);
	std::vector<UINT64> vectorGroupInfo;
	if (t_fb_data->w_id() > 0)
	{
		vectorGroupInfo.push_back((UINT64)t_fb_data->w_id());
	} 
	else
	{
		vectorGroupInfo.push_back((UINT64)t_fb_data->offcial_id());
	}

	COffcialDistributeMgr::GetInstance().DistributeToKafkaWs(-1, vectorGroupInfo, 
		loServerSendPack, false, aiUserID);
	///////////////////////////////////////begin /////////////////////////////////////////////
	flatbuffers::FlatBufferBuilder fbbuilder_offline;
	flatbuffers::Offset<commonpack::S_MSG> s_server_offline_msg = commonpack::CreateS_MSGDirect(fbbuilder_offline,
		t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
		t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
		t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
		t_fb_data->s_msg()->msg_content()->c_str(), t_fb_data->s_msg()->msg_time(),
		t_fb_data->s_msg()->send_user_name()->c_str());
	fbbuilder_offline.Finish(s_server_offline_msg);

	flatbuffers::Offset<offcialpack::T_FANS_OFFLINE_MESSAGE> msg_offline_info = offcialpack::CreateT_FANS_OFFLINE_MESSAGE(
		fbbuilder_offline,
		aiUserID,
		t_fb_data->message_id(),
		s_server_offline_msg,
		t_fb_data->b_id(),
		t_fb_data->w_id(),
		t_fb_data->c_id());
	fbbuilder_offline.Finish(msg_offline_info);
	BYTE*	lpData = fbbuilder_offline.GetBufferPointer();

	std::string strElement;
	strElement.assign((char*)lpData, fbbuilder_offline.GetSize());

	mpServerMgr->mpRedisUtil->ZaddFansOffLineChatInfo(t_fb_data->offcial_id(), t_fb_data->message_id(), strElement);

	
	RJ_CREATE_O_DOCUMENT(doc, l_allocator)
		JsonParse::add<unsigned>(doc, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
	JsonParse::add<unsigned>(doc, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
	JsonParse::add<unsigned>(doc, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
	JsonParse::add<unsigned>(doc, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
	JsonParse::add<unsigned>(doc, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
	JsonParse::add<unsigned>(doc, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
	JsonParse::add<uint64_t>(doc, "msg_time", (uint64_t)t_fb_data->s_msg()->msg_time(), l_allocator);
	string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
	JsonParse::add(doc, "send_user_name", send_user_name, l_allocator);
	string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
	JsonParse::add(doc, "msg_content", msg_content, l_allocator);
	JsonParse::add<uint64_t>(doc, "offcial_id", (uint64_t)t_fb_data->offcial_id(), l_allocator);
	JsonParse::add<uint64_t>(doc, "message_id", (uint64_t)t_fb_data->message_id(), l_allocator);
	JsonParse::add<uint64_t>(doc, "fans_id", (uint64_t)aiUserID, l_allocator);
	JsonParse::add<uint64_t>(doc, "b_id", (uint64_t)t_fb_data->b_id(), l_allocator);
	JsonParse::add<uint64_t>(doc, "w_id", (uint64_t)t_fb_data->w_id(), l_allocator);
	JsonParse::add<uint64_t>(doc, "c_id", (uint64_t)t_fb_data->c_id(), l_allocator);

	std::string out_json = "";
	JsonParse::to_string(doc, out_json);

	LONG nRedisResult = 0;
	mpServerMgr->mpRedisUtil->LpushFanMsgdb(t_fb_data->offcial_id(),out_json,nRedisResult);
	if(nRedisResult <= 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
			<< " redis fail");
	}
	if (t_fb_data->b_id() > 0)
	{
		RJ_CREATE_O_DOCUMENT(l_document, l_allocator)
			JsonParse::add(l_document, "userid", std::to_string(aiUserID), l_allocator);
		JsonParse::add<unsigned>(l_document, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
		JsonParse::add<unsigned>(l_document, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
		JsonParse::add<unsigned>(l_document, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
		JsonParse::add<unsigned>(l_document, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
		JsonParse::add<unsigned>(l_document, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);
		JsonParse::add<unsigned>(l_document, "chat_type", (unsigned)t_fb_data->s_msg()->chat_type(), l_allocator);
		string msg_content = t_fb_data->s_msg()->msg_content()->c_str();
		JsonParse::add(l_document, "msg_content", msg_content, l_allocator);
		JsonParse::add(l_document, "msg_time", to_string(t_fb_data->s_msg()->msg_time()), l_allocator);
		string send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
		JsonParse::add(l_document, "send_user_name", send_user_name, l_allocator);
		JsonParse::add(l_document, "message_id", to_string(t_fb_data->message_id()), l_allocator);
		JsonParse::add(l_document, "op_user_id", to_string(t_fb_data->offcial_id()), l_allocator);
		JsonParse::add(l_document, "b_id", to_string(t_fb_data->b_id()), l_allocator);
		JsonParse::add(l_document, "c_id", to_string(t_fb_data->c_id()), l_allocator);
		JsonParse::add(l_document, "w_id", to_string(t_fb_data->w_id()), l_allocator);

		std::string out_json2 = "";
		JsonParse::to_string(l_document, out_json2);

		LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---JsonParse  out_json2=" << out_json2);

		nRedisResult=0;
		mpServerMgr->mpRedisUtil->RPushEcChatInfoToDbBack(t_fb_data->offcial_id() % EC_CHAT_TO_BACK_COUNT,out_json2,nRedisResult);
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " 2 t_fb_data->offcial_id() = " << t_fb_data->offcial_id()
				<< " redis fail");
		}
	}
	/////////////////////////////////////////end//////////////////////////////////////////////

	return 1;
}

INT CClientSide::FansMessageRS(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	GENERAL_UNPACK_BODY(STRU_SERVER_FANS_MESSAGE_RS, FALSE)
	auto t_fb_data = loUserLoad.t_fb_data;
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "----RS "
		<<" package_session_id="<< t_fb_data->s_rs_head()->pack_session_id()
		<<" user_id=" << t_fb_data->s_rs_head()->user_id());


	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << t_fb_data->s_rs_head()->user_id()
		<< " pack_session_id = " << t_fb_data->s_rs_head()->pack_session_id()
		<< " app_id = " << t_fb_data->s_msg()->app_id()
		<< " session_id = " << t_fb_data->s_msg()->session_id()
		<< " chat_type = " << t_fb_data->s_msg()->chat_type()
		<< " m_type = " << t_fb_data->s_msg()->m_type()
		<< " s_type = " << t_fb_data->s_msg()->s_type() 
		<< " ext_type = " << t_fb_data->s_msg()->ext_type()
		<< " msg_content = " << t_fb_data->s_msg()->msg_content()->c_str()
		<< " msg_time = " << t_fb_data->s_msg()->msg_time()
		<< " message_id = " << t_fb_data->message_id()
		<< " offcial_id = " << t_fb_data->offcial_id());

	mpServerMgr->mpRedisUtil->ZremFansOffLineChatInfobyscore(t_fb_data->offcial_id(), t_fb_data->message_id(),t_fb_data->message_id());

}

INT CClientSide::OffcialGetOfflineMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	CUser* lpUser = (CUser*)apRelationPtr;
	STRU_GENERAL_BODY_PACKAGE<STRU_CLIENT_OFFCIALR_GET_MESSAGE_RQ> loRecvPack;
	const STRU_CLIENT_OFFCIALR_GET_MESSAGE_RQ &loUserLoad = loRecvPack.moUserLoad;
	INT nResult = loRecvPack.UnPack(apData, awLen);
	if (0 >= nResult)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error user_id is invalid user_id = "<< aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_GET_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_UNPACK_FAILED_RESULT));
		return 0;
	}
	auto t_fb_data = loUserLoad.t_fb_data;
	int liResult = MAKE_SUCCESS_RESULT();
	commonpack::S_RS_HEAD s_rs_head(t_fb_data->s_rq_head()->user_id(),
		t_fb_data->s_rq_head()->pack_session_id(),
		liResult, 
		t_fb_data->s_rq_head()->platform());
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);

	if(!t_fb_data->offcial_id())
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," ---OffcialSendMessage t_fb_data->offcial_id error.");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CLIENT_OFFCIALR_GET_MESSAGE_RS::ENUM_TYPE, MAKE_ERROR_RESULT(RET_OFFCIALMSG_CONTENT_BASE))
			return 0;
	}

	STRU_CLIENT_OFFCIALR_GET_MESSAGE_RS loSendPack;
	auto offcialId = loUserLoad.t_fb_data->offcial_id();
	TEXTMSGID message_id = loUserLoad.t_fb_data->message_id();

	std::vector<std::string> vector_chat_info;
	//std::vector<flatbuffers::Offset<flatbuffers::String> > vector_offline_msg;
	std::vector<flatbuffers::Offset<offcialpack::T_FANS_OFFLINE_MESSAGE> > vector_offline_msg;
	mpServerMgr->mpRedisUtil->ZRrangeOffLineFanInfo(
		offcialId, 
		0, DEF_PACK_GROUP_SPAN_NUMBER, 
		&vector_chat_info);

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "OffcialGetOfflineMessageRQ vector_chat_info size = " << vector_chat_info.size());

	for(int index = 0; index < vector_chat_info.size(); index++)
	{
		if (vector_chat_info[index].size() == 0)
		{
			continue;
		}
		auto poffcial=GetT_FANS_OFFLINE_MESSAGE(vector_chat_info[index].c_str());

		if (!poffcial->s_msg() || !poffcial->s_msg()->msg_content() || !poffcial->s_msg()->msg_content()->c_str() 
			 || !poffcial->s_msg()->send_user_name() || !poffcial->s_msg()->send_user_name()->c_str())
		{
			continue;
		}

		flatbuffers::Offset<commonpack::S_MSG> s_server_offline_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder,
			poffcial->s_msg()->app_id(), poffcial->s_msg()->session_id(),
			poffcial->s_msg()->chat_type(), poffcial->s_msg()->m_type(),
			poffcial->s_msg()->s_type(), poffcial->s_msg()->ext_type(),
			poffcial->s_msg()->msg_content()->c_str(), poffcial->s_msg()->msg_time(),
			poffcial->s_msg()->send_user_name()->c_str());

		flatbuffers::Offset<offcialpack::T_FANS_OFFLINE_MESSAGE> msg_offline_info = offcialpack::CreateT_FANS_OFFLINE_MESSAGE(
			loSendPack.fbbuilder,
			poffcial->user_id(),
			poffcial->message_id(),
			s_server_offline_msg,
			poffcial->b_id(),
			poffcial->w_id(),
			poffcial->c_id());
		vector_offline_msg.push_back(msg_offline_info);
		message_id = poffcial->message_id();
		//vector_offline_msg.push_back(loSendPack.fbbuilder.CreateString(vector_chat_info[index]));
	}

	if (vector_offline_msg.size() > 0)
	{
		auto vector_msg_builder = loSendPack.fbbuilder.CreateVector(vector_offline_msg);
		loSendPack.fbbuf = offcialpack::CreateT_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RS(loSendPack.fbbuilder,
			&s_rs_head, 
			t_fb_data->offcial_id(),
			vector_msg_builder);
		mpServerMgr->mpRedisUtil->ZremFansOffLineChatInfobyscore(t_fb_data->offcial_id(), 0,message_id);
	}
	else
	{
		loSendPack.fbbuf = offcialpack::CreateT_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RS(loSendPack.fbbuilder,
			&s_rs_head, 
			t_fb_data->offcial_id(),
			0);
	}

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---RS,  aiUserID=" << aiUserID);
	
	return 1;
}

INT CClientSide::FansGetOfflineMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_CLIENT_FANS_GET_MESSAGE_RQ, aiUserID, STRU_CLIENT_FANS_GET_MESSAGE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "---RQ,  aiUserID=" << aiUserID);
	BYTE is_finish = OFFCIAL_MSG_FINISH;
	STRU_CLIENT_FANS_GET_MESSAGE_RS loSendPack;
	auto list_offcial_offline_msg_request = loUserLoad.t_fb_data->list_offcial_offline_msg_request();
	UINT list_size = list_offcial_offline_msg_request->size();
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " size = " << list_size);
	std::vector<flatbuffers::Offset<offcialpack::T_OFFCIAL_OFFLINE_MESSAGE> > vector_offcial_offline_msg;
	for(UINT i = 0; i < list_size; i++)
	{
		auto pMsg_offcial_base_info = list_offcial_offline_msg_request->Get(i);
		LOG_TRACE(LOG_DEBUG,false, __FUNCTION__, " offcial_id = " << (TEXTMSGID)pMsg_offcial_base_info->offcial_id()
			<< " message_id = " << (TEXTMSGID)pMsg_offcial_base_info->next_message_id());

		TEXTMSGID message_id = pMsg_offcial_base_info->next_message_id();
		if (message_id < 0)
		{
			message_id = 0;
		}
		BYTE is_finish1 = OFFCIAL_MSG_FINISH;
		std::vector<std::string> vector_chat_info;
		//std::vector<flatbuffers::Offset<flatbuffers::String> > vector_offline_msg;
		std::vector<flatbuffers::Offset<offcialpack::T_OFFLINE_MESSAGE> > vector_offline_msg;
		mpServerMgr->mpRedisUtil->ZRrangeOffcialOffLineChatInfoByScoreToMax(
			pMsg_offcial_base_info->offcial_id(), 
			message_id+10, DEF_PACK_GROUP_SPAN_NUMBER, 
			&vector_chat_info);
		if (vector_chat_info.size() >= DEF_PACK_GROUP_SPAN_NUMBER)
		{
			is_finish=OFFCIAL_MSG_NO_FINISH;
			is_finish1=OFFCIAL_MSG_NO_FINISH;
		}
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "vector_chat_info size = " << vector_chat_info.size());
		std::string table_name = std::string(SC_OFFCIAL_CHAT_INFO) + std::to_string(pMsg_offcial_base_info->offcial_id());
		mpServerMgr->mpRedisUtil->ZremrangebyrankSysMsg(table_name,0, -1001);

		for(int index = 0; index < vector_chat_info.size(); index++)
		{
			LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "---- vector_chat_info[index] size = " << vector_chat_info[index].size());
			if (vector_chat_info[index] != "" )
			{
				auto poffcial=GetT_OFFCIAL_MESSAGE(vector_chat_info[index].c_str());
				message_id = poffcial->message_id();

				if (message_id & 1 == 1)
				{
					std::string str = "";
					str = mpServerMgr->mpRedisUtil->HCheckFansId(pMsg_offcial_base_info->offcial_id(),std::to_string(message_id),aiUserID);
					if (str != "1")
					{
						continue;
					}
				}

				flatbuffers::Offset<commonpack::S_MSG> s_offline_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder,
					poffcial->s_msg()->app_id(), poffcial->s_msg()->session_id(),
					poffcial->s_msg()->chat_type(), poffcial->s_msg()->m_type(),
					poffcial->s_msg()->s_type(), poffcial->s_msg()->ext_type(),
					poffcial->s_msg()->msg_content()->c_str(), poffcial->s_msg()->msg_time(),
					poffcial->s_msg()->send_user_name()->c_str());

				flatbuffers::Offset<offcialpack::T_OFFLINE_MESSAGE> msg_offline_info = offcialpack::CreateT_OFFLINE_MESSAGE(
					loSendPack.fbbuilder,
					poffcial->message_id(),
					s_offline_msg);

				vector_offline_msg.push_back(msg_offline_info);
			}
		}
		if (vector_offline_msg.size() > 0)
		{
			auto poffcial=GetT_OFFCIAL_MESSAGE(vector_chat_info[0].c_str());
			auto vector_msg_builder = loSendPack.fbbuilder.CreateVector(vector_offline_msg);
			auto offcial_all_offline_msg = offcialpack::CreateT_OFFCIAL_OFFLINE_MESSAGE(loSendPack.fbbuilder,
				is_finish1,
				pMsg_offcial_base_info->offcial_id(), 
				vector_msg_builder,
				poffcial->b_id(),
				poffcial->w_id(),
				poffcial->c_id());
			vector_offcial_offline_msg.push_back(offcial_all_offline_msg);
		}
	}
	uint64_t message_id2 = 0;
	std::vector<std::string> vector_chat_lst;
	std::vector<flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> > vector_sys_msg;
	mpServerMgr->mpRedisUtil->ZRrangeOffcialOffLinePrivateChatInfo(
		aiUserID, 
		0, 
		DEF_PACK_GROUP_SPAN_NUMBER, 
		&vector_chat_lst);
	if (vector_chat_lst.size() >= DEF_PACK_GROUP_SPAN_NUMBER)
	{
		is_finish=OFFCIAL_MSG_NO_FINISH;
	}
	for(int index = 0; index < vector_chat_lst.size(); index++)
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "--- vector_chat_lst[index] size = " << vector_chat_lst[index].size());
		auto poffcial=GetT_OFFCIAL_MESSAGE(vector_chat_lst[index].c_str());

		flatbuffers::Offset<commonpack::S_MSG> s_sys_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder,
			poffcial->s_msg()->app_id(), poffcial->s_msg()->session_id(),
			poffcial->s_msg()->chat_type(), poffcial->s_msg()->m_type(),
			poffcial->s_msg()->s_type(), poffcial->s_msg()->ext_type(),
			poffcial->s_msg()->msg_content()->c_str(), poffcial->s_msg()->msg_time(),
			poffcial->s_msg()->send_user_name()->c_str());

		flatbuffers::Offset<offcialpack::T_OFFCIAL_MESSAGE> msg_sys_info = offcialpack::CreateT_OFFCIAL_MESSAGE(
			loSendPack.fbbuilder,
			poffcial->message_id(),
			poffcial->offcial_id(),
			s_sys_msg,
			poffcial->b_id(),
			poffcial->w_id(),
			poffcial->c_id());
		message_id2 = poffcial->message_id();
		vector_sys_msg.push_back(msg_sys_info);
	}
	mpServerMgr->mpRedisUtil->ZremFansOffLinePrivateChatInfobyscore(aiUserID, 0,message_id2);
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "---RS,  aiUserID=" << aiUserID);

	auto vector_msg_builder = loSendPack.fbbuilder.CreateVector(vector_sys_msg);
	auto vector_offcial_all_offline_builder = loSendPack.fbbuilder.CreateVector(vector_offcial_offline_msg);
	//if (vector_offcial_offline_msg.size() == 0)
	//{
	//	vector_offcial_all_offline_builder =0;
	//}
	loSendPack.fbbuf = offcialpack::CreateT_CLIENT_FANS_GET_OFFLINE_MESSAGE_RS(loSendPack.fbbuilder,
		&s_rs_head, 
		is_finish,
		vector_offcial_all_offline_builder,
		vector_msg_builder);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	return 1;
}

