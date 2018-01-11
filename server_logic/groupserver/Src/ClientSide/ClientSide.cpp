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
#include "../DbTask/DbTaskMgr.h"

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

	LOG_TRACE(LOG_ALERT, true, __FUNCTION__, " init task cnt=" << mpServerMgr->moConfig.m_taskthreadcnt);
    if (!m_taskthreadpoll.Init(mpServerMgr->moConfig.m_taskthreadcnt))
    {
        LOG_TRACE(LOG_ERR,false,__FUNCTION__," Initialize taskthreadpoll error.");
        return false;
    }

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

INT CClientSide::GroupGetBatchInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GET_BATCH_GROUP_INFO_RQ, aiUserID, STRU_GET_BATCH_GROUP_INFO_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	auto list_group_id = t_fb_data->list_group_id();
	if(!list_group_id || list_group_id->size() <= 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "list_group_id is nil aiUserID = " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GET_BATCH_GROUP_INFO_RS::ENUM_TYPE, 
							MAKE_ERROR_RESULT(RET_GROUP_BATCH_GET_LIST_EMPTY))
		return 0;
	}

	if(list_group_id->size() > DEF_GROUP_BATCH_MAX_COUNT)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "list_group_id aiUserID = " << aiUserID << " size = " << list_group_id->size()
							<< " DEF_GROUP_BATCH_MAX_COUNT = " << DEF_GROUP_BATCH_MAX_COUNT);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GET_BATCH_GROUP_INFO_RS::ENUM_TYPE, 
							MAKE_ERROR_RESULT(RET_GROUP_BATCH_LIST_INVALID))
		return 0;
	}

	GENERATE_LOGIC_TO_DBSERVER_CMD()
	RJ_CREATE_ARRAY(child_array)
	for(INT index = 0; index < list_group_id->size(); index++)
	{
		RJ_CREATE_OBJECT(child_obj2)
		ULONG grouop_id = list_group_id->Get(index);
		JsonParse::push<ULONG>(child_array, grouop_id, l_allocator);
	}
	JsonParse::add(l_document, "list_group_id", child_array, l_allocator);

	std::string szdata = "";
	JsonParse::to_string(l_document, szdata);
	CGroupGetBatchListRQ *task_data = new CGroupGetBatchListRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
    if (NULL == task_data)
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
        return -1;
    }
	
    task_data->SetData(szdata);
    task_data->SetClientSide(this);

	m_taskthreadpoll.AddTask(task_data);
}


INT CClientSide::GroupListRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_LIST_RQ, aiUserID, STRU_GROUP_LIST_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	INT group_page_span = (INT)DEF_GROUP_LIST_PAGE_NUMBER;
	GENERATE_LOGIC_TO_DBSERVER_CMD()
	JsonParse::add(l_document, GroupDetailInfo::GetGroupListIndex(), std::to_string(t_fb_data->group_list_index()), l_allocator);
	JsonParse::add<unsigned>(l_document, GroupDetailInfo::GetGroupListPageNumber(), group_page_span, l_allocator);

	std::string szdata = "";
	JsonParse::to_string(l_document, szdata);
	CGroupListRQ *task_data = new CGroupListRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
    if (NULL == task_data)
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
        return -1;
    }
	
    task_data->SetData(szdata);
    task_data->SetClientSide(this);
	task_data->SetArgvs(t_fb_data->group_list_index(), group_page_span);

	m_taskthreadpoll.AddTask(task_data);
}

INT CClientSide::GroupListIdsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_LIST_IDS_RQ, aiUserID, STRU_GROUP_LIST_IDS_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	INT group_page_span = (INT)DEF_GROUP_LIST_IDS_PAGE_NUMBER;
	GENERATE_LOGIC_TO_DBSERVER_CMD()
	JsonParse::add(l_document, "group_list_index", std::to_string(t_fb_data->group_list_index()), l_allocator);
	JsonParse::add<unsigned>(l_document, "group_page_number", group_page_span, l_allocator);

	std::string szdata = "";
	JsonParse::to_string(l_document, szdata);
	CGroupListIds *task_data = new CGroupListIds(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
    if (NULL == task_data)
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
        return -1;
    }
	
    task_data->SetData(szdata);
    task_data->SetClientSide(this);
	task_data->SetArgvs(t_fb_data->group_list_index(), group_page_span);

	m_taskthreadpoll.AddTask(task_data);
}

INT CClientSide::GroupCreateRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_CREATE_RQ, aiUserID, STRU_GROUP_CREATE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	auto offline_group_msg = t_fb_data->offline_group_msg();
	if(!offline_group_msg)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "offline_group_msg is nil aiUserID = " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CREATE_RS::ENUM_TYPE, 
							MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR))
		return 0;
	}

	if(!offline_group_msg->message_id())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "message_id is nil aiUserID = " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CREATE_RS::ENUM_TYPE, 
							MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR))
		return 0;
	}

	string strMsgId = to_string(offline_group_msg->message_id());
	if(strMsgId.length() != 16)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "message_id = " + strMsgId + " is invalid");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CREATE_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_GROUP_MESSAGE_ID_INVALID))
		return 0;
	}

	if(!offline_group_msg->operate_group_msg())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "operate_group_msg is nil aiUserID = " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CREATE_RS::ENUM_TYPE, 
							MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR))
		return 0;
	}

	auto operate_group_msg = offline_group_msg->operate_group_msg();
	auto operate_user_name = operate_group_msg->operate_user_name();
	auto user_info_list = operate_group_msg->user_info_list();
	if(!operate_user_name || !user_info_list || user_info_list->size() <= 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "operate_user_name is nil or user_info_list is nil aiUserID" << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CREATE_RS::ENUM_TYPE, 
							MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR))
		return 0;
	}

	auto mapGroupTypeInfo = mpServerMgr->moConfig.mapGroupTypeInfo;
	INT group_type = t_fb_data->group_type();
	if(!mapGroupTypeInfo.count(group_type))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "group_typet is invalid group_type = " << t_fb_data->group_type());
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CREATE_RS::ENUM_TYPE, 
							MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR))
		return 0;
	}

	INT group_max_count = mapGroupTypeInfo[t_fb_data->group_type()].group_max_count;
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "group_max_count = " << group_max_count);
	if(group_max_count <= 0)
	{
		group_max_count = 200;
	}
	INT group_add_max_count = mapGroupTypeInfo[t_fb_data->group_type()].group_add_max_count;

	GENERATE_LOGIC_TO_DBSERVER_CMD()
	JsonParse::add(l_document, "pack_session_id", t_fb_data->s_rq_head()->pack_session_id(), l_allocator);
	RJ_CREATE_ARRAY(child_array)
	for(INT index = 0; index < user_info_list->size(); index++)
	{
		RJ_CREATE_OBJECT(child_obj2)
		auto user_base_info = user_info_list->Get(index);

		if(!user_base_info->user_nick_name())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " user_nick_name is nil aiUserID" << aiUserID);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CREATE_RS::ENUM_TYPE,
				MAKE_ERROR_RESULT(RET_CREATE_USER_LIST_EMPTY))
			return 1;
		}

		JsonParse::add(child_obj2, CGroupUserBaseInfo::GetUserId(), to_string(user_base_info->user_id()), l_allocator);
    	string user_nick_name = user_base_info->user_nick_name()->c_str();
		JsonParse::add(child_obj2, CGroupUserBaseInfo::GetUserNickName(), user_nick_name, l_allocator);
    	
		JsonParse::push(child_array, child_obj2, l_allocator);
	}

	RJ_CREATE_ARRAY(child_array_save)
	for(INT index = 0; index < user_info_list->size(); index++)
	{
		RJ_CREATE_OBJECT(child_obj2)
		auto user_base_info = user_info_list->Get(index);

		if(!user_base_info->user_nick_name())
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " user_nick_name is nil aiUserID" << aiUserID);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CREATE_RS::ENUM_TYPE,
				MAKE_ERROR_RESULT(RET_CREATE_USER_LIST_EMPTY))
			return 1;
		}

		JsonParse::add(child_obj2, CGroupUserBaseInfo::GetUserId(), to_string(user_base_info->user_id()), l_allocator);
    	string user_nick_name = user_base_info->user_nick_name()->c_str();
		JsonParse::add(child_obj2, CGroupUserBaseInfo::GetUserNickName(), user_nick_name, l_allocator);
    	
		JsonParse::push(child_array_save, child_obj2, l_allocator);
	}

	string group_name = "";
	if(t_fb_data->group_name())
	{
		group_name = t_fb_data->group_name()->c_str();
		JsonParse::add(l_document, GroupDetailInfo::GetGroupName(), group_name, l_allocator);
	}

	if(t_fb_data->group_remark())
	{
		JsonParse::add(l_document, GroupDetailInfo::GetGroupRemark(), (string)t_fb_data->group_remark()->c_str(), l_allocator);
	}

	JsonParse::add(l_document, GroupDetailInfo::GetGroupCT(), to_string(t_fb_data->group_ct()), l_allocator);
	JsonParse::add(l_document, CGroupModifyMember::GetChangeUserList(), child_array, l_allocator);
	JsonParse::add<int>(l_document, "group_max_count", group_max_count, l_allocator);
	JsonParse::add<int>(l_document, "group_add_max_count", group_add_max_count, l_allocator);
	
	std::string operate_name = operate_user_name->c_str();
	std::string szdata = "";
	JsonParse::to_string(l_document, szdata);
	CGroupCreateRQ *task_data = new CGroupCreateRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
    if (NULL == task_data)
    {
        LOG_TRACE(LOG_INFO, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
        return -1;
    }
	
    task_data->SetData(szdata);
    task_data->SetClientSide(this);
	
	string str_list = "";
	JsonParse::to_string(child_array_save, str_list);
	task_data->SetArgvs(group_name, (INT)GROUP_OFFLINE_CREATE_GROUP, 
				operate_group_msg->msg_time(), operate_name,
				offline_group_msg->message_id(), 
				offline_group_msg->message_old_id(), str_list,
				group_max_count, group_add_max_count);

	m_taskthreadpoll.AddTask(task_data);
}

INT CClientSide::GroupDetailInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_DETAIL_INFO_RQ, aiUserID, STRU_GROUP_DETAIL_INFO_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	INT group_member_index = t_fb_data->group_member_index();
	
	GENERATE_LOGIC_TO_DBSERVER_CMD()

	JsonParse::add(l_document, GroupDetailInfo::GetGroupId(), std::to_string(t_fb_data->group_id()), l_allocator);
	JsonParse::add<unsigned>(l_document, CGroupUserBaseInfo::GetGroupMemberIndex(), (unsigned)group_member_index, l_allocator);
	JsonParse::add<unsigned>(l_document, CGroupUserBaseInfo::GetGroupMemberPageNumber(), (unsigned)DEF_GROUP_USER_INFO_LIST, l_allocator);

	std::string szdata = "";
	JsonParse::to_string(l_document, szdata);
	CGroupDetailInfoRQ *task_data = new CGroupDetailInfoRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
    if (NULL == task_data)
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID ＝ " << aiUserID);
        return -1;
    }
	
    task_data->SetData(szdata);
    task_data->SetClientSide(this);
	task_data->SetArgvs(t_fb_data->group_id(), group_member_index, (INT)DEF_GROUP_USER_INFO_LIST);

	m_taskthreadpoll.AddTask(task_data);
}

INT CClientSide::GroupLeaderChangeRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_LEADER_CHANGE_RQ, aiUserID, STRU_GROUP_LEADER_CHANGE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	UINT64 group_id = t_fb_data->group_id();
	auto offline_group_msg = t_fb_data->offline_group_msg();
	if(!offline_group_msg || !offline_group_msg->operate_group_msg()
		|| !offline_group_msg->operate_group_msg()->operate_user_name()
		|| !offline_group_msg->message_id())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "operate_group_msg is nil or message_id is nil aiUserID = " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_LEADER_CHANGE_RS::ENUM_TYPE, 
						MAKE_ERROR_RESULT(RET_GROUP_LEADER_NAME_IS_NIL))
		return 0;
	}

	string strMsgId = to_string(offline_group_msg->message_id());
	if(strMsgId.length() != 16)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "message_id = " + strMsgId + " is invalid");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_LEADER_CHANGE_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_GROUP_MESSAGE_ID_INVALID))
		return 0;
	}

	auto operate_group_msg = offline_group_msg->operate_group_msg();
	auto operate_user_name = operate_group_msg->operate_user_name();
	auto user_info_list = operate_group_msg->user_info_list();
	if(!user_info_list || user_info_list->size() != 1)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "user_info_list is nil aiUserID = " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_LEADER_CHANGE_RS::ENUM_TYPE, 
							MAKE_ERROR_RESULT(RET_GROUP_OPREATE_USER_ID_INVALID))
		return 0;
	}
	
	USERID op_user_id = user_info_list->Get(0)->user_id();
	auto operated_user_name = user_info_list->Get(0)->user_nick_name();
	if(!op_user_id || !operated_user_name)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "operated_user_name or op_user_id is nil aiUserId = " << (USERID)aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_LEADER_CHANGE_RS::ENUM_TYPE, 
							MAKE_ERROR_RESULT(RET_GROUP_OPREATE_USER_ID_INVALID))
		return 0;
	}

	GENERATE_LOGIC_TO_DBSERVER_CMD()

	JsonParse::add(l_document, CGroupModifyMember::GetGroupId(), std::to_string(group_id), l_allocator);
	JsonParse::add(l_document, CGroupModifyMember::GetOperateUserName(), (string)operate_user_name->c_str(), l_allocator);
	JsonParse::add(l_document, CGroupModifyMember::GetMsgTime(), to_string(operate_group_msg->msg_time()), l_allocator);
	JsonParse::add(l_document, CGroupModifyMember::GetMessageId(), to_string(offline_group_msg->message_id()), l_allocator);
	JsonParse::add(l_document, CGroupModifyMember::GetMessageOldId(), to_string(offline_group_msg->message_old_id()), l_allocator);
	
	RJ_CREATE_ARRAY(child_array)
	RJ_CREATE_OBJECT(child_obj2)
	JsonParse::add(child_obj2, CGroupUserBaseInfo::GetUserId(), to_string(op_user_id), l_allocator);
    JsonParse::add(child_obj2, CGroupUserBaseInfo::GetUserNickName(), (string)operated_user_name->c_str(), l_allocator);
    JsonParse::push(child_array, child_obj2, l_allocator);
	JsonParse::add(l_document, CGroupModifyMember::GetChangeUserList(), child_array, l_allocator);
	
	GENERATE_LOGIC_TO_DBSERVER_END(CGroupLeaderChangeRQ)
}

INT CClientSide::GroupScanRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_SCAN_RQ, aiUserID, STRU_GROUP_SCAN_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	LONG is_exist = 0;
	UINT64 group_id = t_fb_data->group_id();
	UINT64 user_id_share = t_fb_data->user_id_share();
	mpServerMgr->mpRedisUtil->HexistGroupUserId(group_id, user_id_share, 
											is_exist);
	if(is_exist <= 0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "aiUserID = " + to_string(t_fb_data->user_id_share()) + " group_id = "
					<< to_string(t_fb_data->group_id()) + " is not in group");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_SCAN_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_GROUP_USER_NOT_JOIN))
		return 0;
	}

	GENERATE_LOGIC_TO_DBSERVER_CMD()
	JsonParse::add(l_document, CGroupModifyMember::GetGroupId(), std::to_string(group_id), l_allocator);


	std::string szdata = "";
	JsonParse::to_string(l_document, szdata);
	CGroupScanRQ *task_data = new CGroupScanRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
    if (NULL == task_data)
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID ＝ " << aiUserID);
        return -1;
    }
	
    task_data->SetData(szdata);
    task_data->SetClientSide(this);
	task_data->SetArgvs(user_id_share);

	m_taskthreadpoll.AddTask(task_data);
}

INT CClientSide::GroupModifyChangeRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_MODIFY_ChANGE_RQ, aiUserID, STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	auto offline_group_msg = t_fb_data->offline_group_msg();
	if(!offline_group_msg || !offline_group_msg->message_id() || !offline_group_msg->operate_group_msg())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "offline_group_msg or operate_group_msg or message_id is nil aiUserID = " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE,
				MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR))
		return 0;
	}

	string strMsgId = to_string(offline_group_msg->message_id());
	if(strMsgId.length() != 16)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "message_id = " + strMsgId + " is invalid");
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_GROUP_MESSAGE_ID_INVALID))
		return 0;
	}

	auto operate_group_msg = offline_group_msg->operate_group_msg();
	auto operate_user_name = operate_group_msg->operate_user_name();
	auto group_modify_content = operate_group_msg->group_modify_content();

	if(!operate_user_name)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "operate_user_name is nil aiUserID = " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE,
				MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR))
		return 0;
	}

	UINT64 group_id = t_fb_data->group_id();
	if(!group_id)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "group_id is invalid aiUserID << " << aiUserID);
		TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE, 
				MAKE_ERROR_RESULT(RET_GROUP_ID_INVALID))
		return 0;
	}

	INT big_msg_type = offline_group_msg->big_msg_type();
	if(!group_modify_content)
	{
		if(big_msg_type == GROUP_OFFLINE_CHAT_MODIFY_GROUP_NAME || 
			big_msg_type == GROUP_OFFLINE_CHAT_MODIFY_GROUP_REMARK || 
			big_msg_type == GROUP_OFFLINE_CHAT_MODIFY_GROUP_USER_NAME)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "group_modify_content is nil aiUserID = " << aiUserID << " big_msg_type = " << big_msg_type);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR))
			return 0;
		}
	}

	GENERATE_LOGIC_TO_DBSERVER_CMD()
	JsonParse::add(l_document, "pack_session_id", t_fb_data->s_rq_head()->pack_session_id(), l_allocator);
	JsonParse::add(l_document, CGroupModifyMember::GetGroupId(), std::to_string(group_id), l_allocator);
	
	if(big_msg_type == GROUP_OFFLINE_CHAT_KICK_USER || 
		big_msg_type == GROUP_OFFLINE_CHAT_ADD_USER || 
		big_msg_type == GROUP_OFFLINE_CHAT_SCAN_ADD_USER)
	{
		auto user_info_list = operate_group_msg->user_info_list();
		if(!user_info_list || user_info_list->size() <= 0)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "user_info_list is nil aiUserID = " << aiUserID);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_CREATE_USER_LIST_EMPTY))
			return 1;
		}
		
		RJ_CREATE_ARRAY(child_array)
		RJ_CREATE_ARRAY(child_array_save)
		
		for(INT index = 0; index < user_info_list->size(); index++)
		{
			auto user_base_info = user_info_list->Get(index);
			if(!user_base_info->user_nick_name())
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, "user_nick_name is nil aiUserID = " << aiUserID);
				TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_CREATE_USER_LIST_EMPTY))
				return 1;
			}

			RJ_CREATE_OBJECT(child_obj2)
			JsonParse::add(child_obj2, CGroupUserBaseInfo::GetUserId(), to_string(user_base_info->user_id()), l_allocator);
			JsonParse::add(child_obj2, CGroupUserBaseInfo::GetUserNickName(), (string)user_base_info->user_nick_name()->c_str(), l_allocator);
			JsonParse::push(child_array, child_obj2, l_allocator);
		}

		for(INT index = 0; index < user_info_list->size(); index++)
		{
			auto user_base_info = user_info_list->Get(index);
			if(!user_base_info->user_nick_name())
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, "user_nick_name is nil aiUserID = " << aiUserID);
				TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_CREATE_USER_LIST_EMPTY))
				return 1;
			}

			RJ_CREATE_OBJECT(child_obj2)
			JsonParse::add(child_obj2, CGroupUserBaseInfo::GetUserId(), to_string(user_base_info->user_id()), l_allocator);
			JsonParse::add(child_obj2, CGroupUserBaseInfo::GetUserNickName(), (string)user_base_info->user_nick_name()->c_str(), l_allocator);
			JsonParse::push(child_array_save, child_obj2, l_allocator);
		}
		
		JsonParse::add(l_document, CGroupModifyMember::GetChangeUserList(), child_array, l_allocator);

		if(big_msg_type == GROUP_OFFLINE_CHAT_KICK_USER)
		{
			std::string szdata = "";
			JsonParse::to_string(l_document, szdata);
			CGroupKickUserRQ *task_data = new CGroupKickUserRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
			if (NULL == task_data)
			{
				LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
				return -1;
			}
			
			task_data->SetData(szdata);
			task_data->SetClientSide(this);
			
			string str_list = "";
			JsonParse::to_string(child_array_save, str_list);
			string operate_name = operate_user_name->c_str();
			task_data->SetArgvs(group_id, big_msg_type, 
						operate_group_msg->msg_time(), operate_name,
						offline_group_msg->message_id(), offline_group_msg->message_old_id(), str_list);

			m_taskthreadpoll.AddTask(task_data);
		}
		else
		{
			std::string szdata = "";
			JsonParse::to_string(l_document, szdata);
			CGroupAddUserRQ *task_data = new CGroupAddUserRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
			if (NULL == task_data)
			{
				LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
				return -1;
			}
			
			task_data->SetData(szdata);
			task_data->SetClientSide(this);
			
			string operate_name = operate_user_name->c_str();
			string str_list = "";
			string str_reason = "";
			if(group_modify_content)
			{
				str_reason = group_modify_content->c_str();
			} 

			LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "str_reason = " << str_reason
				<< " offline_group_msg->user_id() = " << offline_group_msg->user_id());
		
			JsonParse::to_string(child_array_save, str_list);
			task_data->SetArgvs(group_id, big_msg_type, 
						operate_group_msg->msg_time(), operate_name,
						offline_group_msg->message_id(), 
						offline_group_msg->message_old_id(), 
						str_list,
						str_reason, offline_group_msg->user_id());

			m_taskthreadpoll.AddTask(task_data);
		}


		return 1;
	}
	
	
	if(big_msg_type == GROUP_OFFLINE_CHAT_MODIFY_GROUP_NAME || 
		big_msg_type == GROUP_OFFLINE_CHAT_MODIFY_GROUP_REMARK || 
		big_msg_type == GROUP_OFFLINE_CHAT_MODIFY_GROUP_USER_NAME)
	{
		JsonParse::add(l_document, CGroupModifyMember::GetGroupModifyContent(), (string)group_modify_content->c_str(), l_allocator);
	}

	if(big_msg_type == GROUP_OFFLINE_CHAT_MODIFY_GROUP_REMARK)
	{
		std::string szdata = "";
		JsonParse::to_string(l_document, szdata);
		CGroupChangeRemarkRQ *task_data = new CGroupChangeRemarkRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
		if (NULL == task_data)
		{
			LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
			return -1;
		}
			
		task_data->SetData(szdata);
		task_data->SetClientSide(this);
			
		string operate_name = operate_user_name->c_str();
		string str_list = group_modify_content->c_str();
		
		task_data->SetArgvs(group_id, operate_group_msg->msg_time(), operate_name,
						offline_group_msg->message_id(), offline_group_msg->message_old_id(), str_list);

		m_taskthreadpoll.AddTask(task_data);
			
		return 1;
	}

	if(big_msg_type == GROUP_OFFLINE_CHAT_MODIFY_GROUP_NAME)
	{
		std::string szdata = "";
		JsonParse::to_string(l_document, szdata);
		CGroupChangeGroupNameRQ *task_data = new CGroupChangeGroupNameRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
		if (NULL == task_data)
		{
			LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
			return -1;
		}
			
		task_data->SetData(szdata);
		task_data->SetClientSide(this);
			
		string operate_name = operate_user_name->c_str();
		string str_list = group_modify_content->c_str();
		
		task_data->SetArgvs(group_id, operate_group_msg->msg_time(), operate_name,
						offline_group_msg->message_id(), offline_group_msg->message_old_id(), str_list);

		m_taskthreadpoll.AddTask(task_data);
			
		return 1;
	}
	
	if(big_msg_type == GROUP_OFFLINE_CHAT_MODIFY_GROUP_USER_NAME)
	{
		std::string szdata = "";
		JsonParse::to_string(l_document, szdata);
		CGroupChangeNickNameRQ *task_data = new CGroupChangeNickNameRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
		if (NULL == task_data)
		{
			LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
			return -1;
		}
			
		task_data->SetData(szdata);
		task_data->SetClientSide(this);
			
		string operate_name = operate_user_name->c_str();
		string str_list = group_modify_content->c_str();
		
		task_data->SetArgvs(group_id, operate_group_msg->msg_time(), operate_name,
						offline_group_msg->message_id(), offline_group_msg->message_old_id(), str_list);

		m_taskthreadpoll.AddTask(task_data);
			
		return 1;
	}

	if(big_msg_type == GROUP_OFFLINE_CHAT_ENTER_AGREE)
	{
		std::string szdata = "";
		JsonParse::to_string(l_document, szdata);
		CGroupAddUserAgreeRQ *task_data = new CGroupAddUserAgreeRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
		if (NULL == task_data)
		{
			LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
			return -1;
		}
			
		task_data->SetData(szdata);
		task_data->SetClientSide(this);
			
		string operate_name = operate_user_name->c_str();
		
		task_data->SetArgvs(group_id, operate_group_msg->msg_time(), operate_name,
						offline_group_msg->message_id(), offline_group_msg->message_old_id());

		m_taskthreadpoll.AddTask(task_data);
			
		return 1;
	}
	else if(big_msg_type == GROUP_OFFLINE_CHAT_ENTER_DEFAULT)
	{
		std::string szdata = "";
		JsonParse::to_string(l_document, szdata);
		CGroupAddUserDefaultRQ *task_data = new CGroupAddUserDefaultRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
		if (NULL == task_data)
		{
			LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
			return -1;
		}
			
		task_data->SetData(szdata);
		task_data->SetClientSide(this);
			
		string operate_name = operate_user_name->c_str();
		task_data->SetArgvs(group_id, operate_group_msg->msg_time(), operate_name,
						offline_group_msg->message_id(), offline_group_msg->message_old_id());

		m_taskthreadpoll.AddTask(task_data);
			
		return 1;
	}

	TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_CREATE_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR))
	LOG_TRACE(LOG_ERR, false, __FUNCTION__, " big_msg_type is invalid big_msg_type = " << big_msg_type);
	return 0;
}

INT CClientSide::GroupRemarkDetailRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_REMARK_DETAIL_RQ, aiUserID, STRU_GROUP_REMARK_DETAIL_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	TEXTMSGID group_id = t_fb_data->group_id();

	GENERATE_LOGIC_TO_DBSERVER_CMD()
	JsonParse::add<uint64_t>(l_document, CGroupModifyMember::GetGroupId(), group_id, l_allocator);
	
	GENERATE_LOGIC_TO_DBSERVER_END(CGroupRemarkDetailRQ)
}

INT CClientSide::GroupMessageStatusRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_MESSAGE_STATUS_RQ, aiUserID, STRU_GROUP_MESSAGE_STATUS_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	BYTE message_status = t_fb_data->message_status();
	if(message_status != GROUP_MESSAGE_STATUS_NORMAL && 
		message_status != GROUP_MESSAGE_STATUS_NO_HIT && 
		message_status != GROUP_MESSAGE_IN_HELP_NO_HIT)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " message_status = " << message_status << " aiUserID = " << aiUserID);
			TEMPLATE_GENERAL_FAIL_RESULT(STRU_GROUP_MESSAGE_STATUS_RS::ENUM_TYPE,
					MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR))
			return 0;
		}

	GENERATE_LOGIC_TO_DBSERVER_CMD()
	JsonParse::add<ULONG>(l_document, CGroupModifyMember::GetGroupId(), t_fb_data->group_id(), l_allocator);
	JsonParse::add<BYTE>(l_document, "message_status", t_fb_data->message_status(), l_allocator);

	std::string szdata = "";
	JsonParse::to_string(l_document, szdata);
	CGroupMessageStatusRQ *task_data = new CGroupMessageStatusRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
	if (NULL == task_data)
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
		return -1;
	}
			
	task_data->SetData(szdata);
	task_data->SetClientSide(this);
	task_data->SetArgvs(t_fb_data->group_id(), t_fb_data->message_status());

	m_taskthreadpoll.AddTask(task_data);
	return 1;
}

INT CClientSide::GroupSaveChangeRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_SAVE_CHANGE_RQ, aiUserID, STRU_GROUP_SAVE_CHANGE_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	GENERATE_LOGIC_TO_DBSERVER_CMD()
	JsonParse::add<ULONG>(l_document, CGroupModifyMember::GetGroupId(), t_fb_data->group_id(), l_allocator);
	JsonParse::add<BYTE>(l_document, "save_type", t_fb_data->save_type(), l_allocator);

	std::string szdata = "";
	JsonParse::to_string(l_document, szdata);
	CGroupSaveChangeRQ *task_data = new CGroupSaveChangeRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
	if (NULL == task_data)
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
		return -1;
	}
			
	task_data->SetData(szdata);
	task_data->SetClientSide(this);
	task_data->SetArgvs(t_fb_data->group_id(), t_fb_data->save_type());

	m_taskthreadpoll.AddTask(task_data);
	return 1;
}

INT CClientSide::GroupTypeListRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_TYPE_LIST_RQ, aiUserID, STRU_GROUP_TYPE_LIST_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	STRU_GROUP_TYPE_LIST_RS loSendPack;
	std::vector<flatbuffers::Offset<T_GROUP_TYPE_INFO> > vector_group_type_info;
	auto mapGroupTypeInfo = mpServerMgr->moConfig.mapGroupTypeInfo;
	auto iter = mapGroupTypeInfo.begin();
	for(iter; iter != mapGroupTypeInfo.end(); iter++)
	{
		auto group_type_ini_info = iter->second;
		auto group_type_info = CreateT_GROUP_TYPE_INFO(loSendPack.fbbuilder, 
					group_type_ini_info.group_max_count,
					group_type_ini_info.group_type,
					group_type_ini_info.group_is_show,
					group_type_ini_info.group_add_max_count);

		vector_group_type_info.push_back(group_type_info);
	}

	loSendPack.fbbuf = CreateT_GROUP_TYPE_LIST_RS(loSendPack.fbbuilder,
									&s_rs_head,
									loSendPack.fbbuilder.CreateVector(vector_group_type_info));

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
}

INT CClientSide::GroupGetUserRemarkNameRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	TEMPLATE_GENERAL_UNPACK_BODY_AND_CHECK_UID(STRU_GROUP_GET_USER_REMARK_NAME_RQ, aiUserID, STRU_GROUP_GET_USER_REMARK_NAME_RS::ENUM_TYPE)
	GENERAL_PACK_SAME_BEGIN()

	GENERATE_LOGIC_TO_DBSERVER_CMD()
	JsonParse::add<ULONG>(l_document, CGroupModifyMember::GetGroupId(), t_fb_data->group_id(), l_allocator);

	std::string szdata = "";
	JsonParse::to_string(l_document, szdata);
	CGroupGetUserRemarkNameRQ *task_data = new CGroupGetUserRemarkNameRQ(aiCometID, aiUserID, t_fb_data->s_rq_head()->pack_session_id(), t_fb_data->s_rq_head()->platform());
	if (NULL == task_data)
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "new task_data failed! aiUserID = " << aiUserID);
		return -1;
	}
			
	task_data->SetData(szdata);
	task_data->SetClientSide(this);
	task_data->SetArgvs(t_fb_data->group_id());

	m_taskthreadpoll.AddTask(task_data);
	return 1;
}

/////////////////////////////////// http begin ////////////////////////////////////
INT CClientSide::OnGroupScanRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform, UINT64 user_id_share)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_SCAN_RS::ENUM_TYPE)

	UINT64 group_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupId());

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "group_id = " << (UINT64)group_id);

	INT is_exist = 0;
	JsonParse::get<int>(jsondata, GroupDetailInfo::GetIsMember(), is_exist);

	INT group_max_count = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupMaxCount(), group_max_count);

	INT group_add_max_count = 0;
	JsonParse::get<int>(jsondata, "group_add_max_count", group_add_max_count);

	str_val = "";
	JsonParse::get(jsondata, CGroupModifyMember::GetGroupCT(), str_val);
	UINT64 group_ct = ServerUtilMgr::STR2UINT64(str_val);

	string group_remark = "";
	JsonParse::get(jsondata, GroupDetailInfo::GetGroupRemark(), group_remark);

	string group_name = "";
	JsonParse::get(jsondata, GroupDetailInfo::GetGroupName(), group_name);

	INT group_count = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupCount(), group_count);

	UINT64 group_manager_user_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupManagerUserId());
	
	INT group_add_is_agree = 0;
	JsonParse::get<int>(jsondata, GroupDetailInfo::GetGroupAddIsAgree(), group_add_is_agree);

	BYTE message_status = 0;
	JsonParse::get<BYTE>(jsondata, "message_status", message_status);

	STRU_GROUP_SCAN_RS loSendPack;
	auto group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
			loSendPack.fbbuilder, group_id, 
			loSendPack.fbbuilder.CreateString(group_name), 
			0, group_count, 
			group_manager_user_id,
			group_add_is_agree,
			group_ct, loSendPack.fbbuilder.CreateString(group_remark), 
			group_max_count,
			message_status, 
			group_add_max_count);

	loSendPack.fbbuf = grouppack::CreateT_GROUP_SCAN_RS(loSendPack.fbbuilder,
			&s_rs_head, 
			user_id_share,
			group_base_info, is_exist);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	return 1;
}

INT CClientSide::OnGroupDetailInfoRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 group_id, INT group_member_index,
								INT nSpan)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_DETAIL_INFO_RS::ENUM_TYPE)

	STRU_GROUP_DETAIL_INFO_RS loSendPack;
	std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> list_members_info;

	RJsonValue& memerList = JsonParse::get(jsondata, CGroupUserBaseInfo::GetMemberList());
	INT list_count = JsonParse::count(memerList);
	group_member_index = group_member_index + list_count;
	if(list_count < nSpan)
	{
		group_member_index = -1;
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " size = " << JsonParse::count(memerList));
	for(INT index = 0; index < JsonParse::count(memerList); index++)
	{
		RJsonValue& user_base_info = JsonParse::at(memerList, index);

		ULONG user_id = 0;
		JsonParse::get<ULONG>(user_base_info, CGroupUserBaseInfo::GetUserId(), user_id);

		string user_nick_name = "";
		JsonParse::get(user_base_info, CGroupUserBaseInfo::GetUserNickName(), user_nick_name);

		INT user_group_index = 0;
		JsonParse::get<int>(user_base_info, CGroupUserBaseInfo::GetUserGroupIndex(), user_group_index);
	
		auto user_base_info_fbs = commonpack::CreateUSER_BASE_INFO(loSendPack.fbbuilder, 
						user_id, loSendPack.fbbuilder.CreateString(user_nick_name), user_group_index);
						
		list_members_info.push_back(user_base_info_fbs);
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " group_member_index = " << group_member_index << " nSpan = " << nSpan
			<< " list_count = " << list_count);

	loSendPack.fbbuf = grouppack::CreateT_GROUP_DETAIL_INFO_RS(loSendPack.fbbuilder,
									&s_rs_head,
									loSendPack.fbbuilder.CreateVector(list_members_info), 
									group_id, 
									group_member_index);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}


INT CClientSide::OnGroupListRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								INT group_list_index, INT nSpan)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_LIST_RS::ENUM_TYPE)
	
	STRU_GROUP_LIST_RS loSendPack;
	std::vector<flatbuffers::Offset<grouppack::T_GROUP_BASE_INFO> > vector_list_group_info;

	RJsonValue& group_list = JsonParse::get(jsondata, GroupDetailInfo::GetGroupList());
	INT list_count = JsonParse::count(group_list);
	group_list_index = group_list_index + list_count;
	
	if(list_count < nSpan)
	{
		group_list_index = -1;
	}

	for(INT index = 0; index < list_count; index++)
	{
		RJsonValue& group_info = JsonParse::at(group_list, index);

		UINT64 group_id = GetJsonID(group_info, CGroupModifyMember::GetGroupId());

		string group_name = "";
		JsonParse::get(group_info, GroupDetailInfo::GetGroupName(), group_name);

		INT group_count = 0;
		JsonParse::get<int>(group_info, GroupDetailInfo::GetGroupCount(), group_count);

		UINT64 group_manager_user_id = GetJsonID(group_info, CGroupModifyMember::GetGroupManagerUserId());
		
		INT group_add_is_agree = 0;
		JsonParse::get<int>(group_info, GroupDetailInfo::GetGroupAddIsAgree(), group_add_is_agree);
	
		ULONG group_ct = 0;
		JsonParse::get<ULONG>(group_info, GroupDetailInfo::GetGroupCT(), group_ct);

		string group_remark = "";
		JsonParse::get(group_info, GroupDetailInfo::GetGroupRemark(), group_remark);
	
		INT group_max_count = 0;
		JsonParse::get<int>(group_info, CGroupModifyMember::GetGroupMaxCount(), group_max_count);

		INT group_add_max_count = 0;
		JsonParse::get<int>(group_info, "group_add_max_count", group_add_max_count);

		BYTE message_status = 0;
		JsonParse::get<BYTE>(group_info, "message_status", message_status);

		auto group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
									loSendPack.fbbuilder, 
									group_id, loSendPack.fbbuilder.CreateString(group_name), 
									0, group_count,
									group_manager_user_id,
									group_add_is_agree,
									group_ct,
									loSendPack.fbbuilder.CreateString(group_remark),
									group_max_count,
									message_status,
									group_add_max_count);

		vector_list_group_info.push_back(group_base_info);
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "vector_list_group_info = " << vector_list_group_info.size());
	
	auto vector_msg_builder = loSendPack.fbbuilder.CreateVector(vector_list_group_info);
	loSendPack.fbbuf = grouppack::CreateT_GROUP_LIST_RS(loSendPack.fbbuilder,
									&s_rs_head, 
									vector_msg_builder, group_list_index);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}

INT CClientSide::OnGroupListIdsRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								INT group_list_index, INT nSpan)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_LIST_IDS_RS::ENUM_TYPE)

	STRU_GROUP_LIST_IDS_RS loSendPack;
	std::vector<flatbuffers::Offset<T_GROUP_RELATION_USER_INFO>> vector_list_ids;

	if(!!jsondata.IsArray())
	{
		INT list_count = JsonParse::count(jsondata);
		group_list_index = group_list_index + list_count;
		
		if(list_count < nSpan)
		{
			group_list_index = -1;
		}
		
		if(list_count > nSpan)
		{
			list_count = nSpan;
		}

		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "list_count = " << list_count);

		for(INT index = 0; index < list_count; index++)
		{
			RJsonValue& group_relation_user_info = JsonParse::at(jsondata, index);

			ULONG group_id = 0;
			JsonParse::get<ULONG>(group_relation_user_info, GroupDetailInfo::GetGroupId(), group_id);
			
			INT message_status = 0;
			JsonParse::get<int>(group_relation_user_info, "rcv_type", message_status);

			INT save_type = 0;
			JsonParse::get<int>(group_relation_user_info, "save_type", save_type);
	
			auto group_relation_info = CreateT_GROUP_RELATION_USER_INFO(loSendPack.fbbuilder, 
					group_id, message_status, save_type);
			LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "group_id = " << group_id << " save_type = " << save_type
						<< " message_status = " << message_status);
			vector_list_ids.push_back(group_relation_info);
		}
	}
	else
	{
		group_list_index = -1;
	}
	
	auto vector_msg_builder = loSendPack.fbbuilder.CreateVector(vector_list_ids);
	loSendPack.fbbuf = grouppack::CreateT_GROUP_LIST_IDS_RS(loSendPack.fbbuilder,
									&s_rs_head, 
									vector_msg_builder,
									group_list_index);

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "vector_list_ids = " << vector_list_ids.size()
									<< " fbbuilder_offline.GetSize() = " << loSendPack.fbbuilder.GetSize());

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}

INT CClientSide::OnGroupLeaderChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_LEADER_CHANGE_RS::ENUM_TYPE)

	RJsonValue& member_change_list = JsonParse::get(jsondata, CGroupModifyMember::GetChangeUserList());
	STRU_GROUP_LEADER_CHANGE_RS loSendPack;
	std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> vector_user_info;
	CGroupDistributeMgr::GetInstance().GenerateUserListByJsonValue(member_change_list, vector_user_info, loSendPack.fbbuilder);

	str_val = "";
	JsonParse::get(jsondata, CGroupModifyMember::GetMsgTime(), str_val);
	UINT64 msg_time = ServerUtilMgr::STR2UINT64(str_val);
	
	string operate_user_name = "";
	JsonParse::get(jsondata, CGroupModifyMember::GetOperateUserName(), operate_user_name);
	
	str_val = "";
	JsonParse::get(jsondata, CGroupModifyMember::GetMessageId(), str_val);
	UINT64 message_id = ServerUtilMgr::STR2UINT64(str_val);

	str_val = "";
	JsonParse::get(jsondata, CGroupModifyMember::GetMessageOldId(), str_val);
	UINT64 message_old_id = ServerUtilMgr::STR2UINT64(str_val);
	UINT64 group_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupId());
	UINT64 group_manager_user_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupManagerUserId());
	
	INT group_count = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupCount(), group_count);

	auto operate_group_msg = grouppack::CreateT_OPERATE_GROUP_MSG(loSendPack.fbbuilder,
							msg_time, loSendPack.fbbuilder.CreateString(operate_user_name),
							loSendPack.fbbuilder.CreateVector(vector_user_info));
	
	auto offline_group_msg = grouppack::CreateT_OFFLINE_GROUP_MSG(loSendPack.fbbuilder,
							aiUserID, message_id, 0, GROUP_OFFLINE_CHAT_LEADER_CHANGE, operate_group_msg);
	
	loSendPack.fbbuf = grouppack::CreateT_GROUP_LEADER_CHANGE_RS(loSendPack.fbbuilder,
									&s_rs_head, group_id, offline_group_msg);
	
	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	// end 

	string group_name = "";
	INT big_msg_type = GROUP_OFFLINE_CHAT_LEADER_CHANGE;
	string group_modify_content = "";
	INT group_max_count = 0;
	INT group_add_max_count = 0;
	CGroupDistributeMgr::GetInstance().SendGroupMsgId(mpServerMgr, aiUserID, group_name, group_id);
	// end 

	// 添加离线
	CGroupDistributeMgr::GetInstance().GroupUserChangeMemberChangeOffline(
									mpServerMgr, group_id,
									big_msg_type, 
									aiUserID, operate_user_name, msg_time, 
									message_id, message_old_id, member_change_list,
									group_name, group_count, group_manager_user_id, 
									group_max_count, group_add_max_count, group_modify_content, aiUserID);
	// end 
}

INT CClientSide::OnGroupRemarkDetailRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_REMARK_DETAIL_RS::ENUM_TYPE)
	
	uint64_t group_id = 0;
	JsonParse::get<uint64_t>(jsondata, CGroupModifyMember::GetGroupId(), group_id);
	
	uint64_t op_user_id = 0;
	JsonParse::get<uint64_t>(jsondata, "update_user", op_user_id);
	
	uint64_t op_ct = 0;
	JsonParse::get<uint64_t>(jsondata, "update_date", op_ct);

	std::string op_remark = "";
	JsonParse::get(jsondata, "group_remark", op_remark);

	STRU_GROUP_REMARK_DETAIL_RS loSendPack;
	loSendPack.fbbuf = grouppack::CreateT_GROUP_REMARK_DETAIL_RS(loSendPack.fbbuilder,
									&s_rs_head, group_id, 
									op_user_id, op_ct, 
									loSendPack.fbbuilder.CreateString(op_remark));

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}

INT CClientSide::HandleFailedResultFromHttp(WORD pack_type, SESSIONID aiCometID, USERID aiUserID, 
										INT aiPackSessionID, BYTE aiPlatform)
{
	CUser* lpUser = mpServerMgr->moUserConMgr.getUser(aiCometID, aiUserID);
	if(!lpUser)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << aiUserID);
		return 1;
	}

	TEMPLATE_GENERAL_DB_FAIL_RESULT(pack_type,
				MAKE_ERROR_RESULT(RET_OPERATE_TYPE_ERROR), aiPackSessionID, aiPlatform);

	return 0;
}


INT CClientSide::OnGroupAddUserChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 msg_time,
								string& operate_user_name, UINT64 message_id,
								UINT64 message_old_id, RJsonValue& member_change_list,
								string& group_modify_content,
								UINT64 user_id_initial, INT big_msg_type)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)

	UINT64 group_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupId());	
	UINT64 group_manager_user_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupManagerUserId());
	
	INT group_count = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupCount(), group_count);

	INT group_add_is_agree = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupAddIsAgree(), group_add_is_agree);

	str_val = "";
	JsonParse::get(jsondata, CGroupModifyMember::GetGroupCT(), str_val);
	UINT64 group_ct = ServerUtilMgr::STR2UINT64(str_val);

	string group_remark = "";
	JsonParse::get(jsondata, GroupDetailInfo::GetGroupRemark(), group_remark);
	
	string group_name = "";
	JsonParse::get(jsondata, GroupDetailInfo::GetGroupName(), group_name);
	
	INT group_max_count = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupMaxCount(), group_max_count);

	INT group_add_max_count = 0;
	JsonParse::get<int>(jsondata, "group_add_max_count", group_add_max_count);

	RJsonValue& duplicate_member_ids = JsonParse::get(jsondata, "duplicate_member_ids");
	std::vector<UINT64> vecotr_duplicate_ids;
	if(!duplicate_member_ids.IsNull())
	{
		INT list_count = JsonParse::count(duplicate_member_ids);
		for(int index = 0; index < list_count; index++)
		{
			uint64_t duplicate_user = 0;
			JsonParse::at(duplicate_member_ids, index, duplicate_user);
			vecotr_duplicate_ids.push_back((UINT64)duplicate_user);

			LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "duplicate_user = " << (UINT64)duplicate_user);
		}
	}
	

	STRU_GROUP_MODIFY_CHANGE_RS loSendPack;
	flatbuffers::FlatBufferBuilder& fbbuilder = loSendPack.fbbuilder;

	map<string, string> mapGroupMsgId;
	std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> vector_user_info;
	CGroupDistributeMgr::GetInstance().GenerateUserListByJsonValue(member_change_list, 
			vector_user_info, fbbuilder, mapGroupMsgId, message_id, vecotr_duplicate_ids);
	
	// 需要判断是否发送给群主
	INT is_send_to_manager = 0;
	if(!!group_add_is_agree && aiUserID != group_manager_user_id)
	{
		is_send_to_manager = 1;
	}

	if(big_msg_type == GROUP_OFFLINE_CHAT_ADD_USER)
	{
		if(!!is_send_to_manager)
		{
			big_msg_type = GROUP_OFFLINE_CHAT_ADD_USER_AGREE;
		}
	}
	

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "big_msg_type = " << big_msg_type << " group_id = " << (TEXTMSGID)group_id);

	auto group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
										fbbuilder, group_id, 
										fbbuilder.CreateString(group_name), 0, group_count,
										group_manager_user_id, group_add_is_agree,
										group_ct, fbbuilder.CreateString(group_remark), 
										group_max_count, 0, group_add_max_count);

	auto operate_group_msg = grouppack::CreateT_OPERATE_GROUP_MSG(fbbuilder,
							msg_time, fbbuilder.CreateString(operate_user_name),
							fbbuilder.CreateVector(vector_user_info),
							fbbuilder.CreateString(group_modify_content));

	auto offline_group_msg = grouppack::CreateT_OFFLINE_GROUP_MSG(fbbuilder,
							user_id_initial, message_id, 0, big_msg_type, operate_group_msg, message_old_id);
	loSendPack.fbbuf = grouppack::CreateT_GROUP_MODIFY_ChANGE_RS(fbbuilder,
										&s_rs_head, group_base_info, offline_group_msg);
	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	if(!!is_send_to_manager)
	{
		CGroupDistributeMgr::GetInstance().SendGroupMsgToGroupManager(aiUserID, group_name,
						group_id, group_manager_user_id);
	}
	else
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "group_id = " << group_id << " size = " << mapGroupMsgId.size());
		mpServerMgr->mpRedisUtil->HMsetGroupMsgId(group_id, mapGroupMsgId);
		CGroupDistributeMgr::GetInstance().SendGroupMsgId(mpServerMgr, aiUserID, group_name, group_id);
	}
	
	CGroupDistributeMgr::GetInstance().GroupUserChangeMemberChangeOffline(
									mpServerMgr, group_id,
									big_msg_type, 
									aiUserID, operate_user_name, msg_time, 
									message_id, message_old_id, member_change_list,
									group_name, group_count, group_manager_user_id, 
									group_max_count, group_add_max_count,
									group_modify_content, user_id_initial,
									vecotr_duplicate_ids);

	return 1;
}

INT CClientSide::OnGroupKickUserChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 msg_time,
								string& operate_user_name, UINT64 message_id,
								UINT64 message_old_id, RJsonValue& member_change_list)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)

	INT big_msg_type = GROUP_OFFLINE_CHAT_KICK_USER;
	
	UINT64 group_manager_user_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupManagerUserId());
	UINT64 group_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupId());
	
	INT group_count = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupCount(), group_count);

	INT group_add_is_agree = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupAddIsAgree(), group_add_is_agree);

	str_val = "";
	JsonParse::get(jsondata, CGroupModifyMember::GetGroupCT(), str_val);
	UINT64 group_ct = ServerUtilMgr::STR2UINT64(str_val);

	string group_remark = "";
	JsonParse::get(jsondata, GroupDetailInfo::GetGroupRemark(), group_remark);
	
	string group_name = "";
	JsonParse::get(jsondata, GroupDetailInfo::GetGroupName(), group_name);
	
	string group_modify_content = ""; 
	JsonParse::get(jsondata, CGroupModifyMember::GetGroupModifyContent(), group_modify_content);
	
	INT group_max_count = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupMaxCount(), group_max_count);

	INT group_add_max_count = 0;
	JsonParse::get<int>(jsondata, "group_add_max_count", group_add_max_count);

	STRU_GROUP_MODIFY_CHANGE_RS loSendPack;

	flatbuffers::FlatBufferBuilder& fbbuilder = loSendPack.fbbuilder;

	std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> vector_user_info;
	CGroupDistributeMgr::GetInstance().GenerateUserListByJsonValue(member_change_list, vector_user_info, fbbuilder);

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "big_msg_type = " << big_msg_type << " group_id = " << group_id);

	auto group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
										fbbuilder, group_id, 
										fbbuilder.CreateString(group_name), 0, group_count,
										group_manager_user_id, group_add_is_agree,
										group_ct, fbbuilder.CreateString(group_remark), 
										group_max_count, 0, group_add_max_count);

	auto operate_group_msg = grouppack::CreateT_OPERATE_GROUP_MSG(fbbuilder,
							msg_time, fbbuilder.CreateString(operate_user_name),
							fbbuilder.CreateVector(vector_user_info),
							fbbuilder.CreateString(group_modify_content));

	auto offline_group_msg = grouppack::CreateT_OFFLINE_GROUP_MSG(fbbuilder,
							aiUserID, message_id, 0, big_msg_type, operate_group_msg, message_old_id);
	
	loSendPack.fbbuf = grouppack::CreateT_GROUP_MODIFY_ChANGE_RS(fbbuilder,
										&s_rs_head, group_base_info, offline_group_msg);
	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	CGroupDistributeMgr::GetInstance().SendKickGroupMsgId(mpServerMgr, aiUserID, 
					group_name, group_id, member_change_list);
	
	CGroupDistributeMgr::GetInstance().GroupUserChangeMemberChangeOffline(
									mpServerMgr, group_id,
									big_msg_type, 
									aiUserID, operate_user_name, msg_time, 
									message_id, message_old_id, member_change_list,
									group_name, group_count, group_manager_user_id, 
									group_max_count, group_add_max_count,
									group_modify_content, aiUserID);

	return 1;
}

INT CClientSide::OnGroupCreateUserModifyChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 msg_time,
								string& operate_user_name, UINT64 message_id,
								UINT64 message_old_id, RJsonValue& member_change_list,
								INT group_max_count, INT group_add_max_count)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_CREATE_RS::ENUM_TYPE)

	UINT64 group_manager_user_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupManagerUserId());
	UINT64 group_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupId());
	
	INT group_count = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupCount(), group_count);

	INT group_add_is_agree = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupAddIsAgree(), group_add_is_agree);

	str_val = "";
	JsonParse::get(jsondata, CGroupModifyMember::GetGroupCT(), str_val);
	UINT64 group_ct = ServerUtilMgr::STR2UINT64(str_val);

	string group_remark = "";
	JsonParse::get(jsondata, GroupDetailInfo::GetGroupRemark(), group_remark);
	
	string group_name = "";
	JsonParse::get(jsondata, GroupDetailInfo::GetGroupName(), group_name);
	
	string group_modify_content = ""; 
	JsonParse::get(jsondata, CGroupModifyMember::GetGroupModifyContent(), group_modify_content);
	
	INT big_msg_type = GROUP_OFFLINE_CREATE_GROUP;
	STRU_GROUP_CREATE_RS loSendPackCreate;
	flatbuffers::FlatBufferBuilder& fbbuilder = loSendPackCreate.fbbuilder;

	std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> vector_user_info;
	CGroupDistributeMgr::GetInstance().GenerateUserListByJsonValue(member_change_list, vector_user_info, fbbuilder);
	
	auto group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
										fbbuilder, group_id, 
										fbbuilder.CreateString(group_name), 0, group_count,
										group_manager_user_id, group_add_is_agree,
										group_ct, fbbuilder.CreateString(group_remark), 
										group_max_count, 0, group_add_max_count);

	auto operate_group_msg = grouppack::CreateT_OPERATE_GROUP_MSG(fbbuilder,
							msg_time, fbbuilder.CreateString(operate_user_name),
							fbbuilder.CreateVector(vector_user_info),
							fbbuilder.CreateString(group_modify_content));

	auto offline_group_msg = grouppack::CreateT_OFFLINE_GROUP_MSG(fbbuilder,
							aiUserID, message_id, 0, big_msg_type, operate_group_msg, message_old_id);
	
	loSendPackCreate.fbbuf = grouppack::CreateT_GROUP_CREATE_RS(fbbuilder,
										&s_rs_head, group_base_info, offline_group_msg);
	Send2Comet(loSendPackCreate.GetPackType(), loSendPackCreate, aiUserID, lpUser);

	CGroupDistributeMgr::GetInstance().SendGroupMsgId(mpServerMgr, aiUserID, group_name, group_id);
	
	CGroupDistributeMgr::GetInstance().GroupUserChangeMemberChangeOffline(
									mpServerMgr, group_id,
									big_msg_type, 
									aiUserID, operate_user_name, msg_time, 
									message_id, message_old_id, member_change_list,
									group_name, group_count, group_manager_user_id, 
									group_max_count, group_add_max_count, group_modify_content, aiUserID);

	return 1;
}

INT CClientSide::OnGroupStrChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								INT big_msg_type, UINT64 msg_time,
								string& operate_user_name, UINT64 message_id,
								UINT64 message_old_id, string& str_change_content)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)

	UINT64 group_manager_user_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupManagerUserId());
	UINT64 group_id = GetJsonID(jsondata, CGroupModifyMember::GetGroupId());
	
	INT group_count = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupCount(), group_count);

	INT group_add_is_agree = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupAddIsAgree(), group_add_is_agree);

	str_val = "";
	JsonParse::get(jsondata, CGroupModifyMember::GetGroupCT(), str_val);
	UINT64 group_ct = ServerUtilMgr::STR2UINT64(str_val);

	string group_remark = "";
	JsonParse::get(jsondata, GroupDetailInfo::GetGroupRemark(), group_remark);
	
	string group_name = "";
	JsonParse::get(jsondata, GroupDetailInfo::GetGroupName(), group_name);
	
	string group_modify_content = ""; 
	JsonParse::get(jsondata, CGroupModifyMember::GetGroupModifyContent(), group_modify_content);
	
	INT group_max_count = 0;
	JsonParse::get<int>(jsondata, CGroupModifyMember::GetGroupMaxCount(), group_max_count);

	INT group_add_max_count = 0;
	JsonParse::get<int>(jsondata, "group_add_max_count", group_add_max_count);

	STRU_GROUP_MODIFY_CHANGE_RS loSendPack;
	flatbuffers::FlatBufferBuilder& fbbuilder = loSendPack.fbbuilder;
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "big_msg_type = " << big_msg_type << " group_id = " << group_id);

	auto group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
										fbbuilder, group_id, 
										fbbuilder.CreateString(group_name), 0, group_count,
										group_manager_user_id, group_add_is_agree,
										group_ct, fbbuilder.CreateString(group_remark), group_max_count);

	auto operate_group_msg = grouppack::CreateT_OPERATE_GROUP_MSG(fbbuilder,
							msg_time, fbbuilder.CreateString(operate_user_name), 0,
							fbbuilder.CreateString(group_modify_content));

	auto offline_group_msg = grouppack::CreateT_OFFLINE_GROUP_MSG(fbbuilder,
							aiUserID, message_id, 0, big_msg_type, operate_group_msg, message_old_id);
	
	loSendPack.fbbuf = grouppack::CreateT_GROUP_MODIFY_ChANGE_RS(fbbuilder,
										&s_rs_head, group_base_info, offline_group_msg);
	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	CGroupDistributeMgr::GetInstance().SendGroupMsgId(mpServerMgr, aiUserID, group_name, group_id);
	
	CGroupDistributeMgr::GetInstance().GroupStrContentChangeOffline(
									mpServerMgr, group_id,
									big_msg_type, 
									aiUserID, operate_user_name, msg_time, 
									message_id, message_old_id,
									group_modify_content,
									group_name, group_count,
									group_manager_user_id, 
									group_max_count, group_add_max_count);

	return 1;
}

INT CClientSide::OnGroupMessageStatusRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 group_id, BYTE message_status)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_MESSAGE_STATUS_RS::ENUM_TYPE)
	
	STRU_GROUP_MESSAGE_STATUS_RS loSendPack;
	loSendPack.fbbuf = grouppack::CreateT_GROUP_MESSAGE_STATUS_RS(loSendPack.fbbuilder,
									&s_rs_head, 
									group_id, message_status);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}

INT CClientSide::OnGroupSaveChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 group_id, BYTE save_type)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_SAVE_CHANGE_RS::ENUM_TYPE)
	
	STRU_GROUP_SAVE_CHANGE_RS loSendPack;
	loSendPack.fbbuf = grouppack::CreateT_GROUP_SAVE_CHANGE_RS(loSendPack.fbbuilder,
									&s_rs_head, 
									group_id, save_type);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}

INT CClientSide::OnGroupGetUserRemarkNameRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 group_id)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GROUP_GET_USER_REMARK_NAME_RS::ENUM_TYPE)
	
	string user_remark_name = ""; 
	JsonParse::get(jsondata, "user_remark_name", user_remark_name);

	STRU_GROUP_GET_USER_REMARK_NAME_RS loSendPack;
	loSendPack.fbbuf = grouppack::CreateT_GROUP_GET_USER_REMARK_NAME_RS(loSendPack.fbbuilder,
									&s_rs_head, 
									group_id,
									loSendPack.fbbuilder.CreateString(user_remark_name));

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}

INT CClientSide::OnGroupGetBatchInfoRQ(RJDocument& jsoncmd, 
									SESSIONID aiCometID, USERID aiUserID, 
									INT aiPackSessionID, BYTE aiPlatform)
{
	GENERATE_DBSERVER_TO_LOGIC_CMD(STRU_GET_BATCH_GROUP_INFO_RS::ENUM_TYPE)

	STRU_GET_BATCH_GROUP_INFO_RS loSendPack;
	std::vector<flatbuffers::Offset<grouppack::T_GROUP_BASE_INFO> > vector_list_group_info;

	RJsonValue& group_list = jsondata;
	INT list_count = JsonParse::count(group_list);

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "count = " << list_count);

	std::vector<uint64_t> vector_group_id;
	for(INT index = 0; index < list_count; index++)
	{
		RJsonValue& group_info = JsonParse::at(group_list, index);

		INT is_valid = 0;
		JsonParse::get(group_info, "is_valid", is_valid);

		UINT64 group_id = GetJsonID(group_info, CGroupModifyMember::GetGroupId());
		
		if(is_valid <= 0)
		{
			vector_group_id.push_back(uint64_t(group_id));
			continue;
		}

		string group_name = "";
		JsonParse::get(group_info, GroupDetailInfo::GetGroupName(), group_name);

		INT group_count = 0;
		JsonParse::get<int>(group_info, GroupDetailInfo::GetGroupCount(), group_count);

		UINT64 group_manager_user_id = GetJsonID(group_info, CGroupModifyMember::GetGroupManagerUserId());
		
		INT group_add_is_agree = 0;
		JsonParse::get<int>(group_info, GroupDetailInfo::GetGroupAddIsAgree(), group_add_is_agree);

		ULONG group_ct = 0;
		JsonParse::get<ULONG>(group_info, GroupDetailInfo::GetGroupCT(), group_ct);

		string group_remark = "";
		JsonParse::get(group_info, GroupDetailInfo::GetGroupRemark(), group_remark);

		INT group_max_count = 0;
		JsonParse::get<int>(group_info, CGroupModifyMember::GetGroupMaxCount(), group_max_count);

		INT group_add_max_count = 0;
		JsonParse::get<int>(group_info, "group_add_max_count", group_add_max_count);

		BYTE message_status = 0;
		JsonParse::get<BYTE>(group_info, "message_status", message_status);

		auto group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
				loSendPack.fbbuilder, 
				group_id, loSendPack.fbbuilder.CreateString(group_name), 
				0, group_count,
				group_manager_user_id,
				group_add_is_agree,
				group_ct,
				loSendPack.fbbuilder.CreateString(group_remark),
				group_max_count,
				message_status,
				group_add_max_count);

		vector_list_group_info.push_back(group_base_info);
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "vector_list_group_info = " << vector_list_group_info.size());

	auto vector_msg_builder = loSendPack.fbbuilder.CreateVector(vector_list_group_info);
	loSendPack.fbbuf = grouppack::CreateT_GET_BATCH_GROUP_INFO_RS(loSendPack.fbbuilder,
			&s_rs_head, loSendPack.fbbuilder.CreateVector(vector_group_id),
			vector_msg_builder);

	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
	return 1;
}
///////////////////////////////// http end //////////////////////////////////////////////

UINT64 CClientSide::GetJsonID(RJsonValue &jsondata, string strKey)
{
	uint64_t group_id = 0;
	JsonParse::get<uint64_t>(jsondata, strKey.c_str(), group_id);
	return (UINT64)group_id;
}