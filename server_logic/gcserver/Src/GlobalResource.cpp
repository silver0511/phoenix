// $_FILEHEADER_BEGIN ***************************
// 文件名称: GlobalResource.cpp
// 文件说明: 全局变量定义区
// $_FILEHEADER_END *****************************
#include "stdafx.h"
#include "GlobalResource.h"

STRU_SERVER_INFO_BASE					goLocalServerInfo;		//本地服务器信息

//更改为你的服务器名称
string g_server_name="gcserver";
string g_server_config = g_server_name + ".ini";
string g_server_version = "1.1.100";

time_t giLocalTimer=0;			//本地时间

void SetGlobalLocalTimer(time_t aiLocalTimer)
{
	giLocalTimer = aiLocalTimer;
}
time_t GetGlobalLocalTimer()
{
	return giLocalTimer;
}
