#include "stdafx.h"

#include "ClientSide.h"
#include "../ServerMgr.h"
#include "ServerToKafka.h"
#include "const/PackDef.h"
#include "RedisClass/CommonModelDef.h"
#include "RedisClass/ScModelDef.h"
#include "rjson/JsonParser.h"
#include "network/packet/client/EcServerPackDef.h"
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

INT CClientSide::ChatClientSendMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_CHAT_CLIENT_SEND_MESSAGE_RQ, aiUserID, STRU_CHAT_CLIENT_SEND_MESSAGE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	INT nMaxCount = 0;
	string out_msg_content="";
	if(t_fb_data->s_msg()->m_type() == E_M_TYPE_TEXT)
	{
		nMaxCount = mpServerMgr->mpSensitiveMgr->Replace(t_fb_data->s_msg()->msg_content()->str(), out_msg_content);
	}
	else
	{
		out_msg_content = t_fb_data->s_msg()->msg_content()->str();
		nMaxCount = out_msg_content.size();
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "nMaxCount = " << nMaxCount << " out_msg_content = " << out_msg_content);
	if(nMaxCount > DEF_SC_CHAT_MAX_CONTENT)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "msg_size = " << out_msg_content.size());
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_CHAT_CLIENT_SEND_MESSAGE_RS::ENUM_TYPE,
			MAKE_ERROR_RESULT(RET_CHAT_MSG_CONTENT_MAX))
		return 1;
	}

	STRU_CHAT_CLIENT_SEND_MESSAGE_RS loSendPack;
	flatbuffers::Offset<commonpack::S_MSG> s_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder, 
									t_fb_data->s_msg()->app_id(),
									t_fb_data->s_msg()->session_id(),
									t_fb_data->s_msg()->chat_type(),
									t_fb_data->s_msg()->m_type(),
									t_fb_data->s_msg()->s_type(),
									t_fb_data->s_msg()->ext_type(),
									out_msg_content.c_str(),
									t_fb_data->s_msg()->msg_time(),
									t_fb_data->s_msg()->send_user_name()->c_str());
	loSendPack.fbbuilder.Finish(s_msg);

	LOG_TRACE(LOG_INFO, false, __FUNCTION__, " user_id = " << t_fb_data->s_rq_head()->user_id()
					<< " pack_session_id = " << t_fb_data->s_rq_head()->pack_session_id()
					<< " app_id = " << t_fb_data->s_msg()->app_id()
					<< " session_id = " << t_fb_data->s_msg()->session_id()
					<< " chat_type = " << t_fb_data->s_msg()->chat_type()
					<< " m_type = " << t_fb_data->s_msg()->m_type()
					<< " s_type = " << t_fb_data->s_msg()->s_type() 
					<< " ext_type = " << t_fb_data->s_msg()->ext_type()
					<< " msg_content = " << t_fb_data->s_msg()->msg_content()->c_str()
					<< " msg_time = " << (UINT64)t_fb_data->s_msg()->msg_time()
					<< " op_user_id = " << t_fb_data->op_user_id()
					<< " message_id = " << t_fb_data->message_id()
					<< " b_id = " << t_fb_data->b_id()
					<< " w_id = " << t_fb_data->w_id()
					<< " c_id = " << t_fb_data->c_id());



	loSendPack.fbbuf = scpack::CreateT_CHAT_CLIENT_SEND_MESSAGE_RS(loSendPack.fbbuilder,
									&s_rs_head, t_fb_data->op_user_id(),
									t_fb_data->message_id(), s_msg,
									t_fb_data->b_id(), t_fb_data->w_id(), t_fb_data->c_id());

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	STRU_SERVER_PACKAGE_RQ<T_CHAT_SERVER_SEND_MESSAGE_RQ> loServerSendPack(NEW_DEF_CHAT_SERVER_SEND_MESSAGE_RQ);
	commonpack::S_RQ_HEAD s_rq_head(t_fb_data->op_user_id(), 
							t_fb_data->s_rq_head()->pack_session_id(),
							t_fb_data->s_rq_head()->platform());


	flatbuffers::Offset<commonpack::S_MSG> s_server_msg = commonpack::CreateS_MSGDirect(loServerSendPack.fbbuilder, 
									t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
									t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
									t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
									out_msg_content.c_str(), t_fb_data->s_msg()->msg_time(),
									t_fb_data->s_msg()->send_user_name()->c_str());

	loServerSendPack.fbbuilder.Finish(s_server_msg);

	loServerSendPack.fbbuf = scpack::CreateT_CHAT_SERVER_SEND_MESSAGE_RQ(loServerSendPack.fbbuilder,
									&s_rq_head, t_fb_data->s_rq_head()->user_id(), t_fb_data->message_id(), 
									s_msg, t_fb_data->b_id(), t_fb_data->w_id(), t_fb_data->c_id());

	
	string strResult;
	mpServerMgr->mpRedisUtil->HGetUserSingleChatStatus(t_fb_data->op_user_id(), aiUserID, strResult);
	
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "aiUserId = " << aiUserID << " op_user_id = " << 
			t_fb_data->op_user_id() << " strResult = " << strResult);

	INT nResultToKafka = 0;
	if(!strResult.empty())
	{
		nResultToKafka = 1;
	}

	if(t_fb_data->b_id() > 0 && aiUserID == t_fb_data->c_id())
	{
		nResultToKafka = 10;
	}

	string strKeyKafka =  std::to_string(nResultToKafka);
	CServerToKafka::GetInstance().DistributeMsgToScRoute(loServerSendPack.GetPackType(), 
					t_fb_data->op_user_id(), 
					loServerSendPack, IM_SC_TOPIC, strKeyKafka);

	LONG nRedisResult = 0;
	if(t_fb_data->b_id() > 0 && aiUserID == t_fb_data->c_id())
	{
		flatbuffers::FlatBufferBuilder fbbuilder_bid;
		auto msg_ec_info = ecpack::CreateT_ECMSG_INFO(fbbuilder_bid, 
						t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
						t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
						t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
						aiUserID, t_fb_data->op_user_id(), t_fb_data->b_id(),
						t_fb_data->c_id(), t_fb_data->w_id(),t_fb_data->message_id(),
						t_fb_data->s_msg()->msg_time(),
						fbbuilder_bid.CreateString(out_msg_content.c_str()),
                        fbbuilder_bid.CreateString(t_fb_data->s_msg()->send_user_name()->c_str()));
		fbbuilder_bid.Finish(msg_ec_info);

		BYTE* lpData = fbbuilder_bid.GetBufferPointer();
		std::string strElement_bid;
		strElement_bid.assign((char*)lpData, fbbuilder_bid.GetSize());
		
		mpServerMgr->mpRedisUtil->HsetEcOffLineChatInfo(
									t_fb_data->b_id(),
									t_fb_data->message_id(), 
									strElement_bid, nRedisResult);
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " bid message_id = " << t_fb_data->message_id() 
						<< " is repeated");
		}

		mpServerMgr->mpRedisUtil->RpushEcOfflineMsgId(t_fb_data->b_id(),
									t_fb_data->message_id(), nRedisResult);
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " message_id = " << t_fb_data->message_id() 
							<< " bid is lpush failed");
		}
	}
	else 
	{
		// 需要添加离线消息到redis   
		flatbuffers::FlatBufferBuilder fbbuilder_offline;
		flatbuffers::Offset<commonpack::S_MSG> s_server_offline_msg = commonpack::CreateS_MSGDirect(fbbuilder_offline, 
										t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
										t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
										t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
										out_msg_content.c_str(), t_fb_data->s_msg()->msg_time(),
										t_fb_data->s_msg()->send_user_name()->c_str());
	
		fbbuilder_offline.Finish(s_server_offline_msg);
		flatbuffers::Offset<scpack::T_OFFLINE_MSG> msg_offline_info = scpack::CreateT_OFFLINE_MSG(fbbuilder_offline,
									aiUserID, 
									t_fb_data->message_id(),
									s_server_offline_msg,
									t_fb_data->b_id(), t_fb_data->w_id(), t_fb_data->c_id());
	
		fbbuilder_offline.Finish(msg_offline_info);
	
		BYTE* lpData = fbbuilder_offline.GetBufferPointer();
		std::string strElement;
		strElement.assign((char*)lpData, fbbuilder_offline.GetSize());
	
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " strElement "
					<< " length old = " << strElement.length()
					<< " length new = " << fbbuilder_offline.GetSize()
					<< " user_id = " << t_fb_data->op_user_id()
					<< " message_id = " << t_fb_data->message_id());
		
		LONG nRedisResult = 0;
		mpServerMgr->mpRedisUtil->HsetUserOffLineChatInfo(
										t_fb_data->op_user_id(),
										t_fb_data->message_id(), 
										strElement, nRedisResult);
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " message_id = " << t_fb_data->message_id() 
							<< " is repeated");
		}
	
		nRedisResult = 0;
		mpServerMgr->mpRedisUtil->RpushUserOfflineMsgId(t_fb_data->op_user_id(),
										t_fb_data->message_id(), nRedisResult);
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " nRedisResult = " << nRedisResult);
		if(nRedisResult > DEF_MAX_SINGLE_TEXT_COUNT)
		{
            std::vector<std::string> vector_list_ids;
			mpServerMgr->mpRedisUtil->LRangeUserOfflineMsgId(t_fb_data->op_user_id(), vector_list_ids, DEF_MAX_BATCH_DEL);

            string strDel = "";
            for(INT index = 0; index < vector_list_ids.size(); index++)
            {
                strDel += vector_list_ids[index];
                strDel += " ";
            }
			mpServerMgr->mpRedisUtil->HDelUserOffLineChatInfo(t_fb_data->op_user_id(), strDel);

            mpServerMgr->mpRedisUtil->LTrimUserOfflineMsgId(t_fb_data->op_user_id(), DEF_MAX_BATCH_DEL);
		}
		
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " message_id = " << t_fb_data->message_id() 
							<< " is lpush failed");
		}
	}

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
	
	string Send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
	JsonParse::add(l_document, "send_user_name", Send_user_name, l_allocator);
	JsonParse::add(l_document, "message_id", to_string(t_fb_data->message_id()), l_allocator);
	JsonParse::add(l_document, "op_user_id", to_string(t_fb_data->op_user_id()), l_allocator);
	JsonParse::add(l_document, "b_id", to_string(t_fb_data->b_id()), l_allocator);
	JsonParse::add(l_document, "c_id", to_string(t_fb_data->c_id()), l_allocator);
	JsonParse::add(l_document, "w_id", to_string(t_fb_data->w_id()), l_allocator);
	
	std::string szmsg = "";
	JsonParse::to_string(l_document, szmsg);
	
	nRedisResult = 0;
	mpServerMgr->mpRedisUtil->RPushScChatInfoToDbBack((aiUserID + t_fb_data->op_user_id()) % SC_CHAT_TO_BACK_COUNT, szmsg, nRedisResult);
	if(nRedisResult <= 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " szmsg = " << szmsg 
						<< " is back failed" << " nRedisResult = " << nRedisResult);
	}

	if(t_fb_data->b_id() > 0)
	{
		mpServerMgr->mpRedisUtil->RPushEcChatInfoToDbBack(t_fb_data->b_id() % EC_CHAT_TO_BACK_COUNT, szmsg, nRedisResult);
		if(nRedisResult <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " szmsg = " << szmsg 
							<< "ec is back failed" << " nRedisResult = " << nRedisResult);
		}
	}
}

INT CClientSide::ChatServerSendMessageRS(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	GENERAL_UNPACK_BODY(STRU_CHAT_SERVER_SEND_MESSAGE_RS, FALSE)
	auto t_fb_data = loUserLoad.t_fb_data;

	INT platform = t_fb_data->s_rs_head()->platform();

	LOG_TRACE(LOG_INFO, false, __FUNCTION__, " user_id = " << t_fb_data->s_rs_head()->user_id()
					<< " pack_session_id = " << t_fb_data->s_rs_head()->pack_session_id()
					<< " app_id = " << t_fb_data->s_msg()->app_id()
					<< " session_id = " << t_fb_data->s_msg()->session_id()
					<< " chat_type = " << t_fb_data->s_msg()->chat_type()
					<< " m_type = " << t_fb_data->s_msg()->m_type()
					<< " s_type = " << t_fb_data->s_msg()->s_type() 
					<< " ext_type = " << t_fb_data->s_msg()->ext_type()
					<< " msg_content = " << t_fb_data->s_msg()->msg_content()->c_str()
					<< " msg_time = " << t_fb_data->s_msg()->msg_time()
					<< " op_user_id = " << t_fb_data->op_user_id()
					<< " message_id = " << t_fb_data->message_id()
					<< " b_id = " << t_fb_data->b_id()
					<< " w_id = " << t_fb_data->w_id()
					<< " c_id = " << t_fb_data->c_id());

	
	// 删除离线的信息
	if(platform == PLATFORM_APP)
	{
		mpServerMgr->mpRedisUtil->LremUserOfflineMsgId(
					t_fb_data->s_rs_head()->user_id(),
					t_fb_data->message_id());
	}
	
	mpServerMgr->mpRedisUtil->LremUserWebOfflineMsgId(
		t_fb_data->s_rs_head()->user_id(),
		t_fb_data->message_id());

	if(t_fb_data->b_id() > 0 && platform == PLATFROM_WEB)
	{
		mpServerMgr->mpRedisUtil->LremEcOfflineMsgId(
									t_fb_data->b_id(),
									t_fb_data->message_id());
	}

	return 1;
}

INT CClientSide::ChatGetOfflineMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_CHAT_GET_OFFLINE_MESSAGE_RQ, aiUserID, STRU_CHAT_GET_OFFLINE_MESSAGE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	STRU_CHAT_GET_OFFLINE_MESSAGE_RS loSendPack;
	TEXTMSGID begin_number = t_fb_data->next_message_id();

	INT begin_redis = begin_number;
	INT end_redis = begin_redis + DEF_PACK_OFFLINE_SPAN_NUMBER - 1;
	std::vector<std::string> vector_msg_ids;
	LOG_TRACE(LOG_INFO, false, __FUNCTION__, " begin_number = " << begin_number
			<< " next_message_id = " << t_fb_data->next_message_id());

	INT platform = t_fb_data->s_rq_head()->platform();
	if(platform == PLATFROM_WEB)
	{
		mpServerMgr->mpRedisUtil->LRrangeWebOffLineChatInfo(aiUserID, 
			begin_redis, 
			end_redis, 
			&vector_msg_ids);
	}
	else
	{
		mpServerMgr->mpRedisUtil->LRrangeOffLineChatInfo(aiUserID, 
			begin_redis, 
			end_redis, 
			&vector_msg_ids);
	} 

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " msg_ids size = " << vector_msg_ids.size());
	
	std::vector<std::string> vector_chat_info;
	if(vector_msg_ids.size() > 0)
	{
		mpServerMgr->mpRedisUtil->HMgetUserOffLineChatInfo(t_fb_data->s_rq_head()->user_id(),
								vector_msg_ids, &vector_chat_info);
	}

	INT max_content_size = 0;
	std::vector<flatbuffers::Offset<scpack::T_OFFLINE_MSG> > vector_fbs_chat_info;
	for(INT index = 0; index < vector_chat_info.size(); index++)
	{
		string strElement = vector_chat_info[index];
		if(strElement.empty())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " strContent is nil exist aiUserId = " << (USERID)aiUserID);
			continue ;
		}

		max_content_size += vector_chat_info[index].size();
		if(max_content_size > DEF_PACK_OFFLINE_CONTENT_SIZE)
		{
			break;
		}

		auto t_offline_redis_msg = GetT_OFFLINE_MSG((char*)vector_chat_info[index].c_str());
		if(!t_offline_redis_msg)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " t_offline_redis_msg is nil exist aiUserId = " << (USERID)aiUserID);
			continue ;
		}

		auto s_redis_msg = t_offline_redis_msg->s_msg();
		if(!s_redis_msg)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " s_redis_msg is nil exist aiUserId = " << (USERID)aiUserID);
			continue ;
		}

		char* send_user_name = "";
		if(s_redis_msg->send_user_name())
		{
			send_user_name = (char*)s_redis_msg->send_user_name()->c_str();
		}

		char* msg_content = "";
		if(s_redis_msg->msg_content())
		{
			msg_content = (char*)s_redis_msg->msg_content()->c_str();
		}
		
		auto s_msg = commonpack::CreateS_MSG(loSendPack.fbbuilder, s_redis_msg->app_id(),
							s_redis_msg->session_id(), s_redis_msg->chat_type(),
							s_redis_msg->m_type(), s_redis_msg->s_type(),
							s_redis_msg->ext_type(), 
							loSendPack.fbbuilder.CreateString(msg_content),
							s_redis_msg->msg_time(),
							loSendPack.fbbuilder.CreateString(send_user_name));

		auto t_offline_msg = scpack::CreateT_OFFLINE_MSG(loSendPack.fbbuilder, t_offline_redis_msg->op_user_id(),
						t_offline_redis_msg->message_id(), s_msg, t_offline_redis_msg->b_id(),
						t_offline_redis_msg->w_id(), t_offline_redis_msg->c_id());
		
		
		vector_fbs_chat_info.push_back(t_offline_msg);

		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "vector_chat_info[index].size() = " << vector_chat_info[index].size());
		
		begin_number++;
	}

	TEXTMSGID next_message_id = begin_number;
	if(vector_chat_info.size() <= 0)
	{
		next_message_id = 0xffffffffffffffff;
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " next_message_id = " << next_message_id << " size = " << vector_chat_info.size());

	auto vector_msg_builder = loSendPack.fbbuilder.CreateVector(vector_fbs_chat_info);
	loSendPack.fbbuf = scpack::CreateT_CHAT_GET_OFFLINE_MESSAGE_RS(loSendPack.fbbuilder,
									&s_rs_head, next_message_id, vector_msg_builder);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	if(vector_chat_info.size() <= 0)
	{
		if(platform == PLATFORM_APP)
		{
			// 删除表
			mpServerMgr->mpRedisUtil->DelUserOffLineChatInfo(t_fb_data->s_rq_head()->user_id());
			mpServerMgr->mpRedisUtil->DelUserOffLineListChatIds(t_fb_data->s_rq_head()->user_id());			
			mpServerMgr->mpRedisUtil->DelUserWebOffLineListChatIds(t_fb_data->s_rq_head()->user_id());				
		}
		else
		{
			mpServerMgr->mpRedisUtil->DelUserWebOffLineListChatIds(t_fb_data->s_rq_head()->user_id());				
		}
	}

	return 1;
}


INT CClientSide::GetUserStatusRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GET_USER_STATUS_RQ, aiUserID, STRU_GET_USER_STATUS_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	vector<string> vector_str_user_id;
	mpServerMgr->mpRedisUtil->HKeysUserSingleChatStatus(aiUserID, vector_str_user_id);

	STRU_GET_USER_STATUS_RS loSendPack;
	if(vector_str_user_id.size() <= 0)
	{
		loSendPack.fbbuf = scpack::CreateT_GET_USER_STATUS_RS(loSendPack.fbbuilder,
									&s_rs_head);
	}
	else 
	{
		vector<uint64_t> vector_list_user_id;
		for(INT index = 0; index < vector_str_user_id.size(); index++)
		{
			vector_list_user_id.push_back((uint64_t)ServerUtilMgr::STR2UINT64(vector_str_user_id[index]));
		}

		loSendPack.fbbuf = scpack::CreateT_GET_USER_STATUS_RS(loSendPack.fbbuilder,
									&s_rs_head,
									loSendPack.fbbuilder.CreateVector(vector_list_user_id));
	}

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}

INT CClientSide::SingleChatStatusRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_SINGLE_CHAT_STATUS_RQ, aiUserID, STRU_SINGLE_CHAT_STATUS_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	if(t_fb_data->op_user_id() <= 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " op_user_id is nil aiUserId = " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_SINGLE_CHAT_STATUS_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_CHAT_SINGLE_STATUS_OP_USER_ID_INVALID))
		return 0;
	}

	if(t_fb_data->chat_status() <= 0)
	{
		mpServerMgr->mpRedisUtil->HDelUserSingleChatStatus(aiUserID, t_fb_data->op_user_id());
	}
	else
	{
		mpServerMgr->mpRedisUtil->HSetUserSingleChatStatus(aiUserID, t_fb_data->op_user_id());
	}

	STRU_SINGLE_CHAT_STATUS_RS loSendPack;
	loSendPack.fbbuf = scpack::CreateT_SINGLE_CHAT_STATUS_RS(loSendPack.fbbuilder,
									&s_rs_head, t_fb_data->op_user_id(), t_fb_data->chat_status());

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}

INT CClientSide::GetMsgUnreadRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	GENERAL_UNPACK_BODY(STRU_GET_MSG_UNREAD_RQ, true)
	GENERAL_PACK_SAME_BEGIN()

	STRU_GET_MSG_UNREAD_RS loSendPack;


	auto list_b_id = t_fb_data->list_b_id();
	INT unread_count = 0;
	if(list_b_id != NULL)
	{
		for(INT index = 0; index < list_b_id->size(); index++)
		{
			UINT64 user_id = (UINT64)list_b_id->Get(index);
			LONG nResult = 0;
			mpServerMgr->mpRedisUtil->HExistEcOfflineMsg(user_id, nResult);

			LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "user_id = " << (USERID)user_id << " nResult = " << nResult);
			if(nResult > 0)
			{
				unread_count = 1;
				break;
			}
		}
	}

	loSendPack.fbbuf = scpack::CreateT_GET_MSG_UNREAD_RS(loSendPack.fbbuilder,
	&s_rs_head, unread_count);

	return Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
}

INT CClientSide::GetEcOffLineCntRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
    STRU_PACKAGE_RS<T_EC_GET_OFFLINE_CNT_RS> loSendPack(NEW_DEF_BUSINESS_GETOFFLINECNT_RS);
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_GETOFFLINECNT_RQ, aiUserID, loSendPack.GetPackType())	

	auto t_fb_data = loUserLoad.t_fb_data;
    int liResult = MAKE_SUCCESS_RESULT();
    commonpack::S_RS_HEAD s_rs_head(aiUserID, t_fb_data->s_rq_head()->pack_session_id(),liResult,t_fb_data->s_rq_head()->platform());

    RedisUtil *pRedisUtil = mpServerMgr->mpRedisUtil;
    if (NULL == pRedisUtil)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "pRedisUtil is null");
        return 0;
    }

    const flatbuffers::Vector<uint64_t>* bidlist = t_fb_data->b_id_list();

    std::string szbids("[");
    std::string szcnts("[");

    std::vector<flatbuffers::Offset<ecpack::T_ECOFFLINECNT_INFO> > vector_fbs_offlinecnt_info;
    for (flatbuffers::uoffset_t j = 0; j < bidlist->size(); j++)
    {
        ecpack::T_ECOFFLINECNT_INFOBuilder fbsofflinecntinfo = ecpack::T_ECOFFLINECNT_INFOBuilder(loSendPack.fbbuilder);
    	uint64_t bid = (uint64_t)(bidlist->Get(j));	
        szbids += std::to_string(bid) + std::string(",");					
        long lbidcnt = 0;
		pRedisUtil->LlenGETEcOffLineCnt(bid,lbidcnt); 
        szcnts += std::to_string(lbidcnt) + std::string(","); 
        fbsofflinecntinfo.add_b_id(bid);
        fbsofflinecntinfo.add_cnt(lbidcnt);
        vector_fbs_offlinecnt_info.push_back(fbsofflinecntinfo.Finish());    
    }

    szbids += std::string("]");
    szcnts += std::string("]");
    LOG_TRACE(LOG_INFO, true, __FUNCTION__, " bids= " << szbids << " cnts = " << szcnts);

    auto vector_offlinecnt_builder = loSendPack.fbbuilder.CreateVector(vector_fbs_offlinecnt_info);

    ecpack::T_EC_GET_OFFLINE_CNT_RSBuilder c1 = ecpack::T_EC_GET_OFFLINE_CNT_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_bid_withcnt_list(vector_offlinecnt_builder);
    loSendPack.fbbuf = c1.Finish();

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);   

    return 1;
}

INT CClientSide::GetEcOffLineMsgsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
    #define MAX_EC_LASTMSG_CNT 20
    #define MAX_EC_OFFLINE_SIZE 4096

    STRU_PACKAGE_RS<T_EC_GET_OFFLINE_MESSAGE_RS> loSendPack(NEW_DEF_BUSINESS_GETOFFLINEMSGS_RS);
    TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_EC_GETOFFLINEMSGS_RQ, aiUserID, loSendPack.GetPackType())

    auto t_fb_data = loUserLoad.t_fb_data;	
    
    int liResult = MAKE_SUCCESS_RESULT();
    commonpack::S_RS_HEAD s_rs_head(aiUserID, t_fb_data->s_rq_head()->pack_session_id(),liResult,t_fb_data->s_rq_head()->platform());
  
    RedisUtil *pRedisUtil = mpServerMgr->mpRedisUtil;
    if (NULL == pRedisUtil)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "pRedisUtil is null");
        return 0;
    }
    
    INT ipos = t_fb_data->offset();
    INT isrcpos = ipos;
    INT itoalmsgsize = 0;
    std::vector<std::string> vector_msg_ids;
    std::vector<std::string> vector_chat_info;
    std::vector<flatbuffers::Offset<ecpack::T_ECMSG_INFO> > vector_fbs_chat_info;

    if ( 0 == ipos )
    {
        if (RC_SUCCESS == pRedisUtil->EcOfflineMutexUnLock(t_fb_data->b_id()))
        { 
            pRedisUtil->EcOfflineMutexLock(t_fb_data->b_id());
            pRedisUtil->LRrangeEcOfflineMsgId(t_fb_data->b_id(),
                                                ipos,
                                                ipos+MAX_EC_LASTMSG_CNT,
                                                &vector_msg_ids);

            if(vector_msg_ids.size() > 0)
            {
                pRedisUtil->HMGetEcOffLineChatInfo(t_fb_data->b_id(),
                                                vector_msg_ids, 
                                                &vector_chat_info);

                for(INT index = 0; index < vector_chat_info.size(); index++)
                {
                    ecpack::T_ECMSG_INFO *t_ecmsg_info;
                    t_ecmsg_info = (ecpack::T_ECMSG_INFO*)ecpack::GetT_ECMSG_INFO(vector_chat_info[index].c_str());

                    std::string szmsg("");
                    
                    if (t_ecmsg_info->msg_content())
                    {
                        szmsg = t_ecmsg_info->msg_content()->c_str();
                    }

                    std::string szsendusername("");
                    if (t_ecmsg_info->send_user_name())
                    {
                        szsendusername = t_ecmsg_info->send_user_name()->c_str();
                    }

                    auto fbsopmsg = loSendPack.fbbuilder.CreateString(szmsg.c_str());
                    auto fbssendusername =  loSendPack.fbbuilder.CreateString(szsendusername.c_str());
                    ecpack::T_ECMSG_INFOBuilder fbsecmsginfo = ecpack::T_ECMSG_INFOBuilder(loSendPack.fbbuilder);
                    fbsecmsginfo.add_app_id(t_ecmsg_info->app_id());
                    fbsecmsginfo.add_session_id(t_ecmsg_info->session_id());
                    fbsecmsginfo.add_chat_type(t_ecmsg_info->chat_type());
                    fbsecmsginfo.add_m_type(t_ecmsg_info->m_type());
                    fbsecmsginfo.add_s_type(t_ecmsg_info->s_type());
                    fbsecmsginfo.add_ext_type(t_ecmsg_info->ext_type());
                    fbsecmsginfo.add_from_id(t_ecmsg_info->from_id());
                    fbsecmsginfo.add_to_id(t_ecmsg_info->to_id());
                    fbsecmsginfo.add_b_id(t_ecmsg_info->b_id());                    
                    fbsecmsginfo.add_c_id(t_ecmsg_info->c_id());
                    fbsecmsginfo.add_w_id(t_ecmsg_info->w_id());
                    fbsecmsginfo.add_message_id(t_ecmsg_info->message_id());
                    fbsecmsginfo.add_msg_time(t_ecmsg_info->msg_time());
                    fbsecmsginfo.add_msg_content(fbsopmsg); 
                    fbsecmsginfo.add_send_user_name(fbssendusername);
                  
                    vector_fbs_chat_info.push_back(fbsecmsginfo.Finish());
                    ipos++;
                    itoalmsgsize += vector_chat_info[index].size();

                    if (itoalmsgsize > MAX_EC_OFFLINE_SIZE)
                    {
                        break;
                    }
                }
                
            }
            else
            {
                // 删除表
                pRedisUtil->DelEcOffLineChatInfo(t_fb_data->b_id());
                pRedisUtil->DelEcOfflineMsgId(t_fb_data->b_id());
                pRedisUtil->DelEcMutex(t_fb_data->b_id());
                ipos = 0;
            }        
        }     
    }    
    else
    {
        pRedisUtil->EcOfflineMutexLock(t_fb_data->b_id());
        pRedisUtil->LRrangeEcOfflineMsgId(t_fb_data->b_id(), 
        				                  ipos, 
        				                  ipos+MAX_EC_LASTMSG_CNT, 
        				                  &vector_msg_ids);

        if(vector_msg_ids.size() > 0)
        {
            pRedisUtil->HMGetEcOffLineChatInfo(t_fb_data->b_id(),
            						           vector_msg_ids, 
                                               &vector_chat_info);

            for(INT index = 0; index < vector_chat_info.size(); index++)
            {
                
                ecpack::T_ECMSG_INFO *t_ecmsg_info;
                t_ecmsg_info = (ecpack::T_ECMSG_INFO*)ecpack::GetT_ECMSG_INFO(vector_chat_info[index].c_str());

                std::string szmsg("");
                
                if (t_ecmsg_info->msg_content())
                {
                    szmsg = t_ecmsg_info->msg_content()->c_str();
                }

                auto fbsopmsg = loSendPack.fbbuilder.CreateString(szmsg.c_str()); 
                ecpack::T_ECMSG_INFOBuilder fbsecmsginfo = ecpack::T_ECMSG_INFOBuilder(loSendPack.fbbuilder);
                fbsecmsginfo.add_app_id(t_ecmsg_info->app_id());
                fbsecmsginfo.add_session_id(t_ecmsg_info->session_id());
                fbsecmsginfo.add_chat_type(t_ecmsg_info->chat_type());
                fbsecmsginfo.add_m_type(t_ecmsg_info->m_type());
                fbsecmsginfo.add_s_type(t_ecmsg_info->s_type());
                fbsecmsginfo.add_ext_type(t_ecmsg_info->ext_type());
                fbsecmsginfo.add_from_id(t_ecmsg_info->from_id());
                fbsecmsginfo.add_to_id(t_ecmsg_info->to_id());
                fbsecmsginfo.add_b_id(t_ecmsg_info->b_id());                    
                fbsecmsginfo.add_c_id(t_ecmsg_info->c_id());
                fbsecmsginfo.add_w_id(t_ecmsg_info->w_id());
                fbsecmsginfo.add_message_id(t_ecmsg_info->message_id());
                fbsecmsginfo.add_msg_time(t_ecmsg_info->msg_time());
                fbsecmsginfo.add_msg_content(fbsopmsg);   
              
                vector_fbs_chat_info.push_back(fbsecmsginfo.Finish());
                ipos++;

                itoalmsgsize += vector_chat_info[index].size();

                if (itoalmsgsize > MAX_EC_OFFLINE_SIZE)
                {
                    break;
                }
            }
         
        }
        else
        {
            // 删除表
            pRedisUtil->DelEcOffLineChatInfo(t_fb_data->b_id());
            pRedisUtil->DelEcOfflineMsgId(t_fb_data->b_id());
            pRedisUtil->DelEcMutex(t_fb_data->b_id());
            ipos = 0;
        }
    }   

	LOG_TRACE(LOG_INFO, true, __FUNCTION__, " bid=" << t_fb_data->b_id() << " pos = " << isrcpos << " nextpos=" << ipos << " msgsize = " << vector_chat_info.size());

    auto vector_msg_builder = loSendPack.fbbuilder.CreateVector(vector_fbs_chat_info);

    ecpack::T_EC_GET_OFFLINE_MESSAGE_RSBuilder c1 = ecpack::T_EC_GET_OFFLINE_MESSAGE_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_b_id(t_fb_data->b_id());
    c1.add_offset(ipos);
    c1.add_msg_list(vector_msg_builder);
    loSendPack.fbbuf = c1.Finish();	

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);   
    return 1;
}



