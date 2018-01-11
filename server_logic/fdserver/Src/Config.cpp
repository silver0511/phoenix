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

    moIniFile.GetValue("TaskThreadCnt", "LocalServer", m_taskthreadcnt, 50);
    moIniFile.GetValue("statinterval","Server",m_statinterval,0);
    INT expiretime = 0;
    moIniFile.GetValue("friendexpireminutes","Server",expiretime,43200);
    m_fdexpire_seconds = expiretime*60;
//	goLocalServerInfo.mwServerPort=htons(goLocalServerInfo.mwServerPort);
	goLocalServerInfo.mwServerType = FD_SERVER;
	goLocalServerInfo.GenServerID();
	return true;
}
BOOL CConfig::DynamicRead()
{
	int32 liLogLevel = 7;
	moIniFile.GetValue("LogLevel","Server",liLogLevel,7);
    moIniFile.GetValue("statinterval","Server",m_statinterval,0);
	LOG_SET_LEVEL(liLogLevel);
	return TRUE;
}


