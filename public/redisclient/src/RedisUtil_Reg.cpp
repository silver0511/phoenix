//
// Created by shiyunjie on 2018/1/8.
//

#include "redisclient/RedisUtil.h"
#include "common/basefun.h"


INT RedisUtil::SetAccountVerityCode(uint64 account_type, const std::string target_user_id, std::string verify_info)
{
    auto redis_client = GetRedisClient(REDIS_REG);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(PRE_NIM_ACCOUNT_VERITY_CODE) + std::to_string(account_type) + "_" + target_user_id;

    return redis_client->Set(table_name, verify_info);
}


std::string RedisUtil::GetAccountVerityInfo(uint64 account_type, const std::string target_user_id)
{
    auto redis_client = GetRedisClient(REDIS_REG);
    if(redis_client == nullptr)
    {
        return "";
    }

    std::string table_name = std::string(PRE_NIM_ACCOUNT_VERITY_CODE) + std::to_string(account_type) + "_" + target_user_id;

    std::string result = "";
    redis_client->Get(table_name, &result);
    return result;
}

INT RedisUtil::HSetAccountUser(uint64 account_type, const std::string target_user_id, USERID user_id)
{
    auto redis_client = GetRedisClient(REDIS_REG);
    if(redis_client == nullptr)
    {
        return 1;
    }

    int mod_index = get_asc_mod(target_user_id, ACCOUNT_MOD_COUNT);
    std::string table_name = std::string(PRE_NIM_ACCOUNT_USER_HASH) + std::to_string(account_type) + "_" + to_string(mod_index);
    return redis_client->Hset(table_name, target_user_id, to_string(user_id));
}

string RedisUtil::HGetAccountUser(uint64 account_type, const std::string target_user_id)
{
    auto redis_client = GetRedisClient(REDIS_REG);
    if(redis_client == nullptr)
    {
        return "";
    }

    int mod_index = get_asc_mod(target_user_id, ACCOUNT_MOD_COUNT);
    std::string table_name = std::string(PRE_NIM_ACCOUNT_USER_HASH) + std::to_string(account_type) + "_" + to_string(mod_index);

    std::string result = "";
    redis_client->Hget(table_name, target_user_id, &result);
    return result;
}

INT RedisUtil::ExpireAaccountVerityCode(uint64 account_type, const std::string target_user_id, long expire_time)
{
    auto redis_client = GetRedisClient(REDIS_REG);
    if(redis_client == nullptr)
    {
        return -1;
    }

    std::string table_name = std::string(PRE_NIM_ACCOUNT_VERITY_CODE) + std::to_string(account_type) + "_" + target_user_id;

    return redis_client->Expire(table_name, expire_time);
}

string RedisUtil::SPopUserID()
{
    auto redis_client = GetRedisClient(REDIS_REG);
    if(redis_client == nullptr)
    {
        return "";
    }

    std::string table_name = std::string(NIM_GEN_USER_POOL);

    string str_user_id;
    redis_client->Spop(table_name, &str_user_id);
    return str_user_id;
}

INT RedisUtil::HSetRegPassWord(USERID user_id, string password_info)
{
    auto redis_client = GetRedisClient(REDIS_REG);
    if(redis_client == nullptr)
    {
        return -1;
    }


    int mod_index = user_id % LOGIN_MOD_COUNT;
    std::string table_name = std::string(PRE_NIM_REG_PWD_HASH) + to_string(mod_index);

    std::string str_user_id = std::to_string(user_id);
    redis_client->Hset(table_name, str_user_id, password_info);
    return redis_client->Hset(table_name, str_user_id, password_info);
}

string RedisUtil::HGetRegPassWord(USERID user_id)
{
    auto redis_client = GetRedisClient(REDIS_REG);
    if(redis_client == nullptr)
    {
        return "";
    }


    int mod_index = user_id % LOGIN_MOD_COUNT;
    std::string table_name = std::string(PRE_NIM_REG_PWD_HASH) + to_string(mod_index);

    string password_info = "";
    std::string str_user_id = std::to_string(user_id);
    redis_client->Hget(table_name, str_user_id, &password_info);
    return str_user_id;
}
