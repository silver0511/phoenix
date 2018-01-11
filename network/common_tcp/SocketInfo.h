// $_FILEHEADER_BEGIN ***************************
// 文件名称: SocketInfo.h
// 创 建 人: 史云杰
// 文件说明: socketinfo
// $_FILEHEADER_END *****************************

#ifndef __SOCKETINFO_H
#define __SOCKETINFO_H
#include "base0/platform.h"
#include "common/RefObjMgrTemplate.h"
#include "ContextDQueue.h"
#include "TCPPacket.h"
#include "common/BaseThread.h"
#include "common/Queue.h"
#include "common/Queue2.h"
#include "common/CacheList.h"
#include "common/Sptr.h"
#include "const/ErrorCodeDef.h"
#include "network/ITransProtocolMgr.h"
#include "network/IProxyMgr.h"
#include "ContextDQueue.h"
#ifdef WEBSOCKETPP
#include "websocketpp/common/connection_hdl.hpp"
#endif

using namespace std;
using namespace U9;

class CSocketInfo;
struct STRU_SOCKET_KEY
{
	SOCKET     mhSocket;	//对端的唯一标识
#ifdef WEBSOCKETPP
    websocketpp::connection_hdl mHdl;		//websocket对应hdl
#endif
	WORD       mwPeerPort;	//对端的端口值
	IPTYPE     mPeerIP;		//对端的网络地址
};

#ifdef WIN32
#define DEF_THREAD_CONNECT_SELECT_MAX_COUNT FD_SETSIZE
#else
#define DEF_THREAD_CONNECT_SELECT_MAX_COUNT (64)
#endif

#ifdef WIN32
#include <winsock2.h>
#include <mswsock.h>
#include <hash_map>
#include <map>

#define DEF_SND_NOTIFY          (WM_USER + 100)
#define DEF_SOCKADDR_LEN        (sizeof(struct sockaddr_in) + 16)

#if !defined (SIO_KEEPALIVE_VALS)
#define SIO_KEEPALIVE_VALS  _WSAIOW(IOC_VENDOR, 4)
#endif /* SIO_KEEPALIVE_VALS */



#if !defined (SIO_KEEPALIVE_VALS)
#define SIO_KEEPALIVE_VALS  _WSAIOW(IOC_VENDOR, 4)
#endif /* SIO_KEEPALIVE_VALS */


//
// 网络I/O状态定义
//
enum ENUM_IO_STATE_T
{
	// 一个读操作完成, 
	// 此时可以发出下一个读操作请求
	IOS_READ = 0,

	// 一个写操作完成,
	// 此时可以发出下一个写操作请求
	IOS_WRITE,

	// 一个新的连接被Server接受
	IOS_ACCEPT,

	// 对方已经将连接关闭
	IOS_CLOSED,

	// 错误
	IOS_ERROR
};



//
// Overlapped plus 结构定义
//
// 连接存活选项
struct STRU_TCP_KEEPALIVE
{
	u_long  on_off;               // 存活开关是否打开
	u_long  keepalive_time;       // 第一次检测的时间
	u_long  keepalive_interval;   // 每次检测间隔
};

//socket上下文

struct STRU_SOCKET_BASE_CONTEXT
{
	OVERLAPPED      mstruOv;
	WSABUF          mstruWsabuf;

	//其他属性必须加在后面 IOCP需要访问前面的的东西

	ENUM_IO_STATE_T miIOStatus;	//网络I/O的状态 

	SOCKET          mhSocket;	//对端的唯一标识
	CSocketInfo		*mpSocketInfo;//socket信息
};
struct STRU_SOCKET_RECV_CONTEXT:public STRU_SOCKET_BASE_CONTEXT
{
	INT				miBuffLen;						//实际数据的长度
	BYTE            mpBuff[DEF_RECV_PACK_BUFF_LEN];	//接收缓冲区
};

struct STRU_SOCKET_SEND_CONTEXT:public STRU_SOCKET_BASE_CONTEXT
{
	BYTE            mpBuff[DEF_PACKET_LEN];	//发送 缓冲区
	int32			miSerialID;				//序号ID
	BOOL			mbSendClose;			//发送完关闭
	int16			miPackType;				//包类型
};


//socket 信息结构
class CSocketInfo:public CRefObjectforCache
{
public:
	CSocketInfo()
	{
		mpRecvContext = new STRU_SOCKET_RECV_CONTEXT;
		init();
		

	}
	~CSocketInfo()
	{
		clear();
		if(NULL != mpRecvContext)
		{
			delete mpRecvContext;
			mpRecvContext = NULL;
		}
	}
	inline void init()
	{ 
		InitRefObjectforCache();
		mbHasClear = FALSE;
		mbHasClosed = FALSE;

		mbStopSend = FALSE;

		moKey.mhSocket = INVALID_SOCKET;
		moKey.mPeerIP = 0;
		moKey.mwPeerPort = 0;	

		ZeroMemory(&mpRecvContext->mstruOv, sizeof(mpRecvContext->mstruOv));
		ZeroMemory(&mpRecvContext->mstruWsabuf, sizeof(mpRecvContext->mstruWsabuf));

		mpRecvContext->mhSocket = INVALID_SOCKET;
		mpRecvContext->miIOStatus = IOS_ACCEPT;

		mpRecvContext->miBuffLen = 0;
		mpRecvContext->mstruWsabuf.buf = (char *)mpRecvContext->mpBuff;
		mpRecvContext->mstruWsabuf.len = sizeof(mpRecvContext->mpBuff);

		mpRecvContext->mpSocketInfo = this;

		miCurrentSendContextCount = 0;

		mpTimeoutPos = INVALID_U9_POSITION;
		mpCloseTimeoutPos = INVALID_U9_POSITION;
		mpRelationPtr = NULL;
	}
	void clear()
	{
		if(mbHasClear)
			return ;
		//CloseSocket(moKey.mhSocket);
		mbHasClear = TRUE;
		LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, "clear Socket Info"
			<<" Socket="<<moKey.mhSocket
			<<" IP="<<GetIPString(moKey.mPeerIP)
			<<":"<<ntohs(moKey.mwPeerPort)
			<<" SocketInfo="<<(void*)this
			<<" RefCount="<<GetRefCount()
			<<" CurrentSendContextCount="<<miCurrentSendContextCount
			<<" WaitCount="<<(int32)moWaitSendDQueue.size());
		moCS.Enter();
		if(0 != moWaitSendDQueue.size())
		{
			U9_ASSERT(FALSE);
		}
		moWaitSendDQueue.clear();
		moCS.Leave();
	}
public:
	CCriticalSection	moCS;
	BOOL			mbHasClosed;	// 已经关闭socket
	BOOL			mbStopSend;		// 停止发送

	//STRU_SOCKET_RECV_CONTEXT moRecvContext;		//收包缓冲区

	STRU_SOCKET_RECV_CONTEXT *mpRecvContext;		//收包缓冲区
	int32 miCurrentSendContextCount;			//当前发包上下文数
	CContextDQueue<STRU_SOCKET_SEND_CONTEXT> moWaitSendDQueue;//等待发送的队列

	STRU_SOCKET_KEY moKey;

	U9_POSITION mpTimeoutPos;
	U9_POSITION mpCloseTimeoutPos;
	void*		mpRelationPtr;					//上层处理对象的关联指针
private:
	BOOL mbHasClear;
};

#else //#define WIN32

#include <sys/epoll.h>
#include <netinet/tcp.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <list>
#include <map>
#include <vector>
#include <ext/hash_map>

enum ENUM_SOCKET_STATE_T
{
	ENUM_SOCKET_STATE_CONNECTING = 0,//连接中
	ENUM_SOCKET_STATE_CONNECTED = 1,//连接成功
};

//socket上下文
struct STRU_SOCKET_RECV_CONTEXT
{
	INT				miBuffLen;						//实际数据的长度
	BYTE            mpBuff[DEF_RECV_PACK_BUFF_LEN];	//接收/发送 缓冲区

	SOCKET          mhSocket;		//对端的唯一标识
	CSocketInfo		*mpSocketInfo;	//socket信息
};

struct STRU_SOCKET_SEND_CONTEXT
{
	INT				miOffset;				//偏移
	INT				miBuffLen;				//实际数据的长度
	BYTE            mpBuff[DEF_PACKET_LEN];	//接收/发送 缓冲区
	BOOL			mbSendClose;			//发送完关闭
	int32			miSerialID;				//序号ID
	int16			miPackType;				//包类型
};

//socket 信息结构
class CSocketInfo:public CRefObjectforCache
{
public:
	CSocketInfo()
	{
		mpRecvContext = new STRU_SOCKET_RECV_CONTEXT;
		init();
	}
	~CSocketInfo()
	{
		clear();
		if(NULL != mpRecvContext)
		{
			delete mpRecvContext;
			mpRecvContext = NULL;
		}
	}
	inline void init()
	{ 
		InitRefObjectforCache();
		mbHasClear = FALSE;
		mbHasClosed = FALSE;

		mbSendingData = FALSE;
		mbStopSend = FALSE;

		mpRecvContext->miBuffLen = 0;

		moKey.mhSocket = INVALID_SOCKET;
		moKey.mPeerIP = 0;
		moKey.mwPeerPort = 0;	

		mpTimeoutPos = INVALID_U9_POSITION;
		mpCloseTimeoutPos = INVALID_U9_POSITION;
		mpRelationPtr = NULL;
		miNetState = ENUM_SOCKET_STATE_CONNECTED;
		mbFirstRecv = TRUE;
		mbNeedCheck = TRUE;
	}
	void clear()
	{
		if(mbHasClear)
			return ;
		mbHasClear = TRUE;
	
		LOG_TRACE(LOG_DEBUG_1, true, __FUNCTION__, " CSocketInfo"
			<<" Socket="<< moKey.mhSocket
			<<" IP="<<GetIPString(moKey.mPeerIP)
			<<":"<<ntohs(moKey.mwPeerPort)
			<<" SocketInfo="<<(void*)this
			<<" RefCount="<<GetRefCount()
			<<" WaitCount="<<(int32)moWaitSendDQueue.size()
			<<" TimeoutPos="<<(void*)mpTimeoutPos
			);
		moCS.Enter();
		moWaitSendDQueue.clear();
		moCS.Leave();
		if( moKey.mhSocket != INVALID_SOCKET)
		{
			CloseSocket( moKey.mhSocket);
			moKey.mhSocket = INVALID_SOCKET;
		}

		mbFirstRecv = TRUE;
		mbNeedCheck = TRUE;
	}
public:
	CCriticalSection	moCS;
	BOOL				mbHasClosed;	//已经关闭socket
	BOOL				mbSendingData;	//存在发送数据
	BOOL				mbStopSend;		//停止发送
	CContextDQueue<STRU_SOCKET_SEND_CONTEXT> moWaitSendDQueue;	//等待发送的队列

	STRU_SOCKET_RECV_CONTEXT* mpRecvContext;						//收包缓冲区

	STRU_SOCKET_KEY	moKey;

	U9_POSITION mpTimeoutPos;
	U9_POSITION mpCloseTimeoutPos;
	void*		mpRelationPtr;					//上层处理对象的关联指针
	uint8		miNetState;						//socket状态。连接中还是已经连接
	BOOL		mbFirstRecv;					//是否第一次接收消息
	BOOL		mbNeedCheck;					//是否需要超时检测
private:
	BOOL mbHasClear;
};

#endif //WIN32
#endif //#ifndef __PROXYINFO_H
