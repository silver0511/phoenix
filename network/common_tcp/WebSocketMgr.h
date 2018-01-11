#ifndef __WEBSOCKETMGR_H__
#define __WEBSOCKETMGR_H__

#include "stdafx.h"
#include "common/Event.h"
#include "common/Queue.h"
#include "TCPPacket.h"
#include "SocketInfo.h"
//#include "websocketpp/config/asio_no_tls.hpp"
#include <websocketpp/config/asio.hpp>
#include "websocketpp/server.hpp"
#include "BaseTCPMgr.h"

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::server<websocketpp::config::asio_tls> server_tls;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

class CSocketInfo;
class WebSocketMgr: public ITransProtocolMgr
{
public:
    WebSocketMgr();
    ~WebSocketMgr();
public:
    //ITransProtocolMgr interface
    virtual INT Open();
    virtual INT Close();
    virtual VOID Release(){delete this;}
    virtual INT Init(ITransMgrCallBack *apInterface, STRU_OPTION_BASE *astruOption);
    virtual void InitEncrypt(int8 aiEncryptType) { miEncryptType = aiEncryptType; }
    virtual void SetBindIP(IPTYPE aiIP) { mstruTcpOpt.mulIPAddr = aiIP; }
    virtual void SetBindPort(WORD awPort) { mstruTcpOpt.mwPort= awPort; }
    virtual IPTYPE GetBindIP() { return mstruTcpOpt.mulIPAddr; }
    virtual WORD GetBindPort() { return mstruTcpOpt.mwPort; }
    virtual bool AddRelationPtr(void*apSocketInfo,void* apRelationPtr);
    virtual bool DelRelationPtr(void*apSocketInfo,void* apRelationPtr);
    virtual INT SendData(STRU_SENDDATA_BASE *apstruSendData);
    virtual INT SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData);
    virtual INT SendRawData(STRU_SENDDATA_BASE *apstruSendData) {return 1;}
    virtual INT ConnectTo(const SESSIONID & aiPeerID, IPTYPE aiIPAddr, WORD awPort) { return 1; }
    virtual INT DisConnectTo(SOCKET ahSocket);
    virtual INT GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo);
    virtual	void Processs(){return;}
public:
    //ws callback
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);
    void on_fail(connection_hdl hdl);
    bool on_validate(connection_hdl hdl);
    context_ptr on_tls_init(websocketpp::connection_hdl hdl);
    std::string get_password();
private:
    BOOL RecvTCPData(CSocketInfo *apSocketInfo,BOOL & abNeedClose, server::message_ptr msg);
    //整个的网络数据包到达
    INT RecvData(SOCKET ahSocket,IPTYPE aiIP, WORD awPort,BYTE* apData,WORD awLen,CSocketInfo* apSocketInfo);

    //直接发送数据
    int DirectSendData(CSocketInfo *apSocketInfo,STRU_SOCKET_SEND_CONTEXT *apSendSocketContext);

    void OnCloseNotify(CSocketInfo *apSocketInfo);
    void CloseTCPSocket(CSocketInfo *apSocketInfo);
    void CloseAllSocket();

    U9_POSITION AddTimeoutQueue(CSocketInfo *apSocketInfo);
    U9_POSITION DelTimeoutQueue(CSocketInfo *apSocketInfo);

    LONG IncreaseRef();
    LONG DecreaseRef();

    inline DWORD GetDealThreadIndex(IPTYPE aiIP, WORD awPort)
    {
        return (DWORD)((aiIP +awPort)% mstruTcpOpt.mbyDealThreadCount);
    }

    SESSIONID GenSessionID(const connection_hdl &hdl, sockaddr_in &stru_addr);

private:
    //统计相关
    inline DWORD GetStactisIndex(DWORD index)
    {
        return (DWORD)(index % STATIS_LIST_SIZE);
    }
    inline DWORD GetStactisIndex(IPTYPE aiIP, WORD awPort)
    {
        return (DWORD)((aiIP +awPort)% (STATIS_LIST_SIZE - 1));
    }
    inline void InsLostRecv(DWORD index)
    {
        index = GetStactisIndex(index);
        mpStatisList[index].t_lost_recv++;
    }

    inline void InsLostSend(DWORD index1, DWORD index2)
    {
        DWORD index = GetStactisIndex(index1, index2);
        mpStatisList[index].t_lost_send++;
    }

    inline void InsSend(DWORD index1, DWORD index2)
    {
        DWORD index = GetStactisIndex(index1, index2);
        mpStatisList[index].t_send_count++;
        mpStatisList[index].mulTotalSendPacketCount++;
    }
    inline void InsRecv(DWORD index)
    {
        index = GetStactisIndex(index);
        mpStatisList[index].t_recv_count++;
        mpStatisList[index].mulTotalRecvPacketCount++;
    }
    inline void InsCurCon(DWORD index)
    {
        index = GetStactisIndex(index);
        mpStatisList[index].cur_con++;
    }
    inline void DescCurCon(DWORD index)
    {
        index = GetStactisIndex(index);
        mpStatisList[index].cur_con--;
    }
private:
    //Accept线程
    static void AcceptThreadFunction(VOID *apParam);
    void AcceptTimeoutThread();

    //处理原始包线程
    static void DealRawPackThread(VOID *apParam);
    VOID DealRawPackThread();
    //处理线程
    static void DealThread(VOID *apParam);
    VOID DealThread(DWORD adwIndex);
    void DealThreadEx(DWORD adwIndex);

    //超时检测线程
    static void CheckTimeoutThread(VOID *apParam);
    void CheckTimeoutThread();
public:
    // 事件回调对象
    ITransMgrCallBack		*mpMsgSink;                                         //ws服务器
    STRU_OPTION_TCP			mstruTcpOpt;
protected:
    server                  m_server;
    server_tls              m_server_tls;
    BOOL					mbIsRun;

    // tcpTarnsMgr 选项
    UINT				    miMaxEventCount;
    UINT					miMaxRawRecvPackCount;								//对应mRawRecvPackCacheList
    UINT					miMaxRecvPackCount;									//对应moRecvPackCacheList
    UINT					miMaxSendPackCount;									//对应mpSendCachePool

    time_t					mlLocalTime;                                        //当前时间

    // 网络统计信息
    STRU_STATISTICS_TCP     *mpStatisList;
    BYTE					STATIS_LIST_SIZE;

    CCriticalSection	    moRefCriticalSection;                               //计数器临界区
    LONG				    mlRefCount;                                         //计数器
    int8                    miEncryptType;

public:
    struct STRU_TIMEOUT_ITEM
    {
        CSocketInfo *mpSocketInfo;
        LONG mlLastTime;
    };
public:
    CSimpleCachePool<STRU_SOCKET_SEND_CONTEXT>	                moSendCachePool;
    CRefObjMgrTemplate1<SESSIONID, CSocketInfo>                 moSocketInfoMgr;

    typedef u9_hash_map<std::shared_ptr<void>, SESSIONID>       CSESSIONMAP;
    typedef std::pair<std::shared_ptr<void>, SESSIONID>         CSESSIONMAP_PAIR;
    CSESSIONMAP                                                 moSessionMap;

    typedef CContextDQueue<STRU_TIMEOUT_ITEM> CSocketTimeoutDqueue;
    CSocketTimeoutDqueue* mpSocketTimeoutDqueue;
    CCachePool <STRU_TIMEOUT_ITEM> moTimeoutCacheQueue;

    CSafeCacheDoubleQueue<CRawTCPRecvPack> moRawRecvPackList;	                //原始包接收包队列
    CCachePool<CRawTCPRecvPack> mRawRecvPackCacheList;			                //原始包接收包缓冲队列

    CSafeCacheDoubleQueue<STRU_EVENT_DATA>* mpEventQueue;	                    //事件队列
    CCachePool <STRU_EVENT_DATA> moEventCacheQueue;			                    //事件数据缓存。

    CCachePool<CTCPPacket>  moRecvPackCacheList;			                    //包缓冲队列
};


#endif //__WEBSOCKETMGR_H__
