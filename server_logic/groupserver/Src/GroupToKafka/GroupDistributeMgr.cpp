#include "GroupDistributeMgr.h"
#include "network/packet/client/GcServerPackDef.h"
#include "RedisClass/GcModelDef.h"
#include "ServerUtilMgr.h"
#include "../ServerMgr.h"

CGroupDistributeMgr::CGroupDistributeMgr()
{
    
}

CGroupDistributeMgr::~CGroupDistributeMgr()
{

}

void CGroupDistributeMgr::GenerateUserListByJsonValue(RJsonValue& member_change_list,
	std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> &vector_user_info,
	flatbuffers::FlatBufferBuilder& fbbuilder_offline,
	map<string, string>& mapGroupMsgId, UINT64 message_id,
	std::vector<UINT64>& vector_duplicate_ids)
{
	for(INT index = 0; index < JsonParse::count(member_change_list); index++)
	{
		RJsonValue& user_base_info = JsonParse::at(member_change_list, index);

		std::string str_val = "";
		JsonParse::get(user_base_info, CGroupUserBaseInfo::GetUserId(), str_val);
		mapGroupMsgId[str_val] = to_string(message_id);

		USERID user_id = ServerUtilMgr::STR2UINT64(str_val);

		bool is_find = false;
		for(INT index_dup = 0; index_dup < vector_duplicate_ids.size(); index_dup++)
		{
			if(vector_duplicate_ids[index_dup] == user_id)
			{
				is_find = true;
				break;
			}
		}

		if(is_find)
		{
			continue ;
		}

		string user_nick_name = "";
		JsonParse::get(user_base_info, CGroupUserBaseInfo::GetUserNickName(), user_nick_name);

		INT user_group_index = 0;
		JsonParse::get<int>(user_base_info, CGroupUserBaseInfo::GetUserGroupIndex(), user_group_index);

		auto user_fbs_base_info = commonpack::CreateUSER_BASE_INFO(
					fbbuilder_offline, user_id, 
					fbbuilder_offline.CreateString(user_nick_name), 
					user_group_index);

		vector_user_info.push_back(user_fbs_base_info);
	}
}

void CGroupDistributeMgr::GenerateUserListByJsonValue(RJsonValue& member_change_list,
				std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> &vector_user_info,
				flatbuffers::FlatBufferBuilder& fbbuilder_offline)
{
	for(INT index = 0; index < JsonParse::count(member_change_list); index++)
	{
		RJsonValue& user_base_info = JsonParse::at(member_change_list, index);

		std::string str_val = "";
		JsonParse::get(user_base_info, CGroupUserBaseInfo::GetUserId(), str_val);
		USERID user_id = ServerUtilMgr::STR2UINT64(str_val);

		string user_nick_name = "";
		JsonParse::get(user_base_info, CGroupUserBaseInfo::GetUserNickName(), user_nick_name);
		
		INT user_group_index = 0;
		JsonParse::get<int>(user_base_info, CGroupUserBaseInfo::GetUserGroupIndex(), user_group_index);
		
		auto user_fbs_base_info = commonpack::CreateUSER_BASE_INFO(
						fbbuilder_offline, user_id, 
						fbbuilder_offline.CreateString(user_nick_name), 
						user_group_index);

		vector_user_info.push_back(user_fbs_base_info);
	}
}

INT CGroupDistributeMgr::SendGroupMsgId(CServerMgr* mpServerMgr, UINT64& aiUserID, 
									string group_name, 
									UINT64& group_id) 
{
	STRU_GROUP_NOTIFY_SERVER_RQ loServerSendPack;
	loServerSendPack.fbbuf = grouppack::CreateT_GROUP_CHAT_NOTIFY_RQ(loServerSendPack.fbbuilder,
									group_id, 
									loServerSendPack.fbbuilder.CreateString(group_name));

	std::map<std::string, std::string> mapGroupInfo;
	mpServerMgr->mpRedisUtil->SmembersGroupUserId(group_id, mapGroupInfo);

	CGroupDistributeMgr::GetInstance().DistributeToKafka(aiUserID, mapGroupInfo, 
							loServerSendPack, true, aiUserID, group_id);
}

INT CGroupDistributeMgr::SendGroupMsgToGroupManager(UINT64& aiUserID, string group_name, 
									UINT64& group_id, UINT64& group_manager_user_id) 
{
	STRU_GROUP_NOTIFY_SERVER_RQ loServerSendPack;
	loServerSendPack.fbbuf = grouppack::CreateT_GROUP_CHAT_NOTIFY_RQ(loServerSendPack.fbbuilder,
									group_id, 
									loServerSendPack.fbbuilder.CreateString(group_name));

	std::map<std::string, std::string> mapGroupInfo;
	mapGroupInfo[to_string(group_manager_user_id)] = "0";
	CGroupDistributeMgr::GetInstance().DistributeToKafka(aiUserID, mapGroupInfo, 
							loServerSendPack, true, aiUserID, group_id);
}

INT CGroupDistributeMgr::SendKickGroupMsgId(CServerMgr* mpServerMgr, UINT64& aiUserID, 
									string group_name, 
									UINT64& group_id,
									RJsonValue& member_change_list)
{
	STRU_GROUP_NOTIFY_SERVER_RQ loServerSendPack;
	loServerSendPack.fbbuf = grouppack::CreateT_GROUP_CHAT_NOTIFY_RQ(loServerSendPack.fbbuilder,
									group_id, 
									loServerSendPack.fbbuilder.CreateString(group_name));

	std::map<std::string, std::string> mapGroupInfo;
	mpServerMgr->mpRedisUtil->SmembersGroupUserId(group_id, mapGroupInfo);

	for(INT index = 0; index < JsonParse::count(member_change_list); index++)
	{
		RJsonValue& user_base_info = JsonParse::at(member_change_list, index);
		
		std::string str_val = "";
		JsonParse::get(user_base_info, CGroupUserBaseInfo::GetUserId(), str_val);

		if(ServerUtilMgr::STR2UINT64(str_val) == aiUserID)
		{
			continue ;
		}

		mapGroupInfo[str_val] = "0";
	}

	CGroupDistributeMgr::GetInstance().DistributeToKafka(aiUserID, mapGroupInfo, 
							loServerSendPack, true, aiUserID, group_id);
}

INT CGroupDistributeMgr::GroupUserChangeMemberChangeOffline(
											CServerMgr* mpServerMgr,
											UINT64& group_id,
											INT& big_msg_type, 
											USERID& aiUserID,
											string operate_user_name,
											UINT64& msg_time,
											UINT64& message_id,
											UINT64& message_old_id,
											RJsonValue& member_change_list,
											string& group_name,
											INT& group_count,
											UINT64& group_manager_user_id,
											INT& group_max_count, 
											INT& group_add_max_count,
											string& group_modify_content,
											INT64 user_id_initial)
{
	if(JsonParse::count(member_change_list) <= 0)
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "big_msg_type = " << big_msg_type << " user id list is nil");
		return 0;
	}
	
	if(big_msg_type != GROUP_OFFLINE_CHAT_ADD_USER &&
			big_msg_type != GROUP_OFFLINE_CHAT_ADD_USER_AGREE &&
			big_msg_type != GROUP_OFFLINE_CHAT_KICK_USER &&
			big_msg_type != GROUP_OFFLINE_CHAT_LEADER_CHANGE && 
			big_msg_type != GROUP_OFFLINE_CREATE_GROUP && 
			big_msg_type != GROUP_OFFLINE_CHAT_SCAN_ADD_USER && 
			big_msg_type != GROUP_OFFLINE_CHAT_SCAN_ADD_USER)
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "big_msg_type = " << big_msg_type << " is invalid");
		return 0;
	}

	flatbuffers::FlatBufferBuilder fbbuilder_offline;
	auto group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
									fbbuilder_offline, group_id, 
									fbbuilder_offline.CreateString(group_name), 
									0, group_count, group_manager_user_id,
									0, 0, 0, group_max_count, 0, group_add_max_count);
	
	std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> vector_user_info;
	CGroupDistributeMgr::GetInstance().GenerateUserListByJsonValue(member_change_list, vector_user_info, fbbuilder_offline);
		
	auto operate_group_msg = grouppack::CreateT_OPERATE_GROUP_MSG(fbbuilder_offline,
							msg_time, fbbuilder_offline.CreateString(operate_user_name),
							fbbuilder_offline.CreateVector(vector_user_info),
							fbbuilder_offline.CreateString(group_modify_content));
	
	fbbuilder_offline.Finish(operate_group_msg);
	auto offline_group_msg_offline = grouppack::CreateT_OFFLINE_GROUP_MSG(fbbuilder_offline,
							user_id_initial, message_id, 0, big_msg_type, 
							operate_group_msg, message_old_id, group_base_info);

	fbbuilder_offline.Finish(offline_group_msg_offline);

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " add user offline success " << " offline_group_msg->message_old_id()"
						<< message_old_id << "aiUserId = " << aiUserID << " big_msg_type = " << big_msg_type);
	
	BYTE*	lpData = fbbuilder_offline.GetBufferPointer();
    std::string strElement;
	strElement.assign((char*)lpData, fbbuilder_offline.GetSize());

	mpServerMgr->mpRedisUtil->ZaddGroupOffLineChatInfo(group_id, message_id, strElement);
}


INT CGroupDistributeMgr::GroupUserChangeMemberChangeOffline(
		CServerMgr* mpServerMgr,
		UINT64& group_id,
		INT& big_msg_type,
		USERID& aiUserID,
		string operate_user_name,
		UINT64& msg_time,
		UINT64& message_id,
		UINT64& message_old_id,
		RJsonValue& member_change_list,
		string& group_name,
		INT& group_count,
		UINT64& group_manager_user_id,
		INT& group_max_count,
		INT& group_add_max_count,
		string& group_modify_content,
		INT64 user_id_initial,
		std::vector<UINT64>& vector_duplicate_ids)
{
	if(JsonParse::count(member_change_list) <= 0)
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "big_msg_type = " << big_msg_type << " user id list is nil");
		return 0;
	}

	if(big_msg_type != GROUP_OFFLINE_CHAT_ADD_USER &&
	   big_msg_type != GROUP_OFFLINE_CHAT_ADD_USER_AGREE &&
	   big_msg_type != GROUP_OFFLINE_CHAT_KICK_USER &&
	   big_msg_type != GROUP_OFFLINE_CHAT_LEADER_CHANGE &&
	   big_msg_type != GROUP_OFFLINE_CREATE_GROUP &&
	   big_msg_type != GROUP_OFFLINE_CHAT_SCAN_ADD_USER &&
	   big_msg_type != GROUP_OFFLINE_CHAT_SCAN_ADD_USER)
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "big_msg_type = " << big_msg_type << " is invalid");
		return 0;
	}

	flatbuffers::FlatBufferBuilder fbbuilder_offline;
	auto group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
			fbbuilder_offline, group_id,
			fbbuilder_offline.CreateString(group_name),
			0, group_count, group_manager_user_id,
			0, 0, 0, group_max_count, 0, group_add_max_count);

	std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> vector_user_info;
	map<string, string> mapGroupMsgId;
	CGroupDistributeMgr::GetInstance().GenerateUserListByJsonValue(member_change_list, vector_user_info, fbbuilder_offline,
																   mapGroupMsgId, message_id, vector_duplicate_ids);

	auto operate_group_msg = grouppack::CreateT_OPERATE_GROUP_MSG(fbbuilder_offline,
																  msg_time, fbbuilder_offline.CreateString(operate_user_name),
																  fbbuilder_offline.CreateVector(vector_user_info),
																  fbbuilder_offline.CreateString(group_modify_content));

	fbbuilder_offline.Finish(operate_group_msg);
	auto offline_group_msg_offline = grouppack::CreateT_OFFLINE_GROUP_MSG(fbbuilder_offline,
																		  user_id_initial, message_id, 0, big_msg_type,
																		  operate_group_msg, message_old_id, group_base_info);

	fbbuilder_offline.Finish(offline_group_msg_offline);

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " add user offline success " << " offline_group_msg->message_old_id()"
																		   << message_old_id << "aiUserId = " << aiUserID << " big_msg_type = " << big_msg_type);

	BYTE*	lpData = fbbuilder_offline.GetBufferPointer();
	std::string strElement;
	strElement.assign((char*)lpData, fbbuilder_offline.GetSize());

	mpServerMgr->mpRedisUtil->ZaddGroupOffLineChatInfo(group_id, message_id, strElement);
}

INT CGroupDistributeMgr::GroupStrContentChangeOffline(
											CServerMgr* mpServerMgr,
											UINT64& group_id,
											INT& big_msg_type, 
											USERID& aiUserID,
											string operate_user_name,
											UINT64& msg_time,
											UINT64& message_id,
											UINT64& message_old_id,
											string& group_modify_content,
											string& group_name, 
											INT& group_count, 
											UINT64& group_manager_user_id,
											INT& group_max_count,
											INT& group_add_max_count)
{
	if(big_msg_type != GROUP_OFFLINE_CHAT_ENTER_AGREE &&
		big_msg_type != GROUP_OFFLINE_CHAT_ENTER_DEFAULT && 
		big_msg_type != GROUP_OFFLINE_CHAT_MODIFY_GROUP_NAME && 
		big_msg_type != GROUP_OFFLINE_CHAT_MODIFY_GROUP_REMARK && 
		big_msg_type != GROUP_OFFLINE_CHAT_MODIFY_GROUP_USER_NAME)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "big_msg_type is invalid big_msg_type = " << big_msg_type);
		return 0;
	}

	flatbuffers::FlatBufferBuilder fbbuilder_offline;
	auto group_base_info = grouppack::CreateT_GROUP_BASE_INFO(
									fbbuilder_offline, group_id, 
									fbbuilder_offline.CreateString(group_name), 
									0, group_count, group_manager_user_id,
									0, 0, 0, group_max_count, 0, group_add_max_count);

	auto operate_group_msg = grouppack::CreateT_OPERATE_GROUP_MSG(fbbuilder_offline,
							msg_time, fbbuilder_offline.CreateString(operate_user_name), 0,
							fbbuilder_offline.CreateString(group_modify_content));
	
	fbbuilder_offline.Finish(operate_group_msg);
	auto offline_group_msg = grouppack::CreateT_OFFLINE_GROUP_MSG(fbbuilder_offline,
							aiUserID, message_id, 0, big_msg_type, 
							operate_group_msg, message_old_id, group_base_info);
	fbbuilder_offline.Finish(offline_group_msg);
	
	BYTE*	lpData = fbbuilder_offline.GetBufferPointer();
    std::string strElement;
	strElement.assign((char*)lpData, fbbuilder_offline.GetSize());

	mpServerMgr->mpRedisUtil->ZaddGroupOffLineChatInfo(group_id, message_id, strElement);
}