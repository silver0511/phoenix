#include <fstream> 
#include <sstream>
#include "redisclient/RedisUtil.h"

INT RedisUtil::GetTradeLastWid(USERID ulcid, USERID ulbid, std::string& szwid)
{
    std::string szkey = EC_LASE_WAIT_KEY + std::to_string(ulcid) + "_" + std::to_string(ulbid);
    std::string szfiled = "waitid";

    auto redis_client = GetRedisClient(REDIS_TRADE);
    if(redis_client == nullptr)
    {
        return -1;
    }    

    redis_client->Hget(szkey, szfiled, &szwid);
    return 0;
}

INT RedisUtil::GetTradeLastSessionId(USERID ulcid, USERID ulbid, std::string & szsessionid)
{
    std::string szkey = EC_LASE_WAIT_KEY + std::to_string(ulcid) + "_" + std::to_string(ulbid);
    std::string szfiled = "sessionid";

    auto redis_client = GetRedisClient(REDIS_TRADE);
    if(redis_client == nullptr)
    {
        return -1;
    }    

    redis_client->Hget(szkey, szfiled, &szsessionid);
    return 0;    
}

INT RedisUtil::SetTradeLastWid(USERID ulcid, USERID ulbid, USERID ulwid , int sessionid)
{
    auto redis_client = GetRedisClient(REDIS_TRADE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string sha_key = mLuaShaManager.Getluasha(LUA_EC_SETLASTWID);
    if(sha_key.empty())
    {
        return -1;
    }
    std::string result_val = "";
    std::string szarg = "4 " + std::to_string(ulcid) + " " + std::to_string(ulbid) + " " + std::to_string(ulwid) + " " + std::to_string(sessionid);
    return redis_client->Evalsha(szarg, sha_key, &result_val);
}

INT RedisUtil::GetRecentCidList(const std::string &strKey, std::vector<std::string>& recentcidlist)
{
    std::string strkeytmp = EC_RECENT_USER_KEY + strKey;

    auto redis_client = GetRedisClient(REDIS_TRADE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    return redis_client->Lrange(strkeytmp, 0 , -1, &recentcidlist);
}

INT RedisUtil::SetBusinessInfo(USERID ulbid, const std::string& strinfo)
{
    std::string strkeytmp = EC_BUSINESS_INFO;
    std::string strfield =  std::to_string(ulbid);
    
    auto redis_client = GetRedisClient(REDIS_TRADE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    return redis_client->Hset(strkeytmp, strfield, strinfo);
}

INT RedisUtil::GetBusinessInfo(USERID ulbid, std::string& strinfo)
{
    std::string strkeytmp = EC_BUSINESS_INFO;
    std::string strfield =  std::to_string(ulbid);

    auto redis_client = GetRedisClient(REDIS_TRADE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    return redis_client->Hget(strkeytmp, strfield, &strinfo);
}

INT RedisUtil::SetBusinessPrompts(const std::map<std::string, std::string>& filed_value)
{
    std::string strkeytmp = EC_BUSINESS_PROMPTS;
    

    auto redis_client = GetRedisClient(REDIS_TRADE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    return redis_client->Hmset(strkeytmp, filed_value);
}

INT RedisUtil::GetBusinessPrompts(const std::set<std::string> &setField, std::map<std::string, std::string>& mapVal)
{
    std::string strkeytmp = EC_BUSINESS_PROMPTS;

    auto  redis_client = GetRedisClient(REDIS_TRADE);
    if (redis_client == nullptr)
    {
        return -1;
    }

    return redis_client->Hmget(strkeytmp, setField, &mapVal);
}

INT RedisUtil::GetBusinessPrompts(USERID ulbid, int ipromptstype, std::string & strprompts)
{
    std::string strkeytmp = EC_BUSINESS_PROMPTS;
    std::string strfield = std::to_string(ulbid) + "_" + std::to_string(ipromptstype);

    auto redis_client = GetRedisClient(REDIS_TRADE);
    if(redis_client == nullptr)
    {
        return -1;
    }

    return redis_client->Hget(strkeytmp, strfield, &strprompts);
}

