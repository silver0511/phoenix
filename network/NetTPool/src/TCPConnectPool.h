// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：TCPConnectPool.h
// 创建人： 史云杰
// 文件说明：TCP连接池
// $_FILEHEADER_END *****************************

#ifndef __TCPCONNECTPOOL_H
#define __TCPCONNECTPOOL_H

#include "network/ITransProtocolMgr.h"
#include "network/ITransMgrCallBack.h"
#include "network/ITCPPoolTransMgr.h"

//#include "common/Sptr.h"
#include "common/BaseThread.h"
#include "common/Queue2.h"

#include <map>

class CConfig;
using namespace std;
using namespace U9;

#define DEF_RELINK_CACH_MAX		100

class CTCPConnectPool:public CBaseThread
{
public:
	int miThreadCount;
	CTCPConnectPool();
	~CTCPConnectPool();
	bool Open(ITransMgrCallBack* apTransMgrCallBack,const STRU_TCP_POOL_OPT &aoOpt);
	
	bool Close();
	
	INT GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo);
	bool JudgeIsConnectServer(IPTYPE aiIP, WORD awPort);
public:
	void Run();
	bool ReConnectCheck();
	bool Disconnect(SOCKET ahSocket);
	SOCKET GetSocket(int aiIndex);
	bool ReConnect(int aiIndex);
	STRU_OPTION_TCP mstrTcpOpt;
	CCriticalSection moCriticalSection;
	int32 miConnectPoolNum;
	
	SOCKET *mpTCPSocketList;
	CSafeCacheDoubleQueue<int> moReconnectIndexList;

	ITransProtocolMgr*mpTCPMgr; 
};
#endif //__TCPCONNECTPOOL_H



