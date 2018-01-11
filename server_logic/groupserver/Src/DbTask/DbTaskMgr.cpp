#include "DbTaskMgr.h"
#include "rjson/JsonParser.h"
#include "httpclient.h"
#include "common/Debug_log.h"
#include "network/packet/client/SysPackDef.h"
#include "network/packet/client/FdServerPackDef.h"
#include "ServerUtilMgr.h"
#include "ServerConn/UserConMgr.h"
#include "./ClientSide/ClientSide.h"
#include "ServerMgr.h"
#include "const/NetAddrDef.h"

#define Handle_Respone_Post_Info(url_data, pack_type) if (NULL == m_parm)\
    {\
		m_pclientside->HandleFailedResultFromHttp(pack_type, m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform);\
        return ;\
    }\
    CHttpClient* phttpclient =  (CHttpClient*) m_parm;\
    std::string szUrl = url_data;\
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " szUrl = " << szUrl << "m_data = " << m_data);\
    std::string szReponse;\
	INT result_http = phttpclient->Post(szUrl.c_str(), m_data, szReponse);\
	if (CURLE_OK != result_http)\
	{\
		m_pclientside->HandleFailedResultFromHttp(pack_type, m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform);\
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " result_http = " << result_http);\
		return ;\
	}\
	if (szReponse.empty())\
	{\
		m_pclientside->HandleFailedResultFromHttp(pack_type, m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform);\
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " szReponse is empty ");\
		return ;\
	}\
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "szReponse = " << szReponse);\
    RJDocument jsonResult;\
	JsonParse::parse(jsonResult, szReponse.c_str());\

#define PRINT_QUEUE_IN_OUT_TIME uint64 interval =  GetInterValTime();\
    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "userid" << m_aiUserID << " pop queue time = " << interval);\

void CGroupCreateRQ::run()
{
	if(m_child_array.empty())
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " is empty");
		m_pclientside->HandleFailedResultFromHttp(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE, m_aiCometID,
					m_aiUserID, m_aiPackSessionID, m_aiPlatform);
		return ;
	}

	RJDocument rjdoc_list;
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "m_child_array size = " << m_child_array.size());
	JsonParse::parse(rjdoc_list, m_child_array.c_str());
	Handle_Respone_Post_Info(GROUP_CREATE_RQ_API, STRU_GROUP_CREATE_RS::ENUM_TYPE)
	m_pclientside->OnGroupCreateUserModifyChangeRQ(jsonResult, 
						m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform,
						m_msg_time, m_operate_user_name, m_message_id,
						m_message_old_id, rjdoc_list, m_group_max_count,
						m_group_add_max_count); 
}

void CGroupDetailInfoRQ::run()
{
	Handle_Respone_Post_Info(GROUP_DETAIL_RQ_API, STRU_GROUP_DETAIL_INFO_RS::ENUM_TYPE)
	m_pclientside->OnGroupDetailInfoRQ(jsonResult, m_aiCometID, m_aiUserID,
	 	m_aiPackSessionID, m_aiPlatform, m_group_id, m_group_member_index, m_span); 
}

void CGroupLeaderChangeRQ::run()
{
    Handle_Respone_Post_Info(GROUP_LEADER_CHANGE_RQ_API, STRU_GROUP_LEADER_CHANGE_RS::ENUM_TYPE)
	m_pclientside->OnGroupLeaderChangeRQ(jsonResult, m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform); 
}

void CGroupScanRQ::run()
{
    Handle_Respone_Post_Info(GROUP_GET_INFO_RQ_API, STRU_GROUP_SCAN_RS::ENUM_TYPE)
	m_pclientside->OnGroupScanRQ(jsonResult, m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform, m_user_id_share); 
}

void CGroupListRQ::run()
{
    Handle_Respone_Post_Info(GROUP_LIST_RQ_API, STRU_GROUP_LIST_RS::ENUM_TYPE)
	m_pclientside->OnGroupListRQ(jsonResult, m_aiCometID, m_aiUserID, 
						m_aiPackSessionID, m_aiPlatform, m_group_list_index, m_span); 
}

void CGroupListIds::run()
{
    Handle_Respone_Post_Info(GROUP_LIST_IDS_RQ_API, STRU_GROUP_LIST_IDS_RS::ENUM_TYPE)
	m_pclientside->OnGroupListIdsRQ(jsonResult, m_aiCometID, m_aiUserID, 
						m_aiPackSessionID, m_aiPlatform, m_group_list_index, m_span); 
}

void CGroupAddUserRQ::run()
{
	if(m_child_array.empty())
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " is empty");
		m_pclientside->HandleFailedResultFromHttp(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE, m_aiCometID,
					m_aiUserID, m_aiPackSessionID, m_aiPlatform);
		return ;
	}

	RJDocument rjdoc_list;
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "m_child_array size = " << m_child_array.size());
	JsonParse::parse(rjdoc_list, m_child_array.c_str());
	Handle_Respone_Post_Info(GROUP_ADD_USER_RQ_API, STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)

	m_pclientside->OnGroupAddUserChangeRQ(jsonResult, 
						m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform,
						m_msg_time, m_operate_user_name, m_message_id,
						m_message_old_id, rjdoc_list, m_str_add_reason,
						m_user_id_initial, m_big_msg_type);
}

void CGroupKickUserRQ::run()
{
	if(m_child_array.empty())
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " is empty");
		m_pclientside->HandleFailedResultFromHttp(STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE, m_aiCometID,
					m_aiUserID, m_aiPackSessionID, m_aiPlatform);
		return ;
	}

	RJDocument rjdoc_list;
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "m_child_array size = " << m_child_array.size());
	JsonParse::parse(rjdoc_list, m_child_array.c_str());
	Handle_Respone_Post_Info(GROUP_KICK_USER_RQ_API, STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)
	
	m_pclientside->OnGroupKickUserChangeRQ(jsonResult, 
						m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform,
						m_msg_time, m_operate_user_name, m_message_id,
						m_message_old_id, rjdoc_list); 
}

void CGroupAddUserAgreeRQ::run()
{
	Handle_Respone_Post_Info(GROUP_MODIFY_AGREE_RQ_API, STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)

	string str_content = "";
	m_pclientside->OnGroupStrChangeRQ(jsonResult, 
						m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform,
						GROUP_OFFLINE_CHAT_ENTER_AGREE, m_msg_time, m_operate_user_name, m_message_id,
						m_message_old_id, str_content); 
}

void CGroupAddUserDefaultRQ::run()
{
	Handle_Respone_Post_Info(GROUP_MODIFY_DEFAULT_RQ_API, STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)

	string str_content = "";
	m_pclientside->OnGroupStrChangeRQ(jsonResult, 
						m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform,
						GROUP_OFFLINE_CHAT_ENTER_DEFAULT, m_msg_time, m_operate_user_name, m_message_id,
						m_message_old_id, str_content); 
}

void CGroupChangeGroupNameRQ::run()
{
	Handle_Respone_Post_Info(GROUP_MODIFY_GROUP_NAME_RQ_API, STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)

	m_pclientside->OnGroupStrChangeRQ(jsonResult, 
						m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform,
						GROUP_OFFLINE_CHAT_MODIFY_GROUP_NAME, m_msg_time, m_operate_user_name, m_message_id,
						m_message_old_id, m_child_array); 
}

void CGroupChangeNickNameRQ::run()
{
	Handle_Respone_Post_Info(GROUP_MODIFY_NICK_NAME_RQ_API, STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)

	m_pclientside->OnGroupStrChangeRQ(jsonResult, 
						m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform,
						GROUP_OFFLINE_CHAT_MODIFY_GROUP_USER_NAME, m_msg_time, m_operate_user_name, m_message_id,
						m_message_old_id, m_child_array); 
}

void CGroupChangeRemarkRQ::run()
{
	Handle_Respone_Post_Info(GROUP_MODIFY_REMARK_RQ_API, STRU_GROUP_MODIFY_CHANGE_RS::ENUM_TYPE)

	m_pclientside->OnGroupStrChangeRQ(jsonResult, 
						m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform,
						GROUP_OFFLINE_CHAT_MODIFY_GROUP_REMARK, m_msg_time, m_operate_user_name, m_message_id,
						m_message_old_id, m_child_array); 
}

void CGroupRemarkDetailRQ::run()
{
	Handle_Respone_Post_Info(GROUP_REMARK_DETAIL_RQ_API, STRU_GROUP_REMARK_DETAIL_RS::ENUM_TYPE)
	m_pclientside->OnGroupRemarkDetailRQ(jsonResult, m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform); 
}

void CGroupMessageStatusRQ::run()
{
    Handle_Respone_Post_Info(GROUP_RECV_TYPE_RQ_API, STRU_GROUP_MESSAGE_STATUS_RS::ENUM_TYPE)
	m_pclientside->OnGroupMessageStatusRQ(jsonResult, m_aiCometID, m_aiUserID, 
						m_aiPackSessionID, m_aiPlatform, m_group_id, m_message_status); 
}

void CGroupSaveChangeRQ::run()
{
    Handle_Respone_Post_Info(GROUP_SAVE_CHANGE_RQ_API, STRU_GROUP_SAVE_CHANGE_RS::ENUM_TYPE)
	m_pclientside->OnGroupSaveChangeRQ(jsonResult, m_aiCometID, m_aiUserID, 
						m_aiPackSessionID, m_aiPlatform, m_group_id, m_save_type); 
}

void CGroupGetUserRemarkNameRQ::run()
{
	Handle_Respone_Post_Info(GROUP_GET_USER_REMARK_NAME_RQ_API, STRU_GROUP_GET_USER_REMARK_NAME_RS::ENUM_TYPE)
	m_pclientside->OnGroupGetUserRemarkNameRQ(jsonResult, m_aiCometID, m_aiUserID, 
						m_aiPackSessionID, m_aiPlatform, m_group_id); 
}

void CGroupGetBatchListRQ::run()
{
	Handle_Respone_Post_Info(GROUP_GET_BATCH_INFO_API, STRU_GET_BATCH_GROUP_INFO_RS::ENUM_TYPE)
	m_pclientside->OnGroupGetBatchInfoRQ(jsonResult, m_aiCometID, m_aiUserID, m_aiPackSessionID, m_aiPlatform); 
}