#include "redisclient/RedisUtil.h"

INT RedisUtil::ZaddFdMsg(USERID user_id, unsigned long long ultoken, const std::string &strElem)
{
    auto redis_client = GetRedisClient(REDIS_FRIEND);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(FD_RELEATION_MSG_KYE) + std::to_string(user_id);
    return redis_client->Zadd(table_name, ultoken, strElem);
}

INT RedisUtil::ZRrangeFdMsgByScore(USERID user_id, unsigned long long ultoken, INT nSpan, std::map<std::string, std::string> *pmapVal)
{
    auto redis_client = GetRedisClient(REDIS_FRIEND);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(FD_RELEATION_MSG_KYE) + std::to_string(user_id);
    return redis_client->ZrangeWithScorebySpan(table_name, ultoken, nSpan, pmapVal);	
}

INT RedisUtil::ZcountFdMsg(USERID user_id, long & nCount)
{
    auto redis_client = GetRedisClient(REDIS_FRIEND);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(FD_RELEATION_MSG_KYE) + std::to_string(user_id);
    return redis_client->Zcount(table_name, 0, 9999999999, &nCount);
}

INT RedisUtil::ZremrangebyrankFdMsg(USERID user_id, long nStart, long nStop)
{
    auto redis_client = GetRedisClient(REDIS_FRIEND);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(FD_RELEATION_MSG_KYE) + std::to_string(user_id);
    return redis_client->Zremrangebyrank(table_name, nStart, nStop);
}

INT RedisUtil::SetFdRequestTime(USERID user_id, USERID friend_id, const std::string & expiretime)
{
    auto redis_client = GetRedisClient(REDIS_FRIEND);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(FD_CONFIRM_EXPIR_KEY) + std::to_string(user_id);
    std::string filed = std::to_string(friend_id);
    return redis_client->Hset(table_name, filed, expiretime);
}

INT RedisUtil::GetFdRequestTime(USERID user_id, USERID friend_id,std::string & expiretime)
{
    auto redis_client = GetRedisClient(REDIS_FRIEND);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(FD_CONFIRM_EXPIR_KEY) + std::to_string(user_id);
    std::string filed = std::to_string(friend_id);
    return redis_client->Hget(table_name, filed, &expiretime);
}


INT RedisUtil::DelFdRequestTime(USERID user_id, USERID friend_id)
{
    auto redis_client = GetRedisClient(REDIS_FRIEND);
    if(redis_client == nullptr)
    {
        return -1;
    }
    
    std::string table_name = std::string(FD_CONFIRM_EXPIR_KEY) + std::to_string(user_id);
    std::string filed = std::to_string(friend_id);
    return redis_client->Hdel(table_name, filed);
}

