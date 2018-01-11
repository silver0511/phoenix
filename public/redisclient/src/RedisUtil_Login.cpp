#include "redisclient/RedisUtil.h"
INT RedisUtil::HSetLoginInfo(USERID user_id, int8 platform, const std::string &session)
{
    auto redis_client = GetRedisClient(REDIS_LOGIN);
    if(redis_client == nullptr)
    {
        return -1;
    }

    int mod_index = user_id % LOGIN_MOD_COUNT;
    std::string table_name = std::string(PRE_NIM_LOGIN_HASH) + std::to_string(mod_index) + "_" + std::to_string(platform);
    std::string str_user_id = std::to_string(user_id);

    return redis_client->Hset(table_name, str_user_id, session);
}


INT RedisUtil::HDelLoginInfo(USERID user_id, int8 platform)
{
    auto redis_client = GetRedisClient(REDIS_LOGIN);
    if(redis_client == nullptr)
    {
        return -1;
    }
    int mod_index = user_id % LOGIN_MOD_COUNT;
    std::string table_name = std::string(PRE_NIM_LOGIN_HASH) + std::to_string(mod_index) + "_" + std::to_string(platform);
    std::string str_user_id = std::to_string(user_id);


    return redis_client->Hdel(table_name, str_user_id);
}

std::string RedisUtil::HGetLoginInfo(USERID user_id, int8 platform)
{
    auto redis_client = GetRedisClient(REDIS_LOGIN);
    if(redis_client == nullptr)
    {
        return "";
    }
    int mod_index = user_id % LOGIN_MOD_COUNT;
    std::string table_name = std::string(PRE_NIM_LOGIN_HASH) + std::to_string(mod_index) + "_" + std::to_string(platform);
    std::string str_user_id = std::to_string(user_id);


    std::string result_val = "";
    redis_client->Hget(table_name, str_user_id, &result_val);
    return  result_val;
}

INT RedisUtil::HGetLoginInfo(std::vector<std::string>& userids, std::map<std::string, std::string>&values, int mod_index, int8 platform)
{
	auto redis_client = GetRedisClient(REDIS_LOGIN);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(PRE_NIM_LOGIN_HASH) + std::to_string(mod_index) + "_" + std::to_string(platform);
    return redis_client->Hmget(table_name, userids, &values);		
}

INT RedisUtil::HGetLoginInfo(std::vector<std::string>& userids, std::vector<std::string>&values, int mod_index, int8 platform)
{
	auto redis_client = GetRedisClient(REDIS_LOGIN);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(PRE_NIM_LOGIN_HASH) + std::to_string(mod_index) + "_" + std::to_string(platform);
    return redis_client->Hmget(table_name, userids, &values);		
}

INT RedisUtil::RPushLoginDetail(USERID user_id, std::string str_login)
{
    auto redis_client = GetRedisClient(REDIS_ASYNC_DB);
    if(redis_client == nullptr)
    {
        return -1;
    }

    int mod_index = user_id % LOGIN_DETAIL_MOD_COUNT;
    std::string table_name = std::string(PRE_NIM_LOGIN_DETAIL_LIST) + std::to_string(mod_index);
    std::string str_user_id = std::to_string(user_id);

    return redis_client->Rpush(table_name, str_login);
}

string RedisUtil::LuaGetLoginServer(const string &json_user_ids, int8 platform)
{
    auto redis_client = GetRedisClient(REDIS_LOGIN);
    if(redis_client == nullptr)
    {
        return "";
    }

    std::string sha_key = mLuaShaManager.Getluasha(LUA_LOGIN_SERVER);
    if(sha_key.empty())
    {
        return "";
    }

    std::string result_val = "";
    int key_count = 4;
    std::string str_key = std::to_string(key_count) + " " + PRE_NIM_LOGIN_HASH + " " +
    std::to_string(platform) + " " + std::to_string(LOGIN_MOD_COUNT) + " " + json_user_ids;

    int rtn = redis_client->Evalsha(str_key, sha_key, &result_val);
    return  result_val;
}

INT RedisUtil::HsanLogininfo(int mod_index, int platform, std::string strCursor, long nCount, std::map<std::string, std::string> *pmapVal)
{
    auto redis_client = GetRedisClient(REDIS_LOGIN);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string str_key =  std::string(PRE_NIM_LOGIN_HASH) + std::to_string(mod_index) + "_" + std::to_string(platform);

    return redis_client->Hsan(str_key, strCursor, nCount, pmapVal);     
}