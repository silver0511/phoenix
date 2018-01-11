#include "WebSocketMgr.h"

// 标准构造
WebSocketMgr::WebSocketMgr()
{
    mlRefCount =0;
    mbIsRun = FALSE;

    miEncryptType=DEF_ENCRYPT_VERSION_V1;
    mlLocalTime=CSystem::GetTime();
    mpMsgSink = NULL;
    memset(&mstruTcpOpt, 0, sizeof(mstruTcpOpt));

    mpSocketTimeoutDqueue = NULL;

    mpEventQueue = NULL;

    miMaxEventCount = 0;

    miMaxRawRecvPackCount = 0;
    miMaxRecvPackCount = 0;
    miMaxSendPackCount = 0;
    mpStatisList = NULL;
}

// 标准析构
WebSocketMgr::~WebSocketMgr()
{
    LOG_SET_LEVEL(0);
    mpMsgSink = NULL;
    moTimeoutCacheQueue.clear();
    SAFE_DELETE_ARRAY(mpEventQueue);
    SAFE_DELETE_ARRAY(mpSocketTimeoutDqueue);
    moSendCachePool.clear();
    moSocketInfoMgr.clear();
    moSessionMap.clear();
    SAFE_DELETE_ARRAY(mpStatisList);
}

////////////////////////////////////////////////ITransProtocolMgr  start////////////////////////////////////////////////
INT WebSocketMgr::Open()
{
    mbIsRun=TRUE;

    if(mstruTcpOpt.mbTls)
    {
        // Initialize Asio Transport
        m_server_tls.init_asio();
        // Register handler callbacks
        m_server_tls.set_open_handler(bind(&WebSocketMgr::on_open,this,::_1));
        m_server_tls.set_tls_init_handler(bind(&WebSocketMgr::on_tls_init,this,::_1));
        m_server_tls.set_validate_handler(bind(&WebSocketMgr::on_validate, this, ::_1));
        m_server_tls.set_fail_handler(bind(&WebSocketMgr::on_fail, this, ::_1));
        m_server_tls.set_close_handler(bind(&WebSocketMgr::on_close,this,::_1));
        m_server_tls.set_message_handler(bind(&WebSocketMgr::on_message,this,::_1,::_2));
#ifndef _DEBUG
        m_server_tls.set_access_channels(websocketpp::log::alevel::none);
        m_server_tls.set_error_channels(websocketpp::log::elevel::none);
#endif
        m_server_tls.set_access_channels(websocketpp::log::alevel::endpoint);
        m_server_tls.set_access_channels(websocketpp::log::alevel::fail);


        m_server_tls.set_error_channels(websocketpp::log::elevel::fatal);
        m_server_tls.set_error_channels(websocketpp::log::elevel::rerror);
        m_server_tls.set_error_channels(websocketpp::log::elevel::warn);
    }
    else
    {
        // Initialize Asio Transport
        m_server.init_asio();
        // Register handler callbacks
        m_server.set_open_handler(bind(&WebSocketMgr::on_open,this,::_1));
        m_server.set_validate_handler(bind(&WebSocketMgr::on_validate, this, ::_1));
        m_server.set_fail_handler(bind(&WebSocketMgr::on_fail, this, ::_1));
        m_server.set_close_handler(bind(&WebSocketMgr::on_close,this,::_1));
        m_server.set_message_handler(bind(&WebSocketMgr::on_message,this,::_1,::_2));
#ifndef _DEBUG
        m_server.set_access_channels(websocketpp::log::alevel::none);
        m_server.set_error_channels(websocketpp::log::elevel::none);
#endif
        m_server.set_access_channels(websocketpp::log::alevel::endpoint);
        m_server.set_access_channels(websocketpp::log::alevel::fail);

        m_server.set_error_channels(websocketpp::log::elevel::fatal);
        m_server.set_error_channels(websocketpp::log::elevel::rerror);
        m_server.set_error_channels(websocketpp::log::elevel::warn);
    }

    //初始化监听端口
    CSystem::BeginThread(AcceptThreadFunction, (void *)this);

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

    return RET_SUCCESS;
}

INT WebSocketMgr::Close()
{
    mbIsRun=FALSE;
    STRU_EVENT_DATA	*lpEvent = NULL;
    CSafeCacheDoubleQueue<STRU_EVENT_DATA>* lpEventQueue= NULL;
    if(NULL != mpEventQueue )
    {
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
    moSendCachePool.clear();
    moSocketInfoMgr.clear();

    while (mlRefCount>0)
    {
        CSystem::Sleep(10);
    }

    CloseAllSocket();

    return RET_SUCCESS;
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
INT WebSocketMgr::Init(ITransMgrCallBack *apInterface,
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
    if(!apInterface)
    {
        return RET_ERROR;
    }

    mpMsgSink = apInterface;
    memcpy(&mstruTcpOpt, apstruOption, sizeof(struct STRU_OPTION_TCP));

    moSocketInfoMgr.Initialize(mstruTcpOpt.muiMaxConnCount);
    //发包队列chache初始化
    /*modify by shiyunjie 2017-01-12
      *发送除非内核缓冲区满，一般都是直接发送出去(websocket有自己的处理所以不需要很大)
      *发送逻辑是在处理线程中处理，缓冲区的大小取决于处理线程的个数速度
      *处理线程数 * 发送系数
    */
    miMaxSendPackCount = mstruTcpOpt.mbyDealThreadCount * THREAD_DEAL_MAX_PACKAGE_LEN;
    moSendCachePool.init(miMaxSendPackCount);
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
    LOG_TRACE(LOG_ALERT,1,"WebSocketMgr::Init","miMaxRawRecvPackCount: " << miMaxRawRecvPackCount
                                                                << " miMaxEventCount: " << miMaxEventCount
                                                                << " miMaxRecvPackCount: " << miMaxRecvPackCount
                                                                << " miMaxSendPackCount: " << miMaxSendPackCount
                                                                << " mbyDealThreadCount: " << mstruTcpOpt.mbyDealThreadCount
                                                                << " mpSendCachePool: " << moSendCachePool.remaincount()
                                                                << " moEventCacheQueue: " << moEventCacheQueue.remaincount()
                                                                << " moRecvPackCacheList: " << moRecvPackCacheList.remaincount()
                                                                << " mRawRecvPackCacheList: " << mRawRecvPackCacheList.remaincount());
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
bool WebSocketMgr::AddRelationPtr(void*apSocketInfo,void* apRelationPtr)
{
    if(!mstruTcpOpt.mbUseRelationPtr)
        return true;

    CSocketInfo *lpSocketInfo = (CSocketInfo *)apSocketInfo;
    lpSocketInfo->IncreaseRef();
    lpSocketInfo->mpRelationPtr = apRelationPtr;

    LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "add RelationPtr "
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
bool WebSocketMgr::DelRelationPtr(void*apSocketInfo,void* apRelationPtr)
{
    if(!mstruTcpOpt.mbUseRelationPtr)
        return true;

    CSocketInfo *lpSocketInfo = (CSocketInfo *)apSocketInfo;
    if(apRelationPtr != lpSocketInfo->mpRelationPtr )
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "Error RelationPtr is bad"
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
    CAutoReleaseRefObjMgrTemplate1<SESSIONID, CSocketInfo> loSocketInfo(moSocketInfoMgr, lpSocketInfo);

    LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "del RelationPtr "
            <<" IP="<<GetIPString(lpSocketInfo->moKey.mPeerIP)
            <<":"<<lpSocketInfo->moKey.mwPeerPort
            <<" RelationPtr="<<apRelationPtr
            <<" SocketInfoPtr="<<(void*)lpSocketInfo
            <<" SocketInfRefCount="<<lpSocketInfo->GetRefCount()
    );

    return true;
}

// 函数说明： 发送数据 是否需要发生线程?
INT WebSocketMgr::SendData(STRU_SENDDATA_BASE *apstruSendData)
{
    if (apstruSendData == NULL)
    {
        return RET_ERROR;
    }

    STRU_SENDDATA_TCP *lpTcpSndOpt = (struct STRU_SENDDATA_TCP *)apstruSendData;
    SOCKET lhSocket = lpTcpSndOpt->mhSocket;
    //得到一个SocketInfo
    CAutoReleaseRefObjMgrTemplate1<SESSIONID, CSocketInfo> loSocketInfo(moSocketInfoMgr, lhSocket);
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
INT WebSocketMgr::SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData)
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
                <<" PeerIP="<<GetIPString(loKey.mPeerIP)
                <<":"<< loKey.mwPeerPort
                <<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
                <<" LoadLen="<<lpTcpSndOpt->mwLen
        );
        return RET_ERROR;
    }
    SOCKET lhSocket = lpTcpSndOpt->mhSocket;
    // 分配一块空闲的内存
    STRU_SOCKET_SEND_CONTEXT *lpSendSocketContext = moSendCachePool.malloc();
//    if(moSendCachePool.remaincount() <= 1)
//    {
//        LOG_TRACE(LOG_NOTICE, 1, __FUNCTION__, "mpSendCachePool too large"
//                <<" size="<<moSendCachePool.size());
//    }
    if (NULL == lpSendSocketContext)
    {
        // 如果失败,则直接重新分配
        LOG_TRACE(LOG_ERR, 1, __FUNCTION__, "lpSendSocketContext allocate memory failed111"
                <<" PeerIP="<<GetIPString(loKey.mPeerIP)
                <<":"<< loKey.mwPeerPort
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
                <<" PeerIP="<<GetIPString(loKey.mPeerIP)
                <<":"<< loKey.mwPeerPort
                <<" EncryptType="<<miEncryptType
                <<" NeedClose="<<lpTcpSndOpt->mbNeedClose
                <<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
                <<" PackLen ="<<lpTcpSndOpt->mwLen
                <<" SendSocketContext="<<(void*)lpSendSocketContext
                <<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
                <<" LoadLen="<<lpTcpSndOpt->mwLen
        );

        moSendCachePool.free(lpSendSocketContext);

        DisConnectTo(lpTcpSndOpt->mhSocket);
        return RET_ERROR;
    }

    lpSendSocketContext->miOffset = 0;
    lpSendSocketContext->miPackType =*(WORD*)lpTcpSndOpt->mpData;
    lpSendSocketContext->mbSendClose = lpTcpSndOpt->mbNeedClose;

    CCriticalSection &	loCS = lpSocketInfo->moCS;
    loCS.Enter();

    //直接发送数据
    int liResult = DirectSendData(lpSocketInfo,lpSendSocketContext);
    if(0 > liResult)
    {
        //错误
        InsLostSend(loKey.mPeerIP, loKey.mwPeerPort);
        loCS.Leave();
        moSendCachePool.free(lpSendSocketContext);
        return RET_ERROR;
    }

    InsSend(loKey.mPeerIP, loKey.mwPeerPort);
    loCS.Leave();
    moSendCachePool.free(lpSendSocketContext);
    return RET_SUCCESS;
}

INT WebSocketMgr::DisConnectTo(SOCKET ahSocket)
{
    CAutoReleaseRefObjMgrTemplate1<SESSIONID , CSocketInfo> loSocketInfo(moSocketInfoMgr, ahSocket);
    if(loSocketInfo.Invalid())
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Socket not found");
        return RET_ERROR;
    }
    CSocketInfo *lpSocketInfo = loSocketInfo.GetPtr();
    CloseTCPSocket(lpSocketInfo);
}

INT WebSocketMgr::GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo)
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
////////////////////////////////////////////////ITransProtocolMgr  end//////////////////////////////////////////////////


///////////////////////////////////////////////////ws callback  start///////////////////////////////////////////////////
void WebSocketMgr::on_open(connection_hdl hdl)
{
    STRU_EVENT_DATA *lpEvent=NULL;
    DWORD ldwIndex = 0;

    CSocketInfo *lpSocketInfo = NULL;
    CSocketInfo *lpOldSocketInfo = NULL;

    lpSocketInfo = moSocketInfoMgr.MallocRefObj();
    if (NULL == lpSocketInfo)
    {
        LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error mpSocketInfoMgr->MallocRefObj()");
        if(mstruTcpOpt.mbTls)
        {
            m_server_tls.close(hdl, websocketpp::close::status::normal, "");
        }
        else
        {
            m_server.close(hdl, websocketpp::close::status::normal, "");
        }
        return;
    }

    lpSocketInfo->mpRecvContext->miBuffLen=0;
    STRU_SOCKET_KEY &loKey = lpSocketInfo->moKey;
    sockaddr_in lstruAddr;
    SESSIONID session_id = GenSessionID(hdl, lstruAddr);
    if(session_id <= 0)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "can not get remote ip and port");
        if(mstruTcpOpt.mbTls)
        {
            m_server_tls.close(hdl, websocketpp::close::status::normal, "");
        }
        else
        {
            m_server.close(hdl, websocketpp::close::status::normal, "");
        }
        return;
    }

    loKey.mhSocket = session_id;
    loKey.mHdl = hdl;
    loKey.mPeerIP = lstruAddr.sin_addr.s_addr;
    loKey.mwPeerPort = lstruAddr.sin_port;
    //加入对象
    lpOldSocketInfo = moSocketInfoMgr.AddRefObj(session_id, lpSocketInfo);
    if(lpOldSocketInfo != lpSocketInfo)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error mpSocketInfoMgr->AddRefObj exist obj"
                <<" SocketInfoPtr="<<(void*)lpSocketInfo
                <<" RefCount="<<lpSocketInfo->GetRefCount()
                <<" Old SocketInfoPtr="<<(void*)lpOldSocketInfo
                <<" Old RefCount="<<lpOldSocketInfo->GetRefCount()
        );
        U9_ASSERT(FALSE);
        if(mstruTcpOpt.mbTls)
        {
            m_server_tls.close(hdl, websocketpp::close::status::normal, "");
        }
        else
        {
            m_server.close(hdl, websocketpp::close::status::normal, "");
        }

        moSocketInfoMgr.FreeRefObj(lpSocketInfo);
        return;
    }


    lpSocketInfo->IncreaseRef();//增加一次引用

    //绑定session
    CSESSIONMAP_PAIR pair(hdl.lock(), session_id);
    moSessionMap.insert(pair);
    if (mpMsgSink)
    {
        ldwIndex = GetDealThreadIndex(lstruAddr.sin_addr.s_addr,lstruAddr.sin_port);

        //提交上层处理
        lpEvent = moEventCacheQueue.malloc();

        lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CONNECT_SUCCESS;
        lpEvent->mhSocket = loKey.mhSocket;
        lpEvent->miIP = lstruAddr.sin_addr.s_addr;
        lpEvent->mwPort =lstruAddr.sin_port;
        lpEvent->mpRelationPtr = (void*)lpSocketInfo;
        lpSocketInfo->IncreaseRef();//增加一次引用

        LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "notify new socket connect"
                <<" session= " << lpEvent->mhSocket
                <<" ref=" << lpSocketInfo->GetRefCount()
                <<" ip_i="<< lpEvent->miIP
                <<" ip="<<GetIPString(lpEvent->miIP)
                <<":"<< lpEvent->mwPort
        );

        mpEventQueue[ldwIndex].push_back(lpEvent);
    }

    if( mstruTcpOpt.muiKeepAlive != 0)
        AddTimeoutQueue(lpSocketInfo);

    InsCurCon(0);
}

void WebSocketMgr::on_close(connection_hdl hdl)
{
    CSESSIONMAP::iterator iter = moSessionMap.find(hdl.lock());
    if(iter == moSessionMap.end())
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "socket not found1");
        return;
    }

    //关闭了需要减少引用
    CAutoReleaseRefObjMgrTemplate1<SESSIONID, CSocketInfo> loSocketInfo(moSocketInfoMgr, iter->second);
    if(loSocketInfo.Invalid())
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "socket not found2");
        return;
    }

    //release
    CSocketInfo *lpSocketInfo = loSocketInfo.GetPtr();
    OnCloseNotify(loSocketInfo.GetPtr());
    CAutoReleaseRefObjMgrTemplate1<SESSIONID, CSocketInfo> loSocketInfo2(moSocketInfoMgr, lpSocketInfo);
    moSessionMap.erase(iter);
    moSocketInfoMgr.DeleteRefObj(iter->second);
    DescCurCon(0);
}

void WebSocketMgr::on_message(connection_hdl hdl, server::message_ptr msg)
{
    CSESSIONMAP::iterator iter = moSessionMap.find(hdl.lock());
    if(iter == moSessionMap.end())
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, " Socket not found1");
        return;
    }

    //得到一个SocketInfo
    CAutoReleaseRefObjMgrTemplate1<SESSIONID, CSocketInfo> loSocketInfo(moSocketInfoMgr, iter->second);
    if(loSocketInfo.Invalid())
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, " Socket not found2");
        return;
    }

    CSocketInfo *lpSocketInfo = loSocketInfo.GetPtr();
    STRU_SOCKET_KEY &loKey = lpSocketInfo->moKey;

    BOOL lbNeedClose = FALSE;
    //接收数据
    RecvTCPData(lpSocketInfo,lbNeedClose, msg);
    if(lbNeedClose)
    {   //处理close,接收错误
        LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "socket need close"
                <<" ip="<<GetIPString(loKey.mPeerIP)
                <<":"<< loKey.mwPeerPort
                <<" SocketInfoPtr="<<(void*)lpSocketInfo
                <<" RefCount="<<lpSocketInfo->GetRefCount()
        );
        CloseTCPSocket(lpSocketInfo);
        return;
    }
}

void WebSocketMgr::on_fail(connection_hdl hdl)
{
    //出错
    if(mstruTcpOpt.mbTls)
    {
        server_tls::connection_ptr p1 = m_server_tls.get_con_from_hdl(hdl);
        if(p1 == NULL)
        {
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "hdl not exsist");
            return;
        }
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "err=" << p1->get_ec().message().c_str());
    }
    else
    {
        server::connection_ptr p1 = m_server.get_con_from_hdl(hdl);
        if(p1 == NULL)
        {
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "hdl not exsist");
            return;
        }
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "err=" << p1->get_ec().message().c_str());
    }

    CSESSIONMAP::iterator iter = moSessionMap.find(hdl.lock());
    if(iter == moSessionMap.end())
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, " Socket not found1");
        return;
    }
    CAutoReleaseRefObjMgrTemplate1<SESSIONID, CSocketInfo> loSocketInfo(moSocketInfoMgr, iter->second);
    if(loSocketInfo.Invalid())
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, " Socket not found2");
        return;
    }
    CloseTCPSocket(loSocketInfo.GetPtr());
}

bool WebSocketMgr::on_validate(connection_hdl hdl)
{
    return true;
}


context_ptr WebSocketMgr::on_tls_init(websocketpp::connection_hdl hdl)
{
//    namespace asio = websocketpp::lib::asio;
    context_ptr ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1));

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
//        ctx->set_password_callback(bind(&get_password));
        ctx->use_certificate_chain_file("server.pem");
        ctx->use_private_key_file("server.pem", boost::asio::ssl::context::pem);
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return ctx;
}

std::string WebSocketMgr::get_password()
{
    return "test";
}
///////////////////////////////////////////////////ws callback  end/////////////////////////////////////////////////////



/////////////////////////////////////////////////process thread  start//////////////////////////////////////////////////
void WebSocketMgr::AcceptThreadFunction(VOID *apParam)
{
    WebSocketMgr* lpThis =(WebSocketMgr*) apParam;
    if(NULL ==lpThis )
        return ;
    lpThis->IncreaseRef();
    lpThis->AcceptTimeoutThread();
    lpThis->DecreaseRef();
}
void WebSocketMgr::AcceptTimeoutThread()
{
    if(mstruTcpOpt.mbTls)
    {
        //m_server.set_reuse_addr(true);
        m_server_tls.listen(mstruTcpOpt.mwPort);
        LOG_TRACE(LOG_ALERT, true, __FUNCTION__, "listen sucess port:" << mstruTcpOpt.mwPort);
        // Start the server accept loop
        m_server_tls.start_accept();
        // Start the ASIO io_service run loop
        while(true)
        {
            try
            {
                m_server_tls.run();
            }
            catch (const std::exception & e)
            {
                LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Create socket error:" <<  e.what());
            }

            CSystem::Sleep(100);
        }
    }
    else
    {
        //m_server.set_reuse_addr(true);
        m_server.listen(mstruTcpOpt.mwPort);
        LOG_TRACE(LOG_ALERT, true, __FUNCTION__, "listen sucess port:" << mstruTcpOpt.mwPort);
        // Start the server accept loop
        m_server.start_accept();
        // Start the ASIO io_service run loop
        while(true)
        {
            try
            {
                m_server.run();
            }
            catch (const std::exception & e)
            {
                LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Create socket error:" <<  e.what());
            }

            CSystem::Sleep(100);
        }
    }
}

void WebSocketMgr::DealRawPackThread(VOID *apParam)
{
    CSystem::InitRandom();
    WebSocketMgr* lpThis =(WebSocketMgr*) apParam;
    if(NULL ==lpThis )
        return ;

    lpThis->IncreaseRef();
    lpThis->DealRawPackThread();
    lpThis->DecreaseRef();
}
void WebSocketMgr::DealRawPackThread()
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

void WebSocketMgr::DealThread(DWORD adwIndex)
{
    try
    {
        DealThreadEx(adwIndex);
    }
    catch(std::exception& e)
    {
        cerr << "Catch a exception: " << e.what() << endl;
        LOG_TRACE(LOG_CRIT,1,__FUNCTION__,"Exception"
                <<" Index="<<adwIndex << "exception: " << e.what());
    }
}
void WebSocketMgr::DealThread(VOID *apParam)
{
    CSystem::InitRandom();
    STRU_TRHEAD_CONTEXT *lpThreadContext = (STRU_TRHEAD_CONTEXT *)apParam;
    WebSocketMgr* lpThis =(WebSocketMgr*) lpThreadContext->mpThis;
    if(NULL ==lpThis )
        return ;

    lpThis->IncreaseRef();
    lpThis->DealThread(lpThreadContext->dwIndex);
    lpThis->DecreaseRef();

    delete lpThreadContext;
}
void WebSocketMgr::DealThreadEx(DWORD adwIndex)
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
//                    giLocalTime = CSystem::GetTime();
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
                CAutoReleaseRefObjMgrTemplate1<SESSIONID, CSocketInfo> loSocketInfo(moSocketInfoMgr, lpSocketInfo);
                LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, " SocketInfoPtr="<<(void*)lpSocketInfo
                        <<" SocketInfRefCount="<<lpSocketInfo->GetRefCount()
                );
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
                LOG_TRACE(LOG_DEBUG_1, false, __FUNCTION__, "socket close "
                        <<" IP="<<GetIPString(lpEvent->miIP)
                        <<":"<< lpEvent->mwPort
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
                        <<" IP="<<GetIPString(lpEvent->miIP)
                        <<":"<< lpEvent->mwPort
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

void WebSocketMgr::CheckTimeoutThread(VOID *apParam)
{
    WebSocketMgr* lpThis =(WebSocketMgr*) apParam;
    if(NULL ==lpThis )
        return ;
    lpThis->IncreaseRef();
    lpThis->CheckTimeoutThread();
    lpThis->DecreaseRef();
}
void WebSocketMgr::CheckTimeoutThread()
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
        LOG_TRACE(LOG_INFO, true, __FUNCTION__, "check timeout start"
        );
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
                if(llNow <= lpTimeoutItem->mlLastTime)
                    break;
                lpTempPos = lpPos;
                lpPos=loTimeoutQueue.next(lpPos);

                lpSocketInfo = lpTimeoutItem->mpSocketInfo;

                LOG_TRACE(LOG_NOTICE,true, __FUNCTION__, "Socket Timeout "
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
///////////////////////////////////////////////////process thread  end////////////////////////////////////////////////////
//
//
//
//
//
//
////关闭所有的连接
void WebSocketMgr::CloseAllSocket()
{
    CStaticRefObjMgrTemplate1<SESSIONID, CSocketInfo>::refobjid_map *lpMap = moSocketInfoMgr.GetRefObjMap();
    for(CStaticRefObjMgrTemplate1<SESSIONID, CSocketInfo>::refobjid_map::iterator ite = lpMap->begin();ite != lpMap->end();ite++)
    {
        CSocketInfo *socket_info = ite->second;
        if(mstruTcpOpt.mbTls)
        {
            m_server_tls.close(socket_info->moKey.mHdl, websocketpp::close::status::normal, "");
        }
        else
        {
            m_server.close(socket_info->moKey.mHdl, websocketpp::close::status::normal, "");
        }
    }
    moSocketInfoMgr.clear();
    moSessionMap.clear();
}



int WebSocketMgr::DirectSendData(CSocketInfo *apSocketInfo,STRU_SOCKET_SEND_CONTEXT *apSendSocketContext)
{
    if(0 > apSendSocketContext->miBuffLen)
    {
        //错误
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error SendPacket fail"
                <<" Errcode="<<GETSOCKET_ERRCODE()
                <<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
                <<":"<< apSocketInfo->moKey.mwPeerPort
                <<" SocketInfoPtr="<<(void*)apSocketInfo
                <<" RefCount = "<< apSocketInfo->GetRefCount()
                <<" SendSocketContextPtr="<<(void*)apSendSocketContext
                <<" SerialID="<<apSendSocketContext->miSerialID
                <<" HasSentLen="<<apSendSocketContext->miOffset
        );
        return apSendSocketContext->miBuffLen;
    }

    //发送
    int32 liSentLen = apSendSocketContext->miBuffLen;

    try
    {
        if(mstruTcpOpt.mbTls)
        {
            server_tls::connection_ptr user_con = m_server_tls.get_con_from_hdl(apSocketInfo->moKey.mHdl);
            if(user_con == NULL)
            {
                LOG_TRACE(LOG_ERR, true, __FUNCTION__, "send user hdl invalid ip="
                        << GetIPString(apSocketInfo->moKey.mPeerIP)
                        <<":"<< apSocketInfo->moKey.mwPeerPort);
                return -1;
            }
            m_server_tls.send(apSocketInfo->moKey.mHdl, &apSendSocketContext->mpBuff[apSendSocketContext->miOffset],
                              liSentLen, websocketpp::frame::opcode::binary);
        }
        else
        {
            server::connection_ptr user_con = m_server.get_con_from_hdl(apSocketInfo->moKey.mHdl);
            if(user_con == NULL)
            {
                LOG_TRACE(LOG_ERR, true, __FUNCTION__, "send user hdl invalid ip="
                        << GetIPString(apSocketInfo->moKey.mPeerIP)
                        <<":"<< apSocketInfo->moKey.mwPeerPort);
                return -1;
            }
            m_server.send(apSocketInfo->moKey.mHdl, &apSendSocketContext->mpBuff[apSendSocketContext->miOffset],
                          liSentLen, websocketpp::frame::opcode::binary);
        }
    }
    catch(std::exception& e)
    {
        cerr << "Catch a send exception: " << e.what() << endl;
        LOG_TRACE(LOG_CRIT,1,__FUNCTION__,"Send Exception" << "exception: " << e.what());
        return -1;
    }

    //发送完毕
    LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "SendPacket complete "
            <<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
            <<":"<< apSocketInfo->moKey.mwPeerPort
            <<" SocketInfoPtr="<<(void*)apSocketInfo
            <<" RefCount = "<< apSocketInfo->GetRefCount()
            <<" SendSocketContextPtr="<<(void*)apSendSocketContext
            <<" SerialID="<<apSendSocketContext->miSerialID
            <<" HasSentLen="<< apSendSocketContext->miOffset
            <<" SentLen= " << liSentLen
    );
    apSendSocketContext->miBuffLen = 0;
    apSendSocketContext->miOffset = 0;
    if(TRUE == apSendSocketContext->mbSendClose)
    {
        LOG_TRACE(LOG_DEBUG_1,true, __FUNCTION__, "SendPacket complete and close"
                <<" ip="<<GetIPString(apSocketInfo->moKey.mPeerIP)
                <<":"<< apSocketInfo->moKey.mwPeerPort
                <<" SocketInfoPtr="<<(void*)apSocketInfo
                <<" RefCount = "<< apSocketInfo->GetRefCount()
                <<" SendSocketContextPtr="<<(void*)apSendSocketContext
                <<" SerialID="<<apSendSocketContext->miSerialID
                <<" HasSentLen="<<apSendSocketContext->miOffset
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

BOOL WebSocketMgr::RecvTCPData(CSocketInfo *apSocketInfo,BOOL & abNeedClose, server::message_ptr msg)
{
    //读取数据添加到接收缓冲区 需要做累包和分包处理
    STRU_SOCKET_RECV_CONTEXT& loRecvContext =   *apSocketInfo->mpRecvContext;
    STRU_SOCKET_KEY& loKey=apSocketInfo->moKey;
    BYTE* lpBegin = loRecvContext.mpBuff;
    INT liRecvLen = msg->get_payload().copy((char *)lpBegin + loRecvContext.miBuffLen, msg->get_payload().size());
    LOG_TRACE(LOG_DEBUG_1,TRUE,__FUNCTION__," WebSocketMgr::liRecvLen:" << liRecvLen << "msg_len:" << msg->get_payload().size());

    //重新计算数据长度
    liRecvLen += loRecvContext.miBuffLen;
    if(liRecvLen > DEF_RECV_PACK_BUFF_LEN)
    {//数据错误 重新开始记录
        abNeedClose = TRUE;

        //清除所有的缓冲区
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error recv count is bad"
                <<" IP="<<GetIPString(loKey.mPeerIP)
                <<":"<< loKey.mwPeerPort
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
        LOG_TRACE(LOG_DEBUG_1,TRUE,__FUNCTION__," WebSocketMgr::RecvTCPData:" << liPackLen);
        lpRawTCPPack = mRawRecvPackCacheList.malloc();
        if(NULL == lpRawTCPPack)
        {
            InsLostRecv(0);
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error new RawTCPPack fail"
                    <<" IP="<<GetIPString(loKey.mPeerIP)
                    <<":"<< loKey.mwPeerPort
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

        memcpy(lpRawTCPPack->mpData,lpBegin,liPackLen);
        lpRawTCPPack->miDataLen = liPackLen;

        if( INVALID_U9_POSITION == moRawRecvPackList.push_back(lpRawTCPPack))
        {
            InsLostRecv(0);
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back fail"
                    <<" IP="<<GetIPString(loKey.mPeerIP)
                    <<":"<< loKey.mwPeerPort
                    <<" SocketInfoPtr="<<(void*)apSocketInfo
                    <<" RefCount="<<apSocketInfo->GetRefCount()
                    <<" DataLen="<<loRecvContext.miBuffLen
                    <<" RecvLen="<<liRecvLen
            );
            mRawRecvPackCacheList.free(lpRawTCPPack);
            continue;
        }

        lpBegin += liPackLen;
        liRecvLen -= liPackLen;
    }

    if (abIsSuccess)
    {//为了分析最后收包时间分析
        AddTimeoutQueue(apSocketInfo);
    }

    if(liPackLen < 0)
    {
        abNeedClose = TRUE;
        //数据包错误
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error data excption"
                <<" IP="<<GetIPString(loKey.mPeerIP)
                <<":"<< loKey.mwPeerPort
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

/*
 * 对应处理线程(DealRawPackThread)
 * moRawRecvPackList->moRecvPackCacheList.malloc()->moEventCacheQueue.malloc()->mpEventQueue.push_back()
 * DealRawPackThread的处理线程只有一个，所以moRawRecvPackList不需要很大
*/
INT WebSocketMgr::RecvData(SOCKET ahScoket,IPTYPE aiIP, WORD awPort, BYTE* apData, WORD awLen,CSocketInfo *apSocketInfo)
{
    LOG_TRACE(LOG_DEBUG_1, TRUE, __FUNCTION__, "WebSocketMgr RecvData awLen: " << awLen);
    //printf("PacketArrive Pack %s:%d \n\r",GetIPString(aiIP).c_str(),ntohs(awPort));

    CTCPPacket *lpTCPPacket = moRecvPackCacheList.malloc();
//    if(moRecvPackCacheList.remaincount() <= 1)
//    {
//        LOG_TRACE(LOG_NOTICE, false, __FUNCTION__, "moRecvPackCacheList too large size=" << moRecvPackCacheList.size());
//    }

    if (NULL == lpTCPPacket)
    {
        InsLostRecv(STATIS_LIST_SIZE - 1);
        //printf("New lpTCPPacket fail %s:%d \n\r",GetIPString(aiIP).c_str(),ntohs(awPort));
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error allocate tcp packet failed."
                <<" IP="<<GetIPString(aiIP)
                <<":"<< awPort
                <<" Size="<<awLen
                <<"cachesize" << moRecvPackCacheList.size());
        return -1;
    }
    lpTCPPacket->init();
    if(0 > lpTCPPacket->UnPack(apData, awLen,miEncryptType))
    {
        //解包失败
        InsLostRecv(STATIS_LIST_SIZE - 1);
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Unpack failed."
                <<" IP="<<GetIPString(aiIP)
                <<":"<< awPort
                <<" Size="<<awLen
                <<" EncryptType="<<miEncryptType
        );
        moRecvPackCacheList.free(lpTCPPacket);

        //关闭连接
        DisConnectTo(ahScoket);
        return -1;
    }

    InsRecv(STATIS_LIST_SIZE - 1);
    DWORD ldwIndex = GetDealThreadIndex(aiIP,awPort);

    //LOG_TRACE(LOG_INFO, true, __FUNCTION__, " Recv Pack"
    //	<<" Socket="<<ahScoket
    //	<<" IP="<<GetIPString(aiIP)
    //	<<":"<<ntohs(awPort)
    //	<<" TCPLen="<<awLen
    //	<<" PackLen="<<lpTCPPacket->getDataLen()
    //	<<" PackType="<<*(WORD*)lpTCPPacket->getDataPtr()
    //	<<" EncryptType="<<miEncryptType
    //	<<" Index="<<ldwIndex
    //	);

//    if(moEventCacheQueue.remaincount() <= 1)
//    {
//        LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, " moEventCacheQueue too large size=" << moEventCacheQueue.size());
//    }

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
                <<" IP="<<GetIPString(aiIP)
                <<":"<< awPort
                <<" Size="<<awLen
                <<" EncryptType="<<miEncryptType
                <<" Index="<<ldwIndex
        );
        moEventCacheQueue.free(lpEvent);
    }
    return 1;
}

void WebSocketMgr::OnCloseNotify(CSocketInfo *apSocketInfo)
{// 处理关闭
    //将socket和epoll解除关联。
    BOOL lbNeedClose = FALSE;
    STRU_SOCKET_KEY	&loKey = apSocketInfo->moKey;
    DelTimeoutQueue(apSocketInfo);

    DWORD ldwIndex = GetDealThreadIndex(loKey.mPeerIP,loKey.mwPeerPort);
    STRU_EVENT_DATA *lpEvent = moEventCacheQueue.malloc();

    ZeroMemory(lpEvent,sizeof(lpEvent));
    lpEvent->mTypeEvent = STRU_EVENT_DATA::ENUM_CLOSE;
    lpEvent->mhSocket = loKey.mhSocket;
    lpEvent->miIP = loKey.mPeerIP;
    lpEvent->mwPort = loKey.mwPeerPort;
    lpEvent->miParam = GETSOCKET_ERRCODE();
    lpEvent->mpRelationPtr = (void*)apSocketInfo;

    mpEventQueue[ldwIndex].push_back(lpEvent);
}
void WebSocketMgr::CloseTCPSocket(CSocketInfo *apSocketInfo)
{
    STRU_SOCKET_KEY& loKey = apSocketInfo->moKey;
    websocketpp::session::state::value con_state = websocketpp::session::state::value::closed;
    if(mstruTcpOpt.mbTls)
    {
        server_tls::connection_ptr p1 = m_server_tls.get_con_from_hdl(loKey.mHdl);
        if(p1 == NULL)
        {
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "hdl not exsist");
            return;
        }
        con_state = p1->get_state();
    }
    else
    {
        server::connection_ptr p1 = m_server.get_con_from_hdl(loKey.mHdl);
        if(p1 == NULL)
        {
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "hdl not exsist");
            return;
        }
        con_state = p1->get_state();
    }


    LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "need shutdown socket"
            <<" IP="<<GetIPString(loKey.mPeerIP)
            <<":"<< loKey.mwPeerPort
            <<" SocketInfo="<<(void*)apSocketInfo
            <<" RefCount="<<apSocketInfo->GetRefCount()
            <<"hdl ref count:" << loKey.mHdl.use_count()
            <<"con_state: " << con_state
    );

    if(con_state == websocketpp::session::state::value::open)
    {
        if(mstruTcpOpt.mbTls)
        {
            m_server_tls.close(loKey.mHdl, websocketpp::close::status::normal, "");
        }
        else
        {
            m_server.close(loKey.mHdl, websocketpp::close::status::normal, "");
        }
    }
    else
    {
        LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "socket has been closed"
                <<" IP="<<GetIPString(loKey.mPeerIP)
                <<":"<< loKey.mwPeerPort
                <<" SocketInfo="<<(void*)apSocketInfo
                <<" RefCount="<<apSocketInfo->GetRefCount()
                <<"hdl ref count:" << loKey.mHdl.use_count()
                <<"con_state: " << con_state
        );
    }
}

U9_POSITION WebSocketMgr::AddTimeoutQueue(CSocketInfo *apSocketInfo)
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

    //LOG_TRACE(7, true, __FUNCTION__, " "
    //	<<" Socket="<<loKey.mhSocket
    //	<<" IP="<<GetIPString(loKey.mPeerIP)
    //	<<":"<<ntohs(loKey.mwPeerPort)
    //	<<" SocketInfo="<<(void*)apSocketInfo
    //	<<" RefCount="<<apSocketInfo->GetRefCount()
    //	<<" Old TimeoutPos="<<(void*)lpTempPos
    //	<<" TimeoutPos="<<(void*)lpTimeoutPos
    //	);

    return lpTimeoutPos;
}
U9_POSITION WebSocketMgr::DelTimeoutQueue(CSocketInfo *apSocketInfo)
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

LONG WebSocketMgr::IncreaseRef()
{
    LONG llValue=0;
    moRefCriticalSection.Enter();
    mlRefCount++;
    llValue=mlRefCount;
    moRefCriticalSection.Leave();
    return llValue;
}
//引用计数减1
LONG WebSocketMgr::DecreaseRef()
{
    LONG llValue=0;
    moRefCriticalSection.Enter();
    if(mlRefCount>0)
        mlRefCount--;
    llValue=mlRefCount;
    moRefCriticalSection.Leave();
    return llValue;
}



SESSIONID WebSocketMgr::GenSessionID(const connection_hdl &hdl, sockaddr_in &stru_addr)
{
    std::string remote_endpoint;
    if(mstruTcpOpt.mbTls)
    {
        server_tls::connection_ptr p1 = m_server_tls.get_con_from_hdl(hdl);
        if(p1 == NULL)
        {
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "hdl not exsist");
            return 0;
        }
        remote_endpoint = p1->get_remote_endpoint();
    }
    else
    {
        server::connection_ptr p1 = m_server.get_con_from_hdl(hdl);
        if(p1 == NULL)
        {
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "hdl not exsist");
            return 0;
        }
        remote_endpoint = p1->get_remote_endpoint();
    }

    //兼容老版本转为IP4
    uint64 ip_start = remote_endpoint.find_first_of(':', 3);
    uint64 ip_end = remote_endpoint.find_first_of(']');
    uint64 port_start = remote_endpoint.find_last_of(':');
    if(ip_start == std::string::npos ||
    ip_end == std::string::npos ||
    port_start == std::string::npos ||
    ip_start >= ip_end ||
    port_start >= remote_endpoint.length())
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error parsing end point"
                <<" remote_endpoint="<< remote_endpoint.c_str()
        );
        return 0;
    }

    std::string ip = remote_endpoint.substr(ip_start+1, ip_end-ip_start-1);
    std::string port = remote_endpoint.substr(port_start+1, remote_endpoint.length()-port_start);
    inet_pton(AF_INET,(char *)ip.c_str(), (sockaddr *)(&stru_addr.sin_addr));
    stru_addr.sin_port = atoi(port.c_str());
    SESSIONID session_id = stru_addr.sin_port;
    session_id = (session_id << 32);
    session_id += stru_addr.sin_addr.s_addr;
    return session_id;
}