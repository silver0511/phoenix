#ifndef __LUASHAMANAGER_H
#define __LUASHAMANAGER_H

#include "base0/platform.h"
#include "common/Debug_log.h"
#include <map>

class LuaShaManager
{
public:
	LuaShaManager();
	~LuaShaManager(void);

	void Addluasha(const std::string &config_file, std::string lua_sha);
	std::string Getluasha(const std::string &strfile);
private:
	std::map<std::string, std::string> filesha_;
};

#endif //__LUASHAMANAGER_H
