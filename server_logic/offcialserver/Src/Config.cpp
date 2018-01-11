#include "stdafx.h"
#include "Config.h"

CConfig::CConfig():CServerConfigExt()
{
	// 讨论组默认最多可以拉50人
	m_lastmsgtime = 0;
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
	goLocalServerInfo.mlServerIP=inet_addr(GetIntranetIP().c_str());
//	moIniFile.GetValue("ServerPort","LocalServer",goLocalServerInfo.mwServerPort,0);
//	goLocalServerInfo.mwServerPort=htons(goLocalServerInfo.mwServerPort);
	goLocalServerInfo.mwServerType = OFFCIAL_SERVER;
	goLocalServerInfo.GenServerID();

	moIniFile.GetValue("LastMsgTime", "LocalServer", m_lastmsgtime, 50);

	ReadServerConfigByType(DB_SERVER);
	//后台服务器
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
	else if(enum_type == DB_SERVER)
	{
		key_count = "DBServerCount";
		server_name = "DBServer_";
	}

	if(key_count.empty() || server_name.empty())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, " enum_type = " << enum_type << " is invalid");
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
		LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "server "
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


