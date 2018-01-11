// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：EpollMgr.h
// 创建人： 史云杰
// 文件说明：Epoll tcp 传输通讯实现类实现文件


//  LT模式下 Level triggered
//	EPOLLIN  当socket Receive buffer有数据就触发
//  EPOLLOUT 当socket send buffer 无数据就触发一直触发

//  ET模式下 Edge triggered   socket必须是非阻塞的 
//	EPOLLIN  只在socket Receive buffer 里的数据从无到有时候，触发一次，其他情况不触发
//  EPOLLOUT 只在第一次注册EPOLLOUT时候触发一次（相当于从无到准备好），或者在socket send buffer从满到空的时候触发。
// 
//在ET下，为了避免Starvation，建议 
// 1）文件描述符设置为非阻塞 
// 2）只在read或write返回EAGAIN后，才能调用下一次epoll_wait 
// 3）应用层维护一个就绪链表，进行轮询，可以防止大量IO时在一个描述符上长期read或write（因为只有等到read 
//    或 write返回EAGAIN后才表示该描述符处理完毕）而令其它描述符starve

//    理解ET的含义后，上面那些操作其实都是显然的。以wirte为例说明，LT时只要有一定范围的空闲写缓存区，每次epoll_wait
//都是可写条件就绪,但是ET时从第一次可写就绪后，epoll_wait不再得到该描述符可写就绪通知直到程序使描述符变为非可写就绪
//（比如write收到 EAGAIN）后，epoll_wait才可能继续收到可写就绪通知（比如有空闲可写缓存）其实ET相对于LT来说，把文件
//描述符状态跟踪的部分责任由内核空间推到用户空间，内核只关心状态切换即从未就绪到就绪切换时才通知用户，至于保持就绪
//状态的，内核不再通知用户，这样在实现非阻塞模型时更方便，不需要每次操作都先查看文件描述符状态。上述多数内容取自man epoll
// $_FILEHEADER_END *****************************
#include "stdafx.h"
#include "common/Queue.h"
#include "common/Queue2.h"
#include "common/CacheList.h"

#include "EpollBMgr.h"

// 标准构造
CEpollBMgr::CEpollBMgr()
{
	mhEpollHandle = NULL;
	mpSendCachePool=NULL;
	mpSocketInfoMgr = NULL;

	mlRefCount =0;
	mbIsRun = FALSE;
	mhServerSocket = INVALID_SOCKET;

	miEncryptType=DEF_ENCRYPT_VERSION_V1;
	mpMsgSink = &moNullCallBack;
	memset(&mstruTcpOpt, 0, sizeof(mstruTcpOpt));
	mpStatisList = NULL;


	mpEventQueue = NULL;
	miMaxEventCount = 0;

	miMaxRecvPackCount = 0;
	miMaxSendPackCount = 0;
}

// 标准析构
CEpollBMgr::~CEpollBMgr()
{
	mpSendCachePool=NULL;
	mpSocketInfoMgr = NULL;

	LOG_SET_LEVEL(0);
	SAFE_DELETE_ARRAY(mpEventQueue);
	SAFE_DELETE_ARRAY(mpStatisList);
}

INT CEpollBMgr::Open()
{
	mbIsRun=TRUE;

	if(!InitEpoll())
	{
		mbIsRun = FALSE;
		return RET_ERROR;
	}

	//初始化监听端口
	if(mstruTcpOpt.mbIsServer)
	{
		if(!StartListen(mstruTcpOpt.mulIPAddr, mstruTcpOpt.mwPort))
		{
			mbIsRun=FALSE;
			return RET_ERROR;
		}
	}

	CSystem::BeginThread(AcceptThreadFunction, (void *)this);

	int i=0;

	for(int i=0;i<mstruTcpOpt.mbyEpollCount;++i)
	{
		STRU_TRHEAD_CONTEXT *lpThreadContext = new STRU_TRHEAD_CONTEXT;

		lpThreadContext->mpThis = (void *)this;
		lpThreadContext->dwIndex = i;
		//io线程
		CSystem::BeginThread(NetIOProcessThread, (void*)lpThreadContext);
	}

	//处理线程
	for(i=0;i<mstruTcpOpt.mbyDealThreadCount;i++)
	{
		STRU_TRHEAD_CONTEXT *lpThreadContext = new STRU_TRHEAD_CONTEXT;
		lpThreadContext->mpThis = (void *)this;
		lpThreadContext->dwIndex = i;

		mpEventQueue[i].init(miMaxEventCount);
		CSystem::BeginThread(DealThread, (void *)lpThreadContext);
	}

	return RET_SUCCESS;
}

// 函数说明： 关闭操作
INT CEpollBMgr::Close()
{
	//baseclosed
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

	if(INVALID_SOCKET != mhServerSocket)
	{
		CloseSocket(mhServerSocket);
		mhServerSocket = INVALID_SOCKET;
	}


	for(int i=0;i<mstruTcpOpt.mbyEpollCount;++i)
	{
		if(INVALID_SOCKET != mhEpollHandle[i])
		{
			CloseSocket(mhEpollHandle[i]);
			mhEpollHandle[i] = INVALID_SOCKET;
		}
	}

	if(NULL != mhEpollHandle)
		delete [] mhEpollHandle;

	WaitClose();

	CloseAllSocket();

	return RET_SUCCESS;
}

INT CEpollBMgr::Init(ITransMgrCallBack *apInterface,
					  STRU_OPTION_BASE *apstruOption)
{
	U9_ASSERT(apstruOption);
	U9_ASSERT(apInterface);
	U9_ASSERT(!mstruTcpOpt.mbIsServer);
	if (!apstruOption || !apInterface)
	{
		return RET_ERROR;
	}

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
	CContextDQueue<STRU_SOCKET_SEND_CONTEXT>::initCache(miMaxSendPackCount);

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

	STATIS_LIST_SIZE = mstruTcpOpt.mbyEpollCount + 1;
	mpStatisList = new STRU_STATISTICS_TCP[STATIS_LIST_SIZE];
	time_t aiStatisTime = CSystem::GetTime();
	for(int index_s = 0; index_s < STATIS_LIST_SIZE; ++index_s)
	{
		mpStatisList[index_s].s_start_time = aiStatisTime;
	}
	LOG_TRACE(LOG_ALERT,1,"CEpollBMgr::Init", " miMaxEventCount: " << miMaxEventCount
																		<< " miMaxRecvPackCount: " << miMaxRecvPackCount
																		<< " miMaxSendPackCount: " << miMaxSendPackCount
																		<< " mbyDealThreadCount: " << mstruTcpOpt.mbyDealThreadCount
																		<< " mpSendCachePool: " << mpSendCachePool->remaincount()
																		<< " moEventCacheQueue: " << moEventCacheQueue.remaincount()
																		<< " moRecvPackCacheList: " << moRecvPackCacheList.remaincount());
	return RET_SUCCESS;
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： AddRelationPtr
// 函数参数：
// 返 回 值： 添加设置成功
// 函数说明： 设置关联指针
//
// $_FUNCTION_END ********************************
bool CEpollBMgr::AddRelationPtr(void*apSocketInfo,void* apRelationPtr)
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
bool CEpollBMgr::DelRelationPtr(void*apSocketInfo,void* apRelationPtr)
{
	if(!mstruTcpOpt.mbUseRelationPtr)
		return true;

	CSocketInfo *lpSocketInfo = (CSocketInfo *)apSocketInfo;
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

// 函数说明： 发送数据 是否需要发生线程?
INT CEpollBMgr::SendData(STRU_SENDDATA_BASE *apstruSendData)
{
	if (apstruSendData == NULL)
	{
		return RET_ERROR;
	}

	STRU_SENDDATA_TCP *lpTcpSndOpt = (struct STRU_SENDDATA_TCP *)apstruSendData;
	SOCKET lhSocket = lpTcpSndOpt->mhSocket;
	//得到一个SocketInfo
	CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lhSocket);
	if(loSocketInfo.Invalid())
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, " Socket not found"
				<<" Socket="<<lhSocket
				<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
				<<" LoadLen="<<lpTcpSndOpt->mwLen
		);
		return RET_ERROR ;
	}

	CSocketInfo *lpSocketInfo = loSocketInfo.GetPtr();
	return SendData(lpSocketInfo, apstruSendData);
}

// 函数说明： 发送数据 是否需要发生线程?
INT CEpollBMgr::SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData)
{
	if (apSocketInfo == NULL || apstruSendData == NULL)
	{
		LOG_TRACE(LOG_ERR, 0, __FUNCTION__, "Error len is bad"
				<<" apSocketInfo="<<(void*)apSocketInfo
				<<" apstruSendData="<<(void*)apstruSendData
		);
		return RET_ERROR;
	}
	CSocketInfo *lpSocketInfo = (CSocketInfo *)apSocketInfo;
	STRU_SOCKET_KEY& loKey=lpSocketInfo->moKey;

	STRU_SENDDATA_TCP *lpTcpSndOpt = (struct STRU_SENDDATA_TCP *)apstruSendData;

	if( lpTcpSndOpt->mwLen >= DEF_PACKET_LEN)
	{
		LOG_TRACE(LOG_ERR, 0, __FUNCTION__, "Error len is bad"
				<<" Socket="<<loKey.mhSocket
				<<" PeerIP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
				<<" LoadLen="<<lpTcpSndOpt->mwLen
		);
		return RET_ERROR;
	}
	SOCKET lhSocket = lpTcpSndOpt->mhSocket;
	// 分配一块空闲的内存
	STRU_SOCKET_SEND_CONTEXT *lpSendSocketContext = mpSendCachePool->malloc();
//	if(mpSendCachePool->remaincount() <= 1)
//	{
//		LOG_TRACE(LOG_NOTICE, 1, __FUNCTION__, "mpSendCachePool too large"
//				<<" size="<<mpSendCachePool->size());
//	}
	if (NULL == lpSendSocketContext)
	{
		// 如果失败,则直接重新分配
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "allocate memory failed in SendPacket()"
				<<" Socket="<<loKey.mhSocket
				<<" PeerIP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
				<<" LoadLen="<<lpTcpSndOpt->mwLen);
		return RET_ERROR;
	}
	// 对上层提交的数据包进行封包
	lpSendSocketContext->miBuffLen = CTCPPacketHeader::Pack(lpSendSocketContext->mpBuff, DEF_PACKET_LEN,
															lpTcpSndOpt->mpData,lpTcpSndOpt->mwLen,miEncryptType);
	if (RET_ERROR >= lpSendSocketContext->miBuffLen)
	{
		LOG_TRACE(LOG_ERR,false, __FUNCTION__, "Error packet data failed in SendPacket"
				<<" Socket="<<loKey.mhSocket
				<<" PeerIP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" EncryptType="<<miEncryptType
				<<" NeedClose="<<lpTcpSndOpt->mbNeedClose
				<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
				<<" PackLen ="<<lpTcpSndOpt->mwLen
				<<" SendSocketContext="<<(void*)lpSendSocketContext
				<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
				<<" LoadLen="<<lpTcpSndOpt->mwLen
		);

		mpSendCachePool->free(lpSendSocketContext);

		if(lpTcpSndOpt->mbNeedClose)
		{
			DisConnectTo(lpTcpSndOpt->mhSocket);
		}
		return RET_ERROR;
	}

	lpSendSocketContext->miOffset = 0;
	lpSendSocketContext->miPackType =*(WORD*)lpTcpSndOpt->mpData;
	lpSendSocketContext->mbSendClose = lpTcpSndOpt->mbNeedClose;

	CCriticalSection &	loCS = lpSocketInfo->moCS;
	loCS.Enter();

	if(lpSocketInfo->mbStopSend)
	{//停止发送了，没必要发送了
		loCS.Leave();
		LOG_TRACE(LOG_WARNING, true, __FUNCTION__, " Socket has close"
				<<" Socket="<<loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SerialID="<<lpSendSocketContext->miSerialID
				<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
				<<" LoadLen="<<lpTcpSndOpt->mwLen
				<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" RefCount="<<lpSocketInfo->GetRefCount()
		);
		mpSendCachePool->free(lpSendSocketContext);
		return RET_ERROR;
	}
	if(TRUE == lpSocketInfo->mbSendingData)
	{
		//正在发送数据
		if(0>lpSocketInfo->moWaitSendDQueue.push_back(lpSendSocketContext))
		{
			InsLostSend(loKey.mPeerIP, loKey.mwPeerPort);
			loCS.Leave();
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back fail"
					<<" Socket="<<loKey.mhSocket
					<<" IP="<<GetIPString(loKey.mPeerIP)
					<<":"<<ntohs(loKey.mwPeerPort)
					<<" SerialID="<<lpSendSocketContext->miSerialID
					<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
					<<" LoadLen="<<lpTcpSndOpt->mwLen
					<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
					<<" SocketInfoPtr="<<(void*)lpSocketInfo
					<<" WaitSize="<< lpSocketInfo->moWaitSendDQueue.size()
					<<" RefCount="<<lpSocketInfo->GetRefCount()
			);
			mpSendCachePool->free(lpSendSocketContext);
			return RET_ERROR;
		}

		//发送等待队列中的数据
		if(!SendWaitQueueData(lpSocketInfo))
		{
			lpSocketInfo->mbSendingData = FALSE;
			lpSocketInfo->mbStopSend = TRUE;
			clearWaitQueue(lpSocketInfo);//清除队列
		}
		loCS.Leave();

		LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "Wait when Send Packet"
				<<" Socket="<<loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SerialID="<<lpSendSocketContext->miSerialID
				<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
				<<" LoadLen="<<lpTcpSndOpt->mwLen
				<<" EncryptType="<<miEncryptType
				<<" NeedClose="<<lpTcpSndOpt->mbNeedClose
				<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" WaitSize="<< lpSocketInfo->moWaitSendDQueue.size()
				<<" RefCount="<<lpSocketInfo->GetRefCount()
		);
		return RET_SUCCESS;
	}
	//直接发送数据
	int liResult = DirectSendData(lpSocketInfo,lpSendSocketContext);
	if(0> liResult )
	{
		//错误
		InsLostSend(loKey.mPeerIP, loKey.mwPeerPort);
		loCS.Leave();
		mpSendCachePool->free(lpSendSocketContext);
		return RET_ERROR ;
	}
	if(0 == lpSendSocketContext->miBuffLen)
	{
		InsSend(loKey.mPeerIP, loKey.mwPeerPort);
		//发送完
		//LOG_TRACE(5,true, __FUNCTION__, "Packet direct send complete "
		//	<<" Socket="<<loKey.mhSocket
		//	<<" IP="<<GetIPString(loKey.mPeerIP)
		//	<<":"<<ntohs(loKey.mwPeerPort)
		//	<<" SerialID="<<lpSendSocketContext->miSerialID
		//	<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
		//	<<" LoadLen="<<lpTcpSndOpt->mwLen
		//	<<" EncryptType="<<miEncryptType
		//	<<" NeedClose="<<lpTcpSndOpt->mbNeedClose
		//	<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
		//	<<" SocketInfoPtr="<<(void*)lpSocketInfo
		//	<<" WaitSize="<< lpSocketInfo->moWaitSendDQueue.size()
		//	<<" RefCount="<<lpSocketInfo->GetRefCount()
		//	);
		loCS.Leave();
		mpSendCachePool->free(lpSendSocketContext);
		return RET_SUCCESS;
	}

	//加入到等待队列中
	if(0>lpSocketInfo->moWaitSendDQueue.push_back(lpSendSocketContext))
	{
		InsLostSend(loKey.mPeerIP, loKey.mwPeerPort);
		loCS.Leave();
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back fail"
				<<" Socket="<<loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SerialID="<<lpSendSocketContext->miSerialID
				<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
				<<" LoadLen="<<lpTcpSndOpt->mwLen
				<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" WaitSize="<< lpSocketInfo->moWaitSendDQueue.size()
				<<" RefCount="<<lpSocketInfo->GetRefCount()
		);
		mpSendCachePool->free(lpSendSocketContext);
		return RET_ERROR;
	}
	lpSocketInfo->mbSendingData = TRUE;

	loCS.Leave();
	LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "wish direct send but some deta not send"
			<<" Socket="<<loKey.mhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" SerialID="<<lpSendSocketContext->miSerialID
			<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
			<<" LoadLen="<<lpTcpSndOpt->mwLen
			<<" Remain"<<lpSendSocketContext->miBuffLen
			<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
			<<" SocketInfoPtr="<<(void*)lpSocketInfo
			<<" WaitSize="<< lpSocketInfo->moWaitSendDQueue.size()
			<<" RefCount="<<lpSocketInfo->GetRefCount()
	);
	return RET_SUCCESS;
}

// 函数说明： 关闭一个连接
INT CEpollBMgr::DisConnectTo(SOCKET ahSocket)
{
	if (ahSocket <= 0)
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "Inlidate socket, Socket = " << ahSocket);
		return RET_ERROR;
	}
	CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,ahSocket);
	if(loSocketInfo.Invalid())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Socket not found"
				<<" Socket="<<ahSocket
		);
		return RET_ERROR;
	}
	CSocketInfo *lpSocketInfo = loSocketInfo.GetPtr();
	CloseTCPSocket(lpSocketInfo);
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： GetNetStatisticsInfo
// 函数参数： astruInfo
// 返 回 值：
// 函数说明： 得到本地网络统计信息
//
// $_FUNCTION_END ********************************
INT CEpollBMgr::GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo)
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

// 初始化Epoll
bool CEpollBMgr::InitEpoll()
{
	mhEpollHandle = new int[mstruTcpOpt.mbyEpollCount];


	if(NULL== mhEpollHandle)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error malloc  errno:" << GETSOCKET_ERRCODE());
		return false;
	}
	
	for(int i=0;i<mstruTcpOpt.mbyEpollCount;++i)
	{
		mhEpollHandle[i] = INVALID_SOCKET;
	}

	for(int i=0;i<mstruTcpOpt.mbyEpollCount;++i)
	{
		mhEpollHandle[i] = epoll_create(mstruTcpOpt.muiMaxConnCount);
		if(-1 == mhEpollHandle[i])
		{
			mhEpollHandle[i] = INVALID_SOCKET;
			LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error create epoll errno:" << GETSOCKET_ERRCODE()
				<<" index="<<i
				);
			delete [] mhEpollHandle;

			return false;
		}
	}

	LOG_TRACE(LOG_ALERT, false, __FUNCTION__, "IniEoll Success :" << GETSOCKET_ERRCODE());
	return true;
}

// 开始在指定的端口监听
bool CEpollBMgr::StartListen(DWORD adwIPAddr, UINT aiPort)
{
	int liError =0;
	sockaddr_in lstrServerAddr;
	memset(&lstrServerAddr, 0, sizeof(lstrServerAddr));

	lstrServerAddr.sin_addr.s_addr = adwIPAddr;
	lstrServerAddr.sin_family = AF_INET;
	lstrServerAddr.sin_port   = aiPort;

	mhServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	if(INVALID_SOCKET == mhServerSocket)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Create socket error:" << GETSOCKET_ERRCODE());
		return false;
	}

	//重用端口
	int val  = 1;
	liError = setsockopt(mhServerSocket, SOL_SOCKET, SO_REUSEADDR,
						 (char *)&val, sizeof(int));
	if (SOCKET_ERROR == liError)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "setsockopt with SO_REUSEADDR fault. errno:" << GETSOCKET_ERRCODE());
	}

	//if(!SetSocektNoBlock(mhServerSocket))
	//{
	//	return false;
	//}

	// 绑定指定的地址和端口
	liError = bind(mhServerSocket, (const struct sockaddr *)&lstrServerAddr, sizeof(struct sockaddr_in));
	if (SOCKET_ERROR == liError)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "bind() fault, error:" << GETSOCKET_ERRCODE());
		return false;
	}

	liError = listen(mhServerSocket, SOMAXCONN);
	if (SOCKET_ERROR == liError)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "listen() fault, error:" << GETSOCKET_ERRCODE());
		return false;
	}

	LOG_TRACE(LOG_ALERT, true, __FUNCTION__, "listen sucess port:" << aiPort);
	return true;
}

BOOL CEpollBMgr::AssociateSocketwithEpoll(CSocketInfo *apSocketInfo)
{
	STRU_SOCKET_KEY &loKey = apSocketInfo->moKey;
	epoll_event loEpollEvent;
	ZeroMemory(&loEpollEvent,sizeof(loEpollEvent));
	loEpollEvent.data.fd = loKey.mhSocket;
	loEpollEvent.data.ptr = apSocketInfo;
	loEpollEvent.events = EPOLLIN| EPOLLET| EPOLLHUP|EPOLLOUT;

	int32 liIndex = loKey.mhSocket % mstruTcpOpt.mbyEpollCount;

	if(0 != epoll_ctl(mhEpollHandle[liIndex], EPOLL_CTL_ADD, loKey.mhSocket, &loEpollEvent))
	{
		int liError = errno;
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error epoll_ctl fail"
				<<" errcode=" << liError
				<<" Socket="<<loKey.mhSocket
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
		);
		return FALSE;
	}

	LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "epoll_ctl add success"
			<<" Socket="<<loKey.mhSocket
			<<" EpollCount="<< mstruTcpOpt.mbyEpollCount
			<<" liIndex="<<liIndex
			<<" SocketInfoPtr="<<(void*)apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
	);
	return TRUE;
}

//设置SOCKET为非阻塞状态
bool CEpollBMgr::SetSocektNoBlock(SOCKET ahSocket)
{
	//设置非阻塞方式
	int liResult = fcntl(ahSocket,F_GETFL);
	if(liResult<0)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "fcntl(get) "
				<<" Errcode=" << GETSOCKET_ERRCODE()
				<<" Socket="<<ahSocket);
		return false;
	}
	liResult = liResult|O_NONBLOCK;

	if(fcntl(ahSocket,F_SETFL,liResult)<0)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "fcntl(set)  "
				<<" Errcode=" << GETSOCKET_ERRCODE()
				<<" Socket="<<ahSocket);
		return false;
	}

	struct linger lling;
	lling.l_onoff = 0;
	lling.l_linger = 0; //为秒级的一个值
	int liErr = setsockopt(ahSocket, SOL_SOCKET, SO_LINGER,
						   (char *)&lling, sizeof(struct linger));
	if (SOCKET_ERROR == liErr)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__,  "setsockopt with SO_LINGER failed,  "
				<<" Errcode=" << GETSOCKET_ERRCODE()
				<<"Socket="<<ahSocket);
		return false;
	}

	// 设置接收缓冲区
	int liRecvBuf=1024*1024*1024;
	if(0>setsockopt(ahSocket, SOL_SOCKET, SO_RCVBUF,
					(const char*)&liRecvBuf,sizeof(int)))
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Set recv buffer error. "
				<<" Errcode=" << GETSOCKET_ERRCODE()
				<<" Socket="<<ahSocket);
	}

	// 设置发送缓冲区
	int liSendBuf=1024*1024*1024;
	if(0>setsockopt(ahSocket, SOL_SOCKET, SO_SNDBUF,
					(const char*)&liSendBuf,sizeof(int)))
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Set send buffer error.  "
				<<" Errcode=" << GETSOCKET_ERRCODE()
				<<" Socket="<<ahSocket);
	}
	return true;
}

//设置SOCKET为非阻塞状态
bool CEpollBMgr::SetSocektNODELAY(SOCKET ahSocket)
{
	int flag = 1;
	int result = setsockopt(ahSocket,            /* socket affected */
							IPPROTO_TCP,     /* set option at TCP level */
							TCP_NODELAY,     /* name of option */
							(char *) &flag,  /* the cast is historical               cruft */
							sizeof(int));    /* length of option value */

	if (SOCKET_ERROR == result)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__,  "setsockopt with TCP_NODELAY failed,  "
				<<" Errcode=" << GETSOCKET_ERRCODE()
				<<"Socket="<<ahSocket);
		return false;
	}

	return true;
}

void CEpollBMgr::WaitClose()
{
	int i=0;
	while (mlRefCount>0)
	{
		CSystem::Sleep(10);
	}
}

void CEpollBMgr::CloseTCPSocket(CSocketInfo *apSocketInfo)
{
	BOOL lbNeedClose = FALSE;
	STRU_SOCKET_KEY& loKey = apSocketInfo->moKey;
	SOCKET lhSocket = loKey.mhSocket;
	apSocketInfo->moCS.Enter();
	if(FALSE ==apSocketInfo->mbHasClosed)
	{
		lbNeedClose = TRUE;
		apSocketInfo->mbHasClosed = TRUE;
	}
	apSocketInfo->moCS.Leave();

	if(lbNeedClose)
	{
		LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "need shutdown socket"
				<<" Errcode="<<GETSOCKET_ERRCODE()
				<<" Socket="<<lhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfo="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
		);
		shutdown(lhSocket,SD_BOTH);//lpSocketInfo的clear函数会close socket的
		mpSocketInfoMgr->DeleteRefObj(lhSocket);
	}
	else
	{
		LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "socket has shutdown"
				<<" Errcode="<<GETSOCKET_ERRCODE()
				<<" Socket="<<lhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfo="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
		);
	}
}

void CEpollBMgr::CloseTCPSocketInNetIOProcessThread(CSocketInfo *apSocketInfo, const int32 &aiThreadID)
{// 处理关闭
	//将socket和epoll解除关联。
	BOOL lbNeedClose = FALSE;
	STRU_SOCKET_RECV_CONTEXT *lpRecvContext = apSocketInfo->mpRecvContext;
	STRU_SOCKET_KEY	&loKey = apSocketInfo->moKey;
	SOCKET lhSocket = loKey.mhSocket;
	CCriticalSection&	loCS = apSocketInfo->moCS;
	loCS.Enter();
	if(FALSE ==apSocketInfo->mbHasClosed)
	{
		lbNeedClose = TRUE;
		apSocketInfo->mbHasClosed = TRUE;
	}
	//清除等待队列
	if(0 !=  apSocketInfo->moWaitSendDQueue.size())
		clearWaitQueue(apSocketInfo);

	loCS.Leave();

	if(lbNeedClose)
	{//关闭socket
		LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__,  "need shutdown socket ,success"
				<<" Socket="<<lhSocket
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
		);
		shutdown(lhSocket,SD_BOTH);//lpSocketInfo的clear函数会close socket的
		mpSocketInfoMgr->DeleteRefObj(lhSocket);
	}
	else
	{
		LOG_TRACE(LOG_WARNING, true, __FUNCTION__,  "socket has shutdown"
				<<" Socket="<<lhSocket
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
		);
	}
	//断开socket和epoll的关联 必须在2.6.9 内核以上
	int32 liIndex = lhSocket % mstruTcpOpt.mbyEpollCount;
	if(0 != epoll_ctl(mhEpollHandle[liIndex], EPOLL_CTL_DEL, lhSocket, NULL))
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__,  "Error epoll_ctl fail"
				<<" errno="<<GETSOCKET_ERRCODE()
				<<" Socket="<<lhSocket
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
		);
	};

	DescCurCon(aiThreadID);
	int64 li64Session = loKey.mwPeerPort;
	li64Session = (li64Session << 32) + loKey.mPeerIP;
	if(mstruTcpOpt.mbUseRelationPtr)
	{
		void * lppRelationPtr = apSocketInfo->mpRelationPtr;
		mpMsgSink->OnSessionCloseEx(li64Session,lppRelationPtr, lhSocket,
									loKey.mPeerIP, loKey.mwPeerPort);
	}
	else
	{
		mpMsgSink->OnSessionClose(li64Session, lhSocket, loKey.mPeerIP, loKey.mwPeerPort);
	}
}

//关闭所有的连接
void CEpollBMgr::CloseAllSocket()
{
	CStaticRefObjMgrTemplate1<SOCKET,CSocketInfo>::refobjid_map *lpMap = mpSocketInfoMgr->GetRefObjMap();
	for(CStaticRefObjMgrTemplate1<SOCKET,CSocketInfo>::refobjid_map::iterator ite = lpMap->begin();ite != lpMap->end();ite++)
	{
		CloseSocket(ite->first);
	}
	mpSocketInfoMgr->clear();
}


LONG CEpollBMgr::IncreaseRef()
{
	LONG llValue=0;
	moRefCriticalSection.Enter();
	mlRefCount++;
	llValue=mlRefCount;
	moRefCriticalSection.Leave();
	return llValue;
}

//引用计数减1
LONG CEpollBMgr::DecreaseRef()
{
	LONG llValue=0;
	moRefCriticalSection.Enter();
	if(mlRefCount>0)
		mlRefCount--;
	llValue=mlRefCount;
	moRefCriticalSection.Leave();
	return llValue;
}

int CEpollBMgr::DirectSendData(CSocketInfo *apSocketInfo,STRU_SOCKET_SEND_CONTEXT *apSendSocketContext)
{
	//发送
	int32 liSentLen = SendPacket(apSocketInfo->moKey.mhSocket,
		&apSendSocketContext->mpBuff[apSendSocketContext->miOffset],apSendSocketContext->miBuffLen);
	if(liSentLen == apSendSocketContext->miBuffLen )
	{//发送完毕
		LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "SendPacket complete "
			<<" Socket="<<apSocketInfo->moKey.mhSocket
			<<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
			<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
			<<" SocketInfoPtr="<<(void*)apSocketInfo
			<<" RefCount = "<< apSocketInfo->GetRefCount()
			<<" SendSocketContextPtr="<<(void*)apSendSocketContext
			<<" SerialID="<<apSendSocketContext->miSerialID
			<<" HasSentLen="<< apSendSocketContext->miOffset
			<<" SentLen= " << liSentLen
			);
		apSendSocketContext->miBuffLen =0;
		apSendSocketContext->miOffset =0;
		if(TRUE == apSendSocketContext->mbSendClose)
		{
			LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "SendPacket complete and close"
				<<" Socket="<<apSocketInfo->moKey.mhSocket
				<<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
				<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount = "<< apSocketInfo->GetRefCount()
				<<" SendSocketContextPtr="<<(void*)apSendSocketContext
				<<" SerialID="<<apSendSocketContext->miSerialID
				<<" HasSentLen="<<apSendSocketContext->miOffset
				<<" waitsize="<<apSocketInfo->moWaitSendDQueue.size()
				);

			//这样做。是为了客户端能够有时间处理这次发送的包。而不要直接close
			DWORD ldwIndex = GetDealThreadIndex(apSocketInfo->moKey.mPeerIP,apSocketInfo->moKey.mwPeerPort);
			STRU_EVENT_DATA *lpEvent = moEventCacheQueue.malloc();
			lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CLOSE_SOCKET_AT_SEND_COMPLETE;
			lpEvent->mhSocket = apSocketInfo->moKey.mhSocket;
			lpEvent->miIP = apSocketInfo->moKey.mPeerIP;
			lpEvent->mwPort = apSocketInfo->moKey.mwPeerPort;

			lpEvent->mpRelationPtr = apSocketInfo->mpRelationPtr;

			mpEventQueue[ldwIndex].push_back(lpEvent);
		}
		return liSentLen;
	}

	if(0> liSentLen )
	{//错误
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error SendPacket fail"
			<<" Errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<apSocketInfo->moKey.mhSocket
			<<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
				<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount = "<< apSocketInfo->GetRefCount()
				<<" SendSocketContextPtr="<<(void*)apSendSocketContext
				<<" SerialID="<<apSendSocketContext->miSerialID
				<<" HasSentLen="<<apSendSocketContext->miOffset
				<<" waitsize="<<apSocketInfo->moWaitSendDQueue.size()
			);
		return liSentLen ;
	}

	apSendSocketContext->miBuffLen -= liSentLen;
	apSendSocketContext->miOffset += liSentLen;
	return liSentLen;
}

int32 CEpollBMgr::SendPacket(SOCKET ahScoket,BYTE* apBuffer,int32 aiLen)
{
	int liSendLen = send(ahScoket,apBuffer,aiLen,0);

	int liError = errno;
	if(liSendLen == aiLen)
	{
		//发送完成
		return aiLen;
	}
	if(liSendLen>=0)
	{
		//没有完全发送完一个数据包
		return liSendLen;
	}

	if(EAGAIN == liError)
	{// socket 系统缓存区满。下次发送
		return 0;
	}

	return -1;
}

BOOL CEpollBMgr::SendWaitQueueData(CSocketInfo *apSocketInfo)
{
	STRU_SOCKET_SEND_CONTEXT *lpSendSocketContext = NULL;
	U9_POSITION lpPos = apSocketInfo->moWaitSendDQueue.begin();
	U9_POSITION lpTempPos = NULL;
	while(NULL != lpPos)
	{
		lpSendSocketContext =  apSocketInfo->moWaitSendDQueue.getdata(lpPos);
		int liSentLen = DirectSendData(apSocketInfo,lpSendSocketContext);
		if(0> liSentLen )
		{//错误
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error DirectSendData fail"
				<<" Errcode="<<GETSOCKET_ERRCODE()
				<<" Socket="<<apSocketInfo->moKey.mhSocket
				<<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
				<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount = "<< apSocketInfo->GetRefCount()
				<<" SendSocketContextPtr="<<(void*)lpSendSocketContext
				<<" waitsize="<<apSocketInfo->moWaitSendDQueue.size()
				);
			apSocketInfo->moWaitSendDQueue.erase(lpPos);
			mpSendCachePool->free(lpSendSocketContext);
			return FALSE ;
		}
		if(0 < lpSendSocketContext->miBuffLen)
		{//没有发送完
			LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "send complete and remain data"
				<<" Errcode="<<GETSOCKET_ERRCODE()
				<<" Socket="<<apSocketInfo->moKey.mhSocket
				<<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
				<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount = "<< apSocketInfo->GetRefCount()
				<<" SendSocketContextPtr="<<(void*)lpSendSocketContext
				<<" SerialID="<<lpSendSocketContext->miSerialID
				<<" SentLen="<<liSentLen
				<<" Remainlen="<<lpSendSocketContext->miBuffLen
				<<" HasSentLen="<<lpSendSocketContext->miOffset
				<<" waitsize="<<apSocketInfo->moWaitSendDQueue.size()
				);
			return TRUE;
		}
		//LOG_TRACE(7, true, __FUNCTION__, "send complete" 
		//	<<" Errcode="<<GETSOCKET_ERRCODE()
		//	<<" Socket="<<apSocketInfo->moKey.mhSocket
		//	<<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
		//	<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
		//	<<" SocketInfoPtr="<<(void*)apSocketInfo
		//	<<" RefCount = "<< apSocketInfo->GetRefCount()
		//	<<" SendSocketContextPtr="<<(void*)lpSendSocketContext
		//	<<" SerialID="<<lpSendSocketContext->miSerialID
		//	<<" SentLen="<<liSentLen
		//	<<" waitsize="<<apSocketInfo->moWaitSendDQueue.size()
		//	);
		//
		//发送完
		lpTempPos = lpPos;
		lpPos = apSocketInfo->moWaitSendDQueue.next(lpPos);
		apSocketInfo->moWaitSendDQueue.erase(lpTempPos);
		InsSend(apSocketInfo->moKey.mPeerIP, apSocketInfo->moKey.mwPeerPort);
		mpSendCachePool->free(lpSendSocketContext);
	}
	apSocketInfo->mbSendingData = FALSE;
	return TRUE;
}

void CEpollBMgr::clearWaitQueue(CSocketInfo *apSocketInfo)
{
	CContextDQueue<STRU_SOCKET_SEND_CONTEXT>  &loDqueue =  apSocketInfo->moWaitSendDQueue;
	STRU_SOCKET_KEY &loKey = apSocketInfo->moKey;
	STRU_SOCKET_SEND_CONTEXT *lpSocketContext = NULL;
	int32 liCount=loDqueue.size();
	if(liCount == 0)
		return ;
	loDqueue.GetCriticalSection().Enter();
	U9_POSITION loPos =loDqueue.begin();
	while ( loPos != INVALID_U9_POSITION)
	{
		lpSocketContext = loDqueue.getdata(loPos);
		loPos = loDqueue.next(loPos);
		mpSendCachePool->free(lpSocketContext);

		InsLostSend(loKey.mPeerIP, loKey.mwPeerPort);
		LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, " clear a wait packet "
				<<" Socket="<<loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfo="<<(void*)apSocketInfo
				<<" SendSocketContext="<<(void*)lpSocketContext
				<<" WaitCount="<<loDqueue.size()
				<<" RefCount="<<apSocketInfo->GetRefCount()
		);
	}
	loDqueue.clear();
	loDqueue.GetCriticalSection().Leave();

	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, " "
			<<" RefCount="<<apSocketInfo->GetRefCount()
			<<" Socket="<<loKey.mhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" SocketInfo="<<(void*)apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
			<<" WaitCount="<<liCount
	);

}

BOOL CEpollBMgr::RecvTCPData(CSocketInfo *apSocketInfo,BOOL & abNeedClose, const int32 &aiThreadID)
{//读取数据添加到接收缓冲区 需要做累包和分包处理
	STRU_SOCKET_RECV_CONTEXT& loRecvContext =   *apSocketInfo->mpRecvContext;
	STRU_SOCKET_KEY& loKey=apSocketInfo->moKey;

	//从tcp读取数据
	BYTE* lpBegin = loRecvContext.mpBuff;
	INT liRecvLen = recv(loKey.mhSocket,(char *)lpBegin+loRecvContext.miBuffLen, DEF_RECV_PACK_BUFF_LEN-loRecvContext.miBuffLen,0);
	LOG_TRACE(LOG_DEBUG_1,TRUE,__FUNCTION__," CEpollMgr::liRecvLen:" << liRecvLen);
	INT liError=errno;
	if (SOCKET_ERROR == liRecvLen)
	{
		if(liError==EAGAIN)
		{//数据接收完毕
			//LOG_TRACE(5, false, __FUNCTION__, "recv , errno=" << liError
			//	<<" Socket="<<loRecvContext.mhSocket
			//	<<" IP="<<GetIPString(loKey.mPeerIP)
			//	<<":"<<ntohs(loKey.mwPeerPort)
			//	<<" DataLen="<<loRecvContext.miBuffLen
			//	<<" SocketInfoPtr="<<(void*)&loRecvContext.mpSocketInfo
			//	);
			return FALSE;
		}
		abNeedClose = TRUE;
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error recv failed, "
				<<" errno=" << liError
				<<" Socket="<<loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				<<" DataLen="<<loRecvContext.miBuffLen
		);
		return FALSE;
	}
	else if (0 == liRecvLen)
	{
		abNeedClose = TRUE;
		//关闭方式
		LOG_TRACE(LOG_NOTICE, false, __FUNCTION__, "socket closed"
				<<" Socket="<<loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				<<" DataLen="<<loRecvContext.miBuffLen
		);
		return false;
	}

	//重新计算数据长度
	liRecvLen+=loRecvContext.miBuffLen;
	if(liRecvLen> DEF_RECV_PACK_BUFF_LEN)
	{//数据错误 重新开始记录
		abNeedClose = TRUE;

		//清除所有的缓冲区
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error recv count is bad"
				<<" Socket="<<loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				<<" DataLen="<<loRecvContext.miBuffLen
				<<" RecvLen="<<liRecvLen
		);
		loRecvContext.miBuffLen = 0;
		return false;
	}
	//处理数据包 进行分包处理

	INT liPackLen = 0;
	while((liPackLen = CTCPPacket::GetBPack(lpBegin, liRecvLen))>0)
	{
		LOG_TRACE(LOG_DEBUG_1,TRUE,__FUNCTION__," CEpollMgr::RecvTCPData:" << liPackLen);

		CTCPPacket *lpTCPPacket = moRecvPackCacheList.malloc();
//		if(moRecvPackCacheList.remaincount() <= 0)
//		{
//			LOG_TRACE(LOG_NOTICE, false, __FUNCTION__, "moRecvPackCacheList too large size=" << moRecvPackCacheList.size());
//		}

		if (NULL == lpTCPPacket)
		{
			InsLostRecv(aiThreadID);
			//printf("New lpTCPPacket fail %s:%d \n\r",GetIPString(aiIP).c_str(),ntohs(awPort));
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error allocate tcp packet failed."
					<<" Socket="<< loKey.mhSocket
					<<" IP="<< GetIPString(loKey.mPeerIP)
					<<":"<< ntohs(loKey.mwPeerPort)
					<<" Size="<< liPackLen
					<<"cachesize" << moRecvPackCacheList.size());
			return -1;
		}

		lpTCPPacket->init();
		SESSIONID liSessionID = 0;
		if(0> lpTCPPacket->UnPackB(lpBegin, liPackLen, liSessionID))
		{
			//解包失败
			InsLostRecv(aiThreadID);
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Unpack failed."
					<<" Socket="<<loKey.mhSocket
					<<" IP="<<GetIPString(loKey.mPeerIP)
					<<":"<<ntohs(loKey.mwPeerPort)
					<<" Size="<<liPackLen
					<<" EncryptType="<<miEncryptType
			);
			moRecvPackCacheList.free(lpTCPPacket);

			//关闭连接
			DisConnectTo(loKey.mhSocket);
			return -1;
		}

		DWORD ldwIndex = GetDealThreadIndex(liSessionID);
//		if(moEventCacheQueue.remaincount() <= 1)
//		{
//			LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, " moEventCacheQueue too large size=" << moEventCacheQueue.size());
//		}

		STRU_EVENT_DATA *lpEvent=moEventCacheQueue.malloc();
		lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_RECV;
		lpEvent->mhSocket = loKey.mhSocket;
		lpEvent->miIP = loKey.mPeerIP;
		lpEvent->mwPort = loKey.mwPeerPort;
		lpEvent->mpParam1 = lpTCPPacket;
		lpEvent->mpRelationPtr = (void*)apSocketInfo;

		if(INVALID_U9_POSITION ==  mpEventQueue[ldwIndex].push_back(lpEvent))
		{
			InsLostRecv(aiThreadID);
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back failed."
					<<" Socket="<<loKey.mhSocket
					<<" IP="<<GetIPString(loKey.mPeerIP)
					<<":"<<ntohs(loKey.mwPeerPort)
					<<" Size="<<liPackLen
					<<" EncryptType="<<miEncryptType
					<<" Index="<<ldwIndex
			);
			moEventCacheQueue.free(lpEvent);
		}

		InsRecv(aiThreadID);
		lpBegin += liPackLen;
		liRecvLen -= liPackLen;
	}

	if(liPackLen<0)
	{
		abNeedClose = TRUE;
		//数据包错误
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error data excption"
				<<" Socket="<<loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				<<" DataLen="<<loRecvContext.miBuffLen
				<<" RecvLen="<<liRecvLen

		);
		//清除所有的缓冲区
		loRecvContext.miBuffLen = 0;
		return false;
	}

	if(0 ==liRecvLen)
	{//buff已经没有数据
		loRecvContext.miBuffLen = 0;
	}
	else
	{//buff还有数据
		//更新缓冲区长度
		loRecvContext.miBuffLen = liRecvLen;

		if(lpBegin !=loRecvContext.mpBuff)
		{//对齐缓冲区中的数据
			memmove(loRecvContext.mpBuff,lpBegin, liRecvLen);
		}
	}
	return true;
}

void CEpollBMgr::NetIOProcessThread(VOID *apParam)
{
	STRU_TRHEAD_CONTEXT * lpThreadContext = (STRU_TRHEAD_CONTEXT *)apParam;
	CEpollBMgr* lpThis =(CEpollBMgr*) lpThreadContext->mpThis;
	if(NULL ==lpThis )
		return ;

	lpThis->IncreaseRef();
	lpThis->NetIOProcessThread(lpThreadContext->dwIndex);
	lpThis->DecreaseRef();
}
// 网络通讯主线程函数
#define DEF_MAX_EPOLL_EVENT       (512)
void CEpollBMgr::NetIOProcessThread(int32 aiIndex)
{
	int	liEpollNotifyCount=0;
	epoll_event	lpEventList[DEF_MAX_EPOLL_EVENT];
	int liSessionID=0;
	while (mbIsRun)
	{
		liEpollNotifyCount = epoll_wait(mhEpollHandle[aiIndex], lpEventList, DEF_MAX_EPOLL_EVENT,50);
		if (liEpollNotifyCount <= 0)
			continue;

		liSessionID++;
		for (int i = 0; i < liEpollNotifyCount; i++)
		{
			epoll_event & loEpollEvent = lpEventList[i];
			CSocketInfo *lpSocketInfo = (CSocketInfo *)loEpollEvent.data.ptr;
			STRU_SOCKET_KEY &loKey = lpSocketInfo->moKey;

			//外连而未决的socket
			if(lpSocketInfo->miNetState == ENUM_SOCKET_STATE_CONNECTING)
			{
				LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "not  connect"
						<<" Socket="<<lpSocketInfo->moKey.mhSocket
						<<" ip="<<GetIPString(lpSocketInfo->moKey.mPeerIP)
						<<":"<<ntohs(lpSocketInfo->moKey.mwPeerPort)
						<<" events=:"<<(void*)loEpollEvent.events
				);
				continue;
			}

			if(loEpollEvent.events & EPOLLIN)
			{//接受数据
				//LOG_TRACE(7, true, __FUNCTION__, " EPOLLIN Event "
				//	<<" Socket="<<loKey.mhSocket
				//	<<" ip="<<GetIPString(loKey.mPeerIP)
				//	<<":"<<ntohs(loKey.mwPeerPort)
				//	<<" SocketInfoPtr="<<(void*)lpSocketInfo
				//	<<" RefCount="<<lpSocketInfo->GetRefCount()
				//	);
				BOOL lbNeedClose = FALSE;
				while(RecvTCPData(lpSocketInfo,lbNeedClose, aiIndex))
				{//接收数据

				}
				if(lbNeedClose)
				{//处理close,接收错误
					lpSocketInfo->mbStopSend = true;
					LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "socket need close"
							<<" Socket="<<loKey.mhSocket
							<<" ip="<<GetIPString(loKey.mPeerIP)
							<<":"<<ntohs(loKey.mwPeerPort)
							<<" SocketInfoPtr="<<(void*)lpSocketInfo
							<<" RefCount="<<lpSocketInfo->GetRefCount()
							<<" Session="<<liSessionID
							<<" i="<<i
							<<" EpollNotifyCount="<<liEpollNotifyCount
					);
					CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
					CloseTCPSocketInNetIOProcessThread(lpSocketInfo, aiIndex);
					continue;
				}
				continue;
			}
			if(loEpollEvent.events & EPOLLOUT)
			{//socket buff为空了。
				//LOG_TRACE(7, true, __FUNCTION__, " EPOLLOUT Event:"
				//	<<" Socket="<<loKey.mhSocket
				//	<<" ip="<<GetIPString(loKey.mPeerIP)
				//	<<":"<<ntohs(loKey.mwPeerPort)
				//	<<" SocketInfoPtr="<<(void*)lpSocketInfo
				//	<<" RefCount="<<lpSocketInfo->GetRefCount()
				//	);
				CCriticalSection &	loCS = lpSocketInfo->moCS;
				loCS.Enter();

				if(FALSE == lpSocketInfo->mbStopSend)
				{//没有停止发送
					if(lpSocketInfo->moWaitSendDQueue.size() ==0)
					{//没有数据了
						lpSocketInfo->mbSendingData = FALSE;
					}
					else
					{//发送发送队列
						if(!SendWaitQueueData(lpSocketInfo))
						{
							lpSocketInfo->mbSendingData = FALSE;
							lpSocketInfo->mbStopSend = TRUE;
							clearWaitQueue(lpSocketInfo);//清除队列

						}
					}
				}
				loCS.Leave();
				continue;
			}
			if(loEpollEvent.events & EPOLLHUP)
			{//出错
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, "EPOLLHUP event "
						<<" Socket="<<loKey.mhSocket
						<<" ip="<<GetIPString(loKey.mPeerIP)
						<<":"<<ntohs(loKey.mwPeerPort)
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						<<" Session="<<liSessionID
						<<" i="<<i
						<<" EpollNotifyCount="<<liEpollNotifyCount
				);
				CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
				CloseTCPSocketInNetIOProcessThread(lpSocketInfo, aiIndex);
				continue;
			}
			if(loEpollEvent.events & EPOLLERR)
			{//出错
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, "EPOLLERR event "
						<<" Socket="<<loKey.mhSocket
						<<" ip="<<GetIPString(loKey.mPeerIP)
						<<":"<<ntohs(loKey.mwPeerPort)
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						<<" Session="<<liSessionID
						<<" i="<<i
						<<" EpollNotifyCount="<<liEpollNotifyCount
				);
				CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
				CloseTCPSocketInNetIOProcessThread(lpSocketInfo, aiIndex);
				continue;
			}
			if(loEpollEvent.events & EPOLLPRI)
			{//出错
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, "EPOLLPRI event "
						<<" Socket="<<loKey.mhSocket
						<<" ip="<<GetIPString(loKey.mPeerIP)
						<<":"<<ntohs(loKey.mwPeerPort)
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						<<" Session="<<liSessionID
						<<" i="<<i
						<<" EpollNotifyCount="<<liEpollNotifyCount
				);
				CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
				CloseTCPSocketInNetIOProcessThread(lpSocketInfo, aiIndex);
				continue;
			}
		}
	}
}
void CEpollBMgr::AcceptThreadFunction(VOID *apParam)
{
	CEpollBMgr* lpThis =(CEpollBMgr*) apParam;
	if(NULL ==lpThis )
		return ;
	lpThis->IncreaseRef();
	lpThis->AcceptTimeoutThread();
	lpThis->DecreaseRef();
}
void CEpollBMgr::AcceptTimeoutThread()
{
	STRU_EVENT_DATA *lpEvent=NULL;
	DWORD ldwIndex = 0;

	CSocketInfo *lpSocketInfo = NULL;
	CSocketInfo *lpOldSocketInfo = NULL;

	int liAddrlen=0;
	sockaddr_in	lstruAddr, lstruPeerAddr;
	SOCKET lhNewSocket=INVALID_SOCKET;
	while (mbIsRun)
	{
		liAddrlen = sizeof(sockaddr_in);
		lhNewSocket = accept(mhServerSocket, (sockaddr*)&lstruAddr, (socklen_t*)&liAddrlen);
		int liError = GETSOCKET_ERRCODE();
		if(INVALID_SOCKET == lhNewSocket)
		{
			//错误
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error accept fail "
					<<"errcode="<<liError
			);
			continue;
		}
		LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "accept new socket"
				<<" Socket="<<lhNewSocket
		);
		if(!SetSocektNoBlock(lhNewSocket))
		{
			LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error SetSocektNoBlock fail "
					<<" errcode="<<GETSOCKET_ERRCODE()
					<<" Socket="<<lhNewSocket
			);
			shutdown(lhNewSocket,SD_BOTH);
			CloseSocket(lhNewSocket);
			continue;
		}

		if(!SetSocektNODELAY(lhNewSocket))
		{
			LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error SetSocektNODELAY fail "
					<<" errcode="<<GETSOCKET_ERRCODE()
					<<" Socket="<<lhNewSocket
			);
			shutdown(lhNewSocket,SD_BOTH);
			CloseSocket(lhNewSocket);
			continue;
		}

		lpSocketInfo = mpSocketInfoMgr->MallocRefObj();
		if (NULL == lpSocketInfo)
		{
			LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error mpSocketInfoMgr->MallocRefObj()"
					<<" Socket="<<lhNewSocket
			);
			shutdown(lhNewSocket,SD_BOTH);
			CloseSocket(lhNewSocket);
			continue;
		}
		lpSocketInfo->mpRecvContext->miBuffLen=0;
		//加入对象
		lpOldSocketInfo = mpSocketInfoMgr->AddRefObj(lhNewSocket,lpSocketInfo);
		if(lpOldSocketInfo != lpSocketInfo)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error mpSocketInfoMgr->AddRefObj exist obj"
					<<" Socket="<<lhNewSocket
					<<" SocketInfoPtr="<<(void*)lpSocketInfo
					<<" RefCount="<<lpSocketInfo->GetRefCount()
					<<" Old SocketInfoPtr="<<(void*)lpOldSocketInfo
					<<" Old RefCount="<<lpOldSocketInfo->GetRefCount()
			);
			U9_ASSERT(FALSE);
			shutdown(lhNewSocket,SD_BOTH);
			CloseSocket(lhNewSocket);
			mpSocketInfoMgr->FreeRefObj(lpSocketInfo);
			continue;
		}

		STRU_SOCKET_KEY &loKey = lpSocketInfo->moKey;
		loKey.mhSocket=lhNewSocket;
		loKey.mPeerIP = lstruAddr.sin_addr.s_addr;
		loKey.mwPeerPort = lstruAddr.sin_port;
		lpSocketInfo->IncreaseRef();//增加一次引用

		int64 li64Session = lstruAddr.sin_port;
		li64Session = (li64Session << 32) + lstruAddr.sin_addr.s_addr;

		//直接通知应用层
		if (mpMsgSink)
		{
			if(mstruTcpOpt.mbUseRelationPtr)
				mpMsgSink->OnSessionConnectedEx(li64Session, (void*)lpSocketInfo, lhNewSocket,
												lstruAddr.sin_addr.s_addr, lstruAddr.sin_port);
			else
				mpMsgSink->OnSessionConnected(li64Session, lhNewSocket, lstruAddr.sin_addr.s_addr, lstruAddr.sin_port);
		}

		if(FALSE ==AssociateSocketwithEpoll(lpSocketInfo))
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error AssociateSocketwithEpoll fail "
					<<" errcode="<<GETSOCKET_ERRCODE()
					<<" Socket="<<lhNewSocket
					<<" SocketInfoPtr="<<(void*)lpSocketInfo
					<<" RefCount="<<lpSocketInfo->GetRefCount()
			);

			//直接通知应用层
			if (mpMsgSink)
			{
				if(mstruTcpOpt.mbUseRelationPtr)
				{
					void * lppRelationPtr = lpSocketInfo->mpRelationPtr;
					mpMsgSink->OnSessionCloseEx(li64Session,lppRelationPtr, lhNewSocket,
												lstruAddr.sin_addr.s_addr, lstruAddr.sin_port);
				}
				else
				{
					mpMsgSink->OnSessionClose(li64Session, lhNewSocket, lstruAddr.sin_addr.s_addr, lstruAddr.sin_port);
				}
			}

			lpSocketInfo->DecreaseRef();
			shutdown(lhNewSocket,SD_BOTH);//lpSocketInfo的clear函数会close socket的
			mpSocketInfoMgr->DeleteRefObj(lhNewSocket);
			continue;
		}

		InsCurCon(STATIS_LIST_SIZE - 1);
		continue;
	}
}
void CEpollBMgr::DealThread(VOID *apParam)
{
	CSystem::InitRandom();
	STRU_TRHEAD_CONTEXT *lpThreadContext = (STRU_TRHEAD_CONTEXT *)apParam;
	CEpollBMgr* lpThis =(CEpollBMgr*) lpThreadContext->mpThis;
	if(NULL ==lpThis )
		return ;

	lpThis->IncreaseRef();
	lpThis->DealThread(lpThreadContext->dwIndex);
	lpThis->DecreaseRef();

	delete lpThreadContext;
}
void CEpollBMgr::DealThreadEx(DWORD adwIndex)
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
				{
					li64LastTime=li64NowTime;
					liSecondCount++;
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
		}

		switch(lpEvent->mTypeEvent)
		{
			case STRU_EVENT_DATA::ENUM_RECV://接收数据
			{
				lpTCPPacket = (CTCPPacket*)lpEvent->mpParam1;
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
void CEpollBMgr::DealThread(DWORD adwIndex)
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
void CEpollBMgr::DealThread(DWORD adwIndex)
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

#ifndef WIN32
int32_t set_fd_nonblock(int32_t fd)
{
	int32_t flags = 0;
	int32_t retval = 0;

	flags = fcntl( fd, F_GETFL );
	if ( flags >= 0 )
	{
		flags |= O_NONBLOCK;
		if ( fcntl( fd, F_SETFL, flags ) >= 0 )
			retval = 1;
	}

	return retval;
}
#endif