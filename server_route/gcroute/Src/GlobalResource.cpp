#include "stdafx.h"
#include "GlobalResource.h"

STRU_SERVER_INFO_BASE				goLocalServerInfo;		//本地服务器信息


string g_server_name="gcroute";
string g_server_config = g_server_name + ".ini";
string g_server_version = "1.1.100";

CConfig* gpConfig = NULL;

