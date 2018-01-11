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
	goLocalServerInfo.mlServerIP=inet_addr(GetIntranetIP().c_str());
//	moIniFile.GetValue("ServerPort","LocalServer",goLocalServerInfo.mwServerPort,0);
//	goLocalServerInfo.mwServerPort=htons(goLocalServerInfo.mwServerPort);
	goLocalServerInfo.mwServerType = USER_SERVER;
	goLocalServerInfo.GenServerID();

	//后台服务器
	return true;
}

BOOL CConfig::DynamicRead()
{
	int32 liLogLevel = 7;
	moIniFile.GetValue("LogLevel","Server",liLogLevel,7);
	LOG_SET_LEVEL(liLogLevel);
	return TRUE;
}


