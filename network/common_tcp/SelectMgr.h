// $_FILEHEADER_BEGIN ***************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：SelectMgr.h
// 创建人： 史云杰//
// 文件说明：Select TCP传输通讯实现类实现文件
// $_FILEHEADER_END *****************************

#if !defined (__SELECT_MGR_H__)
#define __SELECT_MGR_H__

#include "common/Event.h"
#include "common/Queue.h"
#include "BaseTCPMgr.h"


#include <list>
class CTCPPacket;
//////////////////////////////////////////////////////////////
// CTCPTransMgr
class CSelectMgr : public CBaseTCPMgr						 
{
private:
	////socket上下文
	//struct STRU_SOCKET_CONTEXT
	//{
	//	IPTYPE			miPeerAddr;
	//	WORD			mwPeerPort;
	//	SOCKET			mhSocket;
	//	INT				miBuffLen;
	//	BYTE			mpBuff[DEF_RECV_PACK_BUFF_LEN];
	//};
	//enum { SELECT_MAX_COUNT = 64,};
public:
	CSelectMgr();
	virtual ~CSelectMgr();
public:
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

	//处理函数
	virtual	void Processs();

private:
	// 删除一个客户端连接
	void        RemoveClientConn(DWORD adwSock);


	// 发送一个数据包
	INT         SendTcpPacket(CSendTCPPacket *apSendTCPPacket);

protected:
	// 网络事件处理线程
	static void RecvThread(VOID *apParam);
	void RecvThread();

	static void SendThread(VOID *apParam);
	VOID SendThread();

protected:
	int DirectConnect (IPTYPE aiIPAddr, WORD aiPort);
	CSocketInfo* CreateSocketSocket(SOCKET ahSocket,IPTYPE aiIPAddr,WORD awPort);
	virtual bool SetSocektNoBlock(SOCKET ahSocket);

	// 接收到一个数据包
	bool        RecvTCPData(CSocketInfo *apSocketInfo);


	void CloseTCPSocket(CSocketInfo *apSocketInfo);
	void CloseTCPSocketInRecv(CSocketInfo *apSocketInfo);

private:
	IPTYPE								miIPAddr;
	UINT								miPort;


	static INT							miInitialized;

private:

	CSafeCacheDoubleQueue<CSendTCPPacket>	moSendPackList;					//发包队列
	CCachePool<CSendTCPPacket>				moSendPackCacheList;			// 包缓冲队列
	typedef std::list<CSocketInfo *> socket_list;

};

#endif // __SELECT_MGR_H__