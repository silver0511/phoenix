#ifndef __OFFCIAL_DISTRRI_BUITE_MGR_H
#define __OFFCIAL_DISTRRI_BUITE_MGR_H
#include "common/Debug_log.h"
#include "network/packet/ServerPack.h"
#include "const/PackDef.h"
#include "ServerToKafka.h"
#include "ServerUtilMgr.h"
#include "network/packet/client/OffcialServerPackDef.h"


class CServerMgr;

class COffcialDistributeMgr
{

private:
    COffcialDistributeMgr();
    COffcialDistributeMgr(const COffcialDistributeMgr &);
    COffcialDistributeMgr & operator = (const COffcialDistributeMgr &);
    
public:
    ~COffcialDistributeMgr();

    void releaseKafka();

public:
    static COffcialDistributeMgr & GetInstance()
    {
        static COffcialDistributeMgr instance;
        return instance;
    }

public:

    template<class T>
    INT DistributeToKafka(const USERID& user_id, std::vector<UINT64>& vectorGroupInfo, 
                    T& loServerSendPack, BOOL is_seperate, const UINT64& partition)
    {
		std::string strKey = DEF_OFFCIAL_TO_KAFKA_KEY;
        USERID list_user_id[DEF_GROUP_TO_KAFKA_NUMBER] = {0};
        DWORD len_send = 0;
        for(auto index = 0; index < vectorGroupInfo.size(); index++)
        {
            if(is_seperate && vectorGroupInfo[index] == user_id)
            {
                continue ;
            }

            if(len_send == DEF_GROUP_TO_KAFKA_NUMBER)
            {
                CServerToKafka::GetInstance().DistributeMsgToGcRoute(len_send, list_user_id,
                        loServerSendPack, IM_OFFCIALR_TOPIC, partition,strKey);

                len_send = 0;
                memset(list_user_id, 0, sizeof(USERID) * DEF_GROUP_TO_KAFKA_NUMBER);
            }

            list_user_id[len_send % DEF_GROUP_TO_KAFKA_NUMBER] = vectorGroupInfo[index];
            len_send++;
        }
        if(len_send > 0)
        {
            CServerToKafka::GetInstance().DistributeMsgToGcRoute(len_send, list_user_id,
                        loServerSendPack, IM_OFFCIALR_TOPIC, partition,strKey);
        }

        return 1;
    }

	template<class T>
	INT DistributeToKafkaWs(const USERID& user_id, std::vector<UINT64>& vectorGroupInfo, 
		T& loServerSendPack, BOOL is_seperate, const UINT64& partition)
	{
		std::string strKey = DEF_ENTERPRISE_TO_KAFKA_KEY;
		USERID list_user_id[DEF_GROUP_TO_KAFKA_NUMBER] = {0};
		DWORD len_send = 0;
		for(auto index = 0; index < vectorGroupInfo.size(); index++)
		{
			if(is_seperate && vectorGroupInfo[index] == user_id)
			{
				continue ;
			}

			if(len_send == DEF_GROUP_TO_KAFKA_NUMBER)
			{
				CServerToKafka::GetInstance().DistributeMsgToGcRoute(len_send, list_user_id,
					loServerSendPack, IM_OFFCIALR_TOPIC, partition,strKey);

				len_send = 0;
				memset(list_user_id, 0, sizeof(USERID) * DEF_GROUP_TO_KAFKA_NUMBER);
			}

			list_user_id[len_send % DEF_GROUP_TO_KAFKA_NUMBER] = vectorGroupInfo[index];
			len_send++;
		}
		if(len_send > 0)
		{
			CServerToKafka::GetInstance().DistributeMsgToGcRoute(len_send, list_user_id,
				loServerSendPack, IM_OFFCIALR_TOPIC, partition,strKey);
		}

		return 1;
	}

    template<class T>
    INT DistributeToKafka(const USERID& user_id, std::vector<std::string>& vectorGroupInfo, 
                    T& loServerSendPack, BOOL is_seperate, const UINT64& partition)
    {
        std::vector<UINT64> vector_user_ids;
        for(auto index_list = 0; index_list < vectorGroupInfo.size(); index_list++)
        {
            vector_user_ids.push_back(ServerUtilMgr::STR2UINT64(vectorGroupInfo[index_list]));
        }
        DistributeToKafka(user_id, vector_user_ids, loServerSendPack, is_seperate, partition);
        return 1;
    }

    template<class T>
    INT DistributeToKafka(const USERID& user_id, std::vector<uint64_t>& vectorGroupInfo, 
                    T& loServerSendPack, BOOL is_seperate, const UINT64& partition)
    {
        std::vector<UINT64> vector_user_ids;
        for(auto index_list = 0; index_list < vectorGroupInfo.size(); index_list++)
        {
            vector_user_ids.push_back((UINT64)(vectorGroupInfo[index_list]));
        }
        DistributeToKafka(user_id, vector_user_ids, loServerSendPack, is_seperate, partition);
        return 1;
    }


};
#endif