#include "stdafx.h"
#include "Config.h"


CConfig::CConfig():CServerConfigExt()
{
	miContactDBCount = 0 ;
	mpContactDBList = NULL;
	miEpollCount = 1;
}

CConfig::~CConfig()
{
	if(NULL !=mpContactDBList)
	{
		delete []mpContactDBList;
		mpContactDBList=NULL;
	}

}

bool CConfig::GetConnectionInfo(string astrName,
	STRU_MYSQL_CONNECT_INFO *apPlayerDBList,
	INT aiCount)
{
	assert(apPlayerDBList);

	char lszDBName[256+1]={0};
	char lszTemp[256+1]={0};

	string lstrTemp;
	WORD lwLen=0;

	for (INT i=0;i<aiCount;i++)
	{
		ZeroMemory(lszDBName,sizeof(lszDBName));

#ifdef WIN32
		_snprintf(lszDBName,256,"%s_%d",astrName.c_str(),i+1);
#else
		snprintf(lszDBName,256,"%s_%d",astrName.c_str(),i);
#endif 

		moIniFile.GetValue("DBIP",lszDBName,apPlayerDBList[i].mstrIP,"");
		moIniFile.GetValue("DBPort",lszDBName,apPlayerDBList[i].mwPort,1);
		moIniFile.GetValue("DBName",lszDBName,apPlayerDBList[i].mstrDBName,"");
		moIniFile.GetValue("USERNAME",lszDBName,apPlayerDBList[i].mstrUserName,"");
		moIniFile.GetValue("PASSWORD",lszDBName,apPlayerDBList[i].mstrUserPwd,"");
		moIniFile.GetValue("CONNECTION",lszDBName,apPlayerDBList[i].miConnectCount,0);
		moIniFile.GetValue("DBINDEX",lszDBName,apPlayerDBList[i].miDBIndex,0);

		lwLen=256;
		//moASCIIEnrypt.Decrypt((char *)apPlayerDBList->mstrUserPwd.c_str(),
		//	(INT)apPlayerDBList[i].mstrUserPwd.size(),lszTemp,lwLen);

		//apPlayerDBList[i].mstrUserPwd=lszTemp;
	}
	return true;
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

	moIniFile.GetValue("CONTACT_DB_COUNT","DB",miContactDBCount,1);
	mpContactDBList=new STRU_MYSQL_CONNECT_INFO[miContactDBCount];
	if(NULL ==mpContactDBList)
		throw;
	GetConnectionInfo("CONTACT_DB",mpContactDBList,miContactDBCount);

	moIniFile.GetValue("statinterval","Server",m_statinterval,0);

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



