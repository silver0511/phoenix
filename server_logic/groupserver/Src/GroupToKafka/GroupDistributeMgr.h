#ifndef __GROUP_DISTRRI_BUITE_MGR_H
#define __GROUP_DISTRRI_BUITE_MGR_H
#include "common/Debug_log.h"
#include "network/packet/ServerPack.h"
#include "const/PackDef.h"
#include "ServerToKafka.h"
#include "ServerUtilMgr.h"
#include "network/packet/client/GcServerPackDef.h"
#include "rjson/JsonParser.h"

class CServerMgr;

class CGroupDistributeMgr
{

private:
    CGroupDistributeMgr();
    CGroupDistributeMgr(const CGroupDistributeMgr &);
    CGroupDistributeMgr & operator = (const CGroupDistributeMgr &);
    
public:
    ~CGroupDistributeMgr();

    void releaseKafka();

public:
    static CGroupDistributeMgr & GetInstance()
    {
        static CGroupDistributeMgr instance;
        return instance;
    }

public:
	template<class T>
    INT DistributeToKafka(const USERID& user_id, std::map<std::string, std::string>& mapGroupInfo, 
                    T& loServerSendPack, BOOL is_seperate, const UINT64& seperate_number, UINT64 group_id)
    {
        USERID list_user_id[DEF_GROUP_TO_KAFKA_NUMBER] = {0};
        DWORD len_send = 0;
        auto iter = mapGroupInfo.begin();
		string strKeyBegin = to_string(group_id) + "_0_";
		INT send_count = 0;

        for(iter; iter != mapGroupInfo.end(); iter++)
		{
			UINT64 user_id_temp = ServerUtilMgr::STR2UINT64(iter->first);
            BYTE message_status = (BYTE)atoi(iter->second.c_str());

            if(is_seperate && user_id_temp == user_id)
            {
                continue ;
            }

            if(len_send == DEF_GROUP_TO_KAFKA_NUMBER)
            {
				string strKey = strKeyBegin + to_string(send_count);
                send_count++;

				CServerToKafka::GetInstance().DistributeMsgToGcRoute(len_send, list_user_id,
                        loServerSendPack, IM_GC_TOPIC, seperate_number, strKey);
                
				len_send = 0;
                memset(list_user_id, 0, sizeof(USERID) * DEF_GROUP_TO_KAFKA_NUMBER);
            }

            list_user_id[len_send % DEF_GROUP_TO_KAFKA_NUMBER] = user_id_temp;
            len_send++;
        }

        if(len_send > 0)
        {
			string strKey = strKeyBegin + to_string(send_count);
			CServerToKafka::GetInstance().DistributeMsgToGcRoute(len_send, list_user_id,
                        loServerSendPack, IM_GC_TOPIC, seperate_number, strKey);
        }
        return 1;
    }

public:

    void GenerateUserListByJsonValue(RJsonValue& member_change_list,
				std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> &vector_user_info,
				flatbuffers::FlatBufferBuilder& fbbuilder_offline);
	
	void GenerateUserListByJsonValue(RJsonValue& member_change_list,
				std::vector<flatbuffers::Offset<commonpack::USER_BASE_INFO>> &vector_user_info,
				flatbuffers::FlatBufferBuilder& fbbuilder_offline,
				map<string, string>& mapGroupMsgId, UINT64 message_id, 
				std::vector<UINT64>& vector_duplicate_ids);
				
    INT GroupUserChangeMemberChangeOffline(
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
											INT64 user_id_initial);

	INT GroupUserChangeMemberChangeOffline(
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
			std::vector<UINT64>& vector_duplicate_ids);

    INT GroupStrContentChangeOffline(
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
											INT& group_add_max_count);

	INT SendGroupMsgId(CServerMgr* mpServerMgr, UINT64& aiUserID, 
									string group_name, 
									UINT64& group_id);

	INT SendGroupMsgToGroupManager(UINT64& aiUserID, string group_name, 
									UINT64& group_id, UINT64& group_manager_user_id);

	INT SendKickGroupMsgId(CServerMgr* mpServerMgr, UINT64& aiUserID, 
									string group_name, 
									UINT64& group_id,
									RJsonValue& member_change_list);
};
#endif