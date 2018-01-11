#include "redisclient/RedisUtil.h"

INT RedisUtil::HsetUserOffLineChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem, LONG& result)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
        << " dScore = " << dScore);
    
    std::string table_name = std::string(SC_CHAT_INFO) + std::to_string(user_id);
    return redis_client->Hset(table_name, to_string(dScore), strElem, &result);	
}

INT RedisUtil::HsetEcOffLineChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem, LONG& result)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
        << " dScore = " << dScore << " finish");
    
    std::string table_name = std::string(EC_CHAT_INFO) + std::to_string(user_id);
    return redis_client->Hset(table_name, to_string(dScore), strElem, &result);	
}

INT RedisUtil::RpushEcOfflineMsgId(USERID user_id, TEXTMSGID message_id, LONG& nRedisResult)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
        << " message_id = " << message_id);
    
    std::string table_name = std::string(EC_CHAT_LIST_MSG) + std::to_string(user_id);
    return redis_client->Rpush(table_name, to_string(message_id), &nRedisResult);
}

INT RedisUtil::LRrangeEcOfflineMsgId(USERID user_id, long nStart, long nStop, std::vector<std::string> *pvecVal)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(EC_CHAT_LIST_MSG) + std::to_string(user_id);
    return redis_client->Lrange(table_name, nStart, nStop, pvecVal);
}

INT RedisUtil::DelEcOffLineChatInfo(USERID user_id)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(EC_CHAT_INFO) + std::to_string(user_id);
    return redis_client->Del(table_name);
}

INT RedisUtil::HExistEcOfflineMsg(USERID user_id, LONG& result)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(EC_CHAT_INFO) + std::to_string(user_id);
    return redis_client->Exists(table_name, &result);
}

INT RedisUtil::DelEcOfflineMsgId(USERID user_id)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(EC_CHAT_LIST_MSG) + std::to_string(user_id);
    return redis_client->Del(table_name);
}

INT RedisUtil::DelEcMutex(USERID user_id)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(EC_CHAT_MUTEX) + std::to_string(user_id);
    return redis_client->Del(table_name);
}

INT RedisUtil::HMGetEcOffLineChatInfo(USERID user_id, const std::vector<std::string> &vecField, std::vector<std::string> *pvecVal)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(EC_CHAT_INFO) + std::to_string(user_id);
    return redis_client->Hmget(table_name, vecField, pvecVal);	
}

INT RedisUtil::LlenGETEcOffLineCnt(USERID user_id, long & msgcnt)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(EC_CHAT_LIST_MSG) + std::to_string(user_id);
    return redis_client->Llen(table_name, &msgcnt);
}

INT RedisUtil::EcOfflineMutexUnLock(USERID user_id)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(EC_CHAT_MUTEX) + std::to_string(user_id); 

    long nVal;

    if (redis_client->Exists(table_name, &nVal) != RC_SUCCESS)
    {
        return -1;
    }

    if (nVal == 1)
    {
        return -1;
    }

    return RC_SUCCESS;
}
	
INT RedisUtil::EcOfflineMutexLock(USERID user_id)
{
    #define MAX_LOCK_TIME 60

    auto redis_client = GetRedisClient(REDIS_MESSAGE);

    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(EC_CHAT_MUTEX) + std::to_string(user_id);

    std::string szval("lock");
    if (redis_client->Setex(table_name, MAX_LOCK_TIME,szval) != RC_SUCCESS)
    {
        return -1;
    }

    return RC_SUCCESS;
}

INT RedisUtil::HMgetUserOffLineChatInfo(USERID user_id, const std::vector<std::string> &vecField, std::vector<std::string> *pvecVal)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(SC_CHAT_INFO) + std::to_string(user_id);
    return redis_client->Hmget(table_name, vecField, pvecVal);	
}

INT RedisUtil::LRrangeOffLineChatInfo(USERID user_id, long nStart, long nStop, std::vector<std::string> *pvecVal)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(SC_CHAT_LIST_MSG) + std::to_string(user_id);
    return redis_client->Lrange(table_name, nStart, nStop, pvecVal);
}

INT RedisUtil::LRrangeWebOffLineChatInfo(USERID user_id, long nStart, long nStop, std::vector<std::string> *pvecVal)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(SC_CHAT_WEB_LIST_MSG) + std::to_string(user_id);
    return redis_client->Lrange(table_name, nStart, nStop, pvecVal);
}

INT RedisUtil::DelUserOffLineChatInfo(USERID user_id)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(SC_CHAT_INFO) + std::to_string(user_id);
    return redis_client->Del(table_name);
}

INT RedisUtil::HDelUserOffLineChatInfo(USERID user_id, string strKey)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(SC_CHAT_INFO) + std::to_string(user_id);
    return redis_client->Hdel(table_name, strKey);
}

INT RedisUtil::DelUserOffLineListChatIds(USERID user_id)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(SC_CHAT_LIST_MSG) + std::to_string(user_id);
    return redis_client->Del(table_name);
}

INT RedisUtil::DelUserWebOffLineListChatIds(USERID user_id)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(SC_CHAT_WEB_LIST_MSG) + std::to_string(user_id);
    return redis_client->Del(table_name);
}

INT RedisUtil::RpushUserOfflineMsgId(USERID user_id, TEXTMSGID message_id, LONG& nRedisResult)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
        << " message_id = " << message_id);
    
    std::string table_name = std::string(SC_CHAT_LIST_MSG) + std::to_string(user_id);
    redis_client->Rpush(table_name, to_string(message_id), &nRedisResult);

    std::string table_web_name = std::string(SC_CHAT_WEB_LIST_MSG) + std::to_string(user_id);
    return redis_client->Rpush(table_web_name, to_string(message_id), &nRedisResult);
}

INT RedisUtil::LpopUserOfflineMsgId(USERID user_id, string& strRedisResult)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(SC_CHAT_LIST_MSG) + std::to_string(user_id);
    redis_client->Lpop(table_name, &strRedisResult);

    std::string table_web_name = std::string(SC_CHAT_WEB_LIST_MSG) + std::to_string(user_id);
    return redis_client->Lpop(table_web_name, &strRedisResult);
}

INT RedisUtil::LRangeUserOfflineMsgId(USERID user_id, std::vector<std::string>& vector_list_ids, INT count_del)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(SC_CHAT_LIST_MSG) + std::to_string(user_id);
    return redis_client->Lrange(table_name, 0, count_del, &vector_list_ids);
}

INT RedisUtil::LTrimUserOfflineMsgId(USERID user_id, INT count_del)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(SC_CHAT_LIST_MSG) + std::to_string(user_id);
    redis_client->Ltrim(table_name, count_del, -1);

    std::string table_web_name = std::string(SC_CHAT_WEB_LIST_MSG) + std::to_string(user_id);
    return redis_client->Ltrim(table_web_name, count_del, -1);
}

INT RedisUtil::LremUserOfflineMsgId(USERID user_id, TEXTMSGID message_id)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
        << " message_id = " << message_id);
    
    std::string table_name = std::string(SC_CHAT_LIST_MSG) + std::to_string(user_id);
    return redis_client->Lrem(table_name, 0, to_string(message_id));
}

INT RedisUtil::LremUserWebOfflineMsgId(USERID user_id, TEXTMSGID message_id)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
        << " message_id = " << message_id);
    
    std::string table_name = std::string(SC_CHAT_WEB_LIST_MSG) + std::to_string(user_id);
    return redis_client->Lrem(table_name, 0, to_string(message_id));
}

INT RedisUtil::LremEcOfflineMsgId(USERID user_id, TEXTMSGID message_id)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(EC_CHAT_LIST_MSG) + std::to_string(user_id);
    return redis_client->Lrem(table_name, 0, to_string(message_id));
}

INT RedisUtil::ZaddGroupOffLineChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
        << " dScore = " << dScore);
    
    std::string table_name = std::string(SC_GROUP_CHAT_INFO) + std::to_string(user_id);
    return redis_client->Zadd(table_name, dScore, strElem);	
}

INT RedisUtil::ZremrangebyrankGroupOffLineChatInfo(USERID user_id, long nStart, long nStop)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(SC_GROUP_CHAT_INFO) + std::to_string(user_id);
    return redis_client->Zremrangebyrank(table_name, nStart, nStop);	
}

INT RedisUtil::ZRrangeGroupOffLineChatInfoByScoreToMax(USERID group_id, unsigned long long message_id, INT nSpan, std::vector<std::string> *pvecVal)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(SC_GROUP_CHAT_INFO) + std::to_string(group_id);
    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "group_id = " << group_id << " message_id = " << (unsigned long long)message_id 
                                << " nSpan = " << nSpan << " table_name = " << table_name);
    return redis_client->Zrangebyscore(table_name, message_id, nSpan, pvecVal);	
}

INT RedisUtil::HUpdateUserTimeByType(USERID user_id, INT timetype, UINT64 time_update)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    int mod_index = user_id % LOGOUT_MOD_COUNT;
    std::string table_name = std::string(GC_GROUP_LAST_TIME) + std::to_string(mod_index);
    return redis_client->Hset(table_name, std::to_string(user_id), std::to_string(time_update));
}

INT RedisUtil::HGetUserTimeByType(USERID user_id, INT timetype, std::string& str_time)
{
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    int mod_index = user_id % LOGOUT_MOD_COUNT;
    std::string table_name = std::string(GC_GROUP_LAST_TIME) + std::to_string(mod_index);
    return redis_client->Hget(table_name, std::to_string(user_id), &str_time);
}

INT RedisUtil::SmembersGroupUserId(const UINT64& group_id, std::map<std::string, std::string>& pvecVal)
{
    auto redis_client = GetRedisClient(REDIS_GROUP_DB);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(GC_GROUP_USER_ID_SET) + std::to_string(group_id);
    return redis_client->Hgetall(table_name, &pvecVal);
}

INT RedisUtil::HexistGroupUserId(const UINT64& group_id, UINT64& user_id, long& result)
{
    auto redis_client = GetRedisClient(REDIS_GROUP_DB);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(GC_GROUP_USER_ID_SET) + std::to_string(group_id);
    return redis_client->Hexists(table_name, to_string(user_id), &result);
}

INT RedisUtil::RPushScChatInfoToDbBack(const INT& partition, const std::string& strChat, LONG& nRedisResult)
{
    auto redis_client = GetRedisClient(REDIS_ASYNC_DB);
    if(redis_client == nullptr)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "REDIS_ASYNC_DB is nil");
        return -1;
    }
    
    std::string table_name = std::string(SC_CHAT_TO_DB_BACK) + std::to_string(partition);
    return redis_client->Rpush(table_name, strChat, &nRedisResult);
}

INT RedisUtil::RPushEcChatInfoToDbBack(const INT& partition, const std::string& strChat, LONG& nRedisResult)
{
    auto redis_client = GetRedisClient(REDIS_ASYNC_DB);
    if(redis_client == nullptr)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "REDIS_ASYNC_DB is nil");
        return -1;
    }
    
    std::string table_name = std::string(EC_CHAT_TO_DB_BACK) + std::to_string(partition);
    return redis_client->Rpush(table_name, strChat, &nRedisResult);
}

INT RedisUtil::RPushGcChatInfoToDbBack(const INT& partition, const std::string& strChat, LONG& nRedisResult)
{
    auto redis_client = GetRedisClient(REDIS_ASYNC_DB);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(GC_CHAT_TO_DB_BACK) + std::to_string(partition);
    return redis_client->Rpush(table_name, strChat, &nRedisResult);
}

INT RedisUtil::HSetUserSingleChatStatus(USERID user_id, USERID op_user_id)
{   
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(SC_CHAT_SINGLE_STATUS) + std::to_string(user_id);
    return redis_client->Hset(table_name, to_string(op_user_id), "1");
}

INT RedisUtil::HDelUserSingleChatStatus(USERID user_id, USERID op_user_id)
{   
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(SC_CHAT_SINGLE_STATUS) + std::to_string(user_id);
    return redis_client->Hdel(table_name, to_string(op_user_id));
}

INT RedisUtil::HKeysUserSingleChatStatus(USERID user_id, vector<string>& vector_list_str_user_id)
{   
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(SC_CHAT_SINGLE_STATUS) + std::to_string(user_id);
    return redis_client->Hkeys(table_name, &vector_list_str_user_id);
}

INT RedisUtil::HGetUserSingleChatStatus(USERID user_id, USERID op_user_id, string& strResult)
{   
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(SC_CHAT_SINGLE_STATUS) + std::to_string(user_id);
    return redis_client->Hget(table_name, to_string(op_user_id), &strResult);
}

INT RedisUtil::HMsetGroupMsgId(UINT64& group_id, map<string, string>& mapGroupMsgId)
{   
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(GC_GROUP_MSG_ID) + std::to_string(group_id);
    return redis_client->Hmset(table_name, mapGroupMsgId);
}

INT RedisUtil::HgetGroupMsgId(UINT64& group_id, UINT64& user_id, string& strResult)
{   
    auto redis_client = GetRedisClient(REDIS_MESSAGE);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(GC_GROUP_MSG_ID) + std::to_string(group_id);
    return redis_client->Hget(table_name, to_string(user_id), &strResult);
}
