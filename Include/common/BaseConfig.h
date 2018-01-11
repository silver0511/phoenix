// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx 版权所有
//			  
// 文件名称：BaseConfig.h
// 创建人： 史云杰
// 文件说明：配置信息
// $_FILEHEADER_END ******************************
#ifndef __BASECONFIG_H_
#define __BASECONFIG_H_

#include <string>
#include "CriticalSection.h"
#include "IniEx.h"
#include "common/ServerTypeEnum.h"

using namespace std;
using namespace U9;
class CBaseConfig
{
public:
	CBaseConfig();
	virtual ~CBaseConfig();
	BOOL Load(const char *aszFileName);
	BOOL DynamicLoad();
	void CheckEvent();
	

	virtual BOOL Read()=0;
	virtual BOOL DynamicRead() { return TRUE; }
protected:
	CCriticalSection		moCriCS;
	string mstrIniFilename ;
    CIniEx moIniFile;
	int miDynamicReadInterval;
};

class CServerConfig:public CBaseConfig
{
public:
	GROUPID  miGourpID;	// 组ID
	SERVERID miServerID;// 服务器id

	int32   miLogLevel;		//日志
	int32   miTCPLogLevel;	//日志

	INT		miEpollCount;				//epoll数
	INT		miMaxConnCount;				//最大连接数
	INT		miRecvThreadCount;			//接收线程数目
	INT		miDealThreadCount;			//处理线程数目
	INT		miConnThreadCount;			//连接线程数目
	INT		miMaxRecvPackageCount;		//最大接收包数
	INT		miMaxSendPackageCount;		//最大接收包数
	string  mstrWithOutCheckIP;			//不需要超时检测的IP
	CServerConfig()
	{
		miGourpID = 0;
		miServerID = 0;
		
		miLogLevel = 0;
		miTCPLogLevel = -1;

		miEpollCount = 1;
		miMaxConnCount=256;
		miRecvThreadCount = 1;				
		miDealThreadCount = 1;	
		miConnThreadCount = 1;
		miMaxRecvPackageCount = 512;		
		miMaxSendPackageCount = 512;

		mstrWithOutCheckIP = "";
	}
	virtual BOOL Read()
	{
		string lstrTemp="";
		
		moIniFile.GetValue("GroupID","Group",miGourpID,0);

		moIniFile.GetValue("LogLevel","Server",miLogLevel,-1);
		moIniFile.GetValue("TCPLogLevel","Server",miTCPLogLevel,-1);
		moIniFile.GetValue("MaxConnCount","Network",miMaxConnCount,256);
		moIniFile.GetValue("EpollCount","Network",miEpollCount,1);
		moIniFile.GetValue("RecvThreadCount","Network",miRecvThreadCount,1);
		moIniFile.GetValue("DealThreadCount","Network",miDealThreadCount,1);
		moIniFile.GetValue("ConnThreadCount","Network",miConnThreadCount,1);
		moIniFile.GetValue("MaxRecvPackageCount", "Network", miMaxRecvPackageCount, 512);
		moIniFile.GetValue("MaxSendPackageCount", "Network", miMaxSendPackageCount, 512);

		moIniFile.GetValue("WithOutCheckIP", "Network", mstrWithOutCheckIP, "");
		return TRUE;
	}
};
//服务器扩展配置
class CServerConfigExt:public CServerConfig
{
public:

	INT		miTCPEpollCount;					//连接数
	INT		miTCPConnCount;					//连接数
	INT		miTCPMaxRecvPackageCount;		//最大接收包数
	INT		miTCPMaxSendPackageCount;		//最大接收包数
	CServerConfigExt()
	{
		miTCPEpollCount = 1;	
		miTCPConnCount = 1;				
		miTCPMaxRecvPackageCount = 32;		
		miTCPMaxSendPackageCount = 32;
	}
	inline virtual BOOL Read()
	{
		CServerConfig::Read();
		//string lstrTemp="";
		
		moIniFile.GetValue("TCPConnCount","TCPPOOL",miTCPConnCount,1);
		moIniFile.GetValue("TCPEpollCount","TCPPOOL",miTCPEpollCount,1);
		moIniFile.GetValue("TCPMaxRecvPackageCount", "TCPPOOL", miTCPMaxRecvPackageCount, 1024);
		moIniFile.GetValue("TCPMaxSendPackageCount", "TCPPOOL", miTCPMaxSendPackageCount, 1024);
		return TRUE;
	}
};


#endif //__CBASECONFIG_H_
