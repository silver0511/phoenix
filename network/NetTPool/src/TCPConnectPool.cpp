// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: TCPConnectPool.cpp
// 创 建 人: 史云杰
// 文件说明: TCP连接池
// $_FILEHEADER_END *****************************

#include "stdafx.h"
#include "TCPConnectPool.h"


CTCPConnectPool::CTCPConnectPool()
{
	miConnectPoolNum = 0;
	mpTCPMgr = NULL;
	mpTCPSocketList = NULL;

}
CTCPConnectPool::~CTCPConnectPool()
{
	{
		CRITICAL_SECTION_HELPER(moReconnectIndexList.GetCriticalSection());
		for(U9_POSITION lpTimeoutPos = moReconnectIndexList.begin();
			lpTimeoutPos != INVALID_U9_POSITION;lpTimeoutPos = moReconnectIndexList.next(lpTimeoutPos))
		{
			int * lpIndex = moReconnectIndexList.getdata(lpTimeoutPos);
			SAFE_DELETE(lpIndex);
		}
		moReconnectIndexList.clear();
	}
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		SAFE_DELETE_ARRAY(mpTCPSocketList);
	}

}
bool CTCPConnectPool::Open(ITransMgrCallBack* apTransMgrCallBack,const STRU_TCP_POOL_OPT & apstruOption)
{
	if(-1 != apstruOption.miLogLevel)
	{
#ifdef TRACE_LOG
#ifdef WIN32
		_snprintf((char*)apstruOption.mszAppName,DEF_MAX_APP_NAME,"%s_tcppoll",apstruOption.mszAppName);
#else
		snprintf((char*)apstruOption.mszAppName,DEF_MAX_APP_NAME,"%s_tcppoll",apstruOption.mszAppName);
#endif 
#endif
	}

	mstrTcpOpt.mbIsServer = FALSE;//使用select方式
	mstrTcpOpt.mulIPAddr = apstruOption.miTCPIP;
	mstrTcpOpt.mwPort =  apstruOption.miTCPPort;
	mstrTcpOpt.mbyDealThreadCount =   1;
	mstrTcpOpt.mbyRecvThreadCount =  1;
	mstrTcpOpt.muiKeepAlive = 0;//服务器间不做连接检查。
	//mstrTcpOpt.mbUseRelink = apstruOption.mbUseRelink;

	mstrTcpOpt.miMaxRecvPackageCount = apstruOption.miTCPMaxRecvPackageCount;
	mstrTcpOpt.miMaxSendPackageCount = apstruOption.miTCPMaxSendPackageCount;

	miConnectPoolNum = apstruOption.miTCPConnCount;
	SAFE_DELETE_ARRAY(mpTCPSocketList);
	mpTCPSocketList = new SOCKET[miConnectPoolNum];
	if(NULL == mpTCPSocketList)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__," << Connect Pool Create >> new mpTCPSocketList fail"
			<< GetIPString(apstruOption.miTCPIP)
			<< " Port: " << htons(apstruOption.miTCPPort)
			);
		return false;
	}
	for(int i=0;i<miConnectPoolNum;i++)
	{
		mpTCPSocketList[i] = INVALID_SOCKET;
	}
	CreateNetTCP("NetTCP", (void**)&mpTCPMgr); 
	if(mpTCPMgr == NULL)
	{
		SAFE_DELETE_ARRAY(mpTCPSocketList);
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__," << Connect Pool Create >> Create TCP TransMgr Error.Connect Fail IP:"
			<< GetIPString(apstruOption.miTCPIP)
			<< " Port: " << htons(apstruOption.miTCPPort)
			);
		return false;
	}
	if(!mpTCPMgr->Init(apTransMgrCallBack,&mstrTcpOpt))
	{
		SAFE_DELETE_ARRAY(mpTCPSocketList);
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__," << Connect Pool Create >>TCP Initial Fail "
			<< GetIPString(apstruOption.miTCPIP)
			<< " Port: " << htons(apstruOption.miTCPPort));
		return false;
	}
	mpTCPMgr->InitEncrypt(0);
	mpTCPMgr->Open();
		
	moReconnectIndexList.init(miConnectPoolNum);

	CRITICAL_SECTION_HELPER(moCriticalSection);
	for(int i=0;i<miConnectPoolNum;i++)
	{
		SOCKET lhSocket = mpTCPMgr->ConnectTo(0,apstruOption.miTCPIP,apstruOption.miTCPPort);
		if(INVALID_SOCKET == lhSocket)
		{
			LOG_TRACE(LOG_CRIT, false, __FUNCTION__," << Connect Pool Create >>Connect Fail IP:"
				<< GetIPString(apstruOption.miTCPIP)
				<< " Port: " << htons(apstruOption.miTCPPort)
				);
			printf("%d Connect Fail IP=%s:%d \n\r",i,GetIPString(apstruOption.miTCPIP).c_str(),htons(apstruOption.miTCPPort));

			if (apstruOption.mbUseRelink)
			{
				int *lpIndex = new int;
				*lpIndex = i;
				moReconnectIndexList.push_back(lpIndex);
				continue;			
			}
			
			return false;
		}
		mpTCPSocketList[i]=lhSocket;
		printf(" %d Conntected socket=%d IP=%s:%d \n\r ",i,lhSocket,GetIPString(apstruOption.miTCPIP).c_str(),htons(apstruOption.miTCPPort));
		LOG_TRACE(LOG_ALERT, 0, __FUNCTION__, " Open"
			<<" mpTCPMgr="<<(void*)mpTCPMgr
			<<" miConnectPoolNum="<<miConnectPoolNum
			);
	}

	miThreadCount=0;
	if (apstruOption.mbUseRelink)
	{
		mbIsRun = true;
		BeginThread();
		Resume();
	}
	return true;
}

bool CTCPConnectPool::Close()
{
	mbIsRun = false;
	while(miThreadCount > 0)
	{
		CSystem::Sleep(10);
	}
	if(NULL != mpTCPMgr)
	{
		{
			CRITICAL_SECTION_HELPER(moCriticalSection);
			for(int i=0;i<miConnectPoolNum;i++)
			{
				mpTCPMgr->DisConnectTo(mpTCPSocketList[i]);
			}	

		}
		mpTCPMgr->Close();
	}


	{
		CRITICAL_SECTION_HELPER(moReconnectIndexList.GetCriticalSection());
		for(U9_POSITION lpTimeoutPos = moReconnectIndexList.begin();
			lpTimeoutPos != INVALID_U9_POSITION;lpTimeoutPos = moReconnectIndexList.next(lpTimeoutPos))
		{
			int * lpIndex = moReconnectIndexList.getdata(lpTimeoutPos);
			SAFE_DELETE(lpIndex);
		}
		moReconnectIndexList.clear();
	}
	LOG_TRACE(LOG_ALERT, 0, __FUNCTION__, " end"
		<<" mpTCPMgr="<<(void*)mpTCPMgr
		<<" miConnectPoolNum="<<miConnectPoolNum
		);
	return true;
}


INT CTCPConnectPool::GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo)
{
	if(NULL != mpTCPMgr)
		mpTCPMgr->GetNetStatisticsInfo(astruInfo);
	return 1;
}

bool CTCPConnectPool::ReConnect(int aiIndex)
{
	if ( aiIndex > miConnectPoolNum ||	aiIndex < 0
		)
	{
		return false;
	}
	//printf(" start reconnect socket=%d IP=%s:%d\n\r",aiIndex, GetIPString(mstrTcpOpt.mulIPAddr).c_str(), htons(mstrTcpOpt.mwPort));
	LOG_TRACE(LOG_WARNING, 0, __FUNCTION__, " ReConnect"
		<<" mpTCPMgr="<<(void*)mpTCPMgr
		<<" miConnectPoolNum="<<miConnectPoolNum
		<<" IP="<<GetIPString(mstrTcpOpt.mulIPAddr)
		<<":"<< htons(mstrTcpOpt.mwPort)
		);
	SOCKET lhSocket = mpTCPMgr->ConnectTo(0,mstrTcpOpt.mulIPAddr,mstrTcpOpt.mwPort);
	if(INVALID_SOCKET == lhSocket)
	{
		//printf("%d Connect Fail IP=%s:%d \n\r",aiIndex,GetIPString(mstrTcpOpt.mulIPAddr).c_str(),htons(mstrTcpOpt.mwPort));

		LOG_TRACE(LOG_ERR, false, __FUNCTION__," Connect Fail "
			<<" mpTCPMgr="<<(void*)mpTCPMgr
			<<" miConnectPoolNum="<<miConnectPoolNum
			<<" IP="<<GetIPString(mstrTcpOpt.mulIPAddr)
			<< " Port: " << htons(mstrTcpOpt.mwPort)
			);
		return false;
	}
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		mpTCPSocketList[aiIndex]=lhSocket;
	}
	printf(" %d Conntected Success socket= %d IP=%s:%d \n\r",aiIndex,lhSocket, GetIPString(mstrTcpOpt.mulIPAddr).c_str(), htons(mstrTcpOpt.mwPort));
	return true;
}

SOCKET CTCPConnectPool::GetSocket(int aiIndex)
{
	if ( aiIndex > miConnectPoolNum ||	aiIndex < 0	)
	{
		return INVALID_SOCKET;
	}

	CRITICAL_SECTION_HELPER(moCriticalSection);
	return mpTCPSocketList[aiIndex];
}

bool CTCPConnectPool::Disconnect(SOCKET ahSocket)
{
	CRITICAL_SECTION_HELPER(moCriticalSection);
	for (int i = 0; i < miConnectPoolNum;i++)
	{
		if(mpTCPSocketList[i] == ahSocket)
		{
			printf(" %d Disconntect : %d  IP=%s:%d \n\r",i,ahSocket,GetIPString(mstrTcpOpt.mulIPAddr).c_str(), htons(mstrTcpOpt.mwPort));

			int *lpIndex = new int;
			*lpIndex = i;
			moReconnectIndexList.push_back(lpIndex);
			mpTCPSocketList[i] = INVALID_SOCKET;
			return true;
		}
	}	
	return false;
}
bool CTCPConnectPool::ReConnectCheck()
{
	if(moReconnectIndexList.size() == 0)
		return false;

	int *lpIndex = moReconnectIndexList.pop_front();
	if( lpIndex == NULL)
		return false;
	
	if(ReConnect(*lpIndex))
	{
		SAFE_DELETE(lpIndex);
		return true;
	}
	moReconnectIndexList.push_back(lpIndex);
	
	return false;
}

bool CTCPConnectPool::JudgeIsConnectServer(IPTYPE aiIP, WORD awPort)
{
	if(aiIP == mstrTcpOpt.mulIPAddr && awPort == mstrTcpOpt.mwPort)
	{
		return true;
	}

	return false;
}

void CTCPConnectPool::Run()
{
	miThreadCount++;
	while (getRunState())
	{
		if(ReConnectCheck())
		{
			CSystem::Sleep(10);
		}
		else
		{
			CSystem::Sleep(10000);
		}
	}
	if( miThreadCount > 0)
		miThreadCount--;
}