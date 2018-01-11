// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//			 
// 文件名称: IOCPMgr
// 创 建 人: 史云杰
// 文件说明: tcp windows完成端口 模型 修改事件处理的按事件处理
// $_FILEHEADER_END *****************************

#include <winsock2.h>
#include "stdafx.h"
#include "IOCPMgr.h"



//////////////////////////////////////////////////////////////////////////////
// CIOCPMgr 标准构造
CIOCPMgr::CIOCPMgr() 
{
	mhCompletionPort = NULL;
	mhNoAcceptEvent = NULL;
	miInitialized = NULL;
	mpfnAcceptExSockAddrs = NULL;
	mpfnTransmitFile = NULL;
	mpfnAcceptEx = NULL;
}

// CIOCPMgr 标准析构
CIOCPMgr::~CIOCPMgr()
{

}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： InitWinSock
// 函数参数： 无.
// 返 回 值： bool, 是否成功
// 函数说明： 初始化Winsock 库, 并获取相关函数的地址
//
// $_FUNCTION_END ********************************
bool CIOCPMgr::InitWinSock()
{
	int     lErr;
	DWORD   ldwResult;

	const GUID GUID_ACCEPTEX     = WSAID_ACCEPTEX;
	const GUID GUID_ACCEPTEXADDR = WSAID_GETACCEPTEXSOCKADDRS;
	const GUID GUID_TRANSMITFILE = WSAID_TRANSMITFILE;

	// 创建一个Socket(Server端监听使用)
	mhServerSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 
		NULL, 0, WSA_FLAG_OVERLAPPED); 

	if (INVALID_SOCKET == mhServerSocket)
	{
		mhServerSocket = NULL;

		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "WSASocket() failed, error code " << GETSOCKET_ERRCODE());

		return false;
	}

	// 获取AcceptEx()函数的地址
	lErr = WSAIoctl(mhServerSocket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		(LPVOID)&GUID_ACCEPTEX, 
		sizeof(GUID_ACCEPTEX),
		&mpfnAcceptEx, 
		sizeof(mpfnAcceptEx),
		&ldwResult, 
		NULL, 
		NULL
		);
	if (SOCKET_ERROR == lErr)
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "Get AcceptEx() pointer failed...");
		return false;
	}

	// 获取TransmitFile()函数的地址
	lErr = WSAIoctl(mhServerSocket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		(LPVOID)&GUID_TRANSMITFILE, 
		sizeof(GUID_TRANSMITFILE),
		&mpfnTransmitFile, 
		sizeof(mpfnTransmitFile),
		&ldwResult, 
		NULL, 
		NULL
		);
	if (SOCKET_ERROR == lErr )
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "Get TransmitFile() pointer failed...");
		return false;
	}

	// 获取GetAcceptExSOckAddrs() 函数的地址
	lErr = WSAIoctl(mhServerSocket, 
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		(LPVOID)&GUID_ACCEPTEXADDR, 
		sizeof(GUID_ACCEPTEXADDR),
		&mpfnAcceptExSockAddrs, 
		sizeof(mpfnAcceptExSockAddrs),
		&ldwResult, 
		NULL, 
		NULL
		);

	if (SOCKET_ERROR == lErr)
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "Get GetAcceptExSockAddrs() pointer failed...");
		return false;
	}

	return true;
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称:  InitCompltionPort
// 函数参数:  aiAcceptExNum - 每次投递的AcceptEx数目
//           aiMempoolNum  - 内存池链表节点数目
//           aiKeepalive   - 连接存活的时间(单位: 秒)
// 返 回 值:  bool, 是否成功
// 函数说明:  完成初始化工作
//
// $_FUNCTION_END ********************************
bool CIOCPMgr::InitIOCP()
{
	// 初始化Overlapped plus 内存池
		
	
	//发包队列chache初始化
	

	// 创建一个完成端口对象
	mhCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
	if (NULL == mhCompletionPort)
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "CreateIoCompletionPort failed in Initialize, "
			"error code " << GETSOCKET_ERRCODE());
		return false;
	}
	return true;
}
// $_FUNCTION_BEGIN ******************************
//
// 函数名称:  RegisterNetEvent
// 函数参数:  无. 
// 返 回 值:  bool, 是否成功
// 函数说明:  注册网络事件
//
// $_FUNCTION_END ********************************
bool CIOCPMgr::RegisterNetEvent()
{
	int    lret=0;

	// 在此创建处理网络事件handle
	mhNoAcceptEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (INVALID_HANDLE_VALUE == mhNoAcceptEvent)
	{
		mhNoAcceptEvent = NULL;
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "CreateEvent failed in RegsiterEvent(), "
			"error code "<< GetLastError());
		return false;
	}

	// 在此只处理Accept事件,其它事件不作处理
	lret = WSAEventSelect(mhServerSocket, mhNoAcceptEvent, FD_ACCEPT);
	if (SOCKET_ERROR == lret)
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "WSAEventSelect() failed in RegisterNetEvent, "
			"error code" << GETSOCKET_ERRCODE());

		return false;
	}

	return true;
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称:  StartListen
// 函数参数:  apszAddr - 指定要绑定的网络地址
//           aiPort   - 指定要绑定的本地端口
// 返 回 值:  bool, 是否成功
// 函数说明:  开始在指定的网络地址上监听
//
// $_FUNCTION_END ********************************
bool CIOCPMgr::StartListen(DWORD adwIPAddr, UINT aiPort)
{
	int liErr;
	struct sockaddr_in lAddr;
	memset(&lAddr, 0, sizeof(lAddr));

	if (!adwIPAddr)
		lAddr.sin_addr.s_addr = INADDR_ANY;
	else
		lAddr.sin_addr.s_addr = adwIPAddr;

	lAddr.sin_family = AF_INET;
	lAddr.sin_port   = aiPort;

	// 注册网络事件
	if (!RegisterNetEvent())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error RegisterNetEvent() failed...");
		return false;
	}	
	// 将ServerSock 同完成端口绑定
	if (!AssociateSockwithIOCP(mhServerSocket))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Associate server sock with completionport failed.");
		return false;
	}

	// 绑定指定的地址和端口
	liErr = bind(mhServerSocket, (const struct sockaddr *)&lAddr, sizeof(struct sockaddr_in));
	if (SOCKET_ERROR == liErr)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "bind() failed, error code " << GETSOCKET_ERRCODE());
		return false;
	}

	liErr = listen(mhServerSocket, SOMAXCONN);
	if (SOCKET_ERROR == liErr)
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "listen() failed, error code " << GETSOCKET_ERRCODE());
		return false;
	}

	//// 投递指定个数的AcceptEx等待客户端的连接
	if (!PostNumOfAcceptEx())
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "PostNumOfAcceptEx() failed");
		return false;
	}

	LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "Windowz TCP server Start listen at " << inet_ntoa(lAddr.sin_addr) <<":"<< htons(aiPort));

	return true;
}


// $_FUNCTION_BEGIN ******************************
// 方法名: PostNumOfAcceptEx
// 访  问: private 
// 返回值: bool
// 说  明: 投递指定个数异步Socket的到链接池，等待客户端的连接
// $_FUNCTION_END ********************************
bool CIOCPMgr::PostNumOfAcceptEx()
{
	BOOL   lbret = FALSE;
	DWORD  ldwBytesRead=0;
	SOCKET liAcceptSock=INVALID_SOCKET;
	UINT   liActualNum = 0;
	int    liErr;
	STRU_SOCKET_RECV_CONTEXT *lpSocketContext=NULL;
	CSocketInfo *lpSocketInfo = NULL;
	CCriticalSection& loCriticalSection = mpSocketInfoMgr->GetCriticalSection();
	while (liActualNum < mstruTcpOpt.muiAcceptExNum)
	{
		liAcceptSock = WSASocket(AF_INET,
			SOCK_STREAM,
			IPPROTO_TCP,
			NULL,
			0,
			WSA_FLAG_OVERLAPPED
			);

		if (INVALID_SOCKET == liAcceptSock)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error WSASocket() failed in PostNumOfAcceptEx()"
				<<" Errcode="<< GETSOCKET_ERRCODE());
			return false;
		}

		lpSocketInfo = mpSocketInfoMgr->MallocRefObj();
		lpSocketInfo->init();
		lpSocketContext = lpSocketInfo->mpRecvContext;
		lpSocketContext->mhSocket = liAcceptSock;

		CSocketInfo *lpOldSocketInfo = mpSocketInfoMgr->AddRefObj(liAcceptSock,lpSocketInfo);
		if(lpOldSocketInfo != lpSocketInfo)
		{
			LOG_TRACE(LOG_ERR, true, __FUNCTION__, "New Socket"
				<<" Socket="<<liAcceptSock
				<<" SocketInfo="<<(void*)lpSocketInfo
				<<" SocketContextPtr="<<(void*)lpSocketContext
				<<" Old SocketInfo="<<(void*)lpOldSocketInfo
				<<" Old SocketContext="<<(void*)lpOldSocketInfo->mpRecvContext
				);
			mpSocketInfoMgr->FreeRefObj(lpSocketInfo);
			CloseSocket(liAcceptSock);
			U9_ASSERT(FALSE);
			continue;
		}
		lpSocketInfo->IncreaseRef();//增加一次引用		
		// 投递一个接受操作,等待客户端的连接
		lbret = mpfnAcceptEx(mhServerSocket,
			lpSocketContext->mhSocket,
			lpSocketContext->mpBuff,
			//0,//不想收数据，立即返回
			lpSocketContext->mstruWsabuf.len - (DEF_SOCKADDR_LEN * 2),//有数据才返回，做为服务器这样不好，容易出现死连接
			DEF_SOCKADDR_LEN,
			DEF_SOCKADDR_LEN,
			&ldwBytesRead,
			&lpSocketContext->mstruOv
			);

		if (!lbret)
		{
			liErr = GETSOCKET_ERRCODE();
			if (ERROR_IO_PENDING != liErr)
			{
				//回收
				lpSocketInfo->DecreaseRef();
				mpSocketInfoMgr->DeleteRefObj(liAcceptSock);
				CloseSocket(liAcceptSock);
					LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error AcceptEx() failed in PostNumOfAcceptEx(),"
						<<" Errcode="<< liErr
						<<" Socket="<<liAcceptSock);
				return false;
			}
			InterlockedIncrement((LONG*)&liActualNum);
		}
		
		LOG_TRACE(LOG_INFO, true, __FUNCTION__, "New Socket"
			<<" Socket="<<liAcceptSock
			<<" SocketInfo="<<(void*)lpSocketInfo
			<<" SocketContextPtr="<<(void*)lpSocketContext);
	}
	InterlockedExchangeAdd((LONG*)&liActualNum, -mstruTcpOpt.muiAcceptExNum);
	return true;
}

void CIOCPMgr::NetIOProcessThread(VOID *apParam)
{
	CSystem::InitRandom();
	STRU_TRHEAD_CONTEXT *lpThreadContext = (STRU_TRHEAD_CONTEXT *)apParam;
	CIOCPMgr* lpThis =(CIOCPMgr*) lpThreadContext->mpThis;
	if(NULL ==lpThis )
		return ;

	lpThis->IncreaseRef();
	lpThis->NetIOProcessThread(lpThreadContext->dwIndex);
	lpThis->DecreaseRef();

	delete lpThreadContext;
}
// $_FUNCTION_BEGIN ******************************
//
// 函数名称:  NetIOProcessThread
// 函数参数:  
// 返 回 值:  void
// 函数说明:  网络IO处理线程
//
// $_FUNCTION_END ********************************
void CIOCPMgr::NetIOProcessThread(int32 aiThreadIndex)
{
	DWORD  ldwBytesTrans = 0 ;
	BOOL   lbRet=0;
	int    liErrorCode=0;

	struct STRU_SOCKET_KEY *lpCompKey = NULL;
	struct STRU_SOCKET_BASE_CONTEXT   *lpSocketContext=NULL;
	STRU_EVENT_DATA *lpEvent=NULL;

	DWORD ldwIndex=0;
	while(mbIsRun)
	{
		lbRet = GetQueuedCompletionStatus(mhCompletionPort, (LPDWORD)&ldwBytesTrans,
			(LPDWORD)&lpCompKey, (LPOVERLAPPED*)&lpSocketContext, INFINITE);


		if (-1 == ldwBytesTrans)
		{
			LOG_TRACE(LOG_ERR,true, __FUNCTION__, " Exit Player ");
			return;
		}

		if(!mbIsRun)
		{
			LOG_TRACE(LOG_ERR,true, __FUNCTION__, " Exit");
			return;
		}

		if(!lbRet)
		{
			liErrorCode = GETSOCKET_ERRCODE();

			//一种错误状态
			if(lpSocketContext == NULL)
			{
				LOG_TRACE(LOG_ERR,false, __FUNCTION__, "Error GetQueuedCompletionStatus return fail, lpSocketContext==NULL"
					<< " ErrorCode = "<< liErrorCode
					);
				continue;
			}

			switch(lpSocketContext->miIOStatus)
			{
			case IOS_WRITE://发送数据
				{
					STRU_SOCKET_SEND_CONTEXT *lpSendSocketContext = (STRU_SOCKET_SEND_CONTEXT *)lpSocketContext;
					CSocketInfo *lpSocketInfo = lpSendSocketContext->mpSocketInfo;
					LOG_TRACE(LOG_INFO, true, __FUNCTION__,  "GetQueuedCompletionStatus return fail when is send data"
						<< " ErrorCode = "<< liErrorCode
						<<" Socket="<<lpSocketContext->mhSocket
						<<" SocketContextPtr="<<(void*)lpSocketContext
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" RefCount="<<lpSocketInfo->GetRefCount());
					CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
					mpSendCachePool->free(lpSendSocketContext);
					CCriticalSection &	loCS = lpSocketInfo->moCS;
					loCS.Enter();
					lpSocketInfo->mbStopSend = TRUE;
					lpSocketInfo->miCurrentSendContextCount--;
					loCS.Leave();

					break;
				}
			case IOS_READ://读取数据
				{
					//  处理关闭
					STRU_SOCKET_RECV_CONTEXT *lpRecvSocketContext = (STRU_SOCKET_RECV_CONTEXT *)lpSocketContext;
					CSocketInfo *lpSocketInfo = lpRecvSocketContext->mpSocketInfo;
					CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
					LOG_TRACE(LOG_INFO, true, __FUNCTION__,  "GetQueuedCompletionStatus return fail when is recv data"
						<< " ErrorCode = "<< liErrorCode
						<<" Socket="<<lpRecvSocketContext->mhSocket
						<<" SocketContextPtr="<<(void*)lpSocketContext
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						);
					CloseTCPSocketInNetIOProcessThread(lpRecvSocketContext);
					break;
				}
			case IOS_ACCEPT://accept
				{
					STRU_SOCKET_RECV_CONTEXT *lpRecvSocketContext = (STRU_SOCKET_RECV_CONTEXT *)lpSocketContext;
					CSocketInfo *lpSocketInfo = lpRecvSocketContext->mpSocketInfo;
					CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
					mpSocketInfoMgr->DeleteRefObj(lpRecvSocketContext->mhSocket);
					shutdown(lpRecvSocketContext->mhSocket,SD_BOTH);
					CloseSocket(lpRecvSocketContext->mhSocket);
					LOG_TRACE(LOG_ERR, true, __FUNCTION__,  "GetQueuedCompletionStatus return fail when is accept"
							<< " ErrorCode = "<< liErrorCode
							<<" Socket="<<lpSocketContext->mhSocket
							<<" SocketInfoPtr="<<(void*)lpSocketInfo
							<<" RefCount="<<lpSocketInfo->GetRefCount()
							<<" SocketContextPtr="<<(void*)lpSocketContext
							);
					break;
				}
			}
			continue;
		}

		switch (lpSocketContext->miIOStatus)
		{
		case IOS_WRITE://发送数据完成
			{
				// 发送包个数递增
				mstruStatisInfo.mulTotalSendPacketCount++;
				mstruStatisInfo.mui64TotalSendSize += ldwBytesTrans;

				mulSendPacketCount_Sec++;
				mulSendSize_Sec+= ldwBytesTrans;

				STRU_SOCKET_SEND_CONTEXT *lpSendSocketContext = (STRU_SOCKET_SEND_CONTEXT *)lpSocketContext;
				CSocketInfo *lpSocketInfo = lpSendSocketContext->mpSocketInfo;
				if(ldwBytesTrans != lpSocketContext->mstruWsabuf.len)
				{
					LOG_TRACE(LOG_INFO, true, __FUNCTION__,  "SentData !!!"
						<<" Socket="<<lpSocketContext->mhSocket
						<<" IP="<<GetIPString(lpCompKey->mPeerIP)
						<<":"<<ntohs(lpCompKey->mwPeerPort)
						<<" SerialID="<<lpSendSocketContext->miSerialID
						<<" PackType="<<lpSendSocketContext->miPackType
						<<" PackLen="<<lpSocketContext->mstruWsabuf.len
						<<" SentLen="<<ldwBytesTrans
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" SocketContextPtr="<<(void*)lpSendSocketContext
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						);
				}
				else
				{
					//LOG_TRACE(5, true, __FUNCTION__,  "SentData" 
					//	<<" Socket="<<lpSocketContext->mhSocket
					//	<<" IP="<<GetIPString(lpCompKey->mPeerIP)
					//	<<":"<<ntohs(lpCompKey->mwPeerPort)
					//	<<" SerialID="<<lpSendSocketContext->miSerialID
					//	<<" PackType="<<lpSendSocketContext->miPackType
					//	<<" PackLen="<<lpSocketContext->mstruWsabuf.len
					//	<<" SentLen="<<ldwBytesTrans
					//	<<" SocketInfoPtr="<<(void*)lpSocketInfo
					//	<<" SocketContextPtr="<<(void*)lpSendSocketContext
					//	<<" RefCount="<<lpSocketInfo->GetRefCount()
					//	);
				}
				BOOL lbNeedClose = FALSE;
				BOOL lbHasClose = FALSE;
				{
					CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
					
					CCriticalSection &	loCS = lpSocketInfo->moCS;
					loCS.Enter();
					lpSocketInfo->miCurrentSendContextCount--;

					if(ldwBytesTrans == 0)
					{
						LOG_TRACE(LOG_ERR, true, __FUNCTION__,  "Sent 0 bytes,and clear all wait packet"
							<<" Socket="<<lpSocketContext->mhSocket
							<<" IP="<<GetIPString(lpCompKey->mPeerIP)
							<<":"<<ntohs(lpCompKey->mwPeerPort)
							<<" SerialID="<<lpSendSocketContext->miSerialID
							<<" PackType="<<lpSendSocketContext->miPackType
							<<" PackLen="<<lpSocketContext->mstruWsabuf.len
							<<" SentLen="<<ldwBytesTrans
							<<" SocketInfoPtr="<<(void*)lpSocketInfo
							<<" CurrentSendContextCount="<<lpSocketInfo->miCurrentSendContextCount
							<<" RefCount="<<lpSocketInfo->GetRefCount()
							);
						loCS.Leave();

						mpSendCachePool->free(lpSendSocketContext);
						break;
					}

					if(lpSocketInfo->mbHasClosed)
					{
						LOG_TRACE(LOG_ERR, true, __FUNCTION__,  "Sent coplete,socket has close,and clear all wait packet"
							<<" Socket="<<lpSocketContext->mhSocket
							<<" IP="<<GetIPString(lpCompKey->mPeerIP)
							<<":"<<ntohs(lpCompKey->mwPeerPort)
							<<" SerialID="<<lpSendSocketContext->miSerialID
							<<" PackType="<<lpSendSocketContext->miPackType
							<<" PackLen="<<lpSocketContext->mstruWsabuf.len
							<<" SentLen="<<ldwBytesTrans
							<<" SocketInfoPtr="<<(void*)lpSocketInfo
							<<" CurrentSendContextCount="<<lpSocketInfo->miCurrentSendContextCount
							<<" WaitCount="<<lpSocketInfo->moWaitSendDQueue.size()
							<<" RefCount="<<lpSocketInfo->GetRefCount()
							);
						loCS.Leave();
						mpSendCachePool->free(lpSendSocketContext);
						break;
					}
					if(lpSendSocketContext->mbSendClose)
					{// 写操作判断是否发包后关闭套接字
						lpSocketInfo->mbStopSend = TRUE;//停止发送数据
						loCS.Leave();
						LOG_TRACE(LOG_ERR, true, __FUNCTION__,  "Sent coplete,post socket colse event"
							<<" Socket="<<lpCompKey->mhSocket
							<<" IP="<<GetIPString(lpCompKey->mPeerIP)
							<<" PackLen="<<lpSocketContext->mstruWsabuf.len
							<<" SentLen="<<ldwBytesTrans
							<<" SocketInfoPtr="<<(void*)lpSocketInfo
							<<" CurrentSendContextCount="<<lpSocketInfo->miCurrentSendContextCount
							<<" WaitCount="<<lpSocketInfo->moWaitSendDQueue.size()
							<<" RefCount="<<lpSocketInfo->GetRefCount()
							);
						mpSendCachePool->free(lpSendSocketContext);

						////这样做。是为了客户端能够有时间处理这次发送的包。而不要直接close
						AddCloseTimeoutQueue(lpSocketInfo);
						//DWORD ldwIndex = GetDealThreadIndex(lpCompKey->mPeerIP,lpCompKey->mwPeerPort);
						//STRU_EVENT_DATA *lpEvent = moEventCacheQueue.malloc();
						//lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CLOSE_SOCKET_AT_SEND_COMPLETE;
						//lpEvent->mhSocket = lpCompKey->mhSocket;
						//lpEvent->miIP = lpCompKey->mPeerIP;
						//lpEvent->mwPort = lpCompKey->mwPeerPort;
						//mpEventQueue[ldwIndex].push_back(lpEvent);
						//break;
					}

					//判断是否还有后续
					STRU_SOCKET_SEND_CONTEXT *lpOldSendSocketContext=lpSendSocketContext;
					lpSendSocketContext = lpSocketInfo->moWaitSendDQueue.pop_front();
					if(NULL == lpSendSocketContext)
					{//没有后续数据包了
						loCS.Leave();
						/*
						LOG_TRACE(4, true, __FUNCTION__,  "Sent coplete" 
							<<" Socket="<<lpSocketContext->mhSocket
							<<" IP="<<GetIPString(lpCompKey->mPeerIP)
							<<":"<<ntohs(lpCompKey->mwPeerPort)
							<<" SerialID="<<lpOldSendSocketContext->miSerialID
							<<" PackType="<<lpOldSendSocketContext->miPackType
							<<" PackLen="<<lpSocketContext->mstruWsabuf.len
							<<" SentLen="<<ldwBytesTrans
							<<" SocketInfoPtr="<<(void*)lpSocketInfo
							<<" RefCount="<<lpSocketInfo->GetRefCount()
							<<" SendSocketContextPtr="<<(void*)lpOldSendSocketContext
							<<" WaitPackCount="<<(int32)lpSocketInfo->moWaitSendDQueue.size()
							<<" CurrentSendContextCount="<<lpSocketInfo->miCurrentSendContextCount
							);
							*/
						mpSendCachePool->free(lpOldSendSocketContext);
						break;
					}

					lpSendSocketContext->mhSocket = lpSocketContext->mhSocket;
					lpSendSocketContext->mpSocketInfo = lpSocketInfo;
					lpSocketInfo->IncreaseRef();//有人引用这个对象
					lpSocketInfo->miCurrentSendContextCount++;//重新放入一个发送包
					
					if(!PostSendPacket(lpSendSocketContext))
					{
						LOG_TRACE(LOG_ERR,false, __FUNCTION__, "Error PostSendPacket fail"
							<<" Socket="<<lpSendSocketContext->mhSocket
							<<" EncryptType="<<miEncryptType
							<<" IP="<<GetIPString(lpCompKey->mPeerIP)
							<<":"<<ntohs(lpCompKey->mwPeerPort)
							<<" SerialID="<<lpSendSocketContext->miSerialID
							<<" PackType="<<lpSendSocketContext->miPackType
							<<" PackLen="<<lpSocketContext->mstruWsabuf.len
							<<" SentLen="<<ldwBytesTrans
							<<" SocketInfoPtr="<<(void*)lpSocketInfo
							<<" CurrentSendContextCount="<<lpSocketInfo->miCurrentSendContextCount
							<<" WaitCount="<<lpSocketInfo->moWaitSendDQueue.size()
							<<" RefCount="<<lpSocketInfo->GetRefCount()
							);
						
						lpSocketInfo->mbStopSend = TRUE;
						lpSocketInfo->miCurrentSendContextCount--;//减少一个发送包数
						loCS.Leave();

						mpSendCachePool->free(lpOldSendSocketContext);
						mpSendCachePool->free(lpSendSocketContext);
					}
					loCS.Leave();
					mpSendCachePool->free(lpOldSendSocketContext);
				}
				break;
			}
		case IOS_READ:// 读取数据
			{
				STRU_SOCKET_RECV_CONTEXT *lpRecvSocketContext = (STRU_SOCKET_RECV_CONTEXT *)lpSocketContext;
				CSocketInfo *lpSocketInfo = lpRecvSocketContext->mpSocketInfo;
				CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
				if(ldwBytesTrans == 0)
				{
					liErrorCode = GETSOCKET_ERRCODE();
					LOG_TRACE(LOG_ERR, true, __FUNCTION__,  "Recv data is zero ,need close "
						<<" Socket="<<lpSocketContext->mhSocket
						<<" IP="<<GetIPString(lpCompKey->mPeerIP)
						<<":"<<ntohs(lpCompKey->mwPeerPort)
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" SocketContextPtr="<<(void*)lpRecvSocketContext
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						);
					CloseTCPSocketInNetIOProcessThread(lpRecvSocketContext);
					break;
				}
				//printf("Recv Pack %s:%d \n\r",GetIPString(lpCompKey->mPeerIP).c_str(),ntohs(lpCompKey->mwPeerPort));
				
				//接收数据处理，里面需要将数据累加
				RecvTCPData(lpSocketInfo,ldwBytesTrans);

				//处理接收到的包，
				lpSocketInfo->IncreaseRef();
				if(1 != PostRecvPacket(lpRecvSocketContext))
				{
					LOG_TRACE(LOG_ERR, false, __FUNCTION__,  "Error PostRecvPacket fail at read data "
						<<" Socket="<<lpSocketContext->mhSocket
						<<" IP="<<GetIPString(lpCompKey->mPeerIP)
						<<":"<<ntohs(lpCompKey->mwPeerPort)
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" SocketContextPtr="<<(void*)lpRecvSocketContext
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						);
					lpSocketInfo->DecreaseRef();
					CloseTCPSocketInNetIOProcessThread(lpRecvSocketContext);
				}
				break;
			}
		case IOS_ACCEPT:
			{
				//设置SOCKET的属性
				STRU_SOCKET_RECV_CONTEXT *lpRecvSocketContext = (STRU_SOCKET_RECV_CONTEXT *)lpSocketContext;
				CSocketInfo *lpSocketInfo = lpRecvSocketContext->mpSocketInfo;
				
				CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpSocketInfo);
				SetSocektNoBlock(lpRecvSocketContext->mhSocket);
				// 将新的Socket同完成端口关联起来.
				if (!AssociateSockwithIOCP(lpRecvSocketContext->mpSocketInfo))
				{
					LOG_TRACE(LOG_ERR, true, __FUNCTION__,  "Error AssociateSocketwithIOCP_Accept fail  "
						<<" Socket="<<lpRecvSocketContext->mhSocket
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" SocketContextPtr="<<(void*)lpRecvSocketContext
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						);
					//邦定失败
					mpSocketInfoMgr->DeleteRefObj(lpRecvSocketContext->mhSocket);
					shutdown(lpRecvSocketContext->mhSocket,SD_BOTH);
					CloseSocket(lpRecvSocketContext->mhSocket);

					continue;
				}
				STRU_SOCKET_KEY &loKey = lpRecvSocketContext->mpSocketInfo->moKey;

				LOG_TRACE(LOG_ERR, true, __FUNCTION__,  " socket connect success "
					<<" Socket="<<lpRecvSocketContext->mhSocket
					<<" IP="<<GetIPString(loKey.mPeerIP)
					<<":"<<ntohs(loKey.mwPeerPort)
					<<" SocketInfoPtr="<<(void*)lpSocketInfo
					<<" SocketContextPtr="<<(void*)lpRecvSocketContext
					<<" RefCount="<<lpSocketInfo->GetRefCount()
					);
				ldwIndex =GetDealThreadIndex(loKey.mPeerIP,loKey.mwPeerPort);

				//提交上层处理
				lpEvent= moEventCacheQueue.malloc();

				lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CONNECT_SUCCESS;
				lpEvent->mhSocket = lpSocketContext->mhSocket;
				lpEvent->miIP = loKey.mPeerIP;
				lpEvent->mwPort = loKey.mwPeerPort;
				lpEvent->mpParam1 = NULL;

				lpEvent->mpRelationPtr = lpSocketInfo;
				lpSocketInfo->IncreaseRef();//增加一次引用

				mpEventQueue[ldwIndex].push_back(lpEvent);

				//更新统计信息
				CSystem::InterlockedIncrement((LONG *)&mstruStatisInfo.muTotalConns);
				mstruStatisInfo.mui64TotalRecvSize += ldwBytesTrans;

				if (ldwBytesTrans > 0)
				{//有数据
					RecvTCPData(lpSocketInfo,ldwBytesTrans);
				}

				//更改操作类型
				lpSocketContext->miIOStatus = IOS_READ;
				lpSocketInfo->IncreaseRef();//增加引用计数
				if(1 != PostRecvPacket(lpRecvSocketContext))
				{
					LOG_TRACE(LOG_ERR, false, __FUNCTION__,  "Error PostRecvPacket fail "
						<<" Socket="<<lpSocketContext->mhSocket
						<<" IP="<<GetIPString(loKey.mPeerIP)
						<<":"<<ntohs(loKey.mwPeerPort)
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" SocketContextPtr="<<(void*)lpRecvSocketContext
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						);
					lpSocketInfo->DecreaseRef();
					CloseTCPSocketInNetIOProcessThread(lpRecvSocketContext);//处理关闭
					continue;
				}
				break;
			}
		default:
			{
				CSocketInfo *lpSocketInfo = lpSocketContext->mpSocketInfo;
				LOG_TRACE(LOG_ERR, false, __FUNCTION__,  "Error context type is bad"
					<<" Socket="<<lpSocketContext->mhSocket
					<<" SocketInfoPtr="<<(void*)lpSocketInfo
					<<" SocketContextPtr="<<(void*)lpSocketContext
					<<" RefCount="<<lpSocketInfo->GetRefCount()
					);
				U9_ASSERT(FALSE)
			}
		}
	}
}

void CIOCPMgr::CheckTimeoutThread(VOID *apParam)
{
	CIOCPMgr* lpThis =(CIOCPMgr*) apParam;
	if(NULL ==lpThis )
		return ;
	lpThis->IncreaseRef();
	lpThis->CheckTimeoutThread();
	lpThis->DecreaseRef();
} 

void CIOCPMgr::CheckTimeoutThread()
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
		if(i%100)//间隔1s取一次时间
			mlLocalTime = CSystem::GetTime();
		if(i<100) //20秒检测一次
			continue;
		i=0;
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "check timeout start");
		for(int j = 0;j<mstruTcpOpt.mbyDealThreadCount;j++)
		{
			if(!mbIsRun)
				break;
			CSocketTimeoutDqueue &loTimeoutQueue = mpSocketTimeoutDqueue[j];

			LONG llNow= mlLocalTime - mstruTcpOpt.muiKeepAlive;

			loTimeoutQueue.GetCriticalSection().Enter();
			lpPos = loTimeoutQueue.begin();
			while(NULL != lpPos)
			{
				if(!mbIsRun)
					break;
				lpTimeoutItem =loTimeoutQueue.getdata(lpPos);
				U9_ASSERT (NULL != lpTimeoutItem);
				if(llNow <= lpTimeoutItem->mlLastTime)
					break;
				lpTempPos = lpPos;
				lpPos=loTimeoutQueue.next(lpPos);

				lpSocketInfo = lpTimeoutItem->mpSocketInfo;

				LOG_TRACE(LOG_ERR,true, __FUNCTION__, "Socket Timeout "
					<<" Socket="<<lpSocketInfo->moKey.mhSocket
					<<" SocketInfoPtr="<<(void*)lpSocketInfo
					<<" RefCount = "<< lpSocketInfo->GetRefCount()
					<<" TimeoutPos="<<(void*)lpSocketInfo->mpTimeoutPos);

				moTimeoutCacheQueue.free(lpTimeoutItem);
				lpSocketInfo->mpTimeoutPos = INVALID_U9_POSITION;
				loTimeoutQueue.erase(lpTempPos);
				//关闭socket
				CloseTCPSocket(lpSocketInfo);
			}
			loTimeoutQueue.GetCriticalSection().Leave();
		}
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "check timeout end");
	}
}


void CIOCPMgr::CheckCloseTimeoutThread(VOID *apParam)
{
	CIOCPMgr* lpThis =(CIOCPMgr*) apParam;
	if(NULL ==lpThis )
		return ;
	lpThis->IncreaseRef();
	lpThis->CheckCloseTimeoutThread();
	lpThis->DecreaseRef();
} 

void CIOCPMgr::CheckCloseTimeoutThread()
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
		if(i%100)//间隔1s取一次时间
		{
			mlLocalTime = CSystem::GetTime();
		}
		if(i<100) //3秒检测一次
		{
			continue;
		}
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "check timeout start");
		i=0;

		LONG llNow= mlLocalTime - 2;//2秒延迟关闭

		moCloseSocketTimeoutDqueue.GetCriticalSection().Enter();
		lpPos = moCloseSocketTimeoutDqueue.begin();
		while(NULL != lpPos)
		{
			if(!mbIsRun)
				break;
			lpTimeoutItem =moCloseSocketTimeoutDqueue.getdata(lpPos);
			U9_ASSERT (NULL != lpTimeoutItem);
			if(llNow <= lpTimeoutItem->mlLastTime)
				break;
			lpTempPos = lpPos;
			lpPos=moCloseSocketTimeoutDqueue.next(lpPos);

			lpSocketInfo = lpTimeoutItem->mpSocketInfo;

			LOG_TRACE(LOG_ERR,true, __FUNCTION__, "Socket delay close "
				<<" Socket="<<lpSocketInfo->moKey.mhSocket
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" RefCount = "<< lpSocketInfo->GetRefCount()
				<<" TimeoutPos="<<(void*)lpSocketInfo->mpTimeoutPos);
			moTimeoutCacheQueue.free(lpTimeoutItem);
			lpSocketInfo->mpCloseTimeoutPos = INVALID_U9_POSITION;
			moCloseSocketTimeoutDqueue.erase(lpTempPos);
			//这样做。是为了客户端能够有时间处理这次发送的包。而不要直接close
			//关闭socket
			CloseTCPSocket(lpSocketInfo);
		}
		moCloseSocketTimeoutDqueue.GetCriticalSection().Leave();

		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "check timeout end");
	}
}
void CIOCPMgr::MoniterThread(VOID *apParam)
{
	CIOCPMgr* lpThis =(CIOCPMgr*) apParam;
	if(NULL ==lpThis )
		return ;

	lpThis->IncreaseRef();
	lpThis->MoniterThread();
	lpThis->DecreaseRef();
}
// $_FUNCTION_BEGIN ******************************
//
// 函数名称:  MoniterConnProc
// 函数参数:  无.
// 返 回 值:  DWORD
// 函数说明:  监视连接状态的线程函数
//				定时投递Accept
//				在MONITER_TIME时间间隔内检查超时的连接
// $_FUNCTION_END ********************************
void CIOCPMgr::MoniterThread()
{
	DWORD ldwRet;
	DWORD ldwLaskCheck = GetTickCount();

	while(mbIsRun)
	{
		ldwRet = WaitForSingleObject(mhNoAcceptEvent, MONITER_TIME);
		if(!mbIsRun)
			return ;
		switch(ldwRet)
		{
		case WAIT_TIMEOUT:
			{
				// 等待超时,检查当前所有连接的状态
				ldwLaskCheck = GetTickCount();

				break;
			}
		case WAIT_FAILED:
			{
				// 等待事件通知失败,可以是其它地方出现了错误,所以
				LOG_TRACE(LOG_ERR, 1, __FUNCTION__,  "Error WaitForSingleObject() failed errno:" << GetLastError());
				break;
			}
		case WAIT_OBJECT_0://STATUS_WAIT_0 出现
			{// 没有异步socket可以分配了，需要重新创建
				ResetEvent(mhNoAcceptEvent);

				// 上次投递的AcceptEx已经用完,继续投递
				if (!PostNumOfAcceptEx())
				{
					LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error PostNumOfAcceptEx() failed.");
				}

				break;
			}
		}
	}
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称:  PostSendPacket
// 函数参数:  apSndContext - 指向STRU_SOCKET_SEND_CONTEXT结构
// 返 回 值:  RET_ERROR - 失败
//        :  RET_SUCCESS    - 成功
// 函数说明:  在指定的Socket上投递一个发送请求,此操作为异步
//           ,操作结果会在GetQueuedCompletionStatus()返回.
//
// $_FUNCTION_END ********************************
int CIOCPMgr::PostSendPacket(STRU_SOCKET_SEND_CONTEXT* apSocketContext)
{
	DWORD  ldwFlags = 0;
	DWORD  ldwBytesToSent=0;

	// 投递一个发送请求.
	int liErr = WSASend(apSocketContext->mhSocket, &apSocketContext->mstruWsabuf, 1, 
		&ldwBytesToSent, ldwFlags, &apSocketContext->mstruOv, 
		NULL);

	if (SOCKET_ERROR == liErr)
	{
		liErr = GETSOCKET_ERRCODE();

		if (liErr != ERROR_IO_PENDING)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__,  "Error ,WSASend() failed "
				<<" Errcode="<<liErr
				<<" Socket="<<apSocketContext->mhSocket
				<<" SocketContextPtr="<<(void*)apSocketContext
				);
			//回收Overlap
			return RET_ERROR;
		}
	}

	return RET_SUCCESS;
}

int CIOCPMgr::PostRecvPacket(STRU_SOCKET_RECV_CONTEXT *apSocketContext)
{
	DWORD  ldwBytesRecv=0;
	DWORD  ldwFlags = 0;
	STRU_SOCKET_KEY& loKey=apSocketContext->mpSocketInfo->moKey;
	// 投递一个接收请求
	int liErr = WSARecv(apSocketContext->mhSocket, &apSocketContext->mstruWsabuf, 1, 
		&ldwBytesRecv, &ldwFlags, &apSocketContext->mstruOv,
		NULL);

	if (SOCKET_ERROR == liErr)
	{
		liErr = GETSOCKET_ERRCODE();
		if (liErr != ERROR_IO_PENDING)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__,  "Error WSARecv fail"
				<<" Errcode="<<liErr
				<<" Socket="<<apSocketContext->mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfo="<<(void*)apSocketContext->mpSocketInfo
				<<" SocketContextPtr="<<(void*)apSocketContext
				);
			return -1;
		}
	}

	return 1;
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称:  SendPacket
// 函数参数:  aiSock - 对端的唯一标识
//        :  apData - 指向发送缓冲区的指针
//        :  aiLen  - 发送缓冲区的长度
// 返 回 值:  RET_ERROR - 失败
//        :  RET_SUCCESS    - 成功
// 函数说明:  添加一个数据包到发送队列中
//
// $_FUNCTION_END ********************************
int CIOCPMgr::SendData(STRU_SENDDATA_BASE* apstruSendData)
{
	if (apstruSendData == NULL)
	{
		return RET_ERROR;
	}
	STRU_SENDDATA_TCP *lpTcpSndOpt = (struct STRU_SENDDATA_TCP *)apstruSendData;

	if( lpTcpSndOpt->mwLen >= DEF_PACKET_LEN)
	{
		LOG_TRACE(LOG_ERR, 0, __FUNCTION__, "Error len is bad"
			<<" Socket="<<lpTcpSndOpt->mhSocket
			<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
			<<" LoadLen="<<lpTcpSndOpt->mwLen);
		return RET_ERROR;
	}
	CTCPPacketHeader            loTcpPacketHeader;

	// 分配一块空闲的内存
	STRU_SOCKET_SEND_CONTEXT *lpSendSocketContext = mpSendCachePool->malloc();
	if (NULL == lpSendSocketContext)
	{
		// 如果失败,则直接重新分配
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "allocate memory failed in SendPacket()"
			<<" Socket="<<lpTcpSndOpt->mhSocket
			<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
			<<" LoadLen="<<lpTcpSndOpt->mwLen);
		return RET_ERROR;
	}

	// 对上层提交的数据包进行封包
	int liErr = loTcpPacketHeader.Pack(lpSendSocketContext->mpBuff, DEF_PACKET_LEN,
		lpTcpSndOpt->mpData,lpTcpSndOpt->mwLen,miEncryptType);
	if (RET_ERROR >= liErr)
	{
		LOG_TRACE(LOG_ERR,false, __FUNCTION__, "Error packet data failed in SendPacket"
			<<" Socket="<<lpTcpSndOpt->mhSocket
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
	
	ZeroMemory(&lpSendSocketContext->mstruOv,sizeof(lpSendSocketContext->mstruOv));
	lpSendSocketContext->mstruWsabuf.buf =(char *) lpSendSocketContext->mpBuff;
	lpSendSocketContext->mstruWsabuf.len = (unsigned long)liErr;
	lpSendSocketContext->miIOStatus = IOS_WRITE;
	lpSendSocketContext->mhSocket = lpTcpSndOpt->mhSocket;

	lpSendSocketContext->mbSendClose = lpTcpSndOpt->mbNeedClose;
	lpSendSocketContext->miPackType =*(WORD*)lpTcpSndOpt->mpData;
	
	CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,lpTcpSndOpt->mhSocket);
	if(loSocketInfo.Invalid())
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, " Socket not found"
			<<" Socket="<<lpTcpSndOpt->mhSocket
			<<" SendSocketContext="<<(void*)lpSendSocketContext
			<<" SerialID="<<lpSendSocketContext->miSerialID
			<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
			<<" LoadLen="<<lpTcpSndOpt->mwLen
			);
		mpSendCachePool->free(lpSendSocketContext);
		//if(lpTcpSndOpt->mbNeedClose)
		//{
		//	DisConnectTo(lpTcpSndOpt->mhSocket);
		//}
		return RET_ERROR ;
	}
	CSocketInfo *lpSocketInfo = loSocketInfo.GetPtr();
	STRU_SOCKET_KEY& loKey=lpSocketInfo->moKey;

	CCriticalSection &	loCS = lpSocketInfo->moCS;
	loCS.Enter();
	int32 liRefCount = lpSocketInfo->GetRefCount();
	if(lpSocketInfo->mbStopSend)
	{//已经关闭，没必要发送了
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, " Socket has close"
			<<" Socket="<<lpSendSocketContext->mhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" SerialID="<<lpSendSocketContext->miSerialID
			<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
			<<" LoadLen="<<lpTcpSndOpt->mwLen
			<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
			<<" SocketInfoPtr="<<(void*)lpSocketInfo
			<<" SocketContextPtr="<<(void*)lpSocketInfo->mpRecvContext
			<<" RefCount="<<liRefCount
			);
		loCS.Leave();
		mpSendCachePool->free(lpSendSocketContext);
		return RET_ERROR;
	}

	if(1 <= lpSocketInfo->miCurrentSendContextCount )
	{//大于同时投递数
		CContextDQueue<STRU_SOCKET_SEND_CONTEXT> &loDqueue = lpSocketInfo->moWaitSendDQueue;
		if(loDqueue.size()>0)
		{//合并包处理。
			STRU_SOCKET_SEND_CONTEXT* lpLastSendSocketContext = loDqueue.back();
			if( lpLastSendSocketContext->mstruWsabuf.len < DEF_PACKET_LEN)
			{
				ULONG liLen = DEF_PACKET_LEN- lpLastSendSocketContext->mstruWsabuf.len;
				if(liLen>=lpSendSocketContext->mstruWsabuf.len)
				{
					memcpy(lpLastSendSocketContext->mpBuff+lpLastSendSocketContext->mstruWsabuf.len,
						lpSendSocketContext->mpBuff,lpSendSocketContext->mstruWsabuf.len);

					lpLastSendSocketContext->mstruWsabuf.len = lpLastSendSocketContext->mstruWsabuf.len+
						lpSendSocketContext->mstruWsabuf.len;

					LOG_TRACE(LOG_ERR,true, __FUNCTION__, " data meta last pack success "
						<<" Socket="<<loKey.mhSocket
						<<" IP="<<GetIPString(loKey.mPeerIP)
						<<":"<<ntohs(loKey.mwPeerPort)
						<<" SerialID="<<lpSendSocketContext->miSerialID
						<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
						<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
						<<" LastSendSocketContextPtr="<<(void *)lpLastSendSocketContext
						<<" Len="<<lpLastSendSocketContext->mstruWsabuf.len 
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						<<" WaitQueueSize="<<loDqueue.size()
						);
					loCS.Leave();
					mpSendCachePool->free(lpSendSocketContext);//合并到前面的包里面去了
					return RET_PROCESSED;
				}
				//搬移数据到老的buffer 从cpu运行内存分配的角度来说这个代码没必要执行。
				//memcpy(lpLastSendSocketContext->mpBuff+lpLastSendSocketContext->mstruWsabuf.len,
				//	lpSendSocketContext->mpBuff,liLen);
				//lpLastSendSocketContext->mstruWsabuf.len = DEF_PACKET_LEN;

				//lpSendSocketContext->mstruWsabuf.len = lpSendSocketContext->mstruWsabuf.len-liLen;
				//BYTE *lpBegin = lpSendSocketContext->mpBuff+liLen;
				//memmove(lpSendSocketContext->mpBuff, lpBegin, lpSendSocketContext->mstruWsabuf.len);
			}
		}
		if(0>loDqueue.push_back(lpSendSocketContext))
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back fail"
				<<" Socket="<< loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SerialID="<<lpSendSocketContext->miSerialID
				<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
				<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" RefCount="<<lpSocketInfo->GetRefCount()
				<<" WaitSize="<<lpSocketInfo->moWaitSendDQueue.size()
				);
			loCS.Leave();
			mpSendCachePool->free(lpSendSocketContext);
			return RET_ERROR;
		}
		LOG_TRACE(LOG_ERR,true, __FUNCTION__, "Wait when Send Packet"
				<<" Socket="<< loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SerialID="<<lpSendSocketContext->miSerialID
				<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
				<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" RefCount="<<lpSocketInfo->GetRefCount()
				<<" WaitSize="<<lpSocketInfo->moWaitSendDQueue.size()
			);
		loCS.Leave();
		return RET_SUCCESS;
	}
	
	lpSendSocketContext->mhSocket = loKey.mhSocket;
	lpSendSocketContext->mpSocketInfo = lpSocketInfo;
	lpSocketInfo->IncreaseRef();//有人引用这个对象
	lpSocketInfo->miCurrentSendContextCount++;
	
	// 添加到发送队列中
	if(!PostSendPacket(lpSendSocketContext))
	{
		LOG_TRACE(LOG_ERR,false, __FUNCTION__, "Error PostSendPacket fail"
				<<" Socket="<< loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SerialID="<<lpSendSocketContext->miSerialID
				<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
				<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" RefCount="<<lpSocketInfo->GetRefCount()
				<<" WaitSize="<<lpSocketInfo->moWaitSendDQueue.size()
			);
		lpSocketInfo->miCurrentSendContextCount--;//没有当前发送包
		lpSocketInfo->DecreaseRef();//放弃对socketinfo的引用
		loCS.Leave();
		mpSendCachePool->free(lpSendSocketContext);
		return RET_ERROR;
	}
	loCS.Leave();

	//if(lpSendSocketContext->miPackType == 0)
	//{
	//	LOG_TRACE(7,true, __FUNCTION__, " cmd packet success "
	//		<<" Socket="<<loKey.mhSocket
	//		<<" IP="<<GetIPString(loKey.mPeerIP)
	//		<<":"<<ntohs(loKey.mwPeerPort)
	//		<<" SerialID="<<lpSendSocketContext->miSerialID
	//		<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
	//		<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
	//		<<" SocketInfoPtr="<<(void*)lpSocketInfo
	//		<<" RefCount="<<lpSocketInfo->GetRefCount()
	//		);
	//}
	//else
	//{
	//	LOG_TRACE(7,true, __FUNCTION__, " data packet success "
	//		<<" Socket="<<loKey.mhSocket
	//		<<" IP="<<GetIPString(loKey.mPeerIP)
	//		<<":"<<ntohs(loKey.mwPeerPort)
	//		<<" SerialID="<<lpSendSocketContext->miSerialID
	//		<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
	//		<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
	//		<<" SocketInfoPtr="<<(void*)lpSocketInfo
	//		<<" RefCount="<<lpSocketInfo->GetRefCount()
	//		);
	//}
	//
	
	return RET_SUCCESS;
} 



// $_FUNCTION_BEGIN ******************************
//
// 函数名称:  SendPacket
// 函数参数:  aiSock - 对端的唯一标识
//        :  apData - 指向发送缓冲区的指针
//        :  aiLen  - 发送缓冲区的长度
// 返 回 值:  RET_ERROR - 失败
//        :  RET_SUCCESS    - 成功
// 函数说明:  添加一个数据包到发送队列中
//
// $_FUNCTION_END ********************************
int CIOCPMgr::SendData(void* apSocketInfo,STRU_SENDDATA_BASE* apstruSendData)
{
	if ( apSocketInfo == NULL ||apstruSendData == NULL)
	{
		return RET_ERROR;
	}

	STRU_SENDDATA_TCP *lpTcpSndOpt = (struct STRU_SENDDATA_TCP *)apstruSendData;

	if( lpTcpSndOpt->mwLen >= DEF_PACKET_LEN)
	{
		LOG_TRACE(LOG_ERR, 0, __FUNCTION__, "Error len is bad"
			<<" Socket="<<lpTcpSndOpt->mhSocket
			<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
			<<" LoadLen="<<lpTcpSndOpt->mwLen);
		return RET_ERROR;
	}
	CTCPPacketHeader            loTcpPacketHeader;

	// 分配一块空闲的内存
	STRU_SOCKET_SEND_CONTEXT *lpSendSocketContext = mpSendCachePool->malloc();
	if (NULL == lpSendSocketContext)
	{
		// 如果失败,则直接重新分配
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "allocate memory failed in SendPacket()"
			<<" Socket="<<lpTcpSndOpt->mhSocket
			<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
			<<" LoadLen="<<lpTcpSndOpt->mwLen);
		return RET_ERROR;
	}

	// 对上层提交的数据包进行封包
	int liErr = loTcpPacketHeader.Pack(lpSendSocketContext->mpBuff, DEF_PACKET_LEN,
		lpTcpSndOpt->mpData,lpTcpSndOpt->mwLen,miEncryptType);
	if (RET_ERROR >= liErr)
	{
		LOG_TRACE(LOG_ERR,false, __FUNCTION__, "Error packet data failed in SendPacket"
			<<" Socket="<<lpTcpSndOpt->mhSocket
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
	
	ZeroMemory(&lpSendSocketContext->mstruOv,sizeof(lpSendSocketContext->mstruOv));
	lpSendSocketContext->mstruWsabuf.buf =(char *) lpSendSocketContext->mpBuff;
	lpSendSocketContext->mstruWsabuf.len = (unsigned long)liErr;
	lpSendSocketContext->miIOStatus = IOS_WRITE;
	lpSendSocketContext->mhSocket = lpTcpSndOpt->mhSocket;

	lpSendSocketContext->mbSendClose = lpTcpSndOpt->mbNeedClose;
	lpSendSocketContext->miPackType =*(WORD*)lpTcpSndOpt->mpData;
	
	CSocketInfo *lpSocketInfo = (CSocketInfo *)apSocketInfo;

	STRU_SOCKET_KEY& loKey=lpSocketInfo->moKey;

	CCriticalSection &	loCS = lpSocketInfo->moCS;
	loCS.Enter();
	int32 liRefCount = lpSocketInfo->GetRefCount();
	if(lpSocketInfo->mbStopSend)
	{//已经关闭，没必要发送了
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, " Socket has close"
			<<" Socket="<<lpSendSocketContext->mhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" SerialID="<<lpSendSocketContext->miSerialID
			<<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
			<<" LoadLen="<<lpTcpSndOpt->mwLen
			<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
			<<" SocketInfoPtr="<<(void*)lpSocketInfo
			<<" SocketContextPtr="<<(void*)lpSocketInfo->mpRecvContext
			<<" RefCount="<<liRefCount
			);
		loCS.Leave();
		mpSendCachePool->free(lpSendSocketContext);
		return RET_ERROR;
	}

	if(1 <= lpSocketInfo->miCurrentSendContextCount )
	{//大于同时投递数
		CContextDQueue<STRU_SOCKET_SEND_CONTEXT> &loDqueue = lpSocketInfo->moWaitSendDQueue;
		if(loDqueue.size()>0)
		{//合并包处理。
			STRU_SOCKET_SEND_CONTEXT* lpLastSendSocketContext = loDqueue.back();
			if( lpLastSendSocketContext->mstruWsabuf.len < DEF_PACKET_LEN)
			{
				ULONG liLen = DEF_PACKET_LEN- lpLastSendSocketContext->mstruWsabuf.len;
				if(liLen>=lpSendSocketContext->mstruWsabuf.len)
				{
					memcpy(lpLastSendSocketContext->mpBuff+lpLastSendSocketContext->mstruWsabuf.len,
						lpSendSocketContext->mpBuff,lpSendSocketContext->mstruWsabuf.len);

					lpLastSendSocketContext->mstruWsabuf.len = lpLastSendSocketContext->mstruWsabuf.len+
						lpSendSocketContext->mstruWsabuf.len;

					LOG_TRACE(LOG_ERR,true, __FUNCTION__, " data meta last pack success "
						<<" Socket="<<loKey.mhSocket
						<<" IP="<<GetIPString(loKey.mPeerIP)
						<<":"<<ntohs(loKey.mwPeerPort)
						<<" SerialID="<<lpSendSocketContext->miSerialID
						<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
						<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
						<<" LastSendSocketContextPtr="<<(void *)lpLastSendSocketContext
						<<" Len="<<lpLastSendSocketContext->mstruWsabuf.len 
						<<" SocketInfoPtr="<<(void*)lpSocketInfo
						<<" RefCount="<<lpSocketInfo->GetRefCount()
						<<" WaitQueueSize="<<loDqueue.size()
						);
					loCS.Leave();
					mpSendCachePool->free(lpSendSocketContext);//合并到前面的包里面去了
					return RET_PROCESSED;
				}
				//搬移数据到老的buffer 从cpu运行内存分配的角度来说这个代码没必要执行。
				//memcpy(lpLastSendSocketContext->mpBuff+lpLastSendSocketContext->mstruWsabuf.len,
				//	lpSendSocketContext->mpBuff,liLen);
				//lpLastSendSocketContext->mstruWsabuf.len = DEF_PACKET_LEN;

				//lpSendSocketContext->mstruWsabuf.len = lpSendSocketContext->mstruWsabuf.len-liLen;
				//BYTE *lpBegin = lpSendSocketContext->mpBuff+liLen;
				//memmove(lpSendSocketContext->mpBuff, lpBegin, lpSendSocketContext->mstruWsabuf.len);
			}
		}
		if(0>loDqueue.push_back(lpSendSocketContext))
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back fail"
				<<" Socket="<< loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SerialID="<<lpSendSocketContext->miSerialID
				<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
				<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" RefCount="<<lpSocketInfo->GetRefCount()
				<<" WaitSize="<<lpSocketInfo->moWaitSendDQueue.size()
				);
			loCS.Leave();
			mpSendCachePool->free(lpSendSocketContext);
			return RET_ERROR;
		}
		LOG_TRACE(LOG_ERR,true, __FUNCTION__, "Wait when Send Packet"
				<<" Socket="<< loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SerialID="<<lpSendSocketContext->miSerialID
				<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
				<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" RefCount="<<lpSocketInfo->GetRefCount()
				<<" WaitSize="<<lpSocketInfo->moWaitSendDQueue.size()
			);
		loCS.Leave();
		return RET_SUCCESS;
	}
	
	lpSendSocketContext->mhSocket = loKey.mhSocket;
	lpSendSocketContext->mpSocketInfo = lpSocketInfo;
	lpSocketInfo->IncreaseRef();//有人引用这个对象
	lpSocketInfo->miCurrentSendContextCount++;
	
	// 添加到发送队列中
	if(!PostSendPacket(lpSendSocketContext))
	{
		LOG_TRACE(LOG_ERR,false, __FUNCTION__, "Error PostSendPacket fail"
				<<" Socket="<< loKey.mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SerialID="<<lpSendSocketContext->miSerialID
				<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
				<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" RefCount="<<lpSocketInfo->GetRefCount()
				<<" WaitSize="<<lpSocketInfo->moWaitSendDQueue.size()
			);
		lpSocketInfo->miCurrentSendContextCount--;//没有当前发送包
		lpSocketInfo->DecreaseRef();//放弃对socketinfo的引用
		loCS.Leave();
		mpSendCachePool->free(lpSendSocketContext);
		return RET_ERROR;
	}
	loCS.Leave();

	//if(lpSendSocketContext->miPackType == 0)
	//{
	//	LOG_TRACE(7,true, __FUNCTION__, " cmd packet success "
	//		<<" Socket="<<loKey.mhSocket
	//		<<" IP="<<GetIPString(loKey.mPeerIP)
	//		<<":"<<ntohs(loKey.mwPeerPort)
	//		<<" SerialID="<<lpSendSocketContext->miSerialID
	//		<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
	//		<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
	//		<<" SocketInfoPtr="<<(void*)lpSocketInfo
	//		<<" RefCount="<<lpSocketInfo->GetRefCount()
	//		);
	//}
	//else
	//{
	//	LOG_TRACE(7,true, __FUNCTION__, " data packet success "
	//		<<" Socket="<<loKey.mhSocket
	//		<<" IP="<<GetIPString(loKey.mPeerIP)
	//		<<":"<<ntohs(loKey.mwPeerPort)
	//		<<" SerialID="<<lpSendSocketContext->miSerialID
	//		<<" Len="<<lpSendSocketContext->mstruWsabuf.len 
	//		<<" SendSocketContextPtr="<<(void *)lpSendSocketContext
	//		<<" SocketInfoPtr="<<(void*)lpSocketInfo
	//		<<" RefCount="<<lpSocketInfo->GetRefCount()
	//		);
	//}
	//
	
	return RET_SUCCESS;
} 

SOCKET CIOCPMgr::DirectConnect (IPTYPE aiIPAddr, WORD aiPort)
{
	if (aiIPAddr <= 0 || aiPort <= 0)
	{
		return INVALID_SOCKET;
	}

	int    liErr;
	SOCKET lhSocket;
	struct sockaddr_in lstruAddr;

	lhSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == lhSocket)
	{
		return INVALID_SOCKET;
	}

	lstruAddr.sin_addr.S_un.S_addr = aiIPAddr;
	lstruAddr.sin_family = AF_INET;
	lstruAddr.sin_port = aiPort;

	liErr = connect(lhSocket, (const sockaddr *)&lstruAddr, 
		sizeof(struct sockaddr_in));
	if (liErr != 0)
	{
		if (GETSOCKET_ERRCODE() != WSAEWOULDBLOCK) 
		{
			closesocket(lhSocket);
			return INVALID_SOCKET;
		}
	}

	// 将Socket句柄同完成端口关联起来
	if (!AssociateSockwithIOCP(lhSocket))
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "AssociateSockWithIoComp() failed in connect()");

		closesocket(lhSocket);
		return INVALID_SOCKET;
	}

	if (mpMsgSink)
	{
		DWORD ldwIndex =GetDealThreadIndex(aiIPAddr, aiPort);
		STRU_EVENT_DATA *lpEvent=moEventCacheQueue.malloc();

		lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CONNECT_SUCCESS;
		lpEvent->mhSocket = lhSocket;
		lpEvent->miIP = aiIPAddr;
		lpEvent->mwPort = aiPort;
		lpEvent->mpParam1 = NULL;
		

		mpEventQueue[ldwIndex].push_back(lpEvent);
	}

	return lhSocket;
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称:  ConnectTo
// 函数参数:  aiIPAddr - IP地址
//           aiPort   - 端口
// 返 回 值:  无.
// 函数说明:  Server端主动发出一个TCP连接
//
// $_FUNCTION_END ********************************
int CIOCPMgr::ConnectTo(const SESSIONID& aiPeerID, IPTYPE aiIPAddr, WORD aiPort)
{
	if(aiPeerID == 0)
	{//不用线程连接
		return DirectConnect(aiIPAddr,aiPort);

	}

	STRU_CONNECT_EVENT *lpEvent = moConnectEventCacheQueue.malloc();
	if(NULL == lpEvent)
		return RET_ERROR;
	lpEvent->miID = aiPeerID;
	lpEvent->miDestIP = aiIPAddr;
	lpEvent->mwDestPort = htons(aiPort);
	lpEvent->miCreateTime = mlLocalTime;
	lpEvent->mhSocket = INVALID_SOCKET;

	DWORD ldwIndex = GetConnnectThreadIndex(aiIPAddr,aiPort);
	if(INVALID_U9_POSITION ==  mpConnectEventQueue[ldwIndex].push_back(lpEvent))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back failed."
			<<" PeerID="<<aiPeerID
			<<" IP="<<GetIPString(aiIPAddr)
			<<":"<<ntohs(aiPort)
			<<" Index="<<ldwIndex
			);
		moConnectEventCacheQueue.free(lpEvent);
		return RET_ERROR;
	}

	return RET_SUCCESS;
}
// $_FUNCTION_BEGIN ******************************
//
// 函数名称:  SetStop
// 函数参数:  无.
// 返 回 值:  无.
// 函数说明:  通知Server停止收发数据
//
// $_FUNCTION_END ********************************
INT CIOCPMgr::Close()
{
	//这东西没法调用 AcceptEx没有关闭
	//关闭所有的连接
#ifdef TRACE_LOG
	LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "Close()");
#endif 
	if (mhCompletionPort)
	{//关闭socket
		::PostQueuedCompletionStatus(mhCompletionPort,-1,0,NULL);
	}

	CBaseTCPMgr::Close();
	SetEvent(mhNoAcceptEvent);

	if (mhCompletionPort)
	{
		CloseHandle(mhCompletionPort);
		mhCompletionPort = 0;
	}

	if(mhServerSocket)
	{
		closesocket(mhServerSocket);
		mhServerSocket = INVALID_SOCKET;
	}

	for(int i=0; i<mstruTcpOpt.mbyRecvThreadCount; i++)
		PostQueuedCompletionStatus(mhCompletionPort, (DWORD)0, (ULONG_PTR)NULL, NULL);

	WaitClose();

	return RET_SUCCESS;
}

INT CIOCPMgr::Open()
{
	mbIsRun=TRUE;

	if(!InitWinSock())
	{
		mbIsRun=FALSE;
#ifdef TRACE_LOG
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "Initialize socket error!");
#endif 
		return RET_ERROR;
	}

	if(!InitIOCP())
	{
		mbIsRun=FALSE;
#ifdef TRACE_LOG
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "Initialize comption port error!");
#endif 
		return RET_ERROR;
	}

	if(mstruTcpOpt.mbIsServer)
	{
		if(!StartListen(mstruTcpOpt.mulIPAddr, mstruTcpOpt.mwPort))
		{
			mbIsRun=FALSE;
#ifdef TRACE_LOG
			LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "Start listen error!");
#endif
			return RET_ERROR;
		}
	}

	//io线程
	for(int i=0;i<mstruTcpOpt.mbyDealThreadCount;i++)
	{
		STRU_TRHEAD_CONTEXT *lpThreadContext = new STRU_TRHEAD_CONTEXT;
		lpThreadContext->mpThis = (void *)this;
		lpThreadContext->dwIndex = i;

		CSystem::BeginThread(NetIOProcessThread, (void *)lpThreadContext);
	}

	//原始包处理线程
	CSystem::BeginThread(DealRawPackThread, this);

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

	CSystem::BeginThread(CheckCloseTimeoutThread, (void *)this);

	//启动监视线程
	CSystem::BeginThread(MoniterThread, this);

	return RET_SUCCESS;
}

//关闭SOCKET
INT CIOCPMgr::DisConnectTo(SOCKET ahSocket)
{
	LOG_TRACE(LOG_ERR, true, __FUNCTION__,  " close "
		<<" Socket="<<ahSocket);
	if(ahSocket == INVALID_SOCKET)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Socket is bad"
			<<" Socket="<<ahSocket
			);
		return 1;
	}

	CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,ahSocket);
	if(loSocketInfo.Invalid())
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Socket not found"
			<<" Socket="<<ahSocket
			);
		return 1;
	}
	CSocketInfo *lpSocketInfo = loSocketInfo.GetPtr();
	CloseTCPSocket(lpSocketInfo);
	return RET_SUCCESS;
}
BOOL CIOCPMgr::AssociateSockwithIOCP(SOCKET ahSocket)
{
	moCompKey.mhSocket = ahSocket;

	HANDLE lhHandle = CreateIoCompletionPort((HANDLE)ahSocket, mhCompletionPort,
		(ULONG_PTR)&moCompKey, 0);
	if (NULL == lhHandle)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error CreateIoCompletionPort() failed "
			<< " Errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<ahSocket
			);
		// 如果操作出现错误，则对此块内存进行回收
		return FALSE;
	}
	return TRUE;
}
BOOL CIOCPMgr::AssociateSockwithIOCP(CSocketInfo *apSocketInfo)
{
	BYTE *lpBuff = apSocketInfo->mpRecvContext->mpBuff;
	int32 liBuffLen =  apSocketInfo->mpRecvContext->mstruWsabuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2);
	sockaddr        *lpRemote=NULL;
	sockaddr        *lpLocal=NULL;
	sockaddr_in      lInAddr;

	STRU_SOCKET_KEY &loKey = apSocketInfo->moKey;
	loKey.mhSocket = apSocketInfo->mpRecvContext->mhSocket;

	HANDLE lhHandle = CreateIoCompletionPort((HANDLE)loKey.mhSocket, mhCompletionPort,
		(ULONG_PTR)&loKey, 0);
	if (NULL == lhHandle)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error CreateIoCompletionPort() failed "
			<< " Errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<loKey.mhSocket
			<<" SocketInfoPtr="<<(void*)apSocketInfo
			);
		return FALSE;
	}

	int  liLen= sizeof (sockaddr);
	// 获取对端的IP和端口
	mpfnAcceptExSockAddrs(
		lpBuff,
		liBuffLen ,
		DEF_SOCKADDR_LEN,
		DEF_SOCKADDR_LEN,
		&lpLocal,
		&liLen,
		&lpRemote,
		&liLen
		);
	memcpy(&lInAddr, lpRemote, sizeof(struct sockaddr));
	loKey.mPeerIP = lInAddr.sin_addr.s_addr;
	loKey.mwPeerPort = lInAddr.sin_port;

	if( mstruTcpOpt.muiKeepAlive != 0 && mhServerSocket != loKey.mhSocket)
		AddTimeoutQueue(apSocketInfo);
	return TRUE;
}

BOOL CIOCPMgr::AssociateSocketwithIOCP_Connect(CSocketInfo *apSocketInfo)
{
	BYTE *lpBuff = apSocketInfo->mpRecvContext->mpBuff;
	int32 liBuffLen =  apSocketInfo->mpRecvContext->mstruWsabuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2);
	sockaddr        *lpRemote=NULL;
	sockaddr        *lpLocal=NULL;
	sockaddr_in      lInAddr;

	STRU_SOCKET_KEY &loKey = apSocketInfo->moKey;
	loKey.mhSocket = apSocketInfo->mpRecvContext->mhSocket;

	HANDLE lhHandle = CreateIoCompletionPort((HANDLE)loKey.mhSocket, mhCompletionPort,
		(ULONG_PTR)&loKey, 0);
	if (NULL == lhHandle)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error CreateIoCompletionPort() failed "
			<< " Errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<loKey.mhSocket
			<<" SocketInfoPtr="<<(void*)apSocketInfo
			);
		return FALSE;
	}

	int  liLen= sizeof (sockaddr);
	// 获取对端的IP和端口
	mpfnAcceptExSockAddrs(
		lpBuff,
		liBuffLen ,
		DEF_SOCKADDR_LEN,
		DEF_SOCKADDR_LEN,
		&lpLocal,
		&liLen,
		&lpRemote,
		&liLen
		);
	memcpy(&lInAddr, lpRemote, sizeof(struct sockaddr));
	loKey.mPeerIP = lInAddr.sin_addr.s_addr;
	loKey.mwPeerPort = lInAddr.sin_port;

	if( mstruTcpOpt.muiKeepAlive != 0 && mhServerSocket != loKey.mhSocket)
		AddTimeoutQueue(apSocketInfo);
	return TRUE;
}

// $_FUNCTION_BEGIN ******************************
// 函数名称:  RecvTCPData
// 函数参数:  STRU_SOCKET_KEY *apCompKey
//           STRU_SOCKET_RECV_CONTEXT*& lpSocketContext
// 返 回 值:  STRU_SOCKET_KEY*, 
// 函数说明:  需要做累包和分包处理
// $_FUNCTION_END ********************************
bool CIOCPMgr::RecvTCPData(CSocketInfo *apSocketInfo,DWORD adwRecvDataLen)
{
	int liPackLen = 0;
	STRU_SOCKET_KEY& loKey=apSocketInfo->moKey;

	BYTE* lpBegin = apSocketInfo->mpRecvContext->mpBuff;
	STRU_SOCKET_RECV_CONTEXT *lpRecvContext = apSocketInfo->mpRecvContext;

	INT liRecvLen = lpRecvContext->miBuffLen+adwRecvDataLen;
	if(liRecvLen> sizeof(lpRecvContext->mpBuff))
	{//数据错误 重新开始记录
		InsLostPack();

		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error recv size is big and buff clean"
			<<" Socket="<<lpRecvContext->mhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" SocketInfoPtr="<<(void*)apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
			<<" RecvLen="<<liRecvLen
			<<" DataLen="<< lpRecvContext->miBuffLen
			<<" adwDataLen="<<adwRecvDataLen
			);

		lpRecvContext->miBuffLen = 0;
		lpRecvContext->mstruWsabuf.buf = (char *)lpRecvContext->mpBuff;
		lpRecvContext->mstruWsabuf.len = sizeof(lpRecvContext->mpBuff);
		return false;
	}
	BOOL abIsSuccess = FALSE;
	CRawTCPRecvPack *lpRawTCPPack = NULL;
	//处理数据包 进行分包处理
	while((liPackLen = CTCPPacket::GetPack(lpBegin, liRecvLen))>0)
	{
		lpRawTCPPack = mRawRecvPackCacheList.malloc();
		if(NULL == lpRawTCPPack)
		{
			InsLostPack();
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error malloc RawTCPPack fail"
				<<" Socket="<<lpRecvContext->mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				<<" RecvLen="<<liRecvLen
				<<" DataLen="<<lpRecvContext->miBuffLen
				<<" PackLen="<<liPackLen
				);

			continue;
		}
		abIsSuccess = TRUE;
		lpRawTCPPack->mhSocket = lpRecvContext->mhSocket;
		lpRawTCPPack->mulIPAddr = loKey.mPeerIP;
		lpRawTCPPack->mwIPPort = loKey.mwPeerPort;

		memcpy(lpRawTCPPack->mpData,lpBegin,liPackLen);
		lpRawTCPPack->miDataLen = liPackLen;
		
		if(INVALID_U9_POSITION  == moRawRecvPackList.push_back(lpRawTCPPack))
		{
	
			InsLostPack();
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push fail"
				<<" Socket="<<lpRecvContext->mhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				<<" RecvLen="<<liRecvLen
				<<" DataLen="<<lpRecvContext->miBuffLen
				<<" PackLen="<<liPackLen
			);
			mRawRecvPackCacheList.free(lpRawTCPPack);
			continue;
		}

		mstruStatisInfo.mulTotalRecvPacketCount++;
		mstruStatisInfo.mui64TotalRecvSize += liPackLen;

		lpBegin += liPackLen;
		liRecvLen -= liPackLen;
	}

	if ( mstruTcpOpt.muiKeepAlive != 0 && abIsSuccess)
	{//为了分析最后收包时间分析
		AddTimeoutQueue(apSocketInfo);
	}

	if(liPackLen<0)
	{//错误
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Pack size is bad and buff clean"
			<<" Socket="<<lpRecvContext->mhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" SocketInfoPtr="<<(void*)apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
			<<" DataLen="<<lpRecvContext->miBuffLen
			<<" PackLen="<<liPackLen
			);

		InsLostPack();
		lpRecvContext->miBuffLen = 0;
		lpRecvContext->mstruWsabuf.buf = (char *)lpRecvContext->mpBuff;
		lpRecvContext->mstruWsabuf.len = sizeof(lpRecvContext->mpBuff);
		return false;
	}

	//更新接收缓冲区的信息
	if(0 == liRecvLen)
	{//buff已经没有数据
		//更新缓冲区长度
		lpRecvContext->miBuffLen = 0;
		lpRecvContext->mstruWsabuf.buf = (char *)lpRecvContext->mpBuff;
		lpRecvContext->mstruWsabuf.len = sizeof(lpRecvContext->mpBuff);
	}
	else
	{//buff还有数据
		//更新缓冲区长度
		lpRecvContext->miBuffLen = liRecvLen;
		//printf("remain len=%d %s:%d \n\r",apSocketContext->mdwDataLen,GetIPString(apCompKey->mPeerIP).c_str(),ntohs(apCompKey->mwPeerPort));
		if(lpBegin != lpRecvContext->mpBuff)
		{//更新缓冲区长度
			memmove(lpRecvContext->mpBuff, lpBegin, liRecvLen);
		}
		lpRecvContext->mstruWsabuf.buf = (char *)lpRecvContext->mpBuff + liRecvLen;
		lpRecvContext->mstruWsabuf.len = sizeof(lpRecvContext->mpBuff) - liRecvLen;
	}
	return true;
}
//设置SOCKET选项
bool CIOCPMgr::SetSocektNoBlock(SOCKET ahSocket)
{
	int    liErr;

	DWORD	ldwUpdateAcceptContext = 1;
	liErr = setsockopt(ahSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		(char *)&mhServerSocket, sizeof(SOCKET));
	if (SOCKET_ERROR == liErr)
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__,  "setsockopt with SO_UPDATE_ACCEPT_CONTEXT failed, "
			<<" Errcode=" << GETSOCKET_ERRCODE()
			<<" Socket="<<ahSocket);

		return false;
	}

	//LINGER 时间设置
	struct linger lling;
	lling.l_onoff = 0;
	lling.l_linger = 0; //为秒级的一个值

	liErr = setsockopt(ahSocket, SOL_SOCKET, SO_LINGER,
		(char *)&lling, sizeof(struct linger));
	if (SOCKET_ERROR == liErr)
	{
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__,  "setsockopt with SO_LINGER failed, error code"
			<< GETSOCKET_ERRCODE());
		return false;
	}

	// 接收缓冲区设为128KB
	DWORD    ldwBufSize =1024*32; // 64KB
	liErr = setsockopt(ahSocket, SOL_SOCKET, SO_RCVBUF, 
		(char *)&ldwBufSize, sizeof(ldwBufSize));
	if (SOCKET_ERROR == liErr)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "setsockopt with SO_RCVBUF failed, error code"
			<<" Errcode=" << GETSOCKET_ERRCODE()
			<<" Socket"<<ahSocket);

		return false;
	}

	// 发送缓冲区设为256KB
	ldwBufSize = 0;// 
	liErr = setsockopt(ahSocket, SOL_SOCKET, SO_SNDBUF,
		(char *)&ldwBufSize, sizeof(ldwBufSize));
	if (SOCKET_ERROR == liErr)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "setsockopt with SO_SNDBUF failed, error code"
			<<" Errcode=" << GETSOCKET_ERRCODE()
			<<" Socket="<<ahSocket);

		return false;
	}

	//
	// 设置连接超时选项,Win2k以后的系统有效
	//
	//STRU_TCP_KEEPALIVE lstruInKeepAlive;
	//lstruInKeepAlive.on_off = 1;                            // SO_KEEPALIVE开关打开
	//lstruInKeepAlive.keepalive_time = TIMEOUT_TIME;         // 5秒之后开始测试
	//lstruInKeepAlive.keepalive_interval = TIMEOUT_INTERVAL; // 每次检测之间间隔5秒

	//STRU_TCP_KEEPALIVE lstruOutKeepAlive;
	//ZeroMemory(&lstruOutKeepAlive, sizeof(STRU_TCP_KEEPALIVE));

	//DWORD ldwBytesRet;
	//int liKeepAliveLen = sizeof(STRU_TCP_KEEPALIVE);
	//liErr = WSAIoctl(ahSocket, SIO_KEEPALIVE_VALS, (LPVOID)&lstruInKeepAlive,
	//	liKeepAliveLen, (LPVOID)&lstruOutKeepAlive, liKeepAliveLen,
	//	&ldwBytesRet, NULL, NULL);
	//if (SOCKET_ERROR == liErr)
	//{
	//	LOG_TRACE(5, 1, __FUNCTION__, "WSAIoctl with SIO_KEEPALIVE_VALS failed, error code" 
	//		<<  GETSOCKET_ERRCODE()
	//		);

	//	return false;
	//}

	return true;
}

//
void CIOCPMgr::clearWaitQueue(CSocketInfo *apSocketInfo)
{
	CContextDQueue<STRU_SOCKET_SEND_CONTEXT>  &loDqueue =  apSocketInfo->moWaitSendDQueue;
	int32 liCount=loDqueue.size();
	if(liCount == 0)
		return ;

	STRU_SOCKET_SEND_CONTEXT *lpSocketContext = NULL;
	loDqueue.GetCriticalSection().Enter();
	U9_POSITION loPos =loDqueue.begin();
	while ( loPos != INVALID_U9_POSITION)
	{
		lpSocketContext = loDqueue.getdata(loPos);
		loPos = loDqueue.next(loPos);
		if(NULL == lpSocketContext)
		{
			LOG_TRACE(LOG_ERR, true, __FUNCTION__, " lpSocketContext is null"
				<<" Socket="<<apSocketInfo->mpRecvContext->mhSocket
				<<" IP="<<GetIPString(apSocketInfo->moKey.mPeerIP)
				<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
				<<" SocketInfo="<<(void*)apSocketInfo
				<<" SocketContextPtr="<<(void*)apSocketInfo->mpRecvContext
				<<" CurrentSendContextCount="<<apSocketInfo->miCurrentSendContextCount
				<<" WaitCount="<<loDqueue.size()
				);
			U9_ASSERT(FALSE);
			continue;
		}
		apSocketInfo->DecreaseRef();//快速方法
		mpSendCachePool->free(lpSocketContext);
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, " clear a wait packet "
			<<" Socket="<<apSocketInfo->mpRecvContext->mhSocket
			<<" IP="<<GetIPString(apSocketInfo->moKey.mPeerIP)
			<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
			<<" SocketInfo="<<(void*)apSocketInfo
			<<" SocketContextPtr="<<(void*)apSocketInfo->mpRecvContext
			<<" CurrentSendContextCount="<<apSocketInfo->miCurrentSendContextCount
			<<" SendSocketContext="<<(void*)lpSocketContext
			<<" WaitCount="<<loDqueue.size()
			<<" RefCount="<<apSocketInfo->GetRefCount()
			);		
	}
	loDqueue.clear();
	loDqueue.GetCriticalSection().Leave();
	LOG_TRACE(LOG_ERR, true, __FUNCTION__, " "
		<<" RefCount="<<apSocketInfo->GetRefCount()
		<<" Socket="<<apSocketInfo->mpRecvContext->mhSocket
		<<" IP="<<GetIPString(apSocketInfo->moKey.mPeerIP)
		<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
		<<" SocketInfo="<<(void*)apSocketInfo
		<<" SocketContextPtr="<<(void*)apSocketInfo->mpRecvContext
		<<" CurrentSendContextCount="<<apSocketInfo->miCurrentSendContextCount
		<<" WaitCount="<<liCount
		);

}
void CIOCPMgr::CloseTCPSocketInNetIOProcessThread(STRU_SOCKET_RECV_CONTEXT* apSocketContext)
{// 在网络ID线程调用关闭 
	BOOL lbNeedClose = FALSE;
	SOCKET lhSocket = apSocketContext->mhSocket;
	int liError = GETSOCKET_ERRCODE();
	STRU_SOCKET_KEY& loKey=apSocketContext->mpSocketInfo->moKey;

	//由于STRU_SOCKET_RECV_CONTEXT的生存周期和lpSocketInfo是一致的。所以不用
	//CAutoReleaseRefObjMgrTemplate1对象里来进行自动释放。
	CSocketInfo *lpSocketInfo = apSocketContext->mpSocketInfo;
	CCriticalSection &	loCS = lpSocketInfo->moCS;

	loCS.Enter();
	if(FALSE ==lpSocketInfo->mbHasClosed)
	{
		lbNeedClose = TRUE;
		lpSocketInfo->mbHasClosed = TRUE;
	}
	lpSocketInfo->mbStopSend = TRUE;
	int32 liWaitCount = lpSocketInfo->moWaitSendDQueue.size();
	if(0 != liWaitCount)
		clearWaitQueue(lpSocketInfo);
	int32 liRefCount = lpSocketInfo->GetRefCount()-1;//因为后面要减1的
	loCS.Leave();

	DelTimeoutQueue(lpSocketInfo);
	DelCloseTimeoutQueue(lpSocketInfo);

	if(lbNeedClose)
	{
		mpSocketInfoMgr->DeleteRefObj(lhSocket);
		shutdown(lhSocket,SD_BOTH);
		CloseSocket(lhSocket);
	}

	DWORD ldwIndex = GetDealThreadIndex(loKey.mPeerIP,loKey.mwPeerPort);

	STRU_EVENT_DATA *lpEvent = moEventCacheQueue.malloc();
	lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CLOSE;
	lpEvent->mhSocket = lhSocket;
	lpEvent->miIP = loKey.mPeerIP;
	lpEvent->mwPort = loKey.mwPeerPort;
	lpEvent->miParam = liError;
	
	lpEvent->mpRelationPtr = (void*)lpSocketInfo;

	if(INVALID_U9_POSITION == mpEventQueue[ldwIndex].push_back(lpEvent))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back fail "
			<<" Errcode="<<liError
			<<" Socket="<<lhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" IsCloseSocket="<<lbNeedClose
			<<" SocketInfoPtr="<<(void*)apSocketContext->mpSocketInfo
			<<" SocketContextPtr="<<(void*)apSocketContext
			<<" WaitCount="<<liWaitCount
			<<" RefCount="<<liRefCount
			);
		U9_ASSERT(FALSE);
		moEventCacheQueue.free(lpEvent);
	}
	else
	{
		if(lbNeedClose)
		{
			LOG_TRACE(LOG_ERR, true, __FUNCTION__, " socket close"
				<<" Errcode="<<liError
				<<" Socket="<<lhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" IsCloseSocket="<<lbNeedClose
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" SocketContextPtr="<<(void*)apSocketContext
				<<" BuffLen="<<apSocketContext->miBuffLen
				<<" WaitCount="<<liWaitCount
				<<" RefCount="<<liRefCount
				);
		}
		else
		{
			LOG_TRACE(LOG_ERR, true, __FUNCTION__, " "
				<<" Errcode="<<liError
				<<" Socket="<<lhSocket
				<<" IP="<<GetIPString(loKey.mPeerIP)
				<<":"<<ntohs(loKey.mwPeerPort)
				<<" IsCloseSocket="<<lbNeedClose
				<<" BuffLen="<<apSocketContext->miBuffLen
				<<" SocketInfoPtr="<<(void*)lpSocketInfo
				<<" SocketContextPtr="<<(void*)apSocketContext
				<<" WaitCount="<<liWaitCount
				<<" RefCount="<<liRefCount
				);
		}
	}
	InterlockedDecrement((LONG *)&mstruStatisInfo.muTotalConns);
}

void CIOCPMgr::CloseTCPSocket(CSocketInfo *apSocketInfo)
{
	BOOL lbNeedClose = FALSE;
	STRU_SOCKET_KEY& loKey = apSocketInfo->moKey;
	CCriticalSection &	loCS = apSocketInfo->moCS;
	loCS.Enter();

	SOCKET lhSocket = loKey.mhSocket;
	if(FALSE ==apSocketInfo->mbHasClosed)
	{
		lbNeedClose = TRUE;
		apSocketInfo->mbHasClosed = TRUE;
	}
	apSocketInfo->mbStopSend = TRUE;
	loCS.Leave();

	if(lbNeedClose)
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "socket close"
			<<" Errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<lhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" SocketInfo="<<(void*)apSocketInfo
			<<" SocketContextPtr="<<(void*)apSocketInfo->mpRecvContext
			<<" RefCount="<<apSocketInfo->GetRefCount()
			);
		mpSocketInfoMgr->DeleteRefObj(lhSocket);
		shutdown(lhSocket,SD_BOTH);
		CloseSocket(lhSocket);
	}
	else
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "socket has close"
			<<" Errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<lhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" SocketInfo="<<(void*)apSocketInfo
			<<" SocketContextPtr="<<(void*)apSocketInfo->mpRecvContext
			<<" RefCount="<<apSocketInfo->GetRefCount()
			);
	}
}

CSocketInfo* CIOCPMgr::CreateSocketSocket(SOCKET ahSocket,IPTYPE aiIPAddr,WORD awPort)
{
	CSocketInfo *lpSocketInfo = mpSocketInfoMgr->MallocRefObj();
	if (NULL == lpSocketInfo)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error mpSocketInfoMgr->MallocRefObj()"
			<<" IP="<<GetIPString(aiIPAddr)
			<<":"<<ntohs(awPort)
			<<" Socket="<<ahSocket
			);
		return NULL;
	}

	lpSocketInfo->init();

	// 将Socket句柄同完成端口关联起来
	STRU_SOCKET_RECV_CONTEXT *lpRecvSocketContext = lpSocketInfo->mpRecvContext;
	lpRecvSocketContext->mhSocket = ahSocket;
	lpRecvSocketContext = lpSocketInfo->mpRecvContext;

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
	lpRecvSocketContext->miIOStatus = IOS_READ;
	if (!AssociateSocketwithIOCP_Connect(lpSocketInfo))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error AssociateSockWithIoComp() failed in connect()"
			<<" errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<ahSocket
			<<" IP="<<GetIPString(aiIPAddr)
			<<":"<<ntohs(awPort));
		mpSocketInfoMgr->DeleteRefObj(ahSocket);
		return NULL;
	}
	//lpSocketInfo->IncreaseRef();
	LOG_TRACE(LOG_ERR, true, __FUNCTION__, " Connected  "
		<<" Socket="<<lpSocketInfo->moKey.mhSocket
		<<" IP="<<GetIPString(aiIPAddr)
		<<":"<<ntohs(awPort)
		<<" SocketInfo="<< (void*)lpSocketInfo
		<<" RefCount="<<lpSocketInfo->GetRefCount()
		);
	return lpSocketInfo;
}


U9_POSITION CIOCPMgr::AddTimeoutQueue(CSocketInfo *apSocketInfo)
{
	if(mstruTcpOpt.muiKeepAlive == 0)
		return INVALID_U9_POSITION;
	STRU_SOCKET_KEY& loKey = apSocketInfo->moKey;
	DWORD ldwIndex = GetDealThreadIndex(loKey.mPeerIP,loKey.mwPeerPort);
	CSocketTimeoutDqueue & loTimeoutQueue = mpSocketTimeoutDqueue[ldwIndex];
	STRU_TIMEOUT_ITEM *lpTimeoutItem = NULL;
	CCriticalSection &loCS = loTimeoutQueue.GetCriticalSection();
	loCS.Enter();
	U9_POSITION lpTimeoutPos = apSocketInfo->mpTimeoutPos;
	U9_POSITION lpTempPos=lpTimeoutPos;
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

	LOG_TRACE(LOG_ERR, true, __FUNCTION__, " "
		<<" Socket="<<loKey.mhSocket
		<<" IP="<<GetIPString(loKey.mPeerIP)
		<<":"<<ntohs(loKey.mwPeerPort)
		<<" SocketInfo="<<(void*)apSocketInfo
		<<" RefCount="<<apSocketInfo->GetRefCount()
		<<" Old TimeoutPos="<<(void*)lpTempPos
		<<" TimeoutPos="<<(void*)lpTimeoutPos
		);

	return lpTimeoutPos;
}

U9_POSITION CIOCPMgr::DelTimeoutQueue(CSocketInfo *apSocketInfo)
{
	if(mstruTcpOpt.muiKeepAlive == 0)
		return INVALID_U9_POSITION;
	STRU_SOCKET_KEY& loKey = apSocketInfo->moKey;
	DWORD ldwIndex = GetDealThreadIndex(loKey.mPeerIP,loKey.mwPeerPort);
	CSocketTimeoutDqueue & loTimeoutQueue = mpSocketTimeoutDqueue[ldwIndex];
	U9_POSITION lpTimeoutPos = INVALID_U9_POSITION;
	CCriticalSection &loCS = loTimeoutQueue.GetCriticalSection();
	loCS.Enter();
	lpTimeoutPos = apSocketInfo->mpTimeoutPos;
	if(INVALID_U9_POSITION != lpTimeoutPos)
	{
		STRU_TIMEOUT_ITEM *lpTimeoutItem =loTimeoutQueue.getdata(lpTimeoutPos);
		moTimeoutCacheQueue.free(lpTimeoutItem);

		loTimeoutQueue.erase(lpTimeoutPos);
		
		//注意这个SocketInfo的变量不能上它自己的临界区，否则要出死锁的，
		//因为loTimeoutQueue.GetCriticalSection()和SocketInfo.moCS的调用顺序不同
		apSocketInfo->mpTimeoutPos = INVALID_U9_POSITION;
	}
	loCS.Leave();

	LOG_TRACE(LOG_ERR, true, __FUNCTION__, " "
		<<" Socket="<<loKey.mhSocket
		<<" IP="<<GetIPString(loKey.mPeerIP)
		<<":"<<ntohs(loKey.mwPeerPort)
		<<" SocketInfo="<<(void*)apSocketInfo
		<<" RefCount="<<apSocketInfo->GetRefCount()
		<<" TimeoutPos="<<(void*)lpTimeoutPos
		);
	return lpTimeoutPos;
}

U9_POSITION CIOCPMgr::AddCloseTimeoutQueue(CSocketInfo *apSocketInfo)
{
	STRU_SOCKET_KEY& loKey = apSocketInfo->moKey;
	STRU_TIMEOUT_ITEM *lpTimeoutItem = NULL;
	CCriticalSection &loCS = moCloseSocketTimeoutDqueue.GetCriticalSection();
	loCS.Enter();
	U9_POSITION lpTimeoutPos = apSocketInfo->mpCloseTimeoutPos;
	U9_POSITION lpTempPos=lpTimeoutPos;
	if(INVALID_U9_POSITION != lpTimeoutPos)
	{
		LOG_TRACE(LOG_ERR, true, __FUNCTION__, "Error not delete "
			<<" Socket="<<loKey.mhSocket
			<<" IP="<<GetIPString(loKey.mPeerIP)
			<<":"<<ntohs(loKey.mwPeerPort)
			<<" SocketInfo="<<(void*)apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
			<<" Old TimeoutPos="<<(void*)lpTempPos
			<<" TimeoutPos="<<(void*)lpTimeoutPos
			);
		U9_ASSERT(FALSE);
		lpTimeoutItem =moCloseSocketTimeoutDqueue.getdata(lpTimeoutPos);
		moTimeoutCacheQueue.free(lpTimeoutItem);
		moCloseSocketTimeoutDqueue.erase(lpTimeoutPos);
	}
	lpTimeoutItem= moTimeoutCacheQueue.malloc();
	lpTimeoutItem->mlLastTime = mlLocalTime;
	lpTimeoutItem->mpSocketInfo = apSocketInfo;
	lpTimeoutPos = moCloseSocketTimeoutDqueue.push_back(lpTimeoutItem);
	//注意这个SocketInfo的变量不能上它自己的临界区，否则要出死锁的，
	//因为loTimeoutQueue.GetCriticalSection()和SocketInfo.moCS的调用顺序不同
	apSocketInfo->mpCloseTimeoutPos = lpTimeoutPos;
	loCS.Leave();

	LOG_TRACE(LOG_ERR, true, __FUNCTION__, " "
		<<" Socket="<<loKey.mhSocket
		<<" IP="<<GetIPString(loKey.mPeerIP)
		<<":"<<ntohs(loKey.mwPeerPort)
		<<" SocketInfo="<<(void*)apSocketInfo
		<<" RefCount="<<apSocketInfo->GetRefCount()
		<<" Old TimeoutPos="<<(void*)lpTempPos
		<<" TimeoutPos="<<(void*)lpTimeoutPos
		);
	return lpTimeoutPos;
}

U9_POSITION CIOCPMgr::DelCloseTimeoutQueue(CSocketInfo *apSocketInfo)
{
	U9_POSITION lpTimeoutPos = apSocketInfo->mpTimeoutPos;
	if(INVALID_U9_POSITION == lpTimeoutPos)
		return INVALID_U9_POSITION;
	STRU_SOCKET_KEY& loKey = apSocketInfo->moKey;
	CCriticalSection &loCS = moCloseSocketTimeoutDqueue.GetCriticalSection();
	loCS.Enter();
	STRU_TIMEOUT_ITEM *lpTimeoutItem =moCloseSocketTimeoutDqueue.getdata(lpTimeoutPos);
	moTimeoutCacheQueue.free(lpTimeoutItem);

	moCloseSocketTimeoutDqueue.erase(lpTimeoutPos);

	//注意这个SocketInfo的变量不能上它自己的临界区，否则要出死锁的，
	//因为loTimeoutQueue.GetCriticalSection()和SocketInfo.moCS的调用顺序不同
	apSocketInfo->mpCloseTimeoutPos = INVALID_U9_POSITION;

	loCS.Leave();

	LOG_TRACE(LOG_ERR, true, __FUNCTION__, " "
		<<" Socket="<<loKey.mhSocket
		<<" IP="<<GetIPString(loKey.mPeerIP)
		<<":"<<ntohs(loKey.mwPeerPort)
		<<" SocketInfo="<<(void*)apSocketInfo
		<<" RefCount="<<apSocketInfo->GetRefCount()
		<<" TimeoutPos="<<(void*)lpTimeoutPos
		);
	return lpTimeoutPos;
}
