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

#include "EpollMgr.h"

#if __cplusplus > 199711L
#include<regex>
#endif

// 标准构造
CEpollMgr::CEpollMgr()
{
	mhEpollHandle = NULL;

	//mpSendDQueue = NULL;
}

// 标准析构
CEpollMgr::~CEpollMgr()
{
	//SAFE_DELETE_ARRAY(mpSendDQueue);
}


// 初始化Epoll
bool CEpollMgr::InitEpoll()
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

		//发送队列
	//mpSendDQueue = new CSocketInfoDQueue[mstruTcpOpt.mbyDealThreadCount];
	//CSocketInfoDQueue::initCache(mstruTcpOpt.muiMaxConnCount);
	
	return true;
}

struct epoll_thread_param
{
	void*	mpEpollMgr;
	int32	miIndex;
};

INT CEpollMgr::Open()
{
	mbIsRun=TRUE;

    AddWithOutCheckIP();

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

    for(int i=0; i<1; ++i)
    {
        CSystem::BeginThread(AcceptThreadFunction, (void *)this);
    }

	for(int i=0;i<mstruTcpOpt.mbyEpollCount;++i)
	{
		STRU_TRHEAD_CONTEXT *lpThreadContext = new STRU_TRHEAD_CONTEXT;

		lpThreadContext->mpThis = (void *)this;
		lpThreadContext->dwIndex = i;
			//io线程
		CSystem::BeginThread(NetIOProcessThread, (void*)lpThreadContext);
	}

	//原始包处理线程
	CSystem::BeginThread(DealRawPackThread, this);

	//处理线程
	for(int i=0;i<mstruTcpOpt.mbyDealThreadCount;i++)
	{	
		STRU_TRHEAD_CONTEXT *lpThreadContext = new STRU_TRHEAD_CONTEXT;
		lpThreadContext->mpThis = (void *)this;
		lpThreadContext->dwIndex = i;

		mpEventQueue[i].init(miMaxEventCount);
		CSystem::BeginThread(DealThread, (void *)lpThreadContext);
	}

	//超时检测线程
	if(mstruTcpOpt.muiKeepAlive != 0)
		CSystem::BeginThread(CheckTimeoutThread, (void *)this);
	
	////启动发送线程数量
	//for(i=0;i<mstruTcpOpt.mbyDealThreadCount;i++)
	//{	
	//	STRU_TRHEAD_CONTEXT *lpThreadContext = new STRU_TRHEAD_CONTEXT;
	//	lpThreadContext->mpThis = (void *)this;
	//	lpThreadContext->dwIndex = i;
	//	CSystem::BeginThread(SendThreadFunction, (void *)lpThreadContext);
	//}
	//
	
	return RET_SUCCESS;
}

//关闭所有的连接
void CEpollMgr::CloseAllSocket()
{
	CStaticRefObjMgrTemplate1<SOCKET,CSocketInfo>::refobjid_map *lpMap = mpSocketInfoMgr->GetRefObjMap();
	for(CStaticRefObjMgrTemplate1<SOCKET,CSocketInfo>::refobjid_map::iterator ite = lpMap->begin();ite != lpMap->end();ite++)
	{
		CloseSocket(ite->first);
	}
	mpSocketInfoMgr->clear();
}

// 函数说明： 关闭操作
INT CEpollMgr::Close()
{
	CBaseTCPMgr::Close();

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

// 函数说明： 发送数据 是否需要发生线程?
INT CEpollMgr::SendData(STRU_SENDDATA_BASE *apstruSendData)
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

int CEpollMgr::DirectSendData(CSocketInfo *apSocketInfo,STRU_SOCKET_SEND_CONTEXT *apSendSocketContext)
{
	//发送
	int32 liSentLen = SendPacket(apSocketInfo->moKey.mhSocket,
		&apSendSocketContext->mpBuff[apSendSocketContext->miOffset],apSendSocketContext->miBuffLen);
	if(liSentLen == apSendSocketContext->miBuffLen )
	{//发送完毕
		LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, "SendPacket complete "
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
			LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, "SendPacket complete and close"
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
BOOL CEpollMgr::SendWaitQueueData(CSocketInfo *apSocketInfo)
{
	STRU_SOCKET_SEND_CONTEXT *lpSendSocketContext = NULL;
	if(apSocketInfo->moWaitSendDQueue.size() > 100)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "moWaitSendDQueue too large = " << apSocketInfo->moWaitSendDQueue.size()
		<< "socket = " << (void*)apSocketInfo);
	}
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
		//LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, "send complete" 
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

//
//int CEpollMgr::DirectConnect(IPTYPE aiIPAddr, WORD awIPPort)
//{
//	if (aiIPAddr <= 0 || awIPPort <= 0)
//	{
//		return INVALID_SOCKET;
//	}
//	SOCKET lhNewSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (INVALID_SOCKET == lhNewSocket)
//	{
//		return INVALID_SOCKET;
//	}
//	struct sockaddr_in lstruAddr;
//	lstruAddr.sin_addr.s_addr = aiIPAddr;
//	lstruAddr.sin_family = AF_INET;
//	lstruAddr.sin_port = awIPPort;
//
//	int liResult = connect(lhNewSocket, (const sockaddr *)&lstruAddr, 
//		sizeof(struct sockaddr_in));
//	if (0 != liResult)
//	{
//		//if (GETSOCKET_ERRCODE() != WSAEWOULDBLOCK) 
//		{
//			shutdown(lhNewSocket,SD_BOTH);
//			CloseSocket(lhNewSocket);
//			return INVALID_SOCKET;
//		}
//	}
//	CSocketInfo* lpSocketInfo = mpSocketInfoMgr->MallocRefObj();
//	if (NULL == lpSocketInfo)
//	{
//		LOG_TRACE(3, false, __FUNCTION__, "Error mpSocketInfoMgr->MallocRefObj()"
//			<<" Socket="<<lhNewSocket
//			);
//		shutdown(lhNewSocket,SD_BOTH);
//		CloseSocket(lhNewSocket);
//		return INVALID_SOCKET;
//	}
//
//	//加入对象
//	CSocketInfo* lpOldSocketInfo = mpSocketInfoMgr->AddRefObj(lhNewSocket,lpSocketInfo);
//	if(lpOldSocketInfo != lpSocketInfo)
//	{
//		LOG_TRACE(4, false, __FUNCTION__, "Error mpSocketInfoMgr->AddRefObj exist obj" 
//			<<" Socket="<<lhNewSocket
//			<<" SocketInfoPtr="<<(void*)lpSocketInfo
//			<<" RefCount="<<lpSocketInfo->GetRefCount()
//			<<" Old SocketInfoPtr="<<(void*)lpOldSocketInfo
//			<<" Old RefCount="<<lpOldSocketInfo->GetRefCount()
//			);
//		U9_ASSERT(FALSE);
//		shutdown(lhNewSocket,SD_BOTH);
//		CloseSocket(lhNewSocket);
//		mpSocketInfoMgr->FreeRefObj(lpSocketInfo);
//		return INVALID_SOCKET;
//	}
//	STRU_SOCKET_KEY &loKey = lpSocketInfo->moKey;
//	//设置socket Infomation key
//	lpSocketInfo->moCS.Enter();	
//	loKey.mhSocket=lhNewSocket;
//	loKey.mPeerIP = lstruAddr.sin_addr.s_addr;
//	loKey.mwPeerPort = lstruAddr.sin_port;
//	lpSocketInfo->mpRecvContext->miBuffLen=0;
//	lpSocketInfo->moCS.Leave();	
//	
//	//将socket句柄同epoll关联起来
//	if(FALSE ==AssociateSocketwithEpoll(lpSocketInfo))
//	{
//		LOG_TRACE(5, false, __FUNCTION__, "Error AssociateSocketwithEpoll fail "
//			<<" errcode="<<GETSOCKET_ERRCODE()
//			<<" Socket="<<lhNewSocket
//			<<" SocketInfoPtr="<<(void*)lpSocketInfo
//			<<" RefCount="<<lpSocketInfo->GetRefCount()
//			);
//		shutdown(lhNewSocket,SD_BOTH);
//		mpSocketInfoMgr->DeleteRefObj(lhNewSocket);
//		return INVALID_SOCKET;
//	}
//	
//	if (mpMsgSink)
//	{
//		DWORD ldwIndex =GetDealThreadIndex(aiIPAddr, awIPPort);
//		STRU_EVENT_DATA *lpEvent=moEventCacheQueue.malloc();
//
//		lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CONNECT_SUCCESS;
//		lpEvent->mhSocket = lhNewSocket;
//		lpEvent->miIP = aiIPAddr;
//		lpEvent->mwPort = awIPPort;
//		lpEvent->mpParam1 = NULL;
//		lpEvent->mpRelationPtr = lpSocketInfo;
//		lpSocketInfo->IncreaseRef();//增加一次引用
//
//		LOG_TRACE(6, true, __FUNCTION__, "notify new socket connect"
//			<<" Socket="<<lhNewSocket
//			<<" ip="<<GetIPString(lpEvent->miIP)
//			<<":"<<ntohs(lpEvent->mwPort)
//			);
//
//		mpEventQueue[ldwIndex].push_back(lpEvent);
//	}
//
//	return lhNewSocket;
//}

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

int CEpollMgr::DirectConnect(IPTYPE aiIPAddr, WORD awIPPort)
{
	if (aiIPAddr <= 0 || awIPPort <= 0)
	{
		return INVALID_SOCKET;
	}
	SOCKET lhNewSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == lhNewSocket)
	{
		return INVALID_SOCKET;
	}
	struct sockaddr_in lstruAddr;
	lstruAddr.sin_addr.s_addr = aiIPAddr;
	lstruAddr.sin_family = AF_INET;
	lstruAddr.sin_port = awIPPort;

	//// 强制设置为非阻塞的
	set_fd_nonblock(lhNewSocket);


	CSocketInfo* lpSocketInfo = mpSocketInfoMgr->MallocRefObj();
	if (NULL == lpSocketInfo)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error mpSocketInfoMgr->MallocRefObj()"
			<<" Socket="<<lhNewSocket
			);
		shutdown(lhNewSocket,SD_BOTH);
		CloseSocket(lhNewSocket);
		return INVALID_SOCKET;
	}

	//加入对象
	CSocketInfo* lpOldSocketInfo = mpSocketInfoMgr->AddRefObj(lhNewSocket,lpSocketInfo);
	if(lpOldSocketInfo != lpSocketInfo)
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error mpSocketInfoMgr->AddRefObj exist obj"
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
		return INVALID_SOCKET;
	}

	STRU_SOCKET_KEY &loKey = lpSocketInfo->moKey;
	//设置socket Infomation key
	lpSocketInfo->moCS.Enter();	
	loKey.mhSocket=lhNewSocket;
	loKey.mPeerIP = lstruAddr.sin_addr.s_addr;
	loKey.mwPeerPort = lstruAddr.sin_port;
	lpSocketInfo->mpRecvContext->miBuffLen=0;
	lpSocketInfo->miNetState = ENUM_SOCKET_STATE_CONNECTING;
	lpSocketInfo->moCS.Leave();	
	lpSocketInfo->IncreaseRef();//增加一次引用

	//先将socket句柄同epoll关联起来
	if(FALSE ==AssociateSocketwithEpoll(lpSocketInfo))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error AssociateSocketwithEpoll fail "
			<<" errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<lhNewSocket
			<<" SocketInfoPtr="<<(void*)lpSocketInfo
			<<" RefCount="<<lpSocketInfo->GetRefCount()
			);
		shutdown(lhNewSocket,SD_BOTH);
		mpSocketInfoMgr->DeleteRefObj(lhNewSocket);
		return INVALID_SOCKET;
	}

	//然后在connect
	int liResult = connect(lhNewSocket, (const sockaddr *)&lstruAddr, 
		sizeof(struct sockaddr_in));

	return lhNewSocket;

//	if (mpMsgSink)
//	{
//		DWORD ldwIndex =GetDealThreadIndex(aiIPAddr, awIPPort);
//		STRU_EVENT_DATA *lpEvent=moEventCacheQueue.malloc();
//
//		lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CONNECT_SUCCESS;
//		lpEvent->mhSocket = lhNewSocket;
//		lpEvent->miIP = aiIPAddr;
//		lpEvent->mwPort = awIPPort;
//		lpEvent->mpParam1 = NULL;
//		lpEvent->mpRelationPtr = lpSocketInfo;
//		lpSocketInfo->IncreaseRef();//增加一次引用
//
//		LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "notify new socket connect"
//			<<" Socket="<<lhNewSocket
//			<<" ip="<<GetIPString(lpEvent->miIP)
//			<<":"<<ntohs(lpEvent->mwPort)
//			);
//
//		mpEventQueue[ldwIndex].push_back(lpEvent);
//	}

}

// 函数说明： 连接一个指定的地址
INT CEpollMgr::ConnectTo(const SESSIONID& aiPeerID, IPTYPE aiIPAddr, WORD awPort)
{
	if(aiPeerID == 0)
	{//不用线程连接
		return DirectConnect(aiIPAddr,awPort);
	}

	STRU_CONNECT_EVENT *lpEvent = moConnectEventCacheQueue.malloc();
	if(NULL == lpEvent)
		return RET_ERROR;
	lpEvent->miID = aiPeerID;
	lpEvent->miDestIP = aiIPAddr;
	lpEvent->mwDestPort = htons(awPort);
	lpEvent->mhSocket = INVALID_SOCKET;
	lpEvent->miCreateTime = 0;

	DWORD ldwIndex = GetConnnectThreadIndex(aiIPAddr,awPort);
	if(INVALID_U9_POSITION ==  mpConnectEventQueue[ldwIndex].push_back(lpEvent))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back failed."
			<<" PeerID="<<aiPeerID
			<<" IP="<<GetIPString(aiIPAddr)
			<<":"<<ntohs(awPort)
			<<" Index="<<ldwIndex
			);
		moConnectEventCacheQueue.free(lpEvent);
		return RET_ERROR;
	}

	return RET_SUCCESS;
}
// 函数说明： 关闭一个连接
INT CEpollMgr::DisConnectTo(SOCKET ahSocket)
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

// 开始在指定的端口监听
bool CEpollMgr::StartListen(DWORD adwIPAddr, UINT aiPort)
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

	// 绑定指定的地址和端口
	liError = bind(mhServerSocket, (const struct sockaddr *)&lstrServerAddr, sizeof(struct sockaddr_in));
	if (SOCKET_ERROR == liError)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "bind() fault, error:" << GETSOCKET_ERRCODE());
		return false;
	}

	//SOMAXCONN
	liError = listen(mhServerSocket, 8192);
	if (SOCKET_ERROR == liError)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "listen() fault, error:" << GETSOCKET_ERRCODE());
		return false;
	}

	LOG_TRACE(LOG_ALERT, true, __FUNCTION__, "listen sucess port:" << aiPort);
	return true;
	//return  AssociateSockwithEpoll(mhServerSocket, lstrServerAddr,EPOLLIN | EPOLLET);
}

void CEpollMgr::NetIOProcessThread(VOID *apParam)
{
	STRU_TRHEAD_CONTEXT * lpThreadContext = (STRU_TRHEAD_CONTEXT *)apParam;
	CEpollMgr* lpThis =(CEpollMgr*) lpThreadContext->mpThis;
	if(NULL ==lpThis )
		return ;

	lpThis->IncreaseRef();
	lpThis->NetIOProcessThread(lpThreadContext->dwIndex);
	lpThis->DecreaseRef();
}

// 网络通讯主线程函数
#define DEF_MAX_EPOLL_EVENT       (512)
void CEpollMgr::NetIOProcessThread(int32 aiIndex)
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
				CheckConnectResult( lpSocketInfo, loEpollEvent.events );
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
void CEpollMgr::AcceptThreadFunction(VOID *apParam)
{
	CEpollMgr* lpThis =(CEpollMgr*) apParam;
	if(NULL ==lpThis )
		return ;
	lpThis->IncreaseRef();
	lpThis->AcceptTimeoutThread();
	lpThis->DecreaseRef();
} 
void CEpollMgr::AcceptTimeoutThread()
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
		//检查白名单
		if(without_check_map.find(loKey.mPeerIP) != without_check_map.end())
		{
			LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "need not check out ip"
					<<" ip="<<loKey.mPeerIP
			);
			lpSocketInfo->mbNeedCheck = FALSE;
		}

		//先通知上层然后再把套接字放到epoll队列中
		if (mpMsgSink)
		{
			ldwIndex =GetDealThreadIndex(lstruAddr.sin_addr.s_addr,lstruAddr.sin_port);

			//提交上层处理
			lpEvent = moEventCacheQueue.malloc();

			lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CONNECT_SUCCESS;
			lpEvent->mhSocket = lhNewSocket;
			lpEvent->miIP = lstruAddr.sin_addr.s_addr;
			lpEvent->mwPort =lstruAddr.sin_port;
			lpEvent->mpRelationPtr = (void*)lpSocketInfo;
			lpSocketInfo->IncreaseRef();//增加一次引用

			LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "notify new socket connect"
				<<" Socket="<<lhNewSocket
				<<" ip="<<GetIPString(lpEvent->miIP)
				<<":"<<ntohs(lpEvent->mwPort)
				);

			mpEventQueue[ldwIndex].push_back(lpEvent);
		}

		if(FALSE ==AssociateSocketwithEpoll(lpSocketInfo))
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error AssociateSocketwithEpoll fail "
				<<" errcode="<<GETSOCKET_ERRCODE()
				<<" Socket="<<lhNewSocket
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" RefCount="<<lpSocketInfo->GetRefCount()
				);

			if (mpMsgSink)
			{
				ldwIndex =GetDealThreadIndex(lstruAddr.sin_addr.s_addr,lstruAddr.sin_port);

				//提交上层处理
				lpEvent = moEventCacheQueue.malloc();

				lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CLOSE;
				lpEvent->mhSocket = lhNewSocket;
				lpEvent->miIP = lstruAddr.sin_addr.s_addr;
				lpEvent->mwPort =lstruAddr.sin_port;
				lpEvent->mpRelationPtr = (void*)lpSocketInfo;;
			
				LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, ""
					<<" Socket="<<lhNewSocket
					<<" ip="<<GetIPString(lpEvent->miIP)
					<<":"<<ntohs(lpEvent->mwPort)
					);

				mpEventQueue[ldwIndex].push_back(lpEvent);
			}

			lpSocketInfo->DecreaseRef();
			shutdown(lhNewSocket,SD_BOTH);//lpSocketInfo的clear函数会close socket的 
			mpSocketInfoMgr->DeleteRefObj(lhNewSocket);
			continue;
		}

		InsCurCon(0);
		continue;
	}
}

void CEpollMgr::CheckTimeoutThread(VOID *apParam)
{
	CEpollMgr* lpThis =(CEpollMgr*) apParam;
	if(NULL ==lpThis )
		return ;
	lpThis->IncreaseRef();
	lpThis->CheckTimeoutThread();
	lpThis->DecreaseRef();
} 
#define DEF_SOCKET_TIMEOUT_TIMEOUT (60) //接收超时
void CEpollMgr::CheckTimeoutThread()
{
	STRU_TIMEOUT_ITEM *lpTimeoutItem=NULL;
	U9_POSITION lpPos = INVALID_U9_POSITION;
	U9_POSITION lpTempPos = INVALID_U9_POSITION;
	CSocketInfo *lpSocketInfo = NULL;
	mlLocalTime = CSystem::GetTime();
	int i = 0;
	while (mbIsRun)
	{
		CSystem::Sleep(10);
		i++;
		if((i%100) == 0)//间隔1s取一次时间
			mlLocalTime = CSystem::GetTime();
		if(i < 3000) //30秒检测一次
			continue;
		i=0;
		LOG_TRACE(LOG_INFO, true, __FUNCTION__, "check timeout start");
		for(int j = 0;j<mstruTcpOpt.mbyDealThreadCount;j++)
		{
			if(!mbIsRun)
				break;
			CSocketTimeoutDqueue &loTimeoutQueue = mpSocketTimeoutDqueue[j];
		
			if(loTimeoutQueue.size() == 0)
				continue;

			LOG_TRACE(LOG_INFO, true, __FUNCTION__, "check timeout "
				<<" size="<<loTimeoutQueue.size());

			LONG llNow= mlLocalTime - mstruTcpOpt.muiKeepAlive;

			loTimeoutQueue.GetCriticalSection().Enter();
			lpPos = loTimeoutQueue.begin();
			while(NULL != lpPos)
			{
				if(!mbIsRun)
					break;
				lpTimeoutItem =loTimeoutQueue.getdata(lpPos);
				U9_ASSERT (NULL != lpTimeoutItem);
                lpSocketInfo = lpTimeoutItem->mpSocketInfo;
                //第一次连接后的发包时间必须是5秒呢
                if(TRUE == lpSocketInfo->mbFirstRecv)
                {
                    llNow = mlLocalTime - FIRST_MAX_TIME_OUT;
                }

				if(llNow <= lpTimeoutItem->mlLastTime)
					break;
				lpTempPos = lpPos;
				lpPos=loTimeoutQueue.next(lpPos);


				LOG_TRACE(LOG_NOTICE,true, __FUNCTION__, "Socket Timeout "
					<<" Socket="<<lpSocketInfo->moKey.mhSocket
					<<" SocketInfoPtr="<<(void*)lpSocketInfo
					<<" RefCount = "<< lpSocketInfo->GetRefCount()
					<<" TimeoutPos="<<(void*)lpSocketInfo->mpTimeoutPos
					<<" size="<<loTimeoutQueue.size());
				moTimeoutCacheQueue.free(lpTimeoutItem);

				lpSocketInfo->mpTimeoutPos = INVALID_U9_POSITION;
				loTimeoutQueue.erase(lpTempPos);
				//关闭socket
				CloseTCPSocket(lpSocketInfo);
			}
			loTimeoutQueue.GetCriticalSection().Leave();
		}
		LOG_TRACE(LOG_INFO, true, __FUNCTION__, "check timeout end");
	}
}

//设置SOCKET为非阻塞状态
bool CEpollMgr::SetSocektNoBlock(SOCKET ahSocket)
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
	int liRecvBuf=1024*1024;
	if(0>setsockopt(ahSocket, SOL_SOCKET, SO_RCVBUF, 
		(const char*)&liRecvBuf,sizeof(int)))
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Set recv buffer error. "
			<<" Errcode=" << GETSOCKET_ERRCODE()
			<<" Socket="<<ahSocket);
	}

	// 设置发送缓冲区
	socklen_t liSendBuf=1024*1024;
	if(0>setsockopt(ahSocket, SOL_SOCKET, SO_SNDBUF,
		(const char*)&liSendBuf,sizeof(int)))
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Set send buffer error.  "
			<<" Errcode=" << GETSOCKET_ERRCODE()
			<<" Socket="<<ahSocket);
	}

	


	//int keepAlive = 1;//设定KeepAlive
	//int keepIdle = 5;//开始首次KeepAlive探测前的TCP空闭时间
	//int keepInterval = 5;//两次KeepAlive探测间的时间间隔
	//int keepCount = 3;//判定断开前的KeepAlive探测次数

	//if(setsockopt(ahSocket,SOL_SOCKET,SO_KEEPALIVE,(void*)&keepAlive,sizeof(keepAlive)) == -1)
	//{
	//	LOG_TRACE(5, false, __FUNCTION__, "Error Create socket SO_KEEPALIVE errcode=" << GETSOCKET_ERRCODE()
	//		<<" Socket="<<ahSocket
	//		);

	//}

	//if(setsockopt(ahSocket,SOL_TCP,TCP_KEEPIDLE,(void *)&keepIdle,sizeof(keepIdle)) == -1)
	//{
	//	LOG_TRACE(5, false, __FUNCTION__, "Error Create socket TCP_KEEPIDLE errcode=" << GETSOCKET_ERRCODE()
	//		<<" Socket="<<ahSocket
	//		);
	//}

	//if(setsockopt(ahSocket,SOL_TCP,TCP_KEEPINTVL,(void *)&keepInterval,sizeof(keepInterval)) == -1)
	//{
	//	LOG_TRACE(5, false, __FUNCTION__, "Error Create socket TCP_KEEPINTVL errcode=" << GETSOCKET_ERRCODE()
	//		<<" Socket="<<ahSocket
	//		);
	//}

	//if(setsockopt(ahSocket,SOL_TCP,TCP_KEEPCNT,(void *)&keepCount,sizeof(keepCount)) == -1)
	//{
	//	LOG_TRACE(5, false, __FUNCTION__, "Error Create socket TCP_KEEPCNT errcode=" << GETSOCKET_ERRCODE()
	//		<<" Socket="<<ahSocket
	//		);
	//}
	return true;    
}

int32 CEpollMgr::SendPacket(SOCKET ahScoket,BYTE* apBuffer,int32 aiLen)
{
	int liSendLen = send(ahScoket,apBuffer,aiLen,0);

	//LOG_TRACE(7, true, __FUNCTION__, "Send Data "
	//	<< "SendDataShow"
	//	<<" Scoket=" << ahScoket
	//	<<" aiLen="<<aiLen
	//	<<" SendLen="<<liSendLen
	//	);

	int liError = errno;
	if(liSendLen == aiLen)
	{//发送完成
		return aiLen;
	}
	if(liSendLen>=0)
	{//没有完全发送完一个数据包
		return liSendLen;
	}

	if(EAGAIN == liError)
	{// socket 系统缓存区满。下次发送
		return 0;
	}	

	return -1;
}


BOOL CEpollMgr::AssociateSocketwithEpoll(CSocketInfo *apSocketInfo)
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

	if( mstruTcpOpt.muiKeepAlive != 0 && mhServerSocket != loKey.mhSocket && apSocketInfo->mbNeedCheck)
		AddTimeoutQueue(apSocketInfo);

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "epoll_ctl add success"
		<<" Socket="<<loKey.mhSocket
		<<" EpollCount="<< mstruTcpOpt.mbyEpollCount
		<<" liIndex="<<liIndex
		<<" SocketInfoPtr="<<(void*)apSocketInfo
		<<" RefCount="<<apSocketInfo->GetRefCount()
		);
	return TRUE;
}

BOOL CEpollMgr::RecvTCPData(CSocketInfo *apSocketInfo,BOOL & abNeedClose, const int32 &aiThreadID)
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
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "socket closed"
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

		InsLostRecv(aiThreadID);
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
	CRawTCPRecvPack *lpRawTCPPack = NULL;
	
	BOOL abIsSuccess = FALSE;
	while((liPackLen = CTCPPacket::GetPack(lpBegin, liRecvLen))>0)
	{
		LOG_TRACE(LOG_DEBUG_1,TRUE,__FUNCTION__," CEpollMgr::RecvTCPData:" << liPackLen);
		lpRawTCPPack = mRawRecvPackCacheList.malloc();
		if(NULL == lpRawTCPPack)
		{
			InsLostRecv(aiThreadID);
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error new RawTCPPack fail"
				<<" Socket="<<loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				<<" DataLen="<<loRecvContext.miBuffLen
				<<" RecvLen="<<liRecvLen
				);
			continue;
		}
		abIsSuccess = TRUE;
		lpRawTCPPack->mhSocket = loKey.mhSocket;
		lpRawTCPPack->mulIPAddr = loKey.mPeerIP;
		lpRawTCPPack->mwIPPort = loKey.mwPeerPort;
		lpRawTCPPack->mpSocketInfo = (void*)apSocketInfo;

		//LOG_TRACE(7, false, __FUNCTION__, "recv data"
		//	<<" Socket="<<loKey.mhSocket
		//	<<" IP="<<GetIPString(loKey.mPeerIP)
		//	<<":"<<ntohs(loKey.mwPeerPort)
		//	<<" SocketInfoPtr="<<(void*)apSocketInfo
		//	<<" RelationPtrPtr="<<apSocketInfo->mpRelationPtr
		//	<<" RefCount="<<apSocketInfo->GetRefCount()
		//	<<" DataLen="<<loRecvContext.miBuffLen
		//	<<" RecvLen="<<liRecvLen
		//	);

		memcpy(lpRawTCPPack->mpData,lpBegin,liPackLen);
		lpRawTCPPack->miDataLen = liPackLen;

		if( INVALID_U9_POSITION == moRawRecvPackList.push_back(lpRawTCPPack))
		{
			InsLostRecv(aiThreadID);
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back fail"
				<<" Socket="<<loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				<<" DataLen="<<loRecvContext.miBuffLen
				<<" RecvLen="<<liRecvLen
				);
			mRawRecvPackCacheList.free(lpRawTCPPack);
			continue;
		}

		InsRecv(aiThreadID);
		lpBegin += liPackLen;
		liRecvLen -= liPackLen;
	}
	
	if (abIsSuccess && apSocketInfo->mbNeedCheck)
	{//为了分析最后收包时间分析
        apSocketInfo->mbFirstRecv = FALSE;
		AddTimeoutQueue(apSocketInfo);
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
//
void CEpollMgr::clearWaitQueue(CSocketInfo *apSocketInfo)
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

void CEpollMgr::CloseTCPSocketInNetIOProcessThread(CSocketInfo *apSocketInfo, const int32 &aiThreadID)
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

	DelTimeoutQueue(apSocketInfo);
	if(lbNeedClose)
	{//关闭socket
		LOG_TRACE(LOG_DEBUG, true, __FUNCTION__,  "need shutdown socket ,success"
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

	DWORD ldwIndex = GetDealThreadIndex(loKey.mPeerIP,loKey.mwPeerPort);
	STRU_EVENT_DATA *lpEvent = moEventCacheQueue.malloc();

	ZeroMemory(lpEvent,sizeof(lpEvent));
	lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CLOSE;
	lpEvent->mhSocket = lhSocket;
	lpEvent->miIP = loKey.mPeerIP;
	lpEvent->mwPort = loKey.mwPeerPort;
	lpEvent->miParam = GETSOCKET_ERRCODE();
	lpEvent->mpRelationPtr = (void*)apSocketInfo;

	mpEventQueue[ldwIndex].push_back(lpEvent);
}
void CEpollMgr::CloseTCPSocket(CSocketInfo *apSocketInfo)
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
		LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "need shutdown socket"
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
		LOG_TRACE(LOG_INFO, true, __FUNCTION__, "socket has shutdown"
			<<" Errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<lhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" SocketInfo="<<(void*)apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
			);
	}
}

CSocketInfo* CEpollMgr::CreateSocketSocket(SOCKET ahSocket,IPTYPE aiIPAddr,WORD awPort)
{
	CSocketInfo *lpSocketInfo = mpSocketInfoMgr->MallocRefObj();
	if (NULL == lpSocketInfo)
	{
		LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error mpSocketInfoMgr->MallocRefObj()"
			<<" IP="<<GetIPString(aiIPAddr)
			<<":"<<ntohs(awPort)
			<<" Socket="<<ahSocket
			);
		return NULL;
	}

	lpSocketInfo->init();

	// 将Socket句柄同完成端口关联起来
	CSocketInfo *lpOldSocketInfo = mpSocketInfoMgr->AddRefObj(ahSocket,lpSocketInfo);
	if(lpOldSocketInfo != lpSocketInfo)
	{//error ,
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "Error AddRefObj fault"
			<<" Socket="<<ahSocket
			<<" ProxySocketInfo="<<(void*)lpSocketInfo
			<<" Old SocketInfo="<<(void*)lpOldSocketInfo
			<<" IP="<<GetIPString(aiIPAddr)
			<<":"<<ntohs(awPort)
			);
		mpSocketInfoMgr->FreeRefObj(lpSocketInfo);
		U9_ASSERT(FALSE);
		return NULL;
	}
	lpSocketInfo->IncreaseRef();
	if(FALSE ==AssociateSocketwithEpoll(lpSocketInfo))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error AssociateSockWithIoComp() failed in connect()"
			<<" errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<ahSocket
			<<" IP="<<GetIPString(aiIPAddr)
			<<":"<<ntohs(awPort));
		mpSocketInfoMgr->DeleteRefObj(ahSocket);
		return NULL;
	}
	
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, " Connected  "
		<<" Socket="<<lpSocketInfo->moKey.mhSocket
		<<" IP="<<GetIPString(aiIPAddr)
		<<":"<<ntohs(awPort)
		<<" SocketInfo="<< (void*)lpSocketInfo
		<<" RefCount="<<lpSocketInfo->GetRefCount()
		);
	return lpSocketInfo;
}
U9_POSITION CEpollMgr::AddTimeoutQueue(CSocketInfo *apSocketInfo)
{
	if(0 == mstruTcpOpt.muiKeepAlive)
		return INVALID_U9_POSITION;
	STRU_SOCKET_KEY& loKey = apSocketInfo->moKey;
	DWORD ldwIndex = GetDealThreadIndex(loKey.mPeerIP,loKey.mwPeerPort);
	CSocketTimeoutDqueue & loTimeoutQueue = mpSocketTimeoutDqueue[ldwIndex];
	STRU_TIMEOUT_ITEM *lpTimeoutItem = NULL;
	CCriticalSection &loCS = loTimeoutQueue.GetCriticalSection();
	loCS.Enter();
	U9_POSITION lpTimeoutPos = apSocketInfo->mpTimeoutPos;

	if(INVALID_U9_POSITION != lpTimeoutPos)
	{
		lpTimeoutItem =loTimeoutQueue.getdata(lpTimeoutPos);
		if(lpTimeoutItem->mlLastTime == mlLocalTime)
		{//时间没有改变
			loCS.Leave();
			return lpTimeoutPos;
		}
		moTimeoutCacheQueue.free(lpTimeoutItem);

		loTimeoutQueue.erase(lpTimeoutPos);
		
	}
	lpTimeoutItem= moTimeoutCacheQueue.malloc();
	lpTimeoutItem->mlLastTime = mlLocalTime;
	lpTimeoutItem->mpSocketInfo = apSocketInfo;
	lpTimeoutPos = loTimeoutQueue.push_back(lpTimeoutItem);
	
	//注意这个SocketInfo的变量不能上它自己的临界区，否则要出死锁的，
	//因为loTimeoutQueue.GetCriticalSection()和SocketInfo.moCS的调用顺序不同
	apSocketInfo->mpTimeoutPos = lpTimeoutPos;
	loCS.Leave();

	return lpTimeoutPos;
}

U9_POSITION CEpollMgr::DelTimeoutQueue(CSocketInfo *apSocketInfo)
{
	if(0 == mstruTcpOpt.muiKeepAlive)
		return INVALID_U9_POSITION;
	STRU_TIMEOUT_ITEM *lpTimeoutItem = NULL;
	STRU_SOCKET_KEY& loKey = apSocketInfo->moKey;
	DWORD ldwIndex = GetDealThreadIndex(loKey.mPeerIP,loKey.mwPeerPort);
	CSocketTimeoutDqueue & loTimeoutQueue = mpSocketTimeoutDqueue[ldwIndex];
	U9_POSITION lpTimeoutPos = INVALID_U9_POSITION;
	CCriticalSection &loCS = loTimeoutQueue.GetCriticalSection();
	loCS.Enter();
	lpTimeoutPos = apSocketInfo->mpTimeoutPos;
	if(INVALID_U9_POSITION != lpTimeoutPos)
	{
		lpTimeoutItem =loTimeoutQueue.getdata(lpTimeoutPos);
		moTimeoutCacheQueue.free(lpTimeoutItem);

		loTimeoutQueue.erase(lpTimeoutPos);
		apSocketInfo->mpTimeoutPos = INVALID_U9_POSITION;

	}
	loCS.Leave();

	//LOG_TRACE(7, true, __FUNCTION__, " "
	//	<<" Socket="<<loKey.mhSocket
	//	<<" IP="<<GetIPString(loKey.mPeerIP)
	//	<<":"<<ntohs(loKey.mwPeerPort)
	//	<<" SocketInfo="<<(void*)apSocketInfo
	//	<<" RefCount="<<apSocketInfo->GetRefCount()
	//	<<" TimeoutPos="<<(void*)lpTimeoutPos
	//	);
	return lpTimeoutPos;
}

void CEpollMgr::AddWithOutCheckIP()
{
    //加载超时检测白名单
    std::string wo_check_ip = mstruTcpOpt.mszWithOutCheckIP;
    LOG_TRACE(LOG_ALERT, true, __FUNCTION__, "with out check ip start"
            <<" ip_str_list=" << wo_check_ip.c_str());
#if __cplusplus > 199711L
    std::string pattern("((?:(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d)))\\.){3}(?:25[0-5]|2[0-4]\\d|((1\\d{2})|([1-9]?\\d))))");
    std::regex r_ip(pattern);
    for(std::sregex_iterator it(wo_check_ip.begin(), wo_check_ip.end(), r_ip), end; it != end; ++it)
    {
        LOG_TRACE(LOG_ALERT, true, __FUNCTION__, "add with out check ip "
                <<" ip_str=" << it->str().c_str()
                <<" ip_int="<< inet_addr(it->str().c_str()));
        without_check_map.insert(make_pair(inet_addr(it->str().c_str()), TRUE));
    }
#endif
    LOG_TRACE(LOG_ALERT, true, __FUNCTION__, "with out check ip end"
            <<" ip map size=" << without_check_map.size());
}


bool CEpollMgr::CheckConnectResult( CSocketInfo *apSocketInfo, int32 events )
{
	bool result = true;

	if ( (events & EPOLLOUT) && (events &EPOLLHUP))
	{//本地没进行connect调用，将套接字送交epoll的结果 events为0x14 
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error not call connect"
			<<" Socket="<<apSocketInfo->moKey.mhSocket
			<<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
			<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
			<<" events=:"<<(void*)events
			);
		result =  true;
	}
	else if ( (events & EPOLLIN) && (events & EPOLLHUP) && (events & EPOLLERR))
	{//本地调用了connect，建立连接失败 events为0x19
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error connect failed!"
			<<" Socket="<<apSocketInfo->moKey.mhSocket
			<<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
			<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
			<<" events=:"<<(void*)events
			);
		result =  false;
		CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,apSocketInfo);
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

		DelTimeoutQueue(apSocketInfo);
		if(lbNeedClose)
		{//关闭socket
			LOG_TRACE(LOG_DEBUG, true, __FUNCTION__,  "need shutdown socket ,success"
				<<" Socket="<<lhSocket
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				);
			shutdown(lhSocket,SD_BOTH);//lpSocketInfo的clear函数会close socket的 
			mpSocketInfoMgr->DeleteRefObj(lhSocket);
		}
		else
		{
			LOG_TRACE(LOG_DEBUG, true, __FUNCTION__,  "socket has shutdown"
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
		
	}
	else if ( events & EPOLLOUT )
	{//连接成功 events为0x4
		CCriticalSection &	loCS = apSocketInfo->moCS;
		loCS.Enter();
		apSocketInfo->miNetState == ENUM_SOCKET_STATE_CONNECTED;
		
		if (mpMsgSink)
		{
			DWORD ldwIndex =GetDealThreadIndex(apSocketInfo->moKey.mPeerIP, apSocketInfo->moKey.mwPeerPort);
			STRU_EVENT_DATA *lpEvent=moEventCacheQueue.malloc();
			STRU_SOCKET_KEY &loKey = apSocketInfo->moKey;

			lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CONNECT_SUCCESS;
			lpEvent->mhSocket = loKey.mhSocket;
			lpEvent->miIP = loKey.mPeerIP;
			lpEvent->mwPort = loKey.mwPeerPort;
			lpEvent->mpParam1 = NULL;
			lpEvent->mpRelationPtr = apSocketInfo;
			apSocketInfo->IncreaseRef();//增加一次引用

			LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "notify new socket connect"
				<<" Socket="<<lpEvent->mhSocket
				<<" ip="<<GetIPString(lpEvent->miIP)
				<<":"<<ntohs(lpEvent->mwPort)
				);

			mpEventQueue[ldwIndex].push_back(lpEvent);
		}
		loCS.Leave();
		result =  true;
	}
	else 
	{//other
		// 出错
		result = false;

		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "notify new socket connect"
			<<" Socket="<<apSocketInfo->moKey.mhSocket
			<<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
			<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
			<<" events=:"<<(void*)events
			);

		CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,apSocketInfo);
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

		DelTimeoutQueue(apSocketInfo);
		if(lbNeedClose)
		{//关闭socket
			LOG_TRACE(LOG_DEBUG, true, __FUNCTION__,  "need shutdown socket ,success"
				<<" Socket="<<lhSocket
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				);
			shutdown(lhSocket,SD_BOTH);//lpSocketInfo的clear函数会close socket的 
			mpSocketInfoMgr->DeleteRefObj(lhSocket);
		}
		else
		{
			LOG_TRACE(LOG_DEBUG, true, __FUNCTION__,  "socket has shutdown"
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
	
	}

	return result;
}



// 函数说明： 发送数据 是否需要发生线程?
INT CEpollMgr::SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData)
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

	//LOG_TRACE(7,true, __FUNCTION__, "show send data"
	//	<< "SendDataShow"
	//	<<" Socket="<<loKey.mhSocket
	//	<<" PeerIP="<<GetIPString(loKey.mPeerIP)
	//	<<":"<<ntohs(loKey.mwPeerPort)
	//	<<" EncryptType="<<miEncryptType
	//	<<" NeedClose="<<lpTcpSndOpt->mbNeedClose
	//	<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
	//	<<" PackLen ="<<lpTcpSndOpt->mwLen
	//	<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
	//	<<" LoadLen="<<lpTcpSndOpt->mwLen
	//	);

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

		LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, "Wait when Send Packet"
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
	LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, "wish direct send but some deta not send"
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



//设置SOCKET为非阻塞状态
bool CEpollMgr::SetSocektNODELAY(SOCKET ahSocket)
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