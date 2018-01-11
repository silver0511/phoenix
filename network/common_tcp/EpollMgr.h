// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：EpollMgr.h
// 创建人： 史云杰
// 文件说明：Epoll tcp 传输通讯实现类实现文件
// $_FILEHEADER_END *****************************

#ifndef __EPOLLMGR_H
#define __EPOLLMGR_H

#include "BaseTCPMgr.h"
#define FIRST_MAX_TIME_OUT 5

/////////////////////////////////////////////////////////////////////////////
// CIOCPMgr
class CEpollMgr		:public CBaseTCPMgr 
{
private:
	//socket上下文

public:
	CEpollMgr();
	virtual ~CEpollMgr();
public:
	//BaseInterface
	// 函数说明： 打开TransmitMgr操作，实际是初始化整个接收
	//           网络处理模块
	virtual INT Open();

	// 函数说明： 关闭操作
	virtual INT Close();

	// 函数说明： 释放对象
	virtual VOID Release(){delete this;}

	// 函数说明： 发送数据
	virtual INT SendData(STRU_SENDDATA_BASE *apstruSendData);
		// 函数说明： 发送数据
	virtual INT SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData);
	// 函数说明： 连接一个指定的地址
	virtual INT ConnectTo(const SESSIONID & aiPeerID, IPTYPE aiIPAddr, WORD awPort);
	
	// 函数说明： 关闭一个连接
	virtual INT DisConnectTo(SOCKET ahSocket);

	void CloseAllSocket();

private:
	// 初始化完成端口
	bool InitEpoll();

	// 开始在指定的端口监听
	bool StartListen(DWORD adwIPAddr, UINT aiPort);

	//将socket描述符同Epoll关联
	BOOL AssociateSocketwithEpoll(CSocketInfo *apSocketInfo);

	BOOL RecvTCPData(CSocketInfo *apSocketInfo,BOOL & abNeedClose, const int32 &aiThreadID);

	bool CheckConnectResult( CSocketInfo *apSocketInfo, int32 events );

	//直接发送数据
	int DirectSendData(CSocketInfo *apSocketInfo,STRU_SOCKET_SEND_CONTEXT *apSendSocketContext);
	//发送等待队列中的数据
	BOOL SendWaitQueueData(CSocketInfo *apSocketInfo);
	// 发送数据包
	int32 SendPacket(SOCKET ahScoket,BYTE* apBuffer, int32 aiLen);
	// 直接发送一个数据包(不经过发送队列)
	int SendPacketDirectly(STRU_SENDDATA_TCP *apSendOpt);

	void clearWaitQueue(CSocketInfo *apSocketInfo);
	void CloseTCPSocketInNetIOProcessThread(CSocketInfo *apSocketInfo, const int32 &aiThreadID);
	void CloseTCPSocket(CSocketInfo *apSocketInfo);
	
	U9_POSITION AddTimeoutQueue(CSocketInfo *apSocketInfo);
	U9_POSITION DelTimeoutQueue(CSocketInfo *apSocketInfo);

    void AddWithOutCheckIP();
protected:
	int DirectConnect (IPTYPE aiIPAddr, WORD aiPort);

	virtual bool SetSocektNoBlock(SOCKET ahSocket);
	bool SetSocektNODELAY(SOCKET ahSocket);

	virtual CSocketInfo* CreateSocketSocket(SOCKET ahSocket,IPTYPE aiIPAddr,WORD awPort);
	
private:
	//网络IO处理线程
	static void NetIOProcessThread(VOID *apParam);
	void NetIOProcessThread(int32 aiIndex);

	//Accept线程
	static void AcceptThreadFunction(VOID *apParam);
	void AcceptTimeoutThread();

	//超时检测线程
	static void CheckTimeoutThread(VOID *apParam);
	void CheckTimeoutThread();
private:
	//epoll 的句柄
	//int mhEpollHandle;
	
	int* mhEpollHandle;

	//typedef CContextDQueue<CSocketInfo> CSocketInfoDQueue;
	//CSocketInfoDQueue* mpSendDQueue;//发送的队列
	std::map<IPTYPE, BOOL> without_check_map;
};

//////////////////////////////////////////////////////////////////////////
//

#endif // __EPOLL_MGR_H__
