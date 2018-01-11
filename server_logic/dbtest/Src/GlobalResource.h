#ifndef __GLOBAL_RESOURCE_H__
#define __GLOBAL_RESOURCE_H__
#include "network/ITransProtocolMgr.h"
#include "struct/SvrBaseStruct.h"

#ifdef WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#endif
using namespace std;

extern STRU_SERVER_INFO_BASE		goLocalServerInfo;	// 本地服务器信息

extern string g_server_name;
extern string g_server_config;
extern string g_server_version;

class CConfig;
extern CConfig* gpConfig;

void SetGlobalLocalTimer(time_t aiLocalTimer);
time_t GetGlobalLocalTimer();

#endif//__GLOBALRESOURCE_H_

