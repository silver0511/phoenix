#include "redisclient/RedisUtil.h"


INT RedisUtil::ZRrangeOffLineFanInfo(USERID user_id, long nStart, long nStop, std::vector<std::string> *pvecVal)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string table_name = std::string(SC_FANS_CHAT_INFO) + std::to_string(user_id);
	return redis_client->Zrange(table_name, nStart, nStop, pvecVal);
}

INT RedisUtil::LpushOffcialMsgdb(USERID user_id,const std::string &strElem, LONG& nRedisResult)
{
	auto redis_client = GetRedisClient(REDIS_ASYNC_DB);
	if(redis_client == nullptr)
	{
		return -1;
	}
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
		<< " strElem = " << strElem);

	int platform = user_id % SC_OFFCIAL_MOD_COUNT;
	std::string table_name = std::string(SC_OFFCIAL_CHAT_INFO_DB) + std::to_string(platform);
	return redis_client->Lpush(table_name, strElem, &nRedisResult);
}

INT RedisUtil::LpushFanMsgdb(USERID user_id, const std::string &strElem, LONG& nRedisResult)
{
	auto redis_client = GetRedisClient(REDIS_ASYNC_DB);
	if(redis_client == nullptr)
	{
		return -1;
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
		<< " strElem = " << strElem);
	int platform = user_id % SC_OFFCIAL_MOD_COUNT;
	std::string table_name = std::string(SC_FANS_CHAT_INFO_DB) + std::to_string(platform);
	return redis_client->Lpush(table_name, strElem, &nRedisResult);
}

INT RedisUtil::ZaddOffcialOffLineChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
		<< " dScore = " << dScore);

	std::string table_name = std::string(SC_OFFCIAL_CHAT_INFO) + std::to_string(user_id);
	return redis_client->Zadd(table_name, dScore, strElem);	
}

INT RedisUtil::ZaddOffcialOffLinePrivateChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
		<< " dScore = " << dScore);

	std::string table_name = std::string(SC_OFFCIAL_PRIVATECHAT_INFO) + std::to_string(user_id);
	return redis_client->Zadd(table_name, dScore, strElem);	
}

INT RedisUtil::ZaddOffcialSysChatInfo(unsigned long long dScore, const std::string &strElem)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " dScore = " << strElem );
	std::string table_name = std::string(SC_OFFCIAL_SYS_INFO);
	return redis_client->Zadd(table_name, dScore, strElem);	
}

INT RedisUtil::ZaddFansOffLineChatInfo(USERID user_id, unsigned long long dScore, const std::string &strElem)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
		<< " dScore = " << dScore);

	std::string table_name = std::string(SC_FANS_CHAT_INFO) + std::to_string(user_id);
	return redis_client->Zadd(table_name, dScore, strElem);	
}

INT RedisUtil::ZaddFansSysMsgInfo(USERID user_id, unsigned long long dScore, const std::string &strElem)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << user_id 
		<< " dScore = " << dScore);

	int platform = user_id % USERSERVER_MOD_COUNT;
	std::string table_name = std::string(SC_FANS_SYS_INFO) + std::to_string(platform);
	return redis_client->Zadd(table_name, dScore, strElem);	
}

INT RedisUtil::ZRrangeOffcialOffLineChatInfoByScoreToMax(USERID group_id, unsigned long long message_id, INT nSpan, std::vector<std::string> *pvecVal)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string table_name = std::string(SC_OFFCIAL_CHAT_INFO) + std::to_string(group_id);
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "group_id = " << group_id << " message_id = " << (unsigned long long)message_id 
		<< " nSpan = " << nSpan << " table_name = " << table_name);
	return redis_client->Zrangebyscore(table_name, message_id, nSpan, pvecVal);	
}

INT RedisUtil::ZRrangeOffcialSysMsgByScoreToMax(unsigned long long message_id, INT nSpan, std::vector<std::string> *pmapVal)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}

	std::string table_name = std::string(SC_OFFCIAL_SYS_INFO);
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " message_id = " << (unsigned long long)message_id 
		<< " nSpan = " << nSpan << " table_name = " << table_name);
	return redis_client->Zrangebyscore(table_name, message_id, nSpan, pmapVal);	

}

INT RedisUtil::ZremrangebyrankSysMsg(const std::string &strKey,long nStart, long nStop)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}

	return redis_client->Zremrangebyrank(strKey, nStart, nStop);	
}

INT RedisUtil::ZremFansOffLineChatInfobyscore(USERID user_id, unsigned long long dMin, unsigned long long dMax)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}
	std::string table_name = std::string(SC_FANS_CHAT_INFO) + std::to_string(user_id);
	return redis_client->Zremrangebyscore(table_name, dMin,dMax);	
}

INT RedisUtil::ZremFansOffLinePrivateChatInfobyscore(USERID user_id, unsigned long long dMin, unsigned long long dMax)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}
	std::string table_name = std::string(SC_OFFCIAL_PRIVATECHAT_INFO) + std::to_string(user_id);
	return redis_client->Zremrangebyscore(table_name, dMin,dMax);	
}

INT RedisUtil::HSetFansLst(const std::string &data_id ,const std::map<std::string, std::string> &mapFv)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}
	std::string table_name = std::string(SC_FANS_SYS_INFO) + data_id;
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " ---SC_FANS_SYS_INFO table_name = " << table_name);
	return redis_client->Hmset(table_name, mapFv);
}

std::string RedisUtil::HCheckFansId(const std::string &data_id ,USERID fans_id)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return "";
	}
	std::string table_name = std::string(SC_FANS_SYS_INFO) + data_id;
	std::string str_user_id = std::to_string(fans_id);
	std::string result_val = "";
	redis_client->Hget(table_name, str_user_id, &result_val);
	return  result_val;
}


INT RedisUtil::HSetFansLst(USERID user_id,const std::string &data_id ,const std::map<std::string, std::string> &mapFv)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}
	std::string table_name = std::string(SC_OFFCIAL_CHAT_INFO)+  std::to_string(user_id) + "_" + data_id;
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " ---HSetFans table_name = " << table_name);

	return redis_client->Hmset(table_name, mapFv);
}

std::string RedisUtil::HCheckFansId(USERID user_id,const std::string &data_id ,USERID fans_id)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return "";
	}
	std::string table_name = std::string(SC_OFFCIAL_CHAT_INFO)+  std::to_string(user_id) +"_"+ data_id;
	std::string str_user_id = std::to_string(fans_id);
	std::string result_val = "";
	redis_client->Hget(table_name, str_user_id, &result_val);
	return  result_val;
}


INT RedisUtil::ZRrangeOffcialOneSysMsgByScoreToMax(USERID user_id, unsigned long long message_id, INT nSpan, std::vector<std::string> *pvecVal)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}
	int platform = user_id % USERSERVER_MOD_COUNT;
	std::string table_name = std::string(SC_FANS_SYS_INFO) + std::to_string(platform);

	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " user_id = " << (unsigned long long)message_id 
		<< " nSpan = " << nSpan << " table_name = " << table_name);
	return redis_client->Zrangebyscore(table_name, message_id, nSpan, pvecVal);	
}

INT RedisUtil::ZremFansOneSysMsgbyscore(USERID user_id, unsigned long long dMin, unsigned long long dMax)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}
	int platform = user_id % USERSERVER_MOD_COUNT;
	std::string table_name = std::string(SC_FANS_SYS_INFO) + std::to_string(platform);
	return redis_client->Zremrangebyscore(table_name, dMin,dMax);	
}

INT RedisUtil::ZRrangeOffcialOffLinePrivateChatInfo(USERID user_id, unsigned long long message_id, INT nSpan, std::vector<std::string> *pvecVal)
{
	auto redis_client = GetRedisClient(REDIS_MESSAGE);
	if(redis_client == nullptr)
	{
		return -1;
	}
	std::string table_name = std::string(SC_OFFCIAL_PRIVATECHAT_INFO) + std::to_string(user_id);
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "group_id = " << user_id << " message_id = " << (unsigned long long)message_id 
		<< " nSpan = " << nSpan << " table_name = " << table_name);
	return redis_client->Zrangebyscore(table_name, message_id, nSpan, pvecVal);	
}
