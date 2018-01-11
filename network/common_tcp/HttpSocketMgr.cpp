#include "HttpSocketMgr.h"
#include <websocketpp/http/constants.hpp>

// 标准构造
HttpSocketMgr::HttpSocketMgr()
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

    miMaxSendPackCount = 0;
    mpStatisList = NULL;
}

// 标准析构
HttpSocketMgr::~HttpSocketMgr()
{
    LOG_SET_LEVEL(0);
    mpMsgSink = NULL;
    moTimeoutCacheQueue.clear();
    SAFE_DELETE_ARRAY(mpEventQueue);
    SAFE_DELETE_ARRAY(mpSocketTimeoutDqueue);
    moSocketInfoMgr.clear();
    moSessionMap.clear();
    SAFE_DELETE_ARRAY(mpStatisList);
}

////////////////////////////////////////////////ITransProtocolMgr  start////////////////////////////////////////////////
INT HttpSocketMgr::Open()
{
    mbIsRun=TRUE;

    if(mstruTcpOpt.mbTls)
    {
        // Initialize Asio Transport
        m_server_tls.init_asio();
        // Register handler callbacks
        m_server_tls.set_http_handler(bind(&HttpSocketMgr::on_http,this,::_1));
        m_server_tls.set_tls_init_handler(bind(&HttpSocketMgr::on_tls_init,this,::_1));
        m_server_tls.set_validate_handler(bind(&HttpSocketMgr::on_validate, this, ::_1));
        m_server_tls.set_fail_handler(bind(&HttpSocketMgr::on_fail, this, ::_1));
        m_server_tls.set_close_handler(bind(&HttpSocketMgr::on_close,this,::_1));
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
        m_server.set_reuse_addr(true);
        m_server.set_http_handler(bind(&HttpSocketMgr::on_http,this,::_1));
        m_server.set_validate_handler(bind(&HttpSocketMgr::on_validate, this, ::_1));
        m_server.set_fail_handler(bind(&HttpSocketMgr::on_fail, this, ::_1));
        m_server.set_close_handler(bind(&HttpSocketMgr::on_close,this,::_1));
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

INT HttpSocketMgr::Close()
{
    mbIsRun=FALSE;
    STRU_EVENT_HTTP_DATA	*lpEvent = NULL;
    CSafeCacheDoubleQueue<STRU_EVENT_HTTP_DATA>* lpEventQueue= NULL;
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
INT HttpSocketMgr::Init(ITransMgrCallBack *apInterface,
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
    
    //事件chache队列
    miMaxEventCount = ((mstruTcpOpt.mbyEpollCount * CON_MAX_PACKAGE_LEN) / ((mstruTcpOpt.mbyDealThreadCount/ 2) + 1)) + 1;
    /*modify by shiyunjie 2017-01-12
      *这个cachelist是DealRawPackThread->DealThread的中转缓存列表
      *DealRawPackThread处理速度恒定，而DealThread处理速度会根据业务逻辑而定
      *因此需要足够多的缓存，事件数和收包数几乎是一一对应的，所以大小可以一样
    */
    moEventCacheQueue.init(miMaxEventCount);
    mpEventQueue = new CSafeCacheDoubleQueue<STRU_EVENT_HTTP_DATA>[mstruTcpOpt.mbyDealThreadCount];


    moTimeoutCacheQueue.init(mstruTcpOpt.muiMaxConnCount);
    CSocketTimeoutDqueue::initCache(mstruTcpOpt.muiMaxConnCount);
    mpSocketTimeoutDqueue = new CSocketTimeoutDqueue[mstruTcpOpt.mbyDealThreadCount];

    STATIS_LIST_SIZE = mstruTcpOpt.mbyEpollCount + 1;
    mpStatisList = new STRU_STATISTICS_TCP[STATIS_LIST_SIZE];
    time_t aiStatisTime = CSystem::GetTime();
    for(int index_s = 0; index_s < STATIS_LIST_SIZE; ++index_s)
    {
        mpStatisList[index_s].s_start_time = aiStatisTime;
        mpStatisList[index_s].s_end_time = aiStatisTime;
    }
    LOG_TRACE(LOG_ALERT,1,"HttpSocketMgr::Init", " miMaxEventCount: " << miMaxEventCount
                                                                << " miMaxSendPackCount: " << miMaxSendPackCount
                                                                << " mbyDealThreadCount: " << mstruTcpOpt.mbyDealThreadCount
                                                                << " moEventCacheQueue: " << moEventCacheQueue.remaincount());
    return RET_SUCCESS;
}

INT HttpSocketMgr::DisConnectTo(SOCKET ahSocket)
{
    LOG_TRACE(LOG_ERR, false, __FUNCTION__, " is not can call this");
    return 1;
}

INT HttpSocketMgr::GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo)
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

INT HttpSocketMgr::SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData)
{
    if (apSocketInfo == NULL || apstruSendData == NULL)
    {
        LOG_TRACE(LOG_ERR, 0, __FUNCTION__, "Error len is bad"
                <<" apSocketInfo="<<(void*)apSocketInfo
                <<" apstruSendData="<<(void*)apstruSendData
        );

        return RET_ERROR;
    }

    STRU_SENDDATA_TCP *lpTcpSndOpt = (struct STRU_SENDDATA_TCP *)apstruSendData;
    
    //得到一个SocketInfo
    CSocketInfo *lpSocketInfo = (CSocketInfo *)apSocketInfo;
    STRU_SOCKET_KEY& loKey=lpSocketInfo->moKey;

    if(lpTcpSndOpt->mwLen >= DEF_PACKET_LEN)
    {
        LOG_TRACE(LOG_ERR, 0, __FUNCTION__, "Error len is bad"
                <<" PeerIP="<<GetIPString(loKey.mPeerIP)
                <<":"<< loKey.mwPeerPort
                <<" PackType="<<*(WORD*)lpTcpSndOpt->mpData
                <<" LoadLen="<<lpTcpSndOpt->mwLen
        );

        return RET_ERROR;
    }

    CCriticalSection &	loCS = lpSocketInfo->moCS;
    loCS.Enter();

    //直接发送数据
    bool bResult = SendBodyToClient(lpSocketInfo->moKey.mHdl, lpTcpSndOpt->mpData, lpTcpSndOpt->mwLen);
    if(!bResult)
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "send user hdl invalid ip="
                << GetIPString(lpSocketInfo->moKey.mPeerIP)
                <<":"<< lpSocketInfo->moKey.mwPeerPort);
    }

    InsSend(loKey.mPeerIP, loKey.mwPeerPort);
    //发送完毕
    LOG_TRACE(LOG_DEBUG,true, __FUNCTION__, "SendPacket complete "
            <<" ip="<<GetIPString(lpSocketInfo->moKey.mPeerIP)
            <<":"<< lpSocketInfo->moKey.mwPeerPort
            <<" SocketInfoPtr="<<(void*)lpSocketInfo
            <<" RefCount = "<< lpSocketInfo->GetRefCount()
            <<" PeerIP="<<GetIPString(loKey.mPeerIP)
            <<":"<< loKey.mwPeerPort
    );

    // 需要关闭客户端

    loCS.Leave();
    return RET_SUCCESS;
}

// 函数说明： 发送数据 是否需要发生线程?
INT HttpSocketMgr::SendData(STRU_SENDDATA_BASE *apstruSendData)
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
////////////////////////////////////////////////ITransProtocolMgr  end//////////////////////////////////////////////////


///////////////////////////////////////////////////ws callback  start///////////////////////////////////////////////////
void HttpSocketMgr::on_http(websocketpp::connection_hdl hdl)
{
    STRU_EVENT_HTTP_DATA *lpEvent=NULL;
    DWORD ldwIndex = 0;

    CSocketInfo *lpSocketInfo = NULL;
    CSocketInfo *lpOldSocketInfo = NULL;

    lpSocketInfo = moSocketInfoMgr.MallocRefObj();
    if (NULL == lpSocketInfo)
    {
        LOG_TRACE(LOG_CRIT, false, __FUNCTION__, "Error mpSocketInfoMgr->MallocRefObj()");
        sendFinishToClient(hdl, "", websocketpp::http::status_code::forbidden);
        return;
    }

    lpSocketInfo->mpRecvContext->miBuffLen=0;
    STRU_SOCKET_KEY &loKey = lpSocketInfo->moKey;
    sockaddr_in lstruAddr;
    SESSIONID session_id = GenSessionID(hdl, lstruAddr);
    if(session_id <= 0)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "can not get remote ip and port");
        sendFinishToClient(hdl, "", websocketpp::http::status_code::forbidden);
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
        sendFinishToClient(hdl, "", websocketpp::http::status_code::forbidden);
        moSocketInfoMgr.FreeRefObj(lpSocketInfo);
        return;
    }

    // lpSocketInfo->IncreaseRef();//增加一次引用

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "lpOldSocketInfo->GetRefCount() = " << lpSocketInfo->GetRefCount());

    //绑定session
    CSESSIONMAP_PAIR pair(hdl.lock(), session_id);
    moSessionMap.insert(pair);

    InsRecv(0);
    if (mpMsgSink)
    {
        ldwIndex = GetDealThreadIndex(lstruAddr.sin_addr.s_addr,lstruAddr.sin_port);

//        if(moEventCacheQueue.remaincount() <= 1)
//        {
//            LOG_TRACE(LOG_NOTICE, true, __FUNCTION__, " moEventCacheQueue too large size=" << moEventCacheQueue.size());
//        }

        STRU_EVENT_HTTP_DATA *lpEvent=moEventCacheQueue.malloc(); 
        lpEvent->mTypeEvent = STRU_EVENT_HTTP_DATA::ENUM_RECV;
        lpEvent->mhSocket = loKey.mhSocket;
        lpEvent->miIP = lstruAddr.sin_addr.s_addr;
        lpEvent->mwPort =lstruAddr.sin_port;
        lpEvent->mStrBody = GetHttpBodyContent(hdl);
        lpEvent->mpRelationPtr = (void*)lpSocketInfo;

        int64 li64Session = 0;
        li64Session = lpEvent->mwPort;
        li64Session = (li64Session << 32) + lpEvent->miIP;
        if(mstruTcpOpt.mbUseRelationPtr)
            mpMsgSink->OnSessionConnectedEx(li64Session,lpEvent->mpRelationPtr,
                                                    lpEvent->mhSocket,lpEvent->miIP,lpEvent->mwPort);
        else
            mpMsgSink->OnSessionConnected(li64Session,
                                                  lpEvent->mhSocket,lpEvent->miIP,lpEvent->mwPort);

        if(INVALID_U9_POSITION == mpEventQueue[ldwIndex].push_back(lpEvent))
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error push_back failed."
                    <<" IP="<<GetIPString(lpEvent->miIP)
                    <<":"<< lpEvent->mwPort
                    <<" EncryptType="<<miEncryptType
                    <<" Index="<<ldwIndex
            );

            moEventCacheQueue.free(lpEvent);

            sendFinishToClient(hdl, "", websocketpp::http::status_code::forbidden);
            moSocketInfoMgr.FreeRefObj(lpSocketInfo);
        }

        DeferResponseHttp(hdl);

        if( mstruTcpOpt.muiKeepAlive != 0)
            AddTimeoutQueue(lpSocketInfo);
    }

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "lpOldSocketInfo->GetRefCount() = " << lpSocketInfo->GetRefCount());

}

void HttpSocketMgr::DeferResponseHttp(websocketpp::connection_hdl hdl)
{
    server::connection_ptr con;
    if(mstruTcpOpt.mbTls)
    {
        server_tls::connection_ptr con = m_server_tls.get_con_from_hdl(hdl);
        con->defer_http_response();
    }
    else
    {
        server::connection_ptr con = m_server.get_con_from_hdl(hdl);
        con->defer_http_response();
    }
}

string HttpSocketMgr::GetHttpBodyContent(websocketpp::connection_hdl hdl)
{
    server::connection_ptr con;
    if(mstruTcpOpt.mbTls)
    {
        server_tls::connection_ptr con = m_server_tls.get_con_from_hdl(hdl);
        return con->get_request_body();
    }
    else
    {
        server::connection_ptr con = m_server.get_con_from_hdl(hdl);
        return con->get_request_body();
    }
}

bool HttpSocketMgr::SendBodyToClient(websocketpp::connection_hdl hdl, BYTE* mpData, WORD mwLen)
{
    std::string smg;
	smg.assign((char*)mpData, mwLen); 

    websocketpp::lib::error_code ec;
    if(mstruTcpOpt.mbTls)
    {
        server_tls::connection_ptr con = m_server_tls.get_con_from_hdl(hdl);
        if(con == NULL)
        {
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "SendBodyToClient failed");
            return false;
        }

        con->set_body(smg);
        con->set_status(websocketpp::http::status_code::ok);
        con->send_http_response(ec);

        CloseTCPByHDL(hdl, "", websocketpp::http::status_code::ok);
    }
    else
    {
        server::connection_ptr con = m_server.get_con_from_hdl(hdl);
        if(con == NULL)
        {
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "SendBodyToClient failed");
            return false;
        }

        con->set_body(smg);
        con->set_status(websocketpp::http::status_code::ok);
        con->send_http_response(ec);

        CloseTCPByHDL(hdl, "", websocketpp::http::status_code::ok);
    }

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "message = " << ec.message());
    return true;
}

void HttpSocketMgr::sendFinishToClient(websocketpp::connection_hdl hdl, string str_describle, websocketpp::http::status_code::value code)
{
    websocketpp::lib::error_code ec;
    if(mstruTcpOpt.mbTls)
    {
        server_tls::connection_ptr con = m_server_tls.get_con_from_hdl(hdl);

        if(code != websocketpp::http::status_code::ok)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "err=" << con->get_ec().message().c_str()
                    << " str_describle = " << str_describle);
        }
        else
        {
            con->set_body(str_describle);
        }

        con->set_status(code); 
        con->send_http_response(ec);
    }
    else
    {
        server::connection_ptr con = m_server.get_con_from_hdl(hdl);
        if(code != websocketpp::http::status_code::ok)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "err=" << con->get_ec().message().c_str()
                    << " str_describle = " << str_describle);
        }
        else
        {
            con->set_body(str_describle);
        }

        con->set_body(str_describle);
        con->set_status(code);
        
        con->send_http_response(ec);
    }
}

void HttpSocketMgr::CloseTCPByHDL(websocketpp::connection_hdl hdl, 
                                    string str_describle, 
                                    websocketpp::http::status_code::value code)
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
    DelTimeoutQueue(lpSocketInfo);

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " SocketInfRefCount = " << lpSocketInfo->GetRefCount());

    if(mstruTcpOpt.mbUseRelationPtr && lpSocketInfo)
        mpMsgSink->OnSessionCloseEx(0,lpSocketInfo->mpRelationPtr,
                                0, 0, 0);
    else
        mpMsgSink->OnSessionClose(0, 0, 0, 0);

    sendFinishToClient(hdl, "", code);

    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " SocketInfRefCount = " << lpSocketInfo->GetRefCount());
    moSessionMap.erase(iter);
    moSocketInfoMgr.DeleteRefObj(iter->second);
}

void HttpSocketMgr::on_close(connection_hdl hdl)
{
    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "closed");
    CloseTCPByHDL(hdl, "", websocketpp::http::status_code::ok);
}

void HttpSocketMgr::on_fail(connection_hdl hdl)
{
    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "closed");
    CloseTCPByHDL(hdl, "", websocketpp::http::status_code::forbidden);
}

bool HttpSocketMgr::on_validate(connection_hdl hdl)
{
    return true;
}


context_ptr HttpSocketMgr::on_tls_init(websocketpp::connection_hdl hdl)
{
    namespace asio = websocketpp::lib::asio;

    std::cout << "on_tls_init called with hdl: " << hdl.lock().get() << std::endl;
    
    context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

    try 
    {
        // if (mode == MOZILLA_MODERN)
        // {
        //     // Modern disables TLSv1
        //     ctx->set_options(asio::ssl::context::default_workarounds |
        //                      asio::ssl::context::no_sslv2 |
        //                      asio::ssl::context::no_sslv3 |
        //                      asio::ssl::context::no_tlsv1 |
        //                      asio::ssl::context::single_dh_use);
        // } 
        // else 
        // {
            
        // }

        ctx->set_options(asio::ssl::context::default_workarounds |
                             asio::ssl::context::no_sslv2 |
                             asio::ssl::context::no_sslv3 |
                             asio::ssl::context::single_dh_use);
        
        // ctx->set_password_callback(bind(&get_password));
        ctx->use_certificate_chain_file("server.pem");
        ctx->use_private_key_file("server.pem", asio::ssl::context::pem);
        
        std::string ciphers;
        
        // if (mode == MOZILLA_MODERN) 
        // {
        //     ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";
        // } 
        // else 
        // {
        //     ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
        // }
        
        // if (SSL_CTX_set_cipher_list(ctx->native_handle() , ciphers.c_str()) != 1) 
        // {
        //     std::cout << "Error setting cipher list" << std::endl;
        // }
    } 
    catch (std::exception& e) 
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return ctx;
}

std::string HttpSocketMgr::get_password()
{
    return "test";
}
///////////////////////////////////////////////////ws callback  end/////////////////////////////////////////////////////



/////////////////////////////////////////////////process thread  start//////////////////////////////////////////////////
void HttpSocketMgr::AcceptThreadFunction(VOID *apParam)
{
    HttpSocketMgr* lpThis =(HttpSocketMgr*) apParam;
    if(NULL ==lpThis )
        return ;
    lpThis->IncreaseRef();
    lpThis->AcceptTimeoutThread();
    lpThis->DecreaseRef();
}
void HttpSocketMgr::AcceptTimeoutThread()
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

void HttpSocketMgr::DealThread(DWORD adwIndex)
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
void HttpSocketMgr::DealThread(VOID *apParam)
{
    CSystem::InitRandom();
    STRU_TRHEAD_CONTEXT *lpThreadContext = (STRU_TRHEAD_CONTEXT *)apParam;
    HttpSocketMgr* lpThis =(HttpSocketMgr*) lpThreadContext->mpThis;
    if(NULL ==lpThis )
        return ;

    lpThis->IncreaseRef();
    lpThis->DealThread(lpThreadContext->dwIndex);
    lpThis->DecreaseRef();

    delete lpThreadContext;
}
void HttpSocketMgr::DealThreadEx(DWORD adwIndex)
{
    DWORD       ldwIndex = adwIndex;
    U9_ASSERT(adwIndex >= 0 && adwIndex <= mstruTcpOpt.mbyDealThreadCount);

    STRU_EVENT_HTTP_DATA	*lpEvent = NULL;
    int64 li64Session =0;

    int32 liCheckCount = 0;
    int64 li64NowTime = 0;
    int64 li64LastTime =0;
    int64 liSecondCount = 0;
    CSafeCacheDoubleQueue<STRU_EVENT_HTTP_DATA>* lpEventQueue= &mpEventQueue[ldwIndex];
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
                    giLocalTime = CSystem::GetTime();
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
            case STRU_EVENT_HTTP_DATA::ENUM_RECV://接收数据
            {
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
                                             (BYTE*)lpEvent->mStrBody.c_str(),
                                             lpEvent->mStrBody.size());
                }

                else
                {
                    mpMsgSink->OnRecvData(lpEvent->mhSocket,
                                          lpEvent->miIP,
                                          lpEvent->mwPort,
                                          (BYTE*)lpEvent->mStrBody.c_str(),
                                          lpEvent->mStrBody.size());
                }

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

void HttpSocketMgr::CheckTimeoutThread(VOID *apParam)
{
    HttpSocketMgr* lpThis =(HttpSocketMgr*) apParam;
    if(NULL ==lpThis )
        return ;
    lpThis->IncreaseRef();
    lpThis->CheckTimeoutThread();
    lpThis->DecreaseRef();
}
void HttpSocketMgr::CheckTimeoutThread()
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
                LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "closed");
                CloseTCPByHDL(lpSocketInfo->moKey.mHdl, "", websocketpp::http::status_code::request_timeout);
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
void HttpSocketMgr::CloseAllSocket()
{
    CStaticRefObjMgrTemplate1<SESSIONID, CSocketInfo>::refobjid_map *lpMap = moSocketInfoMgr.GetRefObjMap();
    for(CStaticRefObjMgrTemplate1<SESSIONID, CSocketInfo>::refobjid_map::iterator ite = lpMap->begin();ite != lpMap->end();ite++)
    {
        CSocketInfo *socket_info = ite->second;

        sendFinishToClient(socket_info->moKey.mHdl, "", websocketpp::http::status_code::forbidden);
    }

    moSocketInfoMgr.clear();
    moSessionMap.clear();
}

U9_POSITION HttpSocketMgr::AddTimeoutQueue(CSocketInfo *apSocketInfo)
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
U9_POSITION HttpSocketMgr::DelTimeoutQueue(CSocketInfo *apSocketInfo)
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

LONG HttpSocketMgr::IncreaseRef()
{
    LONG llValue=0;
    moRefCriticalSection.Enter();
    mlRefCount++;
    llValue=mlRefCount;
    moRefCriticalSection.Leave();
    return llValue;
}
//引用计数减1
LONG HttpSocketMgr::DecreaseRef()
{
    LONG llValue=0;
    moRefCriticalSection.Enter();
    if(mlRefCount>0)
        mlRefCount--;
    llValue=mlRefCount;
    moRefCriticalSection.Leave();
    return llValue;
}

SESSIONID HttpSocketMgr::GenSessionID(const connection_hdl &hdl, sockaddr_in &stru_addr)
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

// $_FUNCTION_BEGIN ******************************
//
// 函数名称： AddRelationPtr
// 函数参数：
// 返 回 值： 添加设置成功
// 函数说明： 设置关联指针
//
// $_FUNCTION_END ********************************
bool HttpSocketMgr::AddRelationPtr(void*apSocketInfo,void* apRelationPtr)
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
bool HttpSocketMgr::DelRelationPtr(void*apSocketInfo,void* apRelationPtr)
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