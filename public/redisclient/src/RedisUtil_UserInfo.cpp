#include "redisclient/RedisUtil.h"
INT RedisUtil::HSetUserInfo(USERID user_id, int platform, const std::string &session)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string table_name = std::string(PRE_USERINFO_LST) + std::to_string(platform);
	std::string str_user_id = std::to_string(user_id);

	return redis_client->Hset(table_name, str_user_id, session);
}

std::string RedisUtil::HGetUserInfo(USERID user_id, int platform)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return "";
	}

	std::string table_name = std::string(PRE_USERINFO_LST) + std::to_string(platform);
	std::string str_user_id = std::to_string(user_id);

	std::string result_val = "";
	redis_client->Hget(table_name, str_user_id, &result_val);
	return  result_val;
}

INT RedisUtil::HSetUserComplaintInfo(USERID user_id, int platform, const std::string &session)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string table_name = std::string(PRE_COMPLAINT_LST) + std::to_string(platform);
	std::string str_user_id = std::to_string(user_id);

	return redis_client->Hset(table_name, str_user_id, session);
}

std::string RedisUtil::HGetUserComplaintInfo(USERID user_id, int platform)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return "";
	}

	std::string table_name = std::string(PRE_COMPLAINT_LST) + std::to_string(platform);
	std::string str_user_id = std::to_string(user_id);

	std::string result_val = "";
	redis_client->Hget(table_name, str_user_id, &result_val);
	return  result_val;
}

INT RedisUtil::HSetTokenInfo(const std::string &data_id , const std::string &session,int platform)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}
	std::string table_name = std::string(PRE_TOKEN_LST) + std::to_string(platform);
	return redis_client->Hset(table_name, data_id, session);
}

std::string RedisUtil::LuaCheckUserMsg(const std::string &mobile,const std::string &name,const std::string &mail)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return "";
	}

	std::string sha_key = mLuaShaManager.Getluasha(LUA_USER_MANAGER);
	if(sha_key.empty())
	{
		return "";
	}

	std::string result_val = "";
	std::string wert;
	char filter[200];
	sprintf(filter,"4 %d %s %s %s",USERSERVER_MOD_COUNT,mobile.c_str(),name.c_str(),mail.c_str()); 
	wert=filter;

	int rtn = redis_client->Evalsha(wert, sha_key, &result_val);
	return  result_val;
}

std::string RedisUtil::LuaHGetUserInfo(const std::string &user_msg)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return "";
	}

	std::string sha_key = mLuaShaManager.Getluasha(LUA_USER_INFO);
	if(sha_key.empty())
	{
		return "";
	}
	std::string result_val = "";
	std::string wert;
	char filter[200];
	sprintf(filter,"1 %d %s",USERSERVER_MOD_COUNT,user_msg.c_str()); 
	wert=filter;
	int rtn = redis_client->Evalsha(wert, sha_key, &result_val);
	return  result_val;
}

int RedisUtil::LuaHGetUserLstByMobile(const std::vector<std::string> &user_lst, std::vector<std::string> *pvecVal)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string sha_key = mLuaShaManager.Getluasha(LUA_USER_LST_BYMOBILE);
	if(sha_key.empty())
	{
		return -1;
	}

	int nom = user_lst.size()+2;
	std::string luastr;
	char filter[200];
	sprintf(filter,"%d %d %d ",nom,nom,USERSERVER_MOD_COUNT); 
	luastr=filter;
	for(int i = 0; i < user_lst.size(); i++)
	{
		std::string str_user_id = user_lst[i];
		std::string wert = str_user_id + " ";
		luastr+=wert;
	}
	int rtn = redis_client->Evalsha(luastr, sha_key, pvecVal);
	return  rtn;
}


int RedisUtil::LuaHGetUserLst(const std::vector<std::string> &user_lst, std::vector<std::string> *pvecVal)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string sha_key = mLuaShaManager.Getluasha(LUA_USER_LST);
	if(sha_key.empty())
	{
		return -1;
	}

	int nom = user_lst.size()+2;
	std::string luastr;
	char filter[200];
	sprintf(filter,"%d %d %d ",nom,nom,USERSERVER_MOD_COUNT);
	luastr=filter;
	for(int i = 0; i < user_lst.size(); i++)
	{
		std::string str_user_id = user_lst[i];
		std::string wert = str_user_id + " ";
		luastr+=wert;
	}
	int rtn = redis_client->Evalsha(luastr, sha_key, pvecVal);
	return  rtn;
}

int RedisUtil::LuaHAddUserInfo(USERID user_id, const std::string &session,const std::string &mobile ,const std::string &name,const std::string &mail)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string sha_key = mLuaShaManager.Getluasha(LUA_USER_SET);
	if(sha_key.empty())
	{
		return -1;
	}

	std::string luastr;
	char filter[200];
	sprintf(filter,"6 %d %d %s %s %s ",USERSERVER_MOD_COUNT,user_id,mobile.c_str(),name.c_str(),mail.c_str()); 
	luastr=filter;

	luastr += session;
	std::string result_val = "";

	int rtn = redis_client->Evalsha(luastr, sha_key, &result_val);
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, " rtn=" << rtn << " result_val=" << result_val);
	return rtn;
}

int RedisUtil::LuaChangeMobile(USERID user_id, const std::string &session,const std::string &new_mobile,const std::string &old_mobile)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string sha_key = mLuaShaManager.Getluasha(LUA_CHANGE_MOBILE);
	if(sha_key.empty())
	{
		return -1;
	}

	std::string luastr;
	char filter[200];
	sprintf(filter,"5 %d %d %s %s ",USERSERVER_MOD_COUNT,user_id,new_mobile.c_str(),old_mobile.c_str()); 
	luastr=filter;

	luastr += session;
	std::string result_val = "";
	int rtn = redis_client->Evalsha(luastr, sha_key, &result_val);
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, " rtn=" << rtn << " result_val=" << result_val);
	return rtn;
}

int RedisUtil::LuaChangeMail(USERID user_id, const std::string &session,const std::string &new_mail,const std::string &old_mail)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string sha_key = mLuaShaManager.Getluasha(LUA_CHANGE_MAIL);
	if(sha_key.empty())
	{
		return -1;
	}

	std::string luastr;
	char filter[200];
	sprintf(filter,"5 %d %d %s %s ",USERSERVER_MOD_COUNT,user_id,new_mail.c_str(),old_mail.c_str()); 
	luastr=filter;

	luastr += session;
	std::string result_val = "";
	int rtn = redis_client->Evalsha(luastr, sha_key, &result_val);
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, " rtn=" << rtn << " result_val=" << result_val);
	return rtn;
}


INT RedisUtil::HSetDeviceToken(USERID user_id, const std::string &device_token)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string table_name = std::string(DEVICE_TOKEN_MAP);
	std::string str_user_id = std::to_string(user_id);

	return redis_client->Hset(table_name, str_user_id, device_token);
}

INT RedisUtil::HDelDeviceToken(USERID user_id)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string table_name = std::string(DEVICE_TOKEN_MAP);
	std::string str_user_id = std::to_string(user_id);

	return redis_client->Hdel(table_name, str_user_id);
}

std::string RedisUtil::HGetDeviceToken(USERID user_id)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return "";
	}

	std::string table_name = std::string(DEVICE_TOKEN_MAP);
	std::string str_user_id = std::to_string(user_id);
	std::string result_val = "";
	redis_client->Hget(table_name, str_user_id, &result_val);
	return result_val;
}

std::string RedisUtil::HGetDeviceUser(const std::string &device_token)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return "";
	}

	std::string table_name = std::string(DEVICE_TOKEN_REVERT_MAP);
	std::string result_val = "";
	redis_client->Hget(table_name, device_token, &result_val);
	return result_val;
}

INT RedisUtil::HSetDeviceUser(const std::string &device_token, USERID user_id)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string table_name = std::string(DEVICE_TOKEN_REVERT_MAP);
	std::string str_user_id = std::to_string(user_id);
	return redis_client->Hset(table_name, device_token, str_user_id);
}


INT RedisUtil::HDelDeviceUser(const std::string &device_token)
{
	auto redis_client = GetRedisClient(REDIS_USER);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string table_name = std::string(DEVICE_TOKEN_REVERT_MAP);
	return redis_client->Hdel(table_name, device_token);
}