#include "stdafx.h"
#include "Config.h"


CConfig::CConfig():CServerConfigExt()
{
	miEpollCount = 1;
}

CConfig::~CConfig()
{

}

BOOL CConfig::Read()
{
	CServerConfigExt::Read();
	string lstrTemp="";

	//读取本机监听IP和服务器

	//consumserver
//	moIniFile.GetValue("ServerIP","LocalServer",lstrTemp,"127.0.0.1");
	goLocalServerInfo.mlServerIP=inet_addr(GetIntranetIP().c_str());
//	moIniFile.GetValue("ServerPort","LocalServer",goLocalServerInfo.mwServerPort,0);
//	goLocalServerInfo.mwServerPort=htons(goLocalServerInfo.mwServerPort);
	goLocalServerInfo.mwServerType = SC_ROUTE;
	goLocalServerInfo.GenServerID();

    moIniFile.GetValue("groupid","Server",m_sgroupid,"scrouter");
	moIniFile.GetValue("statinterval","Server",m_statinterval,0);
    moIniFile.GetValue("kafkathreadcnt","Server",m_kafkathreadcnt,1);
    moIniFile.GetValue("consumethreadcnt","Server",m_consumethreadcnt,1);

	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "LocalServer"
									 <<" IP=" << GetIPString(goLocalServerInfo.mlServerIP)
                                     << ":"<< ntohs(goLocalServerInfo.mwServerPort)
									 <<" ServerID="<< goLocalServerInfo.mlServerID);
	
	return true;
}

BOOL CConfig::DynamicRead()
{
	int32 liLogLevel = 7;
	moIniFile.GetValue("LogLevel","Server",liLogLevel,7);
	LOG_SET_LEVEL(liLogLevel);
	moIniFile.GetValue("statinterval","Server",m_statinterval,0);
		
	return TRUE;
}



