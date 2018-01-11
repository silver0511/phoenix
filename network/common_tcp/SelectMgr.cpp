// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//			 
// 文件名称: SelectMgr.cpp
// 创 建 人: 史云杰
// 文件说明: tcp select模型  传输通讯
// $_FILEHEADER_END *****************************
#ifdef WIN32
#include <winsock2.h>
#endif 

#include "stdafx.h"
#include "SelectMgr.h"


#if (!defined WIN32)
#define closesocket close
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINPROGRESS EINPROGRESS
#endif /*WIN32*/

///////////////////////////////////////////////////////////////////////////////////////////////////
// CTCPTransMgr 构造
CSelectMgr::CSelectMgr() : miPort (0)
			 , miIPAddr (0)
{
#ifdef TRACE_LOG
	//CSystem::CreateDebugInfo(__FUNCTION__,DEF_DISPLAY_TYPE_ALL,7);
#endif 
	int i=0;
}


//
// CTCPTransMgr析构
//
CSelectMgr::~CSelectMgr()
{
//#if (defined WIN32)
//	//关闭SOCKET库
//	WSACleanup();
//#endif
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： Open
// 函数参数： 
// 返 回 值： 1是成功 -1是失败，
// 函数说明： 打开TransmitMgr操作，实际是初始化整个接受
//			 网络处理模块
//
// $_FUNCTION_END ********************************
INT CSelectMgr::Open()
{
	mbIsRun = TRUE;
	mstruTcpOpt.mbyDealThreadCount = 1;
	mstruTcpOpt.mbyRecvThreadCount = 1;

	//发包队列chache初始化
	/*modify by shiyunjie 2017-01-12
  	*发送除非内核缓冲区满，一般都是直接发送出去
  	*发送逻辑是在处理线程中处理，缓冲区的大小取决于处理线程的个数速度
  	*处理线程数 * 发送系数
	*/
	moSendPackList.init(miMaxSendPackCount);
	moSendPackCacheList.init(miMaxSendPackCount);
	int i=0;
	

	mstruTcpOpt.mbyDealThreadCount = 1;
	moEventCacheQueue.init(miMaxEventCount);
	mpEventQueue = new CSafeCacheDoubleQueue<STRU_EVENT_DATA>[mstruTcpOpt.mbyDealThreadCount];
	
	mstruTcpOpt.mbyConnectThreadCount = 1;
	moConnectEventCacheQueue.init(miMaxEventCount);
	mpConnectEventQueue = new CSafeCacheDoubleQueue<STRU_CONNECT_EVENT>[mstruTcpOpt.mbyConnectThreadCount];
	
	
	
	//io线程 必须是1个，因为要保证包序
	CSystem::BeginThread(RecvThread, this);
	CSystem::BeginThread(DealRawPackThread, this);
	//创建多个是无效的，因为tcp同步处理的话窗口是1个
	//自己创建接受处理线程。
	for(i=0;i<mstruTcpOpt.mbyDealThreadCount;i++)
	{
		mpEventQueue[i].init(miMaxEventCount);
		STRU_TRHEAD_CONTEXT *lpThreadContext = new STRU_TRHEAD_CONTEXT;
		lpThreadContext->mpThis = (void *)this;
		lpThreadContext->dwIndex = 0;
		mpEventQueue[i].init(miMaxEventCount);
		CSystem::BeginThread(DealThread, (void *)lpThreadContext);
	}
	for(i=0;i<mstruTcpOpt.mbyDealThreadCount;i++)
	{
		STRU_TRHEAD_CONTEXT *lpThreadContext = new STRU_TRHEAD_CONTEXT;
		lpThreadContext->mpThis = (void *)this;
		lpThreadContext->dwIndex = i;

		mpConnectEventQueue[i].init(miMaxEventCount);
		CSystem::BeginThread(ConnectThread, (void *)lpThreadContext);
	}
	//自己创建发送线程。因为要保证包序
	CSystem::BeginThread(SendThread, this);

	LOG_TRACE(LOG_ALERT,1,"CSelectMgr::Open","miMaxSendPackCount: " << miMaxSendPackCount
															<< " miMaxEventCount: " << miMaxEventCount
															<< " mbyDealThreadCount: " << mstruTcpOpt.mbyDealThreadCount);
	return RET_SUCCESS;
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： Close
// 函数参数： 
// 返 回 值： 1是成功 -1是失败
// 函数说明： 关闭操作
//
// $_FUNCTION_END ********************************
INT CSelectMgr::Close()
{
	LOG_TRACE(LOG_ALERT, 1, __FUNCTION__, "Close()");
 	CBaseTCPMgr::Close();
	moSendPackList.clear();

	CSocketInfo *lpSocketInfo=NULL;
	{
		CRITICAL_SECTION_HELPER(mpSocketInfoMgr->GetCriticalSection());
		for (CRefObjMgrTemplate1<SOCKET,CSocketInfo>::refobjid_iter ite = mpSocketInfoMgr->GetRefObjMap()->begin();
			ite != mpSocketInfoMgr->GetRefObjMap()->end();ite++)
		{
			lpSocketInfo = 	ite->second;
			closesocket(lpSocketInfo->moKey.mhSocket);
		}
	}
	
	//STRU_SOCKET_CONTEXT *lpSocketContext=NULL;
	//{
	//	CRITICAL_SECTION_HELPER(moConnectList.GetCriticalSection());
	//	for (U9_POSITION lpPos = moConnectList.begin();
	//		INVALID_U9_POSITION != lpPos;lpPos = moConnectList.next(lpPos))
	//	{
	//		lpSocketContext = 	moConnectList.getdata(lpPos);
	//		closesocket(lpSocketContext->mhSocket);
	//	}
	//}
	WaitClose();
	return RET_SUCCESS;
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： SendData
// 函数参数： STRU_SENDDATA_BASE *apstruSendData 
//           数据发送设置
// 返 回 值： 1是成功 -1是失败
// 函数说明： 发送数据
//
// $_FUNCTION_END ********************************
INT CSelectMgr::SendData(STRU_SENDDATA_BASE *apstruSendData)
{
	if (NULL == apstruSendData)
		return RET_ERROR;	

	INT liErr;
	STRU_SENDDATA_TCP *lpSendTCPData = (struct STRU_SENDDATA_TCP *)apstruSendData;

	CSendTCPPacket *lpSendTCPPacket = moSendPackCacheList.malloc();
	if (NULL == lpSendTCPPacket)
		return RET_ERROR;

	lpSendTCPPacket->setPeerSocket(lpSendTCPData->mhSocket);

	lpSendTCPPacket->setNeedClose(lpSendTCPData->mbNeedClose);
	lpSendTCPPacket->setPeerAddr(lpSendTCPData->mIPAddress);
	lpSendTCPPacket->setPeerPort(lpSendTCPData->mwIPPort);
	liErr = lpSendTCPPacket->Pack(lpSendTCPData->mpData, lpSendTCPData->mwLen,miEncryptType);

	if (RET_ERROR == liErr)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error pack failed, "
			<<" length="<<lpSendTCPData->mwLen
			<<" PeerIP="<<GetIPString(lpSendTCPData->mIPAddress)
			<<":Port="<<lpSendTCPData->mwIPPort);

		moSendPackCacheList.free(lpSendTCPPacket);
		return RET_ERROR;
	}

	moSendPackList.push_back(lpSendTCPPacket);
	return RET_SUCCESS;
}


// $_FUNCTION_BEGIN ******************************
//
// 函数名称： SendData
// 函数参数： STRU_SENDDATA_BASE *apstruSendData 
//           数据发送设置
// 返 回 值： 1是成功 -1是失败
// 函数说明： 发送数据
//
// $_FUNCTION_END ********************************
INT CSelectMgr::SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData)
{
	if (NULL== apSocketInfo && NULL == apstruSendData)
		return RET_ERROR;	

	INT liErr;
	STRU_SENDDATA_TCP *lpSendTCPData = (struct STRU_SENDDATA_TCP *)apstruSendData;

	CSendTCPPacket *lpSendTCPPacket = moSendPackCacheList.malloc();
	if (NULL == lpSendTCPPacket)
		return RET_ERROR;

	lpSendTCPPacket->setPeerSocket(lpSendTCPData->mhSocket);

	lpSendTCPPacket->setNeedClose(lpSendTCPData->mbNeedClose);
	lpSendTCPPacket->setPeerAddr(lpSendTCPData->mIPAddress);
	lpSendTCPPacket->setPeerPort(lpSendTCPData->mwIPPort);
	liErr = lpSendTCPPacket->Pack(lpSendTCPData->mpData, lpSendTCPData->mwLen,miEncryptType);

	if (RET_ERROR == liErr)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error pack failed, "
			<<" length="<<lpSendTCPData->mwLen
			<<" PeerIP="<<GetIPString(lpSendTCPData->mIPAddress)
			<<":Port="<<lpSendTCPData->mwIPPort);

		moSendPackCacheList.free(lpSendTCPPacket);
		return RET_ERROR;
	}

	moSendPackList.push_back(lpSendTCPPacket);
	return RET_SUCCESS;
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称: SendTcpPacket
// 函数参数: adwSock - socket值
//          apbuf   - 缓冲区指针
//          aiLen   - 缓冲区长度
// 返 回 值: 成功返回发送成功的字节数 -1是失败
// 函数说明: 发送数据
//
// $_FUNCTION_END ********************************
INT CSelectMgr::SendTcpPacket(CSendTCPPacket *apSendTCPPacket)
{

	SOCKET lhSocket = apSendTCPPacket->getPeerSocket();
	BYTE *lpBuff = apSendTCPPacket->getDataPtr();
	int32 liLen  = apSendTCPPacket->getPacketLen();
		
	if (INVALID_SOCKET == lhSocket)
		return RET_ERROR;

	if (!lpBuff || liLen <= 0)
		return RET_ERROR;

	INT liResult=0;

	while (mbIsRun)
	{
		liResult = send(lhSocket, (const char *)lpBuff, liLen, 0);
		if (SOCKET_ERROR == liResult)
		{
			liResult = GETSOCKET_ERRCODE();
			if(10055 == liResult)
			{//对方缓冲区满，需要等待发送
				CSystem::Sleep(5);
				printf("%d socket error = %d \n\r",lhSocket,liResult);			
				LOG_TRACE(LOG_WARNING,true, __FUNCTION__, " peer buffer is buff "
					<<" Socket="<<lhSocket
					<<" IP="<<GetIPString(apSendTCPPacket->getPeerAddr())
					<<":"<<ntohs(apSendTCPPacket->getPeerPort())
					<<" PackLen="<<apSendTCPPacket->getPacketLen()
					<<" SentLen="<<liResult
					<<" SendSocketContextPtr="<<(void *)apSendTCPPacket
					);
				continue;
			}

			InsLostSend(apSendTCPPacket->getPeerAddr(), apSendTCPPacket->getPeerPort());
			printf("%d socket error = %d \n\r",lhSocket,liResult);
			LOG_TRACE(LOG_ERR,true, __FUNCTION__, " Error socket has error "
				<<" Errcode="<<liResult
				<<" Socket="<<lhSocket
				<<" IP="<<GetIPString(apSendTCPPacket->getPeerAddr())
				<<":"<<ntohs(apSendTCPPacket->getPeerPort())
				<<" PackLen="<<apSendTCPPacket->getPacketLen()
				<<" SentLen="<<liResult
				<<" SendSocketContextPtr="<<(void *)apSendTCPPacket
				);
			break;
		}
		else if (liResult < (int)liLen)
		{
			lpBuff += liResult;
			liLen -= liResult;
			LOG_TRACE(LOG_NOTICE,true, __FUNCTION__, " low "
				<<" Socket="<<lhSocket
				<<" IP="<<GetIPString(apSendTCPPacket->getPeerAddr())
				<<":"<<ntohs(apSendTCPPacket->getPeerPort())
				<<" PackLen="<<apSendTCPPacket->getPacketLen()
				<<" SentLen="<<liResult
				<<" SendSocketContextPtr="<<(void *)apSendTCPPacket
				);
		}
		else
		{
			InsSend(apSendTCPPacket->getPeerAddr(), apSendTCPPacket->getPeerPort());
			//LOG_TRACE(5,true, __FUNCTION__, ""
			//	<<" Socket="<<lhSocket
			//	<<" IP="<<GetIPString(apSendTCPPacket->getPeerAddr())
			//	<<":"<<ntohs(apSendTCPPacket->getPeerPort())
			//	<<" PackLen="<<apSendTCPPacket->getPacketLen()
			//	<<" SentLen="<<liResult
			//	<<" SendSocketContextPtr="<<(void *)apSendTCPPacket
			//	);
			break;
		}
	}

	return liResult;
}
bool CSelectMgr::SetSocektNoBlock(SOCKET ahSocket)
{
	//这里select是阻塞式的socket

	// 接收缓冲区设为256KB
	DWORD    ldwBufSize = 1024 * 1024 * 1024;
	int32 liErr = setsockopt(ahSocket, SOL_SOCKET, SO_RCVBUF, 
		(char *)&ldwBufSize, sizeof(ldwBufSize));
	if (SOCKET_ERROR == liErr)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "setsockopt with SO_RCVBUF failed, error code"
			<<" Errcode=" << GETSOCKET_ERRCODE()
			<<" Socket="<<ahSocket);
		return false;
	}

	// 发送缓冲区设为256KB
	ldwBufSize = 1024 * 1024 * 1024;
	liErr = setsockopt(ahSocket, SOL_SOCKET, SO_SNDBUF,
		(char *)&ldwBufSize, sizeof(ldwBufSize));
	if (SOCKET_ERROR == liErr)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "setsockopt with SO_SNDBUF failed, error code"
			<<" Errcode=" << GETSOCKET_ERRCODE()
			<<" Socket="<<ahSocket);

		return false;
	}
	return true;
}
INT CSelectMgr::DirectConnect(IPTYPE aiIPAddr, WORD awPort)
{
	if(mpSocketInfoMgr->size()>= FD_SETSIZE)
	{
		return INVALID_SOCKET;
	}

	SOCKET lhSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == lhSocket)
		return INVALID_SOCKET;
	SetSocektNoBlock(lhSocket);

	struct sockaddr_in lstruAddr;
	memset(&lstruAddr, 0, sizeof(lstruAddr));

	lstruAddr.sin_addr.s_addr = aiIPAddr;
	lstruAddr.sin_port = awPort;
	lstruAddr.sin_family = AF_INET;
	
	int liErr = connect(lhSocket, (const sockaddr *)&lstruAddr, sizeof(lstruAddr));
	if (liErr == SOCKET_ERROR)
	{
		if (mpMsgSink)
		{
			DWORD ldwIndex =GetDealThreadIndex(aiIPAddr, awPort);
			STRU_EVENT_DATA *lpEvent=moEventCacheQueue.malloc();

			lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CONNECT_FAILURE;
			//lpEvent->miID = INVALD_;
			lpEvent->mhSocket = INVALID_SOCKET;
			lpEvent->miIP = aiIPAddr;
			lpEvent->mwPort =awPort;
			lpEvent->mpParam1 = NULL;
			//lpEvent->mpSocketInfo = NULL;
			mpEventQueue[ldwIndex].push_back(lpEvent);
		}
		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "ConnectTo error:" << GETSOCKET_ERRCODE()
			<<" IP="<<GetIPString(aiIPAddr)<<":"<<ntohs(awPort));
		closesocket(lhSocket);
		return INVALID_SOCKET;
	}
	CSocketInfo *lpSocketInfo =CreateSocketSocket(lhSocket,aiIPAddr,awPort);
	if(NULL == lpSocketInfo)
	{
		if (mpMsgSink)
		{
			DWORD ldwIndex =GetDealThreadIndex(aiIPAddr, awPort);
			STRU_EVENT_DATA *lpEvent=moEventCacheQueue.malloc();

			lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CONNECT_FAILURE;
			//lpEvent->miID = INVALD_;
			lpEvent->mhSocket = INVALID_SOCKET;
			lpEvent->miIP = aiIPAddr;
			lpEvent->mwPort =awPort;
			lpEvent->mpParam1 = NULL;
			//lpEvent->mpSocketInfo = NULL;
			mpEventQueue[ldwIndex].push_back(lpEvent);
		}

		LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "CreateSocketSocket fail"
			<<" Socket="<<lhSocket
			<<" IP="<<GetIPString(aiIPAddr)<<":"<<ntohs(awPort));
		closesocket(lhSocket);
		return INVALID_SOCKET;
	}

	LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, " Socket Connect success"
		<<" Socket="<<lhSocket
		<<" IP="<<GetIPString(aiIPAddr)
		<<":"<<ntohs(awPort)
		<<" SocketInfoPtr="<<(void*)lpSocketInfo
		<<" RefCount="<<lpSocketInfo->GetRefCount()
		);

	DWORD ldwIndex =GetDealThreadIndex(aiIPAddr, awPort);
	//提交上层处理
	STRU_EVENT_DATA *lpEvent= moEventCacheQueue.malloc();

	lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CONNECT_SUCCESS;
	lpEvent->mhSocket = lhSocket;
	lpEvent->miIP = aiIPAddr;
	lpEvent->mwPort = awPort;
	lpEvent->mpParam1 = NULL;
	lpEvent->mpRelationPtr = lpSocketInfo;
	
	lpSocketInfo->IncreaseRef();//增加一次引用

	mpEventQueue[ldwIndex].push_back(lpEvent);
	return lhSocket;
}


CSocketInfo* CSelectMgr::CreateSocketSocket(SOCKET ahSocket,IPTYPE aiIPAddr,WORD awPort)
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
	lpSocketInfo->moKey.mhSocket= ahSocket;
	lpSocketInfo->moKey.mPeerIP = aiIPAddr;
	lpSocketInfo->moKey.mwPeerPort = awPort;

	// 将Socket句柄同完成端口关联起来
	//lpSocketInfo->mpRecvContext->mhSocket = ahSocket;
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
	LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, " Connected  "
		<<" Socket="<<lpSocketInfo->moKey.mhSocket
		<<" IP="<<GetIPString(aiIPAddr)
		<<":"<<ntohs(awPort)
		<<" SocketInfo="<< (void*)lpSocketInfo
		<<" RefCount="<<lpSocketInfo->GetRefCount()
		);
	return lpSocketInfo;
}

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： ConnectToSvr
// 函数参数： aiIPAddr 服务器IP， awPort 服务器的监听端口
// 返 回 值： 1是成功 -1是失败
// 函数说明： 发送数据
//
// $_FUNCTION_END ********************************
INT CSelectMgr::ConnectTo(const SESSIONID& aiPeerID, IPTYPE aiIPAddr, WORD aiPort)
{
	if(aiPeerID == 0)
	{//不用线程连接
		return DirectConnect(aiIPAddr,aiPort);
	}
	if(NULL == mpConnectEventQueue)
		return RET_ERROR;

	STRU_CONNECT_EVENT *lpEvent = moConnectEventCacheQueue.malloc();
	if(NULL == lpEvent)
		return RET_ERROR;
	lpEvent->miID = aiPeerID;
	lpEvent->miDestIP = aiIPAddr;
	lpEvent->mwDestPort = aiPort;
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
// 函数名称： CloseTo
// 函数参数： ai64PeerID
// 返 回 值： 1是成功 -1是失败
// 函数说明： 发送数据
//
// $_FUNCTION_END ********************************
INT CSelectMgr::DisConnectTo(SOCKET ahSocket)
{

	LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__,  " close "
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

void CSelectMgr::CloseTCPSocketInRecv(CSocketInfo *apSocketInfo)
{// 处理关闭 

	BOOL lbNeedClose = FALSE;
	SOCKET lhSocket = apSocketInfo->moKey.mhSocket;
	int liError = GETSOCKET_ERRCODE();
	
	//由于STRU_SOCKET_RECV_CONTEXT的生存周期和lpSocketInfo是一致的。所以不用
	//CAutoReleaseRefObjMgrTemplate1对象里来进行自动释放。
	CCriticalSection &	loCS = apSocketInfo->moCS;

	loCS.Enter();
	if(FALSE ==apSocketInfo->mbHasClosed)
	{
		lbNeedClose = TRUE;
		apSocketInfo->mbHasClosed = TRUE;
	}
	apSocketInfo->mbStopSend = TRUE;
	int32 liRefCount = apSocketInfo->GetRefCount()-1;//因为后面要减1的

	DescCurCon(0);
	loCS.Leave();

	if(lbNeedClose)
	{
		shutdown(lhSocket,SD_BOTH);
		CloseSocket(lhSocket);
	}

	DWORD ldwIndex = GetDealThreadIndex(apSocketInfo->moKey.mPeerIP,apSocketInfo->moKey.mwPeerPort);

	STRU_EVENT_DATA *lpEvent = moEventCacheQueue.malloc();
	lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CLOSE;
	lpEvent->mhSocket = lhSocket;
	lpEvent->miIP = apSocketInfo->moKey.mPeerIP;
	lpEvent->mwPort = apSocketInfo->moKey.mwPeerPort;
	lpEvent->miParam = liError;
	lpEvent->mpRelationPtr = (void*)apSocketInfo;

	if(0 > mpSocketInfoMgr->DeleteRefObj(lhSocket))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error DeleteRefObj fail "
			<<" Errcode="<<liError
			<<" Socket="<<lhSocket
			<<" IP="<<GetIPString(apSocketInfo->moKey.mPeerIP)
			<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
			<<" IsCloseSocket="<<lbNeedClose
			<<" SocketInfoPtr="<<(void*)apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
			);
		U9_ASSERT(FALSE);
	}
	if(INVALID_U9_POSITION == mpEventQueue[ldwIndex].push_back(lpEvent))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back fail "
			<<" Errcode="<<liError
			<<" Socket="<<lhSocket
			<<" IP="<<GetIPString(apSocketInfo->moKey.mPeerIP)
			<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
			<<" IsCloseSocket="<<lbNeedClose
			<<" SocketInfoPtr="<<(void*)apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
			);
		U9_ASSERT(FALSE);
		moEventCacheQueue.free(lpEvent);
	}
	else
	{
		if(lbNeedClose)
		{
			LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, " socket close"
				<<" Errcode="<<liError
				<<" Socket="<<lhSocket
				<<" IP="<<GetIPString(apSocketInfo->moKey.mPeerIP)
				<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
				<<" IsCloseSocket="<<lbNeedClose
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				);
		}
		else
		{
			LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, " "
				<<" Errcode="<<liError
				<<" Socket="<<lhSocket
					<<" IP="<<GetIPString(apSocketInfo->moKey.mPeerIP)
				<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
				<<" IsCloseSocket="<<lbNeedClose
				<<" SocketInfoPtr="<<(void*)apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				);
		}
	}
}

void CSelectMgr::CloseTCPSocket(CSocketInfo *apSocketInfo)
{
	BOOL lbNeedClose = FALSE;
	SOCKET lhSocket = apSocketInfo->moKey.mhSocket;
	apSocketInfo->moCS.Enter();
	if(FALSE ==apSocketInfo->mbHasClosed)
	{
		lbNeedClose = TRUE;
		apSocketInfo->mbHasClosed = TRUE;
	}
	apSocketInfo->mbStopSend = TRUE;
	apSocketInfo->moCS.Leave();

	if(lbNeedClose)
	{
		LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "socket close"
			<<" Errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<lhSocket
			<<" IP="<<GetIPString(apSocketInfo->moKey.mPeerIP)
			<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
			<<" SocketInfo="<<(void*)apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
			);
		shutdown(lhSocket,SD_BOTH);
		CloseSocket(lhSocket);
	}
	else
	{
		LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "socket has close"
			<<" Errcode="<<GETSOCKET_ERRCODE()
			<<" Socket="<<lhSocket
			<<" IP="<<GetIPString(apSocketInfo->moKey.mPeerIP)
			<<":"<<ntohs(apSocketInfo->moKey.mwPeerPort)
			<<" SocketInfo="<<(void*)apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
			);
	}
}
// $_FUNCTION_BEGIN ******************************
//
// 函数名称： CreateClientConn
// 函数参数： 无．
// 返 回 值： DWORD
// 函数说明： 创建一个客户端连接
//
// $_FUNCTION_END ********************************


void CSelectMgr::RecvThread(VOID *apParam)
{
	CSystem::InitRandom();
	CSelectMgr* lpThis =(CSelectMgr*) apParam;
	if(NULL ==lpThis )
		return ;

	lpThis->IncreaseRef();
	lpThis->RecvThread();
	lpThis->DecreaseRef();
}
// $_FUNCTION_BEGIN ******************************
//
// 函数名称： RecvThreadProc
// 函数参数： 无．
// 返 回 值： DWORD
// 函数说明： 接收处理线程函数，仅在客户端使用
//
// $_FUNCTION_END ********************************
void CSelectMgr::RecvThread()
{
	fd_set lstruFDSet;
	SOCKET lhMaxSocket = 0;
	struct timeval lstru_tv;
	INT    liResult =0;
	int	   liSelectCount=0;

	lstru_tv.tv_sec = 1;
	lstru_tv.tv_usec = 0;

	CSocketInfo *lpSocketInfo = NULL;
	CSocketInfo *lpSocketInfoList[FD_SETSIZE];
	STRU_EVENT_DATA *lpEvent = NULL;
	
	CCriticalSection &loCriticalSection = mpSocketInfoMgr->GetCriticalSection();


	while (mbIsRun)
	{
		
		if(mpSocketInfoMgr->size() == 0)
		{
			CSystem::Sleep(10);
			continue;
		}
		FD_ZERO(&lstruFDSet); // read fd
		//FD_ZERO(&lstru_fdw); // write fd
		//FD_ZERO(&lstru_fde); // exception fd
		lstru_tv.tv_sec = 1;
		lstru_tv.tv_usec = 0;

		lhMaxSocket = 0;
		liSelectCount=0;
		{
			loCriticalSection.Enter();
			for (CRefObjMgrTemplate1<SOCKET,CSocketInfo>::refobjid_iter ite = mpSocketInfoMgr->GetRefObjMap()->begin();
				ite != mpSocketInfoMgr->GetRefObjMap()->end();ite++)
			{
				lpSocketInfo = 	ite->second;
				if(lpSocketInfo->moKey.mhSocket > lhMaxSocket)
					lhMaxSocket = lpSocketInfo->moKey.mhSocket;
				FD_SET(lpSocketInfo->moKey.mhSocket, &lstruFDSet);
				lpSocketInfoList[liSelectCount]=lpSocketInfo;
				liSelectCount++;
				if(liSelectCount >= FD_SETSIZE)
					break;
			}
			loCriticalSection.Leave();
		}
		if(liSelectCount<=0)
		{
			continue;
		}

#ifdef _WIN32
		liResult = select(0, &lstruFDSet, NULL, NULL, &lstru_tv);
#else
		liResult = select(lhMaxSocket + 1, &lstruFDSet, NULL, NULL, &lstru_tv);		
#endif
		if (SOCKET_ERROR == liResult)
		{
			int liError = GETSOCKET_ERRCODE();
			
			for(int i=0;i<liSelectCount;i++)
			{
				lpSocketInfo = lpSocketInfoList[i];
				if (!FD_ISSET(lpSocketInfo->moKey.mhSocket, &lstruFDSet))
					continue;
				CloseTCPSocketInRecv(lpSocketInfo);
			}
		}
		else if (0 == liResult)
		{// 超时,重新检查
			continue;
		}
		else 
		{
			// 检查当前所有的连接上是否有相应的网络事件
			for(int i=0;i<liSelectCount;i++)
			{
				lpSocketInfo = lpSocketInfoList[i];
				if (!FD_ISSET(lpSocketInfo->moKey.mhSocket, &lstruFDSet))
					continue;
				
				// 接收数据
				if(RecvTCPData(lpSocketInfo))
					continue;
				
				//接收数据失败
				CloseTCPSocketInRecv(lpSocketInfo);
			}
		}
	}
}

void CSelectMgr::SendThread(VOID *apParam)
{
	CSystem::InitRandom();
	CSelectMgr* lpThis =(CSelectMgr*) apParam;
	if(NULL ==lpThis )
		return ;

	lpThis->IncreaseRef();
	lpThis->SendThread();
	lpThis->DecreaseRef();
}
// $_FUNCTION_BEGIN ******************************
//
// 函数名称： SendThread
// 函数参数： 无．
// 返 回 值： DWORD
// 函数说明： 发送处理线程
//
// $_FUNCTION_END ********************************
void CSelectMgr::SendThread()
{
	CSendTCPPacket *lpSendTCPPacket = NULL;

	while (mbIsRun)
	{
		//从队列中取数据
		lpSendTCPPacket=moSendPackList.getheaddataandpop();
		if(NULL == lpSendTCPPacket)
		{
			CSystem::Sleep(5);
			continue;
		}

		SendTcpPacket(lpSendTCPPacket);

		if(lpSendTCPPacket->getNeedClose())
		{
			closesocket(lpSendTCPPacket->getPeerSocket());
		}
		moSendPackCacheList.free(lpSendTCPPacket);
	}

	return;
}




// $_FUNCTION_BEGIN ******************************
//
// 函数名称: RecvTCPData
// 函数参数: adwSock - socket 值
//          apbuf   - 缓冲区指针
//          aiLen   - 缓冲区长度
// 返 回 值: void        
// 函数说明: 删除一个客户端连接
//
// $_FUNCTION_END ********************************
bool CSelectMgr::RecvTCPData(CSocketInfo *apSocketInfo)
{//需要做累包和分包处理
	//
	STRU_SOCKET_RECV_CONTEXT &loRecvContext = *apSocketInfo->mpRecvContext;
	INT liRecvLen = recv(apSocketInfo->moKey.mhSocket,
		(char *)&loRecvContext.mpBuff[loRecvContext.miBuffLen], 
		sizeof(loRecvContext.mpBuff)-loRecvContext.miBuffLen, 
		0);

	if (SOCKET_ERROR == liRecvLen)
	{
		loRecvContext.miBuffLen =0;
		int liError = GETSOCKET_ERRCODE();
		printf("socket error=%d \n\r",liError);
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error recv is bad and buff clean"
			<<" Error="<<liError
			<<" Socket="<<apSocketInfo->moKey.mhSocket
			<<" RecvLen="<<liRecvLen
			<<" DataLen="<<loRecvContext.miBuffLen
			<<" SocketInfoPtr="<<apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
		);
		//mpMsgSink->OnError(DEF_ERR_NET_RECV_FAILED);
		return false;
	}
	else if (0 == liRecvLen)
	{
		int liError = GETSOCKET_ERRCODE();
		printf("socket close ,socket error=%d \n\r",liError);
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error socket closed "
			<<" Error="<<liError
			<<" Socket="<<apSocketInfo->moKey.mhSocket
			<<" RecvLen="<<liRecvLen
			<<" DataLen="<<loRecvContext.miBuffLen
			<<" SocketInfoPtr="<<apSocketInfo
			<<" RefCount="<<apSocketInfo->GetRefCount()
			);
		return false;
	}


	//重新计算数据长度
	liRecvLen += loRecvContext.miBuffLen;

	int liPackLen = 0;
	BYTE* lpBegin = loRecvContext.mpBuff;
	CRawTCPRecvPack *lpRawTCPPack = NULL;
	//处理数据包 进行分包处理
	while((liPackLen = CTCPPacket::GetPack(lpBegin, liRecvLen))>0)
	{
		lpRawTCPPack = mRawRecvPackCacheList.malloc();
		if(NULL == lpRawTCPPack)
		{
			InsLostRecv(0);
			LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error malloc RawTCPPack fail"
				<<" Socket="<<apSocketInfo->moKey.mhSocket
				<<" RecvLen="<<liRecvLen
				<<" DataLen="<<loRecvContext.miBuffLen
				<<" SocketInfoPtr="<<apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				);
			continue;
		}

		lpRawTCPPack->mhSocket = apSocketInfo->moKey.mhSocket;
		lpRawTCPPack->mulIPAddr =apSocketInfo->moKey.mPeerIP;
		lpRawTCPPack->mwIPPort = apSocketInfo->moKey.mwPeerPort;
		
		memcpy(lpRawTCPPack->mpData,lpBegin,liPackLen);
		lpRawTCPPack->miDataLen = liPackLen;

		lpRawTCPPack->mpSocketInfo = apSocketInfo;
		apSocketInfo->IncreaseRef();
		
		if(INVALID_U9_POSITION  == moRawRecvPackList.push_back(lpRawTCPPack))
		{
			InsLostRecv(0);
			CAutoReleaseRefObjMgrTemplate1<SOCKET,CSocketInfo> loSocketInfo(*mpSocketInfoMgr,apSocketInfo);//清除一次对象引用,如果需要删除，析构函数会负责
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push fail"
				<<" Socket="<<apSocketInfo->moKey.mhSocket
				<<" SocketInfoPtr="<<apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
				<<" RecvLen="<<liRecvLen
				<<" PackLen="<<liPackLen
			);
			mRawRecvPackCacheList.free(lpRawTCPPack);
			continue;
		}

		InsRecv(0);
		lpBegin += liPackLen;
		liRecvLen -= liPackLen;
	}

	if(liPackLen<0)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Pack size is bad and buff clean"
				<<" Socket="<<apSocketInfo->moKey.mhSocket
				<<" RecvLen="<<liRecvLen
				<<" DataLen="<<loRecvContext.miBuffLen
				<<" SocketInfoPtr="<<apSocketInfo
				<<" RefCount="<<apSocketInfo->GetRefCount()
			);

		loRecvContext.miBuffLen = 0;
		mpMsgSink->OnError(RET_ERR_NET_RECV_FAILED);
		return true;
	}
	
	if(0 ==liRecvLen)
	{//buff已经没有数据
		//更新缓冲区长度
		loRecvContext.miBuffLen =0;
	}
	else
	{//buff还剩余数据

		//更新缓冲区长度
		 loRecvContext.miBuffLen = liRecvLen;
		if(lpBegin!=loRecvContext.mpBuff)
		{//对齐缓冲区中的数据
			memmove(loRecvContext.mpBuff, lpBegin, liRecvLen);
		}
	}
	
	return true;
}
// $_FUNCTION_BEGIN ******************************
// 函数名称： Processs
// 函数参数：
// 返 回 值： 
// 函数说明： 处理函数，仅仅在客户端有效
// $_FUNCTION_END ********************************
void CSelectMgr::Processs()
{
	ReadAllReceivedData(0);
}


