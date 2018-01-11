// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//			 
// 文件名称: BaseTCPMgr.h
// 创 建 人: 史云杰
// 文件说明: TCP 模型基本类
// $_FILEHEADER_END *****************************

#ifndef __BASETCPMGR_H
#define __BASETCPMGR_H


#include "TCPPacket.h"
#include "SocketInfo.h"
#include "struct/BaseStruct.h"

class CSocketInfo;
class CBaseTCPMgr: public ITransProtocolMgr
{
public:
	CBaseTCPMgr();
	virtual ~CBaseTCPMgr();


public:
	//BaseInterface
	// 函数说明： 打开TransmitMgr操作，实际是初始化整个接收
	//           网络处理模块
	virtual INT Open() {return 1;}

	// 函数说明： 关闭操作
	virtual INT Close();

	// 函数说明： 释放对象
	virtual VOID Release();

	//ITransProtocolMgr
	// 函数说明： 初始化接口及设置成员
	virtual INT Init(ITransMgrCallBack *apInterface,
		STRU_OPTION_BASE *apstruOption);

	virtual void InitEncrypt(int8 aiEncryptType);

	// 函数说明： 设置本机绑定IP
	virtual void SetBindIP(IPTYPE aiIP);

	// 函数说明： 设置本机绑定端口
	virtual void SetBindPort(WORD awPort);

	//添加上层关联指针
	virtual bool AddRelationPtr(void*apSocketInfo,void* apRelationPtr);

	//删除上层关联指针
	virtual bool DelRelationPtr(void*apSocketInfo,void* apRelationPtr);

	// 函数说明： 得到本机绑定端口
	virtual IPTYPE GetBindIP();

	// 函数说明： 得到本机绑定端口
	virtual WORD GetBindPort();

	// 函数说明： 发送数据
	virtual INT SendData(STRU_SENDDATA_BASE *apstruSendData) = 0 ;

	// 函数说明： 连接一个指定的地址
	virtual INT ConnectTo(const SESSIONID & aiPeerID, IPTYPE aiIPAddr, WORD awPort) =0 ;

	// 函数说明： 关闭一个连接
	virtual INT DisConnectTo(SOCKET ahSocket) =0 ;

	// 函数说明： 得到本地网络统计信息
	virtual INT GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo);

	// 函数说明： 设置与某个连接相关联的用户数据(服务端有效)
	virtual	INT SetUserData(SOCKET ahSocket, DWORD adwData)  { return 1; }

	virtual INT SendRawData(STRU_SENDDATA_BASE *apstruSendData) {return 1;}

	virtual	void Processs(){return ;}


protected:
	virtual bool SetSocektNoBlock(SOCKET ahSocket) = 0;

	//处理连接
	BOOL SocketConnecting(STRU_CONNECT_EVENT *apEvent);//开始socket连接

	void ProcessSocketConnectFail(const SESSIONID &aiSessionID,IPTYPE aiDestIP,WORD awDestPort);		//socket连接失败
	void ProcesssSocketConnectSucces(const SESSIONID &aiSessionID,SOCKET ahSocket,IPTYPE aiIPAddr,WORD awPort);//socket连接成功

	virtual CSocketInfo* CreateSocketSocket(SOCKET ahSocket,IPTYPE aiIPAddr,WORD awPort) =0;


public:
	//整个的网络数据包到达
	INT RecvData(SOCKET ahSocket,IPTYPE aiIP, WORD awPort,BYTE* apData,WORD awLen,CSocketInfo* apSocketInfo);
public:
	LONG IncreaseRef();
	LONG DecreaseRef();
	inline BOOL	GetIsRun(){return mbIsRun;}
protected:
	//处理原始包线程
	static void DealRawPackThread(VOID *apParam);
	VOID DealRawPackThread();

	//处理线程
	static void DealThread(VOID *apParam);
	VOID DealThread(DWORD adwIndex);

	// 连接线程
	static void ConnectThread(VOID *apParam);
	VOID ConnectThread(int32 aiIndex);

	void ReadAllReceivedData(DWORD adwIndex);

	inline DWORD GetDealThreadIndex(IPTYPE aiIP, WORD awPort)
	{
		return (DWORD)((aiIP +awPort)% mstruTcpOpt.mbyDealThreadCount);
	}
	inline DWORD GetConnnectThreadIndex(IPTYPE aiIP, WORD awPort)
	{
		return (DWORD)((aiIP +awPort)% mstruTcpOpt.mbyConnectThreadCount);
	}
	void WaitClose();
	void DealThreadEx(DWORD adwIndex);

protected:
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
public:
	// 事件回调对象
	ITransMgrCallBack		*mpMsgSink;
	STRU_OPTION_TCP			mstruTcpOpt;

protected:
	SOCKET					mhServerSocket;	// Server socket handle

	BOOL					mbIsRun;
	//空的回调
	CTransDealMgrNull		moNullCallBack;

	// tcpTarnsMgr 选项
	UINT				    miMaxEventCount;
	UINT					miMaxRawRecvPackCount;								//对应mRawRecvPackCacheList
	UINT					miMaxRecvPackCount;									//对应moRecvPackCacheList
	UINT					miMaxSendPackCount;									//对应mpSendCachePool


	time_t					  mlLocalTime;										//当前时间

	// 网络统计信息
	STRU_STATISTICS_TCP     *mpStatisList;
	BYTE					STATIS_LIST_SIZE;

	CCriticalSection	moRefCriticalSection;//计数器临界区
	LONG				mlRefCount;//计数器
	int8 miEncryptType;


public:
	struct STRU_TIMEOUT_ITEM
	{
		CSocketInfo *mpSocketInfo;
		LONG mlLastTime;
	};
public:
	//@begin
	CSimpleCachePool<STRU_SOCKET_SEND_CONTEXT>	 *mpSendCachePool;//send context cache 
	CRefObjMgrTemplate1<SOCKET,CSocketInfo>		 *mpSocketInfoMgr;//上下文
	//@end 以上变量不要删除，因为是引用外面的变量
	
	typedef CContextDQueue<STRU_TIMEOUT_ITEM> CSocketTimeoutDqueue;
	CSocketTimeoutDqueue* mpSocketTimeoutDqueue;
	CCachePool <STRU_TIMEOUT_ITEM> moTimeoutCacheQueue;

	CSafeCacheDoubleQueue<CRawTCPRecvPack> moRawRecvPackList;	// 原始包接收包队列
	CCachePool<CRawTCPRecvPack> mRawRecvPackCacheList;			// 原始包接收包缓冲队列

	CSafeCacheDoubleQueue<STRU_EVENT_DATA>* mpEventQueue;	// 事件队列
	CCachePool <STRU_EVENT_DATA> moEventCacheQueue;			// 事件数据缓存。

	CCachePool<CTCPPacket>  moRecvPackCacheList;			// 包缓冲队列

	CSafeCacheDoubleQueue<STRU_CONNECT_EVENT>* mpConnectEventQueue;	// 连接事件队列
	CCachePool <STRU_CONNECT_EVENT> moConnectEventCacheQueue;		// 连接事件数据缓存。
};



#endif //__BASETCPMGR_H__