#include "redisclient/LuaShaManager.h"

LuaShaManager::LuaShaManager()
{
}


LuaShaManager::~LuaShaManager(void)
{
}

void LuaShaManager::Addluasha(const std::string &config_file, std::string lua_sha)
{
	std::map<std::string, std::string>::iterator pos = filesha_.find(config_file);
	if (pos != filesha_.end())
	{
		LOG_TRACE(LOG_WARNING, true, __FUNCTION__, " exsist config_file = " << config_file << " lua_sha = " << lua_sha);
	}
	filesha_[config_file] = lua_sha;
}

std::string LuaShaManager::Getluasha(const std::string &config_file)
{
	std::map<std::string, std::string>::iterator pos = filesha_.find(config_file);
	std::string lua_sha = "";
	if (pos != filesha_.end())
	{
		lua_sha = pos->second;
	}

	return lua_sha;
}