#include "stdafx.h"
#include "const/ErrorCodeDef.h"

#include "ClientSide.h"
#include "../ServerMgr.h"
#include "ServerToKafka.h"
#include "network/packet/client/CommonPackDef.h"
#include "const/PackDef.h"
#include "ServerUtilMgr.h"
#include "RedisClass/GcModelDef.h"
#include "RedisClass/CommonModelDef.h"
#include "../GroupToKafka/GroupDistributeMgr.h"

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

INT CClientSide::GroupClientSendMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_CLIENT_SEND_MESSAGE_RQ, aiUserID, STRU_GROUP_CLIENT_SEND_MESSAGE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	INT64 message_id = t_fb_data->message_id();
	string strMsgId = to_string(message_id);
	if(strMsgId.length() != 16)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "message_id = " + strMsgId + " is invalid");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CLIENT_SEND_MESSAGE_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_GROUP_MESSAGE_ID_INVALID))
		return 0;
	}

	LONG is_exist = 0;
	mpServerMgr->mpRedisUtil->HexistGroupUserId(t_fb_data->group_id(), aiUserID, is_exist);
	if(is_exist <= 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "aiUserID = " + to_string(aiUserID) + " group_id = "
					<< to_string(t_fb_data->group_id()) + " is not in group");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CLIENT_SEND_MESSAGE_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_GROUP_USER_NOT_JOIN))
		return 0;
	}

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

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "nMaxCount = " << nMaxCount);
	if(nMaxCount > DEF_SC_CHAT_MAX_CONTENT)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "msg_size = " << out_msg_content.size());
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CLIENT_SEND_MESSAGE_RS::ENUM_TYPE,
			MAKE_ERROR_RESULT(RET_CHAT_MSG_CONTENT_MAX))
		return 1;
	}

	STRU_GROUP_CLIENT_SEND_MESSAGE_RS loSendPack;
	auto s_msg = commonpack::CreateS_MSGDirect(loSendPack.fbbuilder, t_fb_data->s_msg()->app_id(),
									t_fb_data->s_msg()->session_id(), t_fb_data->s_msg()->chat_type(),
									t_fb_data->s_msg()->m_type(), t_fb_data->s_msg()->s_type(),
									t_fb_data->s_msg()->ext_type(), out_msg_content.c_str(),
									t_fb_data->s_msg()->msg_time(), t_fb_data->s_msg()->send_user_name()->c_str());
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
					<< " msg_time = " << t_fb_data->s_msg()->msg_time()
					<< " message_id = " << t_fb_data->message_id()
					<< " group_id = " << (TEXTMSGID)t_fb_data->group_id());


	loSendPack.fbbuf = CreateT_GROUP_CLIENT_SEND_MESSAGE_RS(loSendPack.fbbuilder,
									&s_rs_head, t_fb_data->message_id(),
									t_fb_data->group_id(), s_msg);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);


	///////////////////////////////////////begin /////////////////////////////////////////////
	flatbuffers::FlatBufferBuilder fbbuilder_offline;
	flatbuffers::Offset<commonpack::S_MSG> s_server_offline_msg = commonpack::CreateS_MSGDirect(fbbuilder_offline, 
									t_fb_data->s_msg()->app_id(), t_fb_data->s_msg()->session_id(),
									t_fb_data->s_msg()->chat_type(), t_fb_data->s_msg()->m_type(),
									t_fb_data->s_msg()->s_type(), t_fb_data->s_msg()->ext_type(),
									out_msg_content.c_str(), t_fb_data->s_msg()->msg_time(),
									t_fb_data->s_msg()->send_user_name()->c_str());
	fbbuilder_offline.Finish(s_server_offline_msg);

	flatbuffers::Offset<grouppack::T_OFFLINE_GROUP_MSG> msg_offline_info = grouppack::CreateT_OFFLINE_GROUP_MSG(
								fbbuilder_offline, t_fb_data->s_rq_head()->user_id(),
								t_fb_data->message_id(), s_server_offline_msg, GROUP_OFFLINE_CHAT_NORMAL);
	fbbuilder_offline.Finish(msg_offline_info);
	BYTE*	lpData = fbbuilder_offline.GetBufferPointer();

    std::string strElement;
	strElement.assign((char*)lpData, fbbuilder_offline.GetSize());

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "group_id = " << t_fb_data->group_id()
						<< " message_id = " << t_fb_data->message_id());
						
	mpServerMgr->mpRedisUtil->ZaddGroupOffLineChatInfo(
							t_fb_data->group_id(), t_fb_data->message_id(), strElement);
	/////////////////////////////////////////end//////////////////////////////////////////////

	string group_name = t_fb_data->group_name()->c_str();
	STRU_GROUP_NOTIFY_SERVER_RQ loServerSendPack;
	loServerSendPack.fbbuf = grouppack::CreateT_GROUP_CHAT_NOTIFY_RQ(loServerSendPack.fbbuilder,
									t_fb_data->group_id(), loServerSendPack.fbbuilder.CreateString(group_name));

	std::map<std::string, std::string> mapGroupInfo;
	mpServerMgr->mpRedisUtil->SmembersGroupUserId(t_fb_data->group_id(), mapGroupInfo);

	UINT64 group_id = t_fb_data->group_id();
	CGroupDistributeMgr::GetInstance().DistributeToKafka(aiUserID, mapGroupInfo, 
							loServerSendPack, true, aiUserID, group_id);

	RJ_CREATE_O_DOCUMENT(l_document, l_allocator)
	JsonParse::add(l_document, "userid", std::to_string(aiUserID), l_allocator);
	JsonParse::add(l_document, "group_id", std::to_string(t_fb_data->group_id()), l_allocator);
	JsonParse::add<unsigned>(l_document, "app_id", (unsigned)t_fb_data->s_msg()->app_id(), l_allocator);
	JsonParse::add<unsigned>(l_document, "session_id", (unsigned)t_fb_data->s_msg()->session_id(), l_allocator);
	JsonParse::add<unsigned>(l_document, "m_type", (unsigned)t_fb_data->s_msg()->m_type(), l_allocator);
	JsonParse::add<unsigned>(l_document, "s_type", (unsigned)t_fb_data->s_msg()->s_type(), l_allocator);
	JsonParse::add<unsigned>(l_document, "ext_type", (unsigned)t_fb_data->s_msg()->ext_type(), l_allocator);

	JsonParse::add(l_document, "msg_content", out_msg_content, l_allocator);
	JsonParse::add(l_document, "msg_time", to_string(t_fb_data->s_msg()->msg_time()), l_allocator);
	
	string Send_user_name = t_fb_data->s_msg()->send_user_name()->c_str();
	JsonParse::add(l_document, "send_user_name", Send_user_name, l_allocator);
	JsonParse::add(l_document, "message_id", to_string(t_fb_data->message_id()), l_allocator);

	std::string szmsg = "";
	JsonParse::to_string(l_document, szmsg);
	
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "send to db msg:" << szmsg);
	LONG nRedisResult = 0;
	mpServerMgr->mpRedisUtil->RPushGcChatInfoToDbBack(group_id % GC_CHAT_TO_BACK_COUNT, szmsg, nRedisResult);
	if(nRedisResult <= 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " szmsg = " << szmsg 
						<< " is back failed");
	}
}


INT CClientSide::GroupGetOfflineMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_GET_OFFLINE_MESSAGE_RQ, aiUserID, STRU_GROUP_GET_OFFLINE_MESSAGE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	UINT64 last_login_out_id = 0;
	STRU_GROUP_GET_OFFLINE_MESSAGE_RS loSendPack;
	auto list_group_offline_msg_request = loUserLoad.t_fb_data->list_group_offline_msg_request();
	UINT list_size = list_group_offline_msg_request->size();
	LOG_TRACE(LOG_INFO, false, __FUNCTION__, "aiUserID = " << aiUserID << " list_size = " << list_size);
	
	if(list_size > DEF_GROUP_OFFLINE_MAX_COUNT)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "list_size = " << list_size << " aiUserId = " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_GET_OFFLINE_MESSAGE_RS::ENUM_TYPE, 
							MAKE_ERROR_RESULT(RET_GROUP_OFFLINE_MAX_COUNT))
		return 0;
	}

	INT max_content_size = 0;
	std::vector<flatbuffers::Offset<grouppack::T_GROUP_ALL_OFFLINE_MSG> > vector_group_all_offline_msg;
	for(UINT i = 0; i < list_size; i++)
	{
		auto pMsg_group_base_info = list_group_offline_msg_request->Get(i);
		UINT64 group_id = pMsg_group_base_info->group_id();
		TEXTMSGID message_id = pMsg_group_base_info->next_message_id();
		if(message_id == 0)
		{
			if(last_login_out_id == 0)
			{
				std::string update_time;
				mpServerMgr->mpRedisUtil->HGetUserTimeByType(aiUserID, LOG_OUT_TYPE, update_time);

				last_login_out_id = atol(update_time.c_str());
				if(last_login_out_id <= 0)
				{
					// 默认为是新注册的
//					last_login_out_id = CSystem::GetMicroSystemTime();
					last_login_out_id = 100000;
					LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << (USERID)aiUserID << " is new user");
				}
				else
				{
					last_login_out_id = last_login_out_id - 1 * 1000 * 1000;
				}


				if(last_login_out_id < 0)
				{
					LOG_TRACE(LOG_ERR, false, __FUNCTION__, " last_login_out_id = " << last_login_out_id << " is invalid");
					TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_GET_OFFLINE_MESSAGE_RS::ENUM_TYPE,
								MAKE_ERROR_RESULT(RET_GROUP_MESSAGE_ID_INVALID))
					return 0;
				}
			}

			string strGroupMsgId;
			mpServerMgr->mpRedisUtil->HgetGroupMsgId(group_id, aiUserID, strGroupMsgId);

			message_id = last_login_out_id;
			if(!strGroupMsgId.empty())
			{
				UINT64 message_id_old = ServerUtilMgr::ChangeStringToUll(strGroupMsgId);
				if(message_id_old > message_id)
				{
					message_id = message_id_old;
				}	
			}

			LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "strGroupMsgId = " << strGroupMsgId << " last_login_out_id = " << last_login_out_id);
		}

		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " group_id = " << group_id
						<< " next_message_id = " << message_id << " last_login_out_id = " << last_login_out_id);

		if(max_content_size > DEF_PACK_OFFLINE_CONTENT_SIZE)
		{
			BYTE is_finish = GROUP_OFFLINE_MSG_NO_FINISH;
			auto group_all_offline_msg = grouppack::CreateT_GROUP_ALL_OFFLINE_MSG(loSendPack.fbbuilder,
									is_finish, group_id);
	
			vector_group_all_offline_msg.push_back(group_all_offline_msg);
			break;
		}
		// 首先删除超过1000的消息
		mpServerMgr->mpRedisUtil->ZremrangebyrankGroupOffLineChatInfo(group_id, 0, -1001);

		std::vector<std::string> vector_chat_info;
		mpServerMgr->mpRedisUtil->ZRrangeGroupOffLineChatInfoByScoreToMax(
								group_id, 
								message_id, DEF_PACK_GROUP_SPAN_NUMBER, 
								&vector_chat_info);


		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " group_id = " << group_id 
						<< " message_id = " << message_id << " vector_chat_info size = " << vector_chat_info.size());

		std::vector<flatbuffers::Offset<grouppack::T_OFFLINE_GROUP_MSG> > vector_offline_msg;
		INT msg_handle_number = 0;
		for(int index = 0; index < vector_chat_info.size(); index++)
		{
			max_content_size += vector_chat_info[index].size();
			if(max_content_size > DEF_PACK_OFFLINE_CONTENT_SIZE)
			{
				break;
			}
			msg_handle_number++;
			
			auto offline_group_redis_msg = GetT_OFFLINE_GROUP_MSG((char*)vector_chat_info[index].c_str());

			auto s_redis_msg = offline_group_redis_msg->s_msg();
			flatbuffers::Offset<commonpack::S_MSG> s_msg = 0;
			if(s_redis_msg)
			{
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
				
				s_msg = commonpack::CreateS_MSG(loSendPack.fbbuilder,
						 	s_redis_msg->app_id(),
							s_redis_msg->session_id(), s_redis_msg->chat_type(),
							s_redis_msg->m_type(), s_redis_msg->s_type(),
							s_redis_msg->ext_type(), 
							loSendPack.fbbuilder.CreateString(msg_content),
							s_redis_msg->msg_time(),
							loSendPack.fbbuilder.CreateString(send_user_name));
			}

			auto operate_group_redis_msg = offline_group_redis_msg->operate_group_msg();
			flatbuffers::Offset<T_OPERATE_GROUP_MSG> operate_group_msg = 0;
			if(operate_group_redis_msg)
			{
				char* operate_user_name = "";
				if(operate_group_redis_msg->operate_user_name())
				{
					operate_user_name = (char*)operate_group_redis_msg->operate_user_name()->c_str();
				}

				char* group_modify_content = "";
				if(operate_group_redis_msg->group_modify_content())
				{
					group_modify_content = (char*)operate_group_redis_msg->group_modify_content()->c_str();
				}

				auto user_info_redis_list = operate_group_redis_msg->user_info_list();
				std::vector<flatbuffers::Offset<USER_BASE_INFO>> vector_user_list;
				if(user_info_redis_list)
				{
					for(INT index = 0; index < user_info_redis_list->size(); index++)
					{
						auto user_base_redis_info = user_info_redis_list->Get(index);
						
						char* user_nick_name = "";
						if(user_base_redis_info->user_nick_name())
						{
							user_nick_name = (char*)user_base_redis_info->user_nick_name()->c_str();
						}
						
						auto user_base_info = CreateUSER_BASE_INFO(loSendPack.fbbuilder,
									user_base_redis_info->user_id(),
									loSendPack.fbbuilder.CreateString(user_nick_name), 
									user_base_redis_info->user_group_index());

						vector_user_list.push_back(user_base_info);
					}
				}

				operate_group_msg = CreateT_OPERATE_GROUP_MSG(loSendPack.fbbuilder,
								operate_group_redis_msg->msg_time(),
								loSendPack.fbbuilder.CreateString(operate_user_name),
								loSendPack.fbbuilder.CreateVector(vector_user_list),
								loSendPack.fbbuilder.CreateString(group_modify_content));
			}

			auto group_info_redis = offline_group_redis_msg->group_info();
			flatbuffers::Offset<T_GROUP_BASE_INFO> group_base_info = 0;
			if(group_info_redis)
			{
				char* group_name = "";
				if(group_info_redis->group_name())
				{
					group_name = (char*)group_info_redis->group_name()->c_str();
				}

				group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
									loSendPack.fbbuilder, 
									group_info_redis->group_id(), 
									loSendPack.fbbuilder.CreateString(group_name), 
									0, group_info_redis->group_count(), 
									group_info_redis->group_manager_user_id(),
									0, 0, 0, 
									group_info_redis->group_max_count(), 0, 
									group_info_redis->group_add_max_count());
			}
			

			auto offline_group_msg = grouppack::CreateT_OFFLINE_GROUP_MSG(loSendPack.fbbuilder, 
								offline_group_redis_msg->user_id(),
								offline_group_redis_msg->message_id(),
								s_msg,
								offline_group_redis_msg->big_msg_type(),
								operate_group_msg,
								offline_group_redis_msg->message_old_id(),
								group_base_info);




			vector_offline_msg.push_back(offline_group_msg);
		}

		BYTE is_finish = GROUP_OFFLINE_MSG_NO_FINISH;
		if (vector_chat_info.size() < DEF_PACK_GROUP_SPAN_NUMBER && 
				vector_chat_info.size() == msg_handle_number)
		{
			is_finish = GROUP_OFFLINE_MSG_FINISH;
		}

		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "vector_size = " << vector_chat_info.size()
						<< " msg_handle_number = " << msg_handle_number 
						<< " group_id = " << group_id << " is_finish = " << is_finish)

		auto vector_msg_builder = loSendPack.fbbuilder.CreateVector(vector_offline_msg);
		auto group_all_offline_msg = grouppack::CreateT_GROUP_ALL_OFFLINE_MSG(loSendPack.fbbuilder,
								is_finish, group_id, vector_msg_builder);

		vector_group_all_offline_msg.push_back(group_all_offline_msg);
	}

	auto vector_group_all_offline_builder = loSendPack.fbbuilder.CreateVector(vector_group_all_offline_msg);
	loSendPack.fbbuf = grouppack::CreateT_GROUP_GET_OFFLINE_MESSAGE_RS(loSendPack.fbbuilder,
									&s_rs_head, vector_group_all_offline_builder);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	
	return 1;
}

INT CClientSide::GroupUpdateTimeID(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	//unpack客户端实体包
	GENERAL_UNPACK_BODY(STRU_GROUP_UPDATE_LOGOUT_ID, FALSE)
	
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "user logout "
			<<" user_id=" << aiUserID
			<< " update_time = " << loUserLoad.miUpdateTime);

	if(loUserLoad.miUpdateTime <= 0)
	{
		return -1;
	}

	mpServerMgr->mpRedisUtil->HUpdateUserTimeByType(aiUserID, LOG_OUT_TYPE, loUserLoad.miUpdateTime);
	return 1;
}
