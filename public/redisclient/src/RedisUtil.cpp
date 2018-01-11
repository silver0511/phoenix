#include "redisclient/RedisUtil.h"

RedisUtil::RedisUtil()
{
	for(int index = 0; index < REDIS_COUNT; ++index)
	{
		mRedisClientList[index] = nullptr;
	}

	//init lua file map
	//login
	mLuaFileMap[REDIS_LOGIN].push_back(LUA_LOGIN_SERVER);
	//user
	mLuaFileMap[REDIS_USER].push_back(LUA_USER_INFO);
	mLuaFileMap[REDIS_USER].push_back(LUA_USER_MANAGER);
	mLuaFileMap[REDIS_USER].push_back(LUA_USER_LST);
	mLuaFileMap[REDIS_USER].push_back(LUA_USER_SET);
	mLuaFileMap[REDIS_USER].push_back(LUA_USER_LST_BYMOBILE);
	mLuaFileMap[REDIS_USER].push_back(LUA_CHANGE_MAIL);
	mLuaFileMap[REDIS_USER].push_back(LUA_CHANGE_MOBILE);
	//ec
	mLuaFileMap[REDIS_TRADE].push_back(LUA_EC_SETLASTWID);
}

RedisUtil::~RedisUtil()
{
	for(int index = 0; index < REDIS_COUNT; ++index)
	{
		mRedisClientList[index] = nullptr;
	}
}

INT RedisUtil::Open(std::vector<ENUM_REDIS_TYPE> open_redis_list)
{
	if(-1 == moRedisConfig.Load(DEF_REDIS_INI))
	{
		LOG_TRACE(LOG_CRIT,false,__FUNCTION__," moRedisConfig.Load Fail");
		return -1;
	}

	for(int index = 0; index < open_redis_list.size(); ++index)
	{
		ENUM_REDIS_TYPE redis_type = open_redis_list.at(index);
		if(redis_type >= moRedisConfig.moRedisInfoList.size() ||
				redis_type < 0 ||
				redis_type >= REDIS_COUNT)
		{
			LOG_TRACE(LOG_CRIT,false,__FUNCTION__,"invalid redis_type: " << redis_type
			 	<< " size = " << moRedisConfig.moRedisInfoList.size());
			return -1;
		}
		STRU_REDIS_INFO &redis_info = moRedisConfig.moRedisInfoList.at(redis_type);
		if(redis_info.Check() < 0)
		{
			LOG_TRACE(LOG_CRIT,false,__FUNCTION__,"invalid redis info redis_type: " << redis_type);
			return -1;
		}

		mRedisClientList[redis_type] = new CRedisClient();
		if (nullptr == mRedisClientList[redis_type])
		{
			LOG_TRACE(LOG_CRIT,false,__FUNCTION__,"invalid redis init failed redis_type: " << redis_type);
			return -1;
		}

		if (!mRedisClientList[redis_type]->Initialize(redis_info.mstrIP,
													   redis_info.mwPort,
													   redis_info.miTimeOut,
													   redis_info.miMaxCnt))
		{
			LOG_TRACE(LOG_CRIT,false,__FUNCTION__," Error Initialize open failed" << " redis_type = " << redis_type);
			return -1;
		}
	}


    //init lua sha
    for(int index = 0; index < open_redis_list.size(); ++index)
    {
        ENUM_REDIS_TYPE redis_type = open_redis_list.at(index);
		ITER_LUA_FILE iter = mLuaFileMap.find(redis_type);
		if(iter != mLuaFileMap.end())
		{
			vector<string> &lua_file_list = iter->second;
			for(int file_index = 0; file_index < lua_file_list.size(); ++file_index)
			{
				ScriptLoadFile(redis_type, lua_file_list.at(file_index));
			}
		}
    }
    return 1;
}

bool RedisUtil::ScriptLoadFile(ENUM_REDIS_TYPE redis_type, const string &config_file)
{
	auto redis_client = GetRedisClient(redis_type);
	if(redis_client == nullptr)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "invalid redis_type: " << redis_type);
		return false;
	}

	int buf_len = 0;
	unsigned char *lua_content = ServerUtilMgr::GetData(config_file, true, buf_len);
	std::string str_lua_content =std::string((const char *)lua_content);
	std::string lua_sha = "";
	int result = redis_client->Scriptload(str_lua_content, &lua_sha);
	if(result < 0)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "invalid config_file: " << config_file << " redis_type: " << redis_type);
		return false;
	}

	mLuaShaManager.Addluasha(config_file, lua_sha);
	free(lua_content);
	return true;
}

INT RedisUtil::Close()
{

}

CRedisClient *RedisUtil::GetRedisClient(ENUM_REDIS_TYPE redis_type)
{
	if(redis_type < 0 ||
	   redis_type >= REDIS_COUNT)
	{
		LOG_TRACE(LOG_CRIT,false,__FUNCTION__,"invalid redis_type: " << redis_type);
		return nullptr;
	}

	return mRedisClientList[redis_type];
}

RedisConfig::RedisConfig()
{
	moRedisInfoList.reserve(RedisUtil::REDIS_COUNT);
}

RedisConfig::~RedisConfig()
{

}

BOOL RedisConfig::Read()
{
	//PS顺序必须一致
    STRU_REDIS_INFO redis_info;

    //login
	moIniFile.GetValue("ip","LOGIN",redis_info.mstrIP, "127.0.0.1");
	moIniFile.GetValue("port","LOGIN",redis_info.mwPort, 0);
	moIniFile.GetValue("timeout","LOGIN",redis_info.miTimeOut, 60);
	moIniFile.GetValue("maxcnt","LOGIN",redis_info.miMaxCnt, 1);
    moRedisInfoList.push_back(redis_info);

	//reg
	moIniFile.GetValue("ip","REG",redis_info.mstrIP, "127.0.0.1");
	moIniFile.GetValue("port","REG",redis_info.mwPort, 0);
	moIniFile.GetValue("timeout","REG",redis_info.miTimeOut, 60);
	moIniFile.GetValue("maxcnt","REG",redis_info.miMaxCnt, 1);
	moRedisInfoList.push_back(redis_info);

    //user
	moIniFile.GetValue("ip","USER",redis_info.mstrIP, "127.0.0.1");
	moIniFile.GetValue("port","USER",redis_info.mwPort, 0);
	moIniFile.GetValue("timeout","USER",redis_info.miTimeOut, 60);
	moIniFile.GetValue("maxcnt","USER",redis_info.miMaxCnt, 1);
	moRedisInfoList.push_back(redis_info);

    //message
	moIniFile.GetValue("ip","MESSAGE",redis_info.mstrIP, "127.0.0.1");
	moIniFile.GetValue("port","MESSAGE",redis_info.mwPort, 0);
	moIniFile.GetValue("timeout","MESSAGE",redis_info.miTimeOut, 60);
	moIniFile.GetValue("maxcnt","MESSAGE",redis_info.miMaxCnt, 1);
    moRedisInfoList.push_back(redis_info);
 
    //trade
	moIniFile.GetValue("ip","TRADE",redis_info.mstrIP, "127.0.0.1");
	moIniFile.GetValue("port","TRADE",redis_info.mwPort, 0);
	moIniFile.GetValue("timeout","TRADE",redis_info.miTimeOut, 60);
	moIniFile.GetValue("maxcnt","TRADE",redis_info.miMaxCnt, 1);
    moRedisInfoList.push_back(redis_info);

	// ASYNC_DB
	moIniFile.GetValue("ip","ASYNC_DB",redis_info.mstrIP, "127.0.0.1");
	moIniFile.GetValue("port","ASYNC_DB",redis_info.mwPort, 0);
	moIniFile.GetValue("timeout","ASYNC_DB",redis_info.miTimeOut, 60);
	moIniFile.GetValue("maxcnt","ASYNC_DB",redis_info.miMaxCnt, 1);
    moRedisInfoList.push_back(redis_info);

    // friend
	moIniFile.GetValue("ip","FRIEND",redis_info.mstrIP, "127.0.0.1");
	moIniFile.GetValue("port","FRIEND",redis_info.mwPort, 0);
	moIniFile.GetValue("timeout","FRIEND",redis_info.miTimeOut, 60);
	moIniFile.GetValue("maxcnt","FRIEND",redis_info.miMaxCnt, 1);
    moRedisInfoList.push_back(redis_info);

	// group->db
	moIniFile.GetValue("ip","GROUP",redis_info.mstrIP, "127.0.0.1");
	moIniFile.GetValue("port","GROUP",redis_info.mwPort, 0);
	moIniFile.GetValue("timeout","GROUP",redis_info.miTimeOut, 60);
	moIniFile.GetValue("maxcnt","GROUP",redis_info.miMaxCnt, 1);
    moRedisInfoList.push_back(redis_info);
    
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "moRedisInfoList size = " << moRedisInfoList.size());
	return true;
}

BOOL RedisConfig::DynamicRead()
{
	return TRUE;
}
