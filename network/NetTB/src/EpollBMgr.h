#ifndef __EPOLLBMGR_H
#define __EPOLLBMGR_H

#include "TCPPacket.h"
#include "SocketInfo.h"
#include "struct/BaseStruct.h"

class CSocketInfo;
/////////////////////////////////////////////////////////////////////////////
class CEpollBMgr: public ITransProtocolMgr
{
public:
	CEpollBMgr();
	virtual ~CEpollBMgr();
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
	virtual	void Processs(){return ;}

private:
    bool InitEpoll();
    bool StartListen(DWORD adwIPAddr, UINT aiPort);
    BOOL AssociateSocketwithEpoll(CSocketInfo *apSocketInfo);
    virtual bool SetSocektNoBlock(SOCKET ahSocket);
    bool SetSocektNODELAY(SOCKET ahSocket);
    void WaitClose();
	void CloseTCPSocket(CSocketInfo *apSocketInfo);
    void CloseTCPSocketInNetIOProcessThread(CSocketInfo *apSocketInfo, const int32 &aiThreadID);
	void CloseAllSocket();
private:
	LONG IncreaseRef();
	LONG DecreaseRef();
	inline DWORD GetDealThreadIndex(IPTYPE aiIP, WORD awPort)
	{
		return (DWORD)((aiIP +awPort)% mstruTcpOpt.mbyDealThreadCount);
	}

	inline DWORD GetDealThreadIndex(uint32 aiProcessID)
	{
		return (DWORD)(aiProcessID % mstruTcpOpt.mbyDealThreadCount);
	}

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
	//直接发送数据
	int DirectSendData(CSocketInfo *apSocketInfo,STRU_SOCKET_SEND_CONTEXT *apSendSocketContext);
	// 发送数据包
	int32 SendPacket(SOCKET ahScoket,BYTE* apBuffer, int32 aiLen);
	//等待队列中的数据
	BOOL SendWaitQueueData(CSocketInfo *apSocketInfo);
	void clearWaitQueue(CSocketInfo *apSocketInfo);
	BOOL RecvTCPData(CSocketInfo *apSocketInfo,BOOL & abNeedClose, const int32 &aiThreadID);

private:
	//网络IO处理线程
	static void NetIOProcessThread(VOID *apParam);
	void NetIOProcessThread(int32 aiIndex);
	//Accept线程
	static void AcceptThreadFunction(VOID *apParam);
	void AcceptTimeoutThread();
    //处理线程
    static void DealThread(VOID *apParam);
    VOID DealThread(DWORD adwIndex);
    void DealThreadEx(DWORD adwIndex);

public:
	//@begin
	CSimpleCachePool<STRU_SOCKET_SEND_CONTEXT>	 *mpSendCachePool;	//send context cache
	CRefObjMgrTemplate1<SOCKET,CSocketInfo>		 *mpSocketInfoMgr;	//上下文
	//@end 以上变量不要删除，因为是引用外面的变量
private:
    CSafeCacheDoubleQueue<STRU_EVENT_DATA>* mpEventQueue;			// 事件队列
    CCachePool <STRU_EVENT_DATA> moEventCacheQueue;					// 事件数据缓存。
    CCachePool<CTCPPacket>  moRecvPackCacheList;					// 包缓冲队列

    UINT				    miMaxEventCount;
    UINT					miMaxRecvPackCount;						//对应moRecvPackCacheList
    UINT					miMaxSendPackCount;						//对应mpSendCachePool
private:
    //epoll 的句柄
    int*                    mhEpollHandle;
	STRU_OPTION_TCP			mstruTcpOpt;
	int8                    miEncryptType;
	// 事件回调对象
	ITransMgrCallBack		*mpMsgSink;
	SOCKET					mhServerSocket;							// Server socket handle
	BOOL					mbIsRun;
	//空的回调
	CTransDealMgrNull		moNullCallBack;

	// 网络统计信息
	STRU_STATISTICS_TCP     *mpStatisList;
	BYTE					STATIS_LIST_SIZE;

	CCriticalSection	    moRefCriticalSection;					//线程计数器临界区
	LONG				    mlRefCount;								//线程计数器
};

//////////////////////////////////////////////////////////////////////////
//

#endif // __EPOLL_MGR_H__
