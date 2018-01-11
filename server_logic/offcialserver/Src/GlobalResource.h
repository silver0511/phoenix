// $_FILEHEADER_BEGIN ***************************
// 文件名称: GlobalResource.h
// 文件说明: 全局变量定义区
// $_FILEHEADER_END *****************************


#ifndef __GLOBALRESOURCE_H_
#define __GLOBALRESOURCE_H_
#include <time.h>
#include <list>
#include "common/Queue2.h"
#include "common/CacheList.h"
#include "struct/SvrBaseStruct.h"
#include "network/ITCPPoolTransMgr.h"
#include "network/ITransProtocolMgr.h"
#ifdef WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#endif
using namespace std;

extern time_t						        giLocalTimer;
extern STRU_SERVER_INFO_BASE		        goLocalServerInfo;	// 本地服务器信息

extern std::map<EnumServerLogicType, std::list<STRU_SERVER_INFO_BASE> > mMapServerLogicConfig;

extern string g_server_name;
extern string g_server_config;
extern string g_server_version;

void SetGlobalLocalTimer(time_t aiLocalTimer);
time_t GetGlobalLocalTimer();

#endif//__GLOBALRESOURCE_H_
