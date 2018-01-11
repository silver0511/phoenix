// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: BaseTCPMgr.cpp
// 创 建 人: 史云杰
// 文件说明: TCP 模型基本类
// $_FILEHEADER_END *****************************
#ifdef WIN32
#include <winsock2.h>
#endif
#include "stdafx.h"
#include <assert.h>
#include "BaseTCPMgr.h"

#ifdef WIN32
#include <Psapi.h>
#include <dbghelp.h>
#include <vdmdbg.h>
#include "base0/IExceptionHandler.h"
#endif
#ifdef __NETSPEEDPROXY
#include "../../common/ExceptionHandler/src/ExceptionHandler_Win.h"
// $_FUNCTION_BEGIN ******************************
// 函数名称：CreateProtocol_TCP
// 函数参数：
// 返 回 值：long UVS对象句柄
// 函数说明：CreateProtocol_TCP的创建函数
// $_FUNCTION_END ********************************
///////////////////////////////////////////////////////////////////////
// 导出函数
INT WriteMinidumpWithException(EXCEPTION_POINTERS* apExceptionInfo)
{
	goExceptionHandler.WriteMinidumpForException(apExceptionInfo);
	return 0;
}
#endif 
#ifdef WIN32
CSimpleCachePool<CSafeDoubleQueue<STRU_SOCKET_SEND_CONTEXT>::STRU_NODE_INFO> CContextDQueue<STRU_SOCKET_SEND_CONTEXT>::moCachePool;

CSimpleCachePool<CSafeDoubleQueue<CBaseTCPMgr::STRU_TIMEOUT_ITEM>::STRU_NODE_INFO> CContextDQueue<CBaseTCPMgr::STRU_TIMEOUT_ITEM>::moCachePool;
#endif 


CBaseTCPMgr::CBaseTCPMgr()
{
	mlRefCount =0;
	mbIsRun = FALSE;
	mhServerSocket = INVALID_SOCKET;

	miEncryptType=DEF_ENCRYPT_VERSION_V1;
	mlLocalTime=CSystem::GetTime();
	mpMsgSink = &moNullCallBack;
	memset(&mstruTcpOpt, 0, sizeof(mstruTcpOpt));

	mpSendCachePool=NULL;
	mpSocketInfoMgr = NULL;
	mpSocketTimeoutDqueue = NULL;

	mpEventQueue = NULL;
	mpConnectEventQueue = NULL;

	miMaxEventCount = 0;

	miMaxRawRecvPackCount = 0;
	miMaxRecvPackCount = 0;
	miMaxSendPackCount = 0;
	mpStatisList = NULL;
}


CBaseTCPMgr::~CBaseTCPMgr()
{	
	LOG_SET_LEVEL(0);
	moTimeoutCacheQueue.clear();
	SAFE_DELETE_ARRAY(mpEventQueue);
	SAFE_DELETE_ARRAY(mpConnectEventQueue);
	SAFE_DELETE_ARRAY(mpSocketTimeoutDqueue);
	SAFE_DELETE_ARRAY(mpStatisList);
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetBindIP
// 函数参数：
// 返 回 值： IPTYPE IP地址
// 函数说明： 设置本机绑定IP
//
// $_FUNCTION_END ********************************
VOID CBaseTCPMgr::SetBindIP(IPTYPE aiIP)
{
	mstruTcpOpt.mulIPAddr = aiIP;
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetBindPort
// 函数参数： 
// 返 回 值： 绑定端口
// 函数说明： 设置本机绑定端口
//
// $_FUNCTION_END ********************************
VOID CBaseTCPMgr::SetBindPort(WORD awPort)
{
	mstruTcpOpt.mwPort= awPort;
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： AddRelationPtr
// 函数参数： 
// 返 回 值： 添加设置成功
// 函数说明： 设置关联指针
//
// $_FUNCTION_END ********************************
bool CBaseTCPMgr::AddRelationPtr(void*apSocketInfo,void* apRelationPtr)
{
	if(!mstruTcpOpt.mbUseRelationPtr)
		return true;

	CSocketInfo *lpSocketInfo = (CSocketInfo *)apSocketInfo;
	lpSocketInfo->IncreaseRef();
	lpSocketInfo->mpRelationPtr = apRelationPtr;

	LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "add RelationPtr "
		<<" Socket="<<lpSocketInfo->moKey.mhSocket
		<<" IP="<<GetIPString(lpSocketInfo->moKey.mPeerIP)
		<<":"<<lpSocketInfo->moKey.mwPeerPort
		<<" RelationPtr="<<apRelationPtr
		<<" SocketInfoPtr="<<(void*)lpSocketInfo
		<<" SocketInfRefCount="<<lpSocketInfo->GetRefCount()
		);
	return true;
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： DelRelationPtr
// 函数参数： 
// 返 回 值： 是否删除成功
// 函数说明： 删除关联指针
//
// $_FUNCTION_END ********************************
bool CBaseTCPMgr::DelRelationPtr(void*apSocketInfo,void* apRelationPtr)
{
	if(!mstruTcpOpt.mbUseRelationPtr)
		return true;

	CSocketInfo *lpSocketInfo = (CSocketInfo *)apSocketInfo;
	if(NULL == lpSocketInfo)
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "Error RelationPtr is null");
		return true;
	}

	if(apRelationPtr != lpSocketInfo->mpRelationPtr )
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "Error RelationPtr is bad"
			<<" Socket="<<lpSocketInfo->moKey.mhSocket
			<<" IP="<<GetIPString(lpSocketInfo->moKey.mPeerIP)
			<<":"<<lpSocketInfo->moKey.mwPeerPort
			<<" RelationPtr="<<apRelationPtr
			<<" SocketRelationPtr="<<lpSocketInfo->mpRelationPtr 
			<<" SocketInfoPtr="<<(void*)lpSocketInfo
			<<" SocketInfRefCount="<<lpSocketInfo->GetRefCount()
			);
	}
	lpSocketInfo->mpRelationPtr = NULL;

	//减少引用
	CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
	
	LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "del RelationPtr "
		<<" Socket="<<lpSocketInfo->moKey.mhSocket
		<<" IP="<<GetIPString(lpSocketInfo->moKey.mPeerIP)
		<<":"<<lpSocketInfo->moKey.mwPeerPort
		<<" RelationPtr="<<apRelationPtr
		<<" SocketInfoPtr="<<(void*)lpSocketInfo
		<<" SocketInfRefCount="<<lpSocketInfo->GetRefCount()
		);

	return true;
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetBindIP
// 函数参数：
// 返 回 值： IPTYPE IP地址
// 函数说明： 得到本机绑定IP
//
// $_FUNCTION_END ********************************
IPTYPE CBaseTCPMgr::GetBindIP()
{
	return mstruTcpOpt.mulIPAddr;
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetBindPort
// 函数参数： 
// 返 回 值： 绑定端口
// 函数说明： 得到本机绑定端口
//
// $_FUNCTION_END ********************************
WORD CBaseTCPMgr::GetBindPort()
{
	return mstruTcpOpt.mwPort;
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： Init
// 函数参数：	apInterface 事件通告接口
//		    apstruOption 设置属性
// 返 回 值： 
// 函数说明： 初始化接口及设置成员
//
// $_FUNCTION_END ********************************
INT CBaseTCPMgr::Init(ITransMgrCallBack *apInterface, 
	STRU_OPTION_BASE *apstruOption)
{
	U9_ASSERT(apstruOption);
	U9_ASSERT(apInterface);
	U9_ASSERT(!mstruTcpOpt.mbIsServer);
	if (!apstruOption || !apInterface)
	{
		return RET_ERROR;
	}
	char lszName[32+1] = {0};

	//只在提供回调的时候设置
	if(apInterface)
	{
		mpMsgSink = apInterface;
	}

	memcpy(&mstruTcpOpt, apstruOption, sizeof(struct STRU_OPTION_TCP));

	mpSocketInfoMgr->Initialize(mstruTcpOpt.muiMaxConnCount);
	//发包队列chache初始化
	/*modify by shiyunjie 2017-01-12
  	*发送除非内核缓冲区满，一般都是直接发送出去
  	*发送逻辑是在处理线程中处理，缓冲区的大小取决于处理线程的个数速度
  	*处理线程数 * 发送系数
	*/
	miMaxSendPackCount = mstruTcpOpt.mbyDealThreadCount * THREAD_DEAL_MAX_PACKAGE_LEN;
	mpSendCachePool->init(miMaxSendPackCount);
	{
		STRU_SOCKET_SEND_CONTEXT* lpSocketSendContext = NULL;
		for(UINT index = 0; index < miMaxSendPackCount; ++index)
		{
			lpSocketSendContext = mpSendCachePool->malloc();
			if(NULL == lpSocketSendContext)
				throw;
			mpSendCachePool->free(lpSocketSendContext);
		}
	}

	//事件chache队列
	miMaxRecvPackCount = ((mstruTcpOpt.mbyEpollCount * CON_MAX_PACKAGE_LEN) / ((mstruTcpOpt.mbyDealThreadCount/ 2) + 1)) + 1;
	/*modify by shiyunjie 2017-01-12
  	*这个cachelist是DealRawPackThread->DealThread的中转缓存列表
  	*DealRawPackThread处理速度恒定，而DealThread处理速度会根据业务逻辑而定
  	*因此需要足够多的缓存，事件数和收包数几乎是一一对应的，所以大小可以一样
	*/
	miMaxEventCount = miMaxRecvPackCount;
	moEventCacheQueue.init(miMaxEventCount);
	mpEventQueue = new CSafeCacheDoubleQueue<STRU_EVENT_DATA>[mstruTcpOpt.mbyDealThreadCount];


	moTimeoutCacheQueue.init(mstruTcpOpt.muiMaxConnCount);
	CSocketTimeoutDqueue::initCache(mstruTcpOpt.muiMaxConnCount);
	mpSocketTimeoutDqueue = new CSocketTimeoutDqueue[mstruTcpOpt.mbyDealThreadCount];

	//初始化配置信息
	/*modify by shiyunjie 2017-01-12
 	 *这个cachelist是DealRawPackThread->DealThread的中转缓存列表
 	 *DealRawPackThread处理速度恒定，而DealThread处理速度会根据业务逻辑而定
 	 *因此需要足够多的缓存，为最大EPOLL数 * 单连接最大缓存数 / 处理线程数的一半
	*/
	moRecvPackCacheList.init(miMaxRecvPackCount);
	{
		CTCPPacket * lpTCPPacket = NULL;
		for(UINT i=0;i < miMaxRecvPackCount; i++)
		{
			lpTCPPacket = moRecvPackCacheList.malloc();
			if(NULL == lpTCPPacket)
				throw;
			lpTCPPacket->init();
			moRecvPackCacheList.free(lpTCPPacket);
		}
	}

	/*modify by shiyunjie 2017-01-12
	 *这个cachelist是NetIOProcessThread->DealRawPackThread的中转缓存列表
	 *DealRawPackThread处理速度很快，因此不需要多少缓存，只需根据io线程数做个等比扩大即可
	*/
	miMaxRawRecvPackCount = mstruTcpOpt.mbyEpollCount * THREAD_RAW_MAX_PACKAGE_LEN;
	mRawRecvPackCacheList.init(miMaxRawRecvPackCount);
	{
		CRawTCPRecvPack * lpRawTCPPacket = NULL;
		for(UINT i=0;i < miMaxRawRecvPackCount; i++)
		{
			lpRawTCPPacket = mRawRecvPackCacheList.malloc();
			if(NULL == lpRawTCPPacket)
				throw;
			mRawRecvPackCacheList.free(lpRawTCPPacket);
		}
	}


	STATIS_LIST_SIZE = mstruTcpOpt.mbyEpollCount + 1;
	mpStatisList = new STRU_STATISTICS_TCP[STATIS_LIST_SIZE];
	time_t aiStatisTime = CSystem::GetTime();
	for(int index_s = 0; index_s < STATIS_LIST_SIZE; ++index_s)
	{
		mpStatisList[index_s].s_start_time = aiStatisTime;
		mpStatisList[index_s].s_end_time = aiStatisTime;
	}
	LOG_TRACE(LOG_ALERT,1,"CBaseTCPMgr::Init","miMaxRawRecvPackCount: " << miMaxRawRecvPackCount
	<< " miMaxEventCount: " << miMaxEventCount
	<< " miMaxRecvPackCount: " << miMaxRecvPackCount
	<< " miMaxSendPackCount: " << miMaxSendPackCount
	<< " mbyDealThreadCount: " << mstruTcpOpt.mbyDealThreadCount
	<< " mpSendCachePool: " << mpSendCachePool->remaincount()
	<< " moEventCacheQueue: " << moEventCacheQueue.remaincount()
	<< " moRecvPackCacheList: " << moRecvPackCacheList.remaincount()
	<< " mRawRecvPackCacheList: " << mRawRecvPackCacheList.remaincount());
	return RET_SUCCESS;
}

void CBaseTCPMgr::InitEncrypt(int8 aiEncryptType)
{
	miEncryptType = aiEncryptType;
}
INT CBaseTCPMgr::Close()
{
	mbIsRun=FALSE;
	U9_POSITION lpPos =INVALID_U9_POSITION;
	STRU_EVENT_DATA	*lpEvent = NULL;
	CSafeCacheDoubleQueue<STRU_EVENT_DATA>* lpEventQueue= NULL;
	if(NULL != mpEventQueue )
	{
		int liMaxValue= mstruTcpOpt.mbyDealThreadCount;
		if (!mstruTcpOpt.mbIsServer)
		{//服务器方式
			liMaxValue= 1;
		}
		for (int i = 0; i < mstruTcpOpt.mbyDealThreadCount; i++)
		{
			lpEventQueue= &mpEventQueue[i];
			while (NULL != (lpEvent = lpEventQueue->getheaddataandpop()))
			{
				moEventCacheQueue.free(lpEvent);
			}
		}
	}
	moEventCacheQueue.clear();
	moRecvPackCacheList.clear();
	return 1;
}

void CBaseTCPMgr::WaitClose()
{
	int i=0;
	while (mlRefCount>0)
	{
		CSystem::Sleep(10);
	}
}
// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetNetStatisticsInfo
// 函数参数： astruInfo
// 返 回 值： 
// 函数说明： 得到本地网络统计信息
//
// $_FUNCTION_END ********************************
INT CBaseTCPMgr::GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo)
{
	time_t lstrNow;
	time_t lduration;
	STRU_STATISTICS_TCP lstruStatsInfo;
	lstrNow = CSystem::GetTime();

	for(int index = 0; index < STATIS_LIST_SIZE; ++index)
	{
		lduration = (lstrNow - mpStatisList[index].s_end_time);
		if (0 >= lduration)
			continue;

		mpStatisList[index].s_start_time = mpStatisList[index].s_end_time;
		lstruStatsInfo.t_send_count += mpStatisList[index].t_send_count;
		lstruStatsInfo.t_recv_count += mpStatisList[index].t_recv_count;
		lstruStatsInfo.mulTotalSendPacketCount += mpStatisList[index].mulTotalSendPacketCount;
		mpStatisList[index].mulTotalSendPacketCount = 0;
		lstruStatsInfo.mulTotalRecvPacketCount += mpStatisList[index].mulTotalRecvPacketCount;
		mpStatisList[index].mulTotalRecvPacketCount = 0;
		lstruStatsInfo.cur_con += mpStatisList[index].cur_con;
		lstruStatsInfo.t_lost_send += mpStatisList[index].t_lost_send;
		lstruStatsInfo.t_lost_recv += mpStatisList[index].t_lost_recv;
		lstruStatsInfo.s_start_time = mpStatisList[index].s_start_time;
		lstruStatsInfo.s_end_time = lstrNow;
	}

	if(lduration >= 0)
	{
		lstruStatsInfo.send_count_sec = (lstruStatsInfo.mulTotalSendPacketCount / lduration);
		lstruStatsInfo.recv_count_sec = (lstruStatsInfo.mulTotalRecvPacketCount / lduration);
	}

	memcpy(astruInfo,  &lstruStatsInfo, sizeof(STRU_STATISTICS_TCP));
	return RET_SUCCESS;
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： Release
// 函数参数：
// 返 回 值： VOID
// 函数说明： 释放对象
//
// $_FUNCTION_END ********************************
VOID CBaseTCPMgr::Release()
{
	delete this;
}
void CBaseTCPMgr::DealRawPackThread(VOID *apParam)
{
	CSystem::InitRandom();
	CBaseTCPMgr* lpThis =(CBaseTCPMgr*) apParam;
	if(NULL ==lpThis )
		return ;

	lpThis->IncreaseRef();
	lpThis->DealRawPackThread();
	lpThis->DecreaseRef();
}
void CBaseTCPMgr::DealRawPackThread()
{
	CSystem::InitRandom();
	LOG_TRACE(LOG_ALERT,1,"DealRawPackThread","Run");

	CRawTCPRecvPack *lpRawTCPPack = NULL;
	while (mbIsRun)
	{
		if(moRawRecvPackList.size() == 0)
		{
			CSystem::Sleep(5);
			continue;
		}

		if(moRawRecvPackList.size() >= miMaxRawRecvPackCount)
		{
			LOG_TRACE(LOG_NOTICE,1,"DealRawPackThread","moRawRecvPackList size too large: " << moRawRecvPackList.size());
		}

		lpRawTCPPack  = moRawRecvPackList.getheaddataandpop();
		if(NULL == lpRawTCPPack)
			continue;
		RecvData(lpRawTCPPack->mhSocket,lpRawTCPPack->mulIPAddr,
			lpRawTCPPack->mwIPPort, 
			lpRawTCPPack->mpData, 
			lpRawTCPPack->miDataLen,
			(CSocketInfo*)lpRawTCPPack->mpSocketInfo);
		mRawRecvPackCacheList.free(lpRawTCPPack);	
	}//while
}



void CBaseTCPMgr::DealThread(VOID *apParam)
{
	CSystem::InitRandom();
	STRU_TRHEAD_CONTEXT *lpThreadContext = (STRU_TRHEAD_CONTEXT *)apParam;
	CBaseTCPMgr* lpThis =(CBaseTCPMgr*) lpThreadContext->mpThis;
	if(NULL ==lpThis )
		return ;

	lpThis->IncreaseRef();
	lpThis->DealThread(lpThreadContext->dwIndex);
	lpThis->DecreaseRef();

	delete lpThreadContext;
}

void CBaseTCPMgr::DealThreadEx(DWORD adwIndex)
{
	CTCPPacket* lpTCPPacket = NULL;
	DWORD       ldwIndex = adwIndex;
	U9_ASSERT(adwIndex >= 0 && adwIndex <= mstruTcpOpt.mbyDealThreadCount);

	STRU_EVENT_DATA	*lpEvent = NULL;
	int64 li64Session =0;

	int32 liCheckCount = 0;
	int64 li64NowTime = 0;
	int64 li64LastTime =0;
	int64 liSecondCount = 0;
	CSafeCacheDoubleQueue<STRU_EVENT_DATA>* lpEventQueue= &mpEventQueue[ldwIndex];
	while(mbIsRun)
	{
		if(adwIndex == 0)
		{//只有第一个处理线程来做 检测事件计算
			if(liCheckCount>=10)//10次循环计算一次
			{
				liCheckCount=0;
				li64NowTime = CSystem::GetSystemTime();
				if((li64NowTime-li64LastTime>=100))
				{//100毫秒运行一次
					//LOG_TRACE(4, true, __FUNCTION__,  " " 
					//<<" li64NowTime="<<li64NowTime
					//<<" li64LastTime="<<li64LastTime
					//<<" SecondCount="<<liSecondCount
					//);

					li64LastTime=li64NowTime;
					liSecondCount++;
//					giLocalTime = CSystem::GetTime();
					if(liSecondCount>=10)
					{//1秒运行一次
						liSecondCount=0;
						mpMsgSink->CheckEvent(true);
					}
					else
					{//100毫秒运行一次
						mpMsgSink->CheckEvent(false);
					}
				}
			}
		}	
		lpEvent=lpEventQueue->getheaddataandpop();
		if(NULL == lpEvent)
		{
			if(adwIndex == 0)
				liCheckCount++;
			CSystem::Sleep(10);
			continue;
		}
		if(adwIndex == 0)
		{
			liCheckCount++;
			//LOG_TRACE(4, true, __FUNCTION__,  "Deal packet start" 
			//	<<" Socket="<<lpEvent->mhSocket
			//	<<" IP="<<GetIPString(lpEvent->miIP)
			//	<<":"<<ntohs(lpEvent->mwPort)
			//	<<" CheckCount="<<liCheckCount
			//	);
		}

		switch(lpEvent->mTypeEvent)
		{
		case STRU_EVENT_DATA::ENUM_RECV://接收数据
			{
				lpTCPPacket = (CTCPPacket*)lpEvent->mpParam1;
				//printf("Deal Pack %s:%d \n\r",GetIPString(lpEvent->miIP).c_str(),ntohs(lpEvent->mwPort));
				//printf("Deal Pack %s:%d \n\r",GetIPString(lpEvent->miIP).c_str(),ntohs(lpEvent->mwPort));
				

				if(mstruTcpOpt.mbUseRelationPtr)
				{
					void * lppRelationPtr = NULL;
					if(lpEvent->mpRelationPtr)
						lppRelationPtr = ((CSocketInfo*)lpEvent->mpRelationPtr)->mpRelationPtr;

					mpMsgSink->OnNewRecvData(lpEvent->mhSocket,
						lppRelationPtr,
						lpEvent->miIP,
						lpEvent->mwPort,
						lpTCPPacket->getDataPtr(),
						lpTCPPacket->getDataLen());
				}

				else
				{
					mpMsgSink->OnRecvData(lpEvent->mhSocket, 
						lpEvent->miIP,
						lpEvent->mwPort,
						lpTCPPacket->getDataPtr(),
						lpTCPPacket->getDataLen());
				}

				lpTCPPacket->init();
				moRecvPackCacheList.free(lpTCPPacket);
				break;
			}
		case STRU_EVENT_DATA::ENUM_CONNECT_SUCCESS:
			{
				li64Session = lpEvent->mwPort;
				li64Session = (li64Session << 32) + lpEvent->miIP;

				//主要担心OnSesskkionConnectedEx不一定相应。这里一定要释放一次引用
				CSocketInfo *lpSocketInfo = (CSocketInfo *)lpEvent->mpRelationPtr;
				CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);

				if(mstruTcpOpt.mbUseRelationPtr)
					mpMsgSink->OnSessionConnectedEx(li64Session,lpEvent->mpRelationPtr,
					lpEvent->mhSocket,lpEvent->miIP,lpEvent->mwPort);
				else
					mpMsgSink->OnSessionConnected(li64Session,
						lpEvent->mhSocket,lpEvent->miIP,lpEvent->mwPort);				
				break;
			}
		case STRU_EVENT_DATA::ENUM_CONNECT_FAILURE:
			{
				li64Session = lpEvent->mwPort;
				li64Session = (li64Session << 32) + lpEvent->miIP;
				mpMsgSink->OnSessionConnectFailure(li64Session,
					lpEvent->mhSocket,lpEvent->miIP,lpEvent->mwPort);
				break;
			}
		case STRU_EVENT_DATA::ENUM_CLOSE:
			{
				LOG_TRACE(LOG_DEBUG_1, false, __FUNCTION__, "Error socket close "
					<<" errcode="<< GETSOCKET_ERRCODE()
					<<" socket="<<GetIPString(lpEvent->mhSocket)
					<<" IP="<<GetIPString(lpEvent->miIP)
					<<":"<<ntohs(lpEvent->mwPort)
					);
				li64Session = lpEvent->miParam;
				if(mstruTcpOpt.mbUseRelationPtr)
				{
					void * lppRelationPtr = NULL;
					if(lpEvent->mpRelationPtr)
						lppRelationPtr = ((CSocketInfo*)lpEvent->mpRelationPtr)->mpRelationPtr;
					mpMsgSink->OnSessionCloseEx(li64Session,lppRelationPtr,
						lpEvent->mhSocket,lpEvent->miIP,lpEvent->mwPort);
				}

				else
					mpMsgSink->OnSessionClose(li64Session,
						lpEvent->mhSocket,lpEvent->miIP,lpEvent->mwPort);
				break;
			}
		case STRU_EVENT_DATA::ENUM_CLOSE_SOCKET_AT_SEND_COMPLETE:
			{//需要关闭socket
				LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__,  "Send coplete,process close "
					<<" Socket="<<lpEvent->mhSocket
					<<" IP="<<GetIPString(lpEvent->miIP)
					<<":"<<ntohs(lpEvent->mwPort)
					);
				DisConnectTo(lpEvent->mhSocket);
				break;
			}
		default:
			{
				break;
			}
		}
		//if(adwIndex == 0)
		//{
		//	LOG_TRACE(4, true, __FUNCTION__,  "Deal packet end" 
		//		<<" Socket="<<lpEvent->mhSocket
		//		<<" IP="<<GetIPString(lpEvent->miIP)
		//		<<":"<<ntohs(lpEvent->mwPort)
		//		<<" CheckCount="<<liCheckCount
		//		);
		//}
		moEventCacheQueue.free(lpEvent);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称:  ReceiveThread
// 函数参数:  无.
// 返 回 值:  
// 函数说明:  接收线程
// $_FUNCTION_END ********************************
#ifdef WIN32
int filter(struct _EXCEPTION_POINTERS *apException)
{
	WriteMinidumpWithException(apException);
	return EXCEPTION_EXECUTE_HANDLER;
}
void CBaseTCPMgr::DealThread(DWORD adwIndex)
{
#ifndef _DEBUG
	__try
	{
		DealThreadEx(adwIndex);
	}
	__except(filter(GetExceptionInformation()))
	{
		cerr << "Catch a excetion" <<endl;
		LOG_TRACE(LOG_CRIT,1,__FUNCTION__,"Exception"
			<<" Index="<<adwIndex);
	}
#else
	DealThreadEx(adwIndex);
#endif
}
#else
void CBaseTCPMgr::DealThread(DWORD adwIndex)
{
	try
	{
		DealThreadEx(adwIndex);
	}
	catch(...)
	{
		cerr << "Catch a excetion" <<endl;
		LOG_TRACE(LOG_CRIT,1,__FUNCTION__,"Exception"
			<<" Index="<<adwIndex);
	}
}
#endif

void  CBaseTCPMgr::ReadAllReceivedData(DWORD adwIndex)
{
	if(FALSE ==mbIsRun)
		return ;

	CTCPPacket* lpTCPPacket = NULL;
	int32 i=0;
	STRU_EVENT_DATA	*lpEvent = NULL;
	U9_POSITION lpPos = NULL;
	int64 li64Session =0;
	CSafeCacheDoubleQueue<STRU_EVENT_DATA>* lpEventQueue= &mpEventQueue[adwIndex];

	while(mbIsRun)
	{
		lpEvent=lpEventQueue->getheaddataandpop();
		if(NULL == lpEvent)
		{
			return;
		}
		switch(lpEvent->mTypeEvent)
		{
		case STRU_EVENT_DATA::ENUM_RECV://接收数据
			{
				lpTCPPacket = (CTCPPacket*)lpEvent->mpParam1;
				mpMsgSink->OnRecvData(lpEvent->mhSocket, 
					lpEvent->miIP,
					lpEvent->mwPort,
					lpTCPPacket->getDataPtr(),
					lpTCPPacket->getDataLen());

				mpMsgSink->OnNewRecvData(lpEvent->mhSocket,
					lpEvent->mpRelationPtr,
					lpEvent->miIP,
					lpEvent->mwPort,
					lpTCPPacket->getDataPtr(),
					lpTCPPacket->getDataLen());

				lpTCPPacket->init();
				moRecvPackCacheList.free(lpTCPPacket);
				break;
			}
		case STRU_EVENT_DATA::ENUM_CONNECT_SUCCESS:
			{
				li64Session = lpEvent->mwPort;
				li64Session = (li64Session << 32) + lpEvent->miIP;
				mpMsgSink->OnSessionConnected(li64Session,
					lpEvent->mhSocket,lpEvent->miIP,lpEvent->mwPort);
				break;
			}
		case STRU_EVENT_DATA::ENUM_CONNECT_FAILURE:
			{
				li64Session = lpEvent->miID;//关闭原因
				mpMsgSink->OnSessionConnectFailure(li64Session,
					lpEvent->mhSocket,lpEvent->miIP,lpEvent->mwPort);
				break;
			}
		case STRU_EVENT_DATA::ENUM_CLOSE:
			{
				LOG_TRACE(LOG_DEBUG_1, false, __FUNCTION__, "Error socket close "
					<<" errcode="<< GETSOCKET_ERRCODE()
					<<" socket="<<GetIPString(lpEvent->mhSocket)
					<<" IP="<<GetIPString(lpEvent->miIP)
					<<":"<<ntohs(lpEvent->mwPort)
					);

				li64Session = lpEvent->miParam;//关闭原因
				mpMsgSink->OnSessionClose(li64Session,
					lpEvent->mhSocket,lpEvent->miIP,lpEvent->mwPort);
				break;
			}
		default:
			{
				break;
			}
		}
		moEventCacheQueue.free(lpEvent);
		i++;
		if(i>4)
			break;
	}
}


BOOL CBaseTCPMgr::SocketConnecting(STRU_CONNECT_EVENT *apEvent)
{
#ifdef WIN32
	apEvent->mhSocket = WSASocket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP,
		NULL,
		0,
		WSA_FLAG_OVERLAPPED
		);
#else
	apEvent->mhSocket= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif 

	if (INVALID_SOCKET == apEvent->mhSocket)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error socket fail "
			<<" errcode="<< GETSOCKET_ERRCODE()
			<<" IP="<<GetIPString(apEvent->miDestIP)
			<<":"<<ntohs(apEvent->mwDestPort)
			);
		return FALSE;
	}
	struct sockaddr_in lstruAddr;
	ZeroMemory(&lstruAddr,sizeof(lstruAddr));
	//	//lstruAddr.sin_addr.S_un.S_addr = apEvent->miDestIP;
	lstruAddr.sin_addr.s_addr = apEvent->miDestIP;
	lstruAddr.sin_family = AF_INET;
	lstruAddr.sin_port = apEvent->mwDestPort;

	apEvent->miCreateTime = mlLocalTime;	
	if(!SetSocektNoBlock(apEvent->mhSocket))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error SetSocektNoBlock fail "
			<<" errcode="<< GETSOCKET_ERRCODE()
			<<" Socket="<<apEvent->mhSocket
			<<" IP="<<GetIPString(apEvent->miDestIP)
			<<":"<<ntohs(apEvent->mwDestPort)
			);
		shutdown(apEvent->mhSocket, SD_BOTH);
		CloseSocket(apEvent->mhSocket);
		return FALSE;
	}

	int liResult = connect(apEvent->mhSocket, (const sockaddr *)&lstruAddr, 
		sizeof(struct sockaddr_in));
	if (liResult != 0)
	{
		int32 liError =  GETSOCKET_ERRCODE();
#ifdef WIN32
		if(WSAEWOULDBLOCK != liError)
#else
		if(EINPROGRESS != liError)
#endif
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error connect fail"
				<<" Error="<< GETSOCKET_ERRCODE()
				<<" Socket="<<apEvent->mhSocket
				<<" IP="<<GetIPString(apEvent->miDestIP)
				<<":"<<ntohs(apEvent->mwDestPort)
				);
			shutdown(apEvent->mhSocket, SD_BOTH);
			CloseSocket(apEvent->mhSocket);
			return FALSE;
		}
	}
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "begin connect"
		<<" Socket="<<apEvent->mhSocket
		<<" IP="<<GetIPString(apEvent->miDestIP)
		<<":"<<ntohs(apEvent->mwDestPort)
		);
	return TRUE;
}

void CBaseTCPMgr::ProcesssSocketConnectSucces(const SESSIONID &aiSessionID,SOCKET ahSocket,IPTYPE aiIPAddr,WORD awPort)
{
	if (mpMsgSink == NULL )
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error ProcesssSocketConnectSucces arg is null"
			<<" Error="<< GETSOCKET_ERRCODE()
			<<" Socket="<<ahSocket
			<<" IP="<<GetIPString(aiIPAddr)
			<<":"<<ntohs(awPort)
			);
		shutdown(ahSocket, SD_BOTH);
		CloseSocket(ahSocket);
		return;
	}

	CSocketInfo * lpSocketInfo =CreateSocketSocket(ahSocket,aiIPAddr,awPort);
	if(lpSocketInfo == NULL)
	{
		mpMsgSink->OnSessionConnectFailure(INVALID_SESSIONID,INVALID_SOCKET,
			aiIPAddr,
			awPort);
		shutdown(ahSocket, SD_BOTH);
		CloseSocket(ahSocket);

		return;
	}

	InsCurCon(0);
	mpMsgSink->OnSessionConnected(aiSessionID,lpSocketInfo->moKey.mhSocket,
		lpSocketInfo->moKey.mPeerIP,
		lpSocketInfo->moKey.mwPeerPort);

	mpMsgSink->OnSessionConnectedEx(aiSessionID,lpSocketInfo,lpSocketInfo->moKey.mhSocket,
		lpSocketInfo->moKey.mPeerIP,
		lpSocketInfo->moKey.mwPeerPort);
}
void CBaseTCPMgr::ProcessSocketConnectFail(const SESSIONID &aiSessionID,IPTYPE aiDestIP,WORD awDestPort)
{
	mpMsgSink->OnSessionConnectFailure(aiSessionID,INVALID_SOCKET,
		aiDestIP,
		awDestPort);

}

void CBaseTCPMgr::ConnectThread(VOID *apParam)
{
	CSystem::InitRandom();
	STRU_TRHEAD_CONTEXT *lpThreadContext = (STRU_TRHEAD_CONTEXT *)apParam;
	CBaseTCPMgr* lpThis =(CBaseTCPMgr*) lpThreadContext->mpThis;
	if(NULL ==lpThis )
		return ;

	lpThis->IncreaseRef();
	lpThis->ConnectThread(lpThreadContext->dwIndex);
	lpThis->DecreaseRef();

	delete lpThreadContext;
}

void CBaseTCPMgr::ConnectThread(int32 aiThreadIndex)
{
	U9_ASSERT(aiThreadIndex >= 0 && aiThreadIndex <= mstruTcpOpt.mbyConnectThreadCount);
	STRU_CONNECT_EVENT* lpEvent = NULL;
	SOCKET	lhSocket =INVALID_SOCKET;
	CSafeCacheDoubleQueue<STRU_CONNECT_EVENT>* lpConnectQueue= &mpConnectEventQueue[aiThreadIndex];
	CCriticalSection &loCriticalSection = lpConnectQueue->GetCriticalSection();
	fd_set lstruFDSet;
	SOCKET lhMaxSocket = 0;
	//设置select 中断时间
	struct timeval lstru_tv;
	int32  liResult =0;
	int32 liSelectCount = 0;
	int32 liNow =0;
	U9_POSITION lpPos		= INVALID_U9_POSITION;
	U9_POSITION lpOldPos	= INVALID_U9_POSITION;
	while(mbIsRun)
	{
		if(lpConnectQueue->size() == 0)
		{//没有数据
			CSystem::Sleep(10);
			continue;
		}
		liSelectCount=0;
		FD_ZERO(&lstruFDSet); // write fd
		lstru_tv.tv_sec = 1;		//单位 秒
		lstru_tv.tv_usec =0;
		//#ifdef WIN32 
		//		lstru_tv.tv_usec = 100;		//单位 毫秒 100ms 
		//#else
		//		lstru_tv.tv_usec = 100000;	//单位 微妙 100ms=100*1000
		//#endif
		lhMaxSocket = 0;
		liNow = CSystem::GetTime();

		loCriticalSection.Enter();
		lpPos = lpConnectQueue->begin();
		while (INVALID_U9_POSITION != lpPos)
		{
			lpEvent = 	lpConnectQueue->getdata(lpPos);

			if(lpEvent->mhSocket == INVALID_SOCKET)
			{//建立一个新的socket
				if(!SocketConnecting(lpEvent))
				{//连接失败
					LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error SocketConnecting failed "
						<<" errno="<<GETSOCKET_ERRCODE()
						<<" Socket="<<lpEvent->mhSocket
						<<" IP="<<GetIPString(lpEvent->miDestIP)
						<<":"<<ntohs(lpEvent->mwDestPort)
						);

					lpOldPos = lpPos;
					lpPos = lpConnectQueue->next(lpPos);//得到下一个

					lpConnectQueue->erase(lpOldPos);	//删除老的节点

					ProcessSocketConnectFail(lpEvent->miID,lpEvent->miDestIP,lpEvent->mwDestPort);

					moConnectEventCacheQueue.free(lpEvent);
					continue;
				}
				lpEvent->miCreateTime = liNow;
			}
			else
			{//已经创建过的用户
				if(liNow - lpEvent->miCreateTime >= 8)
				{//超时8秒了
					LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error select timeout 8 second "
						<<" Socket="<<lpEvent->mhSocket
						<<" IP="<<GetIPString(lpEvent->miDestIP)
						<<":"<<ntohs(lpEvent->mwDestPort)
						);
					lpOldPos = lpPos;
					lpPos = lpConnectQueue->next(lpPos);//得到下一个

					lpConnectQueue->erase(lpOldPos);	//删除老的节点

					ProcessSocketConnectFail(lpEvent->miID,lpEvent->miDestIP,lpEvent->mwDestPort);

					shutdown(lpEvent->mhSocket, SD_BOTH);
					CloseSocket(lpEvent->mhSocket);

					moConnectEventCacheQueue.free(lpEvent);
					continue;
				}
			}

			if(lpEvent->mhSocket > lhMaxSocket)
				lhMaxSocket = lpEvent->mhSocket;
			
			FD_SET(lpEvent->mhSocket, &lstruFDSet);
			liSelectCount++;
			if(liSelectCount>= FD_SETSIZE)
			{//停止往lstruFDSet放数据了
				break;
			}
			lpPos = lpConnectQueue->next(lpPos);//得到下一个
		}
		loCriticalSection.Leave();

		if(liSelectCount == 0)
		{//没有数据
			continue;
		}

#ifdef _WIN32
		liResult = select(0, NULL, &lstruFDSet, NULL, &lstru_tv);
#else
		liResult = select(lhMaxSocket + 1, NULL, &lstruFDSet, NULL, &lstru_tv);		
#endif

		if (0 == liResult)
		{//超时,重新检查
			continue;
		}

		if (SOCKET_ERROR == liResult)
		{//错误
			int liError = GETSOCKET_ERRCODE();
			loCriticalSection.Enter();

			lpPos = lpConnectQueue->begin();
			int i=0;
			while (INVALID_U9_POSITION != lpPos && i<liSelectCount)
			{
				lpEvent =  lpConnectQueue->getdata(lpPos);
				lpOldPos = lpPos;
				lpPos = lpConnectQueue->next(lpPos);
				i++;
				if(lpEvent->mhSocket == INVALID_SOCKET)//后面添加进来的event，
					continue;
				if (!FD_ISSET(lpEvent->mhSocket, &lstruFDSet))
					continue;

				LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error select failed "
					<<" errno="<<liError
					<<" Socket="<<lpEvent->mhSocket
					<<" IP="<<GetIPString(lpEvent->miDestIP)
					<<":"<<ntohs(lpEvent->mwDestPort)
					);
				lpConnectQueue->erase(lpOldPos);	//删除老的节点

				ProcessSocketConnectFail(lpEvent->miID,lpEvent->miDestIP,lpEvent->mwDestPort);
				shutdown(lpEvent->mhSocket, SD_BOTH);
				CloseSocket(lpEvent->mhSocket);
				moConnectEventCacheQueue.free(lpEvent);
			}

			loCriticalSection.Leave();

			continue;
		}

		//有写事件
		//检查当前所有的连接上是否有相应的网络事件
		loCriticalSection.Enter();

		lpPos = lpConnectQueue->begin();
		int i=0;
		while (INVALID_U9_POSITION != lpPos && i<liSelectCount)
		{
			lpEvent = lpConnectQueue->getdata(lpPos);
			lpOldPos = lpPos;
			lpPos = lpConnectQueue->next(lpPos);
			i++;
			if(lpEvent->mhSocket == INVALID_SOCKET)//后面添加进来的event，
				continue;
			if (!FD_ISSET(lpEvent->mhSocket, &lstruFDSet))
				continue;

			lpConnectQueue->erase(lpOldPos);	//删除老的节点

			//是有socket事件
#ifdef WIN32
			int liOptoinVal =0;
			int liLen =  sizeof(int);
			getsockopt(lpEvent->mhSocket, SOL_SOCKET, SO_ERROR,(char *)&liOptoinVal,&liLen); 
#else //WIN32
			socklen_t liOptoinVal =0;
			socklen_t liLen =  sizeof(socklen_t);
			getsockopt(lpEvent->mhSocket, SOL_SOCKET, SO_ERROR,(char *)&liOptoinVal,&liLen); 
#endif //WIN32
			if(liOptoinVal == 0)
			{
				LOG_TRACE(LOG_INFO, true, __FUNCTION__, " Socket connect "
					<<" Socket="<<lpEvent->mhSocket
					<<" IP="<<GetIPString(lpEvent->miDestIP)
					<<":"<<ntohs(lpEvent->mwDestPort)
					);
				ProcesssSocketConnectSucces(lpEvent->miID,lpEvent->mhSocket,lpEvent->miDestIP,lpEvent->mwDestPort);
			}
			else
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error select failed "
					<<" errno="<<liOptoinVal
					<<" Socket="<<lpEvent->mhSocket
					<<" IP="<<GetIPString(lpEvent->miDestIP)
					<<":"<<ntohs(lpEvent->mwDestPort)
					);

				ProcessSocketConnectFail(lpEvent->miID,lpEvent->miDestIP,lpEvent->mwDestPort);
				shutdown(lpEvent->mhSocket, SD_BOTH);
				CloseSocket(lpEvent->mhSocket);
			}
			moConnectEventCacheQueue.free(lpEvent);//释放
		}

		loCriticalSection.Leave();
	}
}

LONG CBaseTCPMgr::IncreaseRef()
{
	LONG llValue=0;
	moRefCriticalSection.Enter();
	mlRefCount++;
	llValue=mlRefCount;
	moRefCriticalSection.Leave();
	return llValue;
}

//引用计数减1
LONG CBaseTCPMgr::DecreaseRef()
{
	LONG llValue=0;
	moRefCriticalSection.Enter();
	if(mlRefCount>0)
		mlRefCount--;
	llValue=mlRefCount;
	moRefCriticalSection.Leave();
	return llValue;
}

/*
 * 对应处理线程(DealRawPackThread)
 * moRawRecvPackList->moRecvPackCacheList.malloc()->moEventCacheQueue.malloc()->mpEventQueue.push_back()
 * DealRawPackThread的处理线程只有一个，所以moRawRecvPackList不需要很大
*/
INT CBaseTCPMgr::RecvData(SOCKET ahScoket,IPTYPE aiIP, WORD awPort, BYTE* apData, WORD awLen,CSocketInfo *apSocketInfo)
{
	LOG_TRACE(LOG_DEBUG_1, TRUE, __FUNCTION__, "CBaseTCPMgr RecvData awLen: " << awLen);
	//printf("PacketArrive Pack %s:%d \n\r",GetIPString(aiIP).c_str(),ntohs(awPort));

	CTCPPacket *lpTCPPacket = (CTCPPacket *)moRecvPackCacheList.malloc();
//	if(moRecvPackCacheList.remaincount() <= 1)
//	{
//		LOG_TRACE(LOG_NOTICE, false, __FUNCTION__, "moRecvPackCacheList too large size=" << moRecvPackCacheList.size());
//	}

	if (NULL == lpTCPPacket)
	{
		InsLostRecv(STATIS_LIST_SIZE - 1);
		//printf("New lpTCPPacket fail %s:%d \n\r",GetIPString(aiIP).c_str(),ntohs(awPort));
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error allocate tcp packet failed."
			<<" Socket="<<ahScoket
			<<" IP="<<GetIPString(aiIP)
			<<":"<<ntohs(awPort)
			<<" Size="<<awLen
			<<"cachesize" << moRecvPackCacheList.size());
		return -1;
	}
	lpTCPPacket->init();
	if(0> lpTCPPacket->UnPack(apData, awLen,miEncryptType))
	{
		//解包失败
		InsLostRecv(STATIS_LIST_SIZE - 1);
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Unpack failed."
			<<" Socket="<<ahScoket
			<<" IP="<<GetIPString(aiIP)
			<<":"<<ntohs(awPort)
			<<" Size="<<awLen
			<<" EncryptType="<<miEncryptType
			);
		moRecvPackCacheList.free(lpTCPPacket);

		//关闭连接
		DisConnectTo(ahScoket);
		return -1;
	}

	DWORD ldwIndex = GetDealThreadIndex(aiIP,awPort);

	//LOG_TRACE(5, true, __FUNCTION__, " Recv Pack"
	//	<<" Socket="<<ahScoket
	//	<<" IP="<<GetIPString(aiIP)
	//	<<":"<<ntohs(awPort)
	//	<<" TCPLen="<<awLen
	//	<<" PackLen="<<lpTCPPacket->getDataLen()
	//	<<" PackType="<<*(WORD*)lpTCPPacket->getDataPtr()
	//	<<" EncryptType="<<miEncryptType
	//	<<" Index="<<ldwIndex
	//	);

//	if(moEventCacheQueue.remaincount() <= 1)
//	{
//		LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, " moEventCacheQueue too large size=" << moEventCacheQueue.size());
//	}

	STRU_EVENT_DATA *lpEvent=moEventCacheQueue.malloc();

	lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_RECV;
	lpEvent->mhSocket = ahScoket;
	lpEvent->miIP = aiIP;
	lpEvent->mwPort = awPort;
	lpEvent->mpParam1 = lpTCPPacket;
	lpEvent->mpRelationPtr = (void*)apSocketInfo;

	if(INVALID_U9_POSITION ==  mpEventQueue[ldwIndex].push_back(lpEvent))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back failed."
			<<" Socket="<<ahScoket
			<<" IP="<<GetIPString(aiIP)
			<<":"<<ntohs(awPort)
			<<" Size="<<awLen
			<<" EncryptType="<<miEncryptType
			<<" Index="<<ldwIndex
			);
		moEventCacheQueue.free(lpEvent);
	}
	return 1;
}