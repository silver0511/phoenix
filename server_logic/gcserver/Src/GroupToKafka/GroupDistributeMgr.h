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
                    T& loServerSendPack, BOOL is_seperate, const UINT64& seperate_number, 
                    UINT64& group_id)
    {
        DWORD len_send_apns = 0;
        DWORD len_send_no_apns = 0;
       
        USERID list_user_apns[DEF_GROUP_TO_KAFKA_NUMBER] = {0};
        USERID list_user_no_apns[DEF_GROUP_TO_KAFKA_NUMBER] = {0};

        string strKeyBegin = to_string(group_id) + "_";
        INT send_count = 0;
        INT send_no_hit_count = 0;
        auto iter = mapGroupInfo.begin();
        for(iter; iter != mapGroupInfo.end(); iter++)
        {
            UINT64 user_id_temp = ServerUtilMgr::STR2UINT64(iter->first);
            BYTE message_status = (BYTE)atoi(iter->second.c_str());

            if(is_seperate && user_id_temp == user_id)
            {
                continue ;
            }

            if(len_send_apns == DEF_GROUP_TO_KAFKA_NUMBER)
            {
                string strKey = strKeyBegin + to_string(GROUP_MESSAGE_STATUS_NORMAL) + "_" + to_string(send_count);
                send_count++;
                CServerToKafka::GetInstance().DistributeMsgToGcRoute(len_send_apns, list_user_apns,
                        loServerSendPack, IM_GC_TOPIC, seperate_number, strKey);

                len_send_apns = 0;
                memset(list_user_apns, 0, sizeof(USERID) * DEF_GROUP_TO_KAFKA_NUMBER);

                LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " strKey = " << strKey);
            }

            if(len_send_no_apns == DEF_GROUP_TO_KAFKA_NUMBER)
            {
                string strKey = strKeyBegin + to_string(GROUP_MESSAGE_STATUS_NO_HIT) + "_" + to_string(send_no_hit_count);
                send_no_hit_count++;
                CServerToKafka::GetInstance().DistributeMsgToGcRoute(len_send_apns, list_user_apns,
                        loServerSendPack, IM_GC_TOPIC, seperate_number, strKey);

                len_send_no_apns = 0;
                memset(list_user_apns, 0, sizeof(USERID) * DEF_GROUP_TO_KAFKA_NUMBER);

                LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " strKey = " << strKey);
            }

            if(message_status == GROUP_MESSAGE_STATUS_NORMAL)
            {
                list_user_apns[len_send_apns % DEF_GROUP_TO_KAFKA_NUMBER] = user_id_temp;
                len_send_apns++;

                LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id_temp = " << user_id_temp << " apns");
            }
            else
            {
                LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id_temp = " << user_id_temp << " no apns");
                list_user_no_apns[len_send_no_apns % DEF_GROUP_TO_KAFKA_NUMBER] = user_id_temp;
                len_send_no_apns++;
            }
        }

        if(len_send_apns > 0)
        {
            string strKey = strKeyBegin + to_string(GROUP_MESSAGE_STATUS_NORMAL) + "_" + to_string(send_count);
            CServerToKafka::GetInstance().DistributeMsgToGcRoute(len_send_apns, list_user_apns,
                        loServerSendPack, IM_GC_TOPIC, seperate_number, strKey);

            LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " strKey = " << strKey);
        }

        if(len_send_no_apns > 0)
        {
            string strKey = strKeyBegin + to_string(GROUP_MESSAGE_STATUS_NO_HIT) + "_" + to_string(send_no_hit_count);
            CServerToKafka::GetInstance().DistributeMsgToGcRoute(len_send_no_apns, list_user_no_apns,
                        loServerSendPack, IM_GC_TOPIC, seperate_number, strKey);

            LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " strKey = " << strKey);
        }

        return 1;
    }
};
#endif