#include "stdafx.h"
#include "Config.h"

CConfig::CConfig():CServerConfigExt()
{
}

CConfig::~CConfig()
{

}


BOOL CConfig::Read()
{
	CServerConfigExt::Read();
	string lstrTemp="";

	//读取本机监听IP和服务器
//	moIniFile.GetValue("ServerIP","LocalServer",lstrTemp,"127.0.0.1");
	goLocalServerInfo.mlServerIP=inet_addr("0.0.0.0");
//	moIniFile.GetValue("ServerPort","LocalServer",goLocalServerInfo.mwServerPort,0);
//	goLocalServerInfo.mwServerPort=htons(goLocalServerInfo.mwServerPort);
	goLocalServerInfo.mwServerType = TCP_COMET;
	goLocalServerInfo.GenServerID();
	printf("CConfig Read server_id = %ld, ip = %s\n", goLocalServerInfo.mlServerID, GetIntranetIP().c_str());


	ReadServerConfigByType(USER_SERVER);
	ReadServerConfigByType(GC_SERVER);
	ReadServerConfigByType(SC_SERVER);
	ReadServerConfigByType(FD_SERVER);
	ReadServerConfigByType(EC_SERVER);
	ReadServerConfigByType(OFFCIAL_SERVER);
	ReadServerConfigByType(GROUP_SERVER);
	ReadServerConfigByType(REG_SERVER);

	ReadServerConfigByType(DB_SERVER);

	ReadServerConfigByType(SC_ROUTE);
	ReadServerConfigByType(GC_ROUTE);
	ReadServerConfigByType(PC_ROUTE);

	return true;
}

void CConfig::ReadServerConfigByType(EnumServerLogicType enum_type)
{
	std::string key_count = "";
	std::string server_name = "";
	if(enum_type == USER_SERVER)
	{
		key_count = "UserServerCount";
		server_name = "UserServer_";
	}
	else if(enum_type == REG_SERVER)
	{
		key_count = "RegServerCount";
		server_name = "RegServer_";
	}
	else if(enum_type == SC_SERVER)
	{
		key_count = "SCServerCount";
		server_name = "SCServer_";
	}
	else if(enum_type == GC_SERVER)
	{
		key_count = "GCServerCount";
		server_name = "GCServer_";
	}
	else if(enum_type == FD_SERVER)
	{
		key_count = "FDServerCount";
		server_name = "FDServer_";
	}
	else if(enum_type == EC_SERVER)
	{
		key_count = "ECServerCount";
		server_name = "ECServer_";
	}
	else if(enum_type == OFFCIAL_SERVER)
	{
		key_count = "OffcialServerCount";
		server_name = "OffcialServer_";
	}
	else if(enum_type == GROUP_SERVER)
	{
		key_count = "GROUPServerCount";
		server_name = "GROUPServer_";
	}
	else if(enum_type == DB_SERVER)
	{
		key_count = "DBServerCount";
		server_name = "DBServer_";
	}
	else if(enum_type == SC_ROUTE)
	{
		key_count = "SCRouteCount";
		server_name = "SCRoute_";
	}
	else if(enum_type == GC_ROUTE)
	{
		key_count = "GCRouteCount";
		server_name = "GCRoute_";
	}
    else if(enum_type == PC_ROUTE)
	{
		key_count = "PCRouteCount";
		server_name = "PCRoute_";
	}

	if(key_count.empty() || server_name.empty())
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, " enum_type = " << enum_type << " is invalid");
		return ;
	}

	INT server_count = 0;
	moIniFile.GetValue(key_count, "LocalServer", server_count, 0);
	for(int i = 0; i < server_count; ++i)
	{
		STRU_SERVER_INFO_BASE server_info_base;
		string str_temp;
		string server_name_read = server_name + to_string(i);
		moIniFile.GetValue("ServerIP", server_name_read, str_temp, "127.0.0.1");
		server_info_base.mlServerIP = inet_addr(str_temp.c_str());
		moIniFile.GetValue("ServerPort", server_name_read, server_info_base.mwServerPort, 0);
		server_info_base.mwServerPort = htons(server_info_base.mwServerPort);
		server_info_base.mwServerType = enum_type;
		server_info_base.GenServerID();
		LOG_TRACE(LOG_ALERT, true, __FUNCTION__, "server "
				<<" IP=" << GetIPString(server_info_base.mlServerIP)<< ":"<< ntohs(server_info_base.mwServerPort)
				<<" ServerID="<< server_info_base.mlServerID
		);

		mMapServerLogicConfig[enum_type].push_back(server_info_base);
	}
}

BOOL CConfig::DynamicRead()
{
	int32 liLogLevel = 7;
	moIniFile.GetValue("LogLevel","Server",liLogLevel,7);
	LOG_SET_LEVEL(liLogLevel);
	return TRUE;
}


