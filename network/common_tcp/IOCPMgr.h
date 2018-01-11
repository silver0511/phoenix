// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//			 
// 文件名称: IOCPMgr.h
// 创 建 人: 史云杰
// 文件说明: windows 完成端口 tcp 传输通讯Server端实现类实现文件 修改事件处理的按事件处理
// $_FILEHEADER_END *****************************


#ifndef __IOCPMGR_H
#define __IOCPMGR_H

#include "BaseTCPMgr.h"

/////////////////////////////////////////////////////////////////////////////
// CIOCPMgr
class CIOCPMgr:public CBaseTCPMgr 
{
public:
	CIOCPMgr();
	virtual ~CIOCPMgr();
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

	virtual INT SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData);

	// 函数说明： 连接一个指定的地址
	virtual INT ConnectTo(const SESSIONID& aiPeerID, IPTYPE aiIPAddr, WORD awPort);
	
	// 函数说明： 关闭一个连接
	virtual INT DisConnectTo(SOCKET ahSocket);
	
	
private:
	//
	enum { DEFAULT_ACCEPEX_NUM = 0x40, MONITER_TIME = 3000};

	// 初始化完成端口
	bool InitIOCP();

	// 开始在指定的端口监听
	bool StartListen(DWORD adwIPAddr, UINT aiPort);

	// 投递一个发送请求
	int  PostSendPacket(STRU_SOCKET_SEND_CONTEXT*apContext);
	// 投递一个接收请求
	int PostRecvPacket(STRU_SOCKET_RECV_CONTEXT *apContext);

	//bool	PostRecvPacket(SOCKET aiSock);

	// 释放资源
	void Destroy();

	// 初始化Winsock库
	bool InitWinSock();

	// 将完成端口同Sock句柄关联
	BOOL AssociateSockwithIOCP(SOCKET ahSocket);
	BOOL AssociateSockwithIOCP(CSocketInfo *apSocketInfo);
	BOOL AssociateSocketwithIOCP_Connect(CSocketInfo *apSocketInfo);

	SOCKET DirectConnect (IPTYPE aiIPAddr, WORD aiPort);
	// 注册网络事件
	bool RegisterNetEvent();

	// 投递指定个数的AcceptEx等待客户端的连接
	bool PostNumOfAcceptEx();
	

	bool RecvTCPData(CSocketInfo *apSocketInfo,DWORD adwRecvDataLen);
		
	void CloseTCPSocket(CSocketInfo *apSocketInfo);
	void CloseTCPSocketInNetIOProcessThread(STRU_SOCKET_RECV_CONTEXT* apSocketContext);
	void clearWaitQueue(CSocketInfo *apSocketInfo);

	U9_POSITION AddTimeoutQueue(CSocketInfo *apSocketInfo);
	U9_POSITION DelTimeoutQueue(CSocketInfo *apSocketInfo);

	U9_POSITION AddCloseTimeoutQueue(CSocketInfo *apSocketInfo);
	U9_POSITION DelCloseTimeoutQueue(CSocketInfo *apSocketInfo);
protected:
	//网络IO处理线程
	static void NetIOProcessThread(VOID *apParam);
	VOID NetIOProcessThread(int32 aiThreadIndex);

	// 监视线程
	static void MoniterThread(VOID *apParam);
	VOID MoniterThread();
		
	//超时检测线程
	static void CheckTimeoutThread(VOID *apParam);
	void CheckTimeoutThread();

	//超时检测关闭线程
	static void CheckCloseTimeoutThread(VOID *apParam);
	void CheckCloseTimeoutThread();
protected:
	virtual bool SetSocektNoBlock(SOCKET ahSocket);
	virtual CSocketInfo* CreateSocketSocket(SOCKET ahSocket,IPTYPE aiIPAddr,WORD awPort);
private:
	INT							miInitialized;// 初始化计数

	LPFN_TRANSMITFILE			mpfnTransmitFile;// 函数地址 - TransmitFile
	LPFN_ACCEPTEX				mpfnAcceptEx;// 函数地址 - AcceptEx
	
	LPFN_GETACCEPTEXSOCKADDRS	mpfnAcceptExSockAddrs;			// 函数地址 - GetAcceptExSockAddrs


	HANDLE      mhCompletionPort;	// IO Completion Port Handle
	HANDLE      mhNoAcceptEvent;	// AcceptEvent

	STRU_SOCKET_KEY moCompKey;

};

#endif // __IOCP_MGR_H__