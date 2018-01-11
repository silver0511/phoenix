// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: ITransMgrCallBack.h
// 创 建 人: 史云杰
// 文件说明: 网络传输回调接口
// $_FILEHEADER_END *****************************

#ifndef __ITRANSMGRCALLBACE_H
#define __ITRANSMGRCALLBACE_H

#include "base0/platform.h"
#include "const/constdef.h"
#include "common/ISerialize.h"
#include "common/Debug_log.h"
#ifdef _WIN32
#include <winsock.h>
#endif

//消息中心错误类型
enum ENUM_MSGCENTER_ERROR
{
	OPEN_UDP_MGR_ERROR = -100,
	OPEN_TCP_MGR_ERROR,
	OPEN_HTTP_MGR_ERROR,
	OPEN_TUDP_MGR_ERROR,
	CLOSE_UDP_MGR_ERROR,
	CLOSE_TCP_MGR_ERROR,
	CLOSE_HTTP_MGR_ERROR,
	CLOSE_TUDP_MGR_ERROR,
	CREATE_CHECK_THREAD_ERROR,
	SOCKET_INIT_ERROR,
	SOCKET_BIND_ERROR,
	SENDDATA_TO_LONGTH,
	RECV_THREAD_EXCEPTION_ERROR,
	DEAL_THREAD_EXCEPTION_ERROR,
	SEND_THREAD_EXCEPTION_ERROR,
	RESEND_THREAD_EXCEPTION_ERROR,
	RETRY_SEND_DATA_CHECK_EXCEPTION_ERROR,
	PRE_DEAL_SOCKET_DATA_ERROR
};

///////////////////////////////////////////////////////////////////////////////////

//设置信息
#define DEF_DEFAULT_MAX_RECV_COUNT			(1000)
#define DEF_DEFAULT_MAX_RECV_CACHE_COUNT	(20)
#define DEF_DEFAULT_MAX_SEND_COUNT			(1000)
#define DEF_DEFAULT_MAX_SEND_CACHE_COUNT	(20)
#define DEF_DEFAULT_MAX_SEND_QUEUE_COUNT	(100)

struct STRU_OPTION_BASE
{
	char	mszAppName[DEF_MAX_APP_NAME+1];			//  APP名
	int32	miLogLevel;					//  日志等级
	SESSIONID miPeerID;					//  用户唯一标识
	IPTYPE	mulIPAddr;					//	本地地址
	WORD	mwPort;						//	本地端口
	BOOL	mbOnlyBindOne;				//	是否仅仅绑定一次端口
	BOOL	mbIsServer;					//  是否是服务器

	BYTE	mbyRecvThreadCount;			//	接收线程数目
	BYTE	mbyDealThreadCount;			//	处理线程数目

	UINT miMaxRecvPackageCount;				// 发送队列长度
	UINT miMaxSendPackageCount;				// 接收队列长度

	STRU_OPTION_BASE()
	{
		miLogLevel = -1;				//-1是不记录日志
		miPeerID = 0;
		mulIPAddr = 0;					//	本地地址
		mwPort = 3000;					//	本地端口
		mbOnlyBindOne = FALSE;			//	是否仅仅绑定一次端口
		mbIsServer = FALSE;				//  是否是服务器

		mbyRecvThreadCount = 1;			//	接收线程数目
		mbyDealThreadCount = 1;			//	处理线程数目
		miMaxRecvPackageCount = DEF_DEFAULT_MAX_RECV_COUNT;				//	最大接收包数
		miMaxSendPackageCount = DEF_DEFAULT_MAX_SEND_COUNT;				//	最大接收包数
		ZeroMemory(&mszAppName,sizeof(mszAppName));
		
	};
};
#define DEF_MAX_MODUL_NAME	(32)
struct STRU_OPTION_UDP: public STRU_OPTION_BASE
{
	enum ENUM_PACKET_RECORD_OPTION
	{
		PACKET_RECORD_TYPE_NONE  = 0,// 正常
		PACKET_RECORD_TYPE_WRITE = 1,// 记录
		PACKET_RECORD_TYPE_PLAY	 = 2,// 回放
	};
	int32	miRecordOption;
	int32	miPlaySpeed;	//	播放速率 在miRecordOption is PACKET_RECORD_TYPE_PLAY 有效
	U9_CHAR	mszModulName[DEF_MAX_MODUL_NAME+1];
	STRU_OPTION_UDP()
	{
		miRecordOption = PACKET_RECORD_TYPE_NONE;
		miPlaySpeed =1 ;
		ZeroMemory(&mszModulName,sizeof(mszModulName));
	};
};


struct STRU_OPTION_TCP: public STRU_OPTION_BASE
{
	UINT muiAcceptExNum;                //每次投递的AcceptEx的个数
	UINT muiMemPoolSize;                //内存池链表节点个数
	UINT muiKeepAlive;                  //连接存活时间
	
	UINT muiMaxConnCount;				//最大连接数
	BYTE mbyConnectThreadCount;			//连接线程数目
	BYTE mbyEpollCount;					//开启epoll数目

	BOOL mbUseRelationPtr;
	BOOL mbTls;							//是否是tls
	char mszWithOutCheckIP[2048];		//无需超时检测的IP
	STRU_OPTION_TCP()
	{
		muiAcceptExNum = 64;                // 每次投递64个AcceptEx
		
		muiMemPoolSize = 32;   
                                  
		muiKeepAlive   = 30;                // 连接存活时间为30秒

		muiMaxConnCount=128;
		mbUseRelationPtr = false;

		mbyConnectThreadCount = 1 ;
		mbyEpollCount = 1 ;

		mbTls = FALSE;
		ZeroMemory(&mszWithOutCheckIP,sizeof(mszWithOutCheckIP));
	};
};

struct STRU_OPTION_TCP_PROXY: public STRU_OPTION_TCP
{
	IPTYPE	mulProxyIPAddr;				//代理地址
	WORD	mwProxyPort;				//本地端口
	IPTYPE	mulProxyOutIPAddr;			//代理out地址
	INT32	miFluxstatIntervalTime;		//统计时间间隔 单位：秒
	STRU_OPTION_TCP_PROXY()
	{
		mulProxyIPAddr = 0;                
		mwProxyPort = 0;
		mulProxyOutIPAddr = 0 ;
		miFluxstatIntervalTime = 60;//60秒
	};
};

struct STRU_OPTION_HTTP: public STRU_OPTION_BASE
{
	STRU_OPTION_HTTP()
	{
		
	};
};

struct STRU_OPTION_TUDP: public STRU_OPTION_BASE
{

	STRU_OPTION_TUDP()
	{
	};
};


/////////////////////////////////////////////////////////////////////////////////////////////////////

// 发送数据
struct STRU_SENDDATA_BASE
{
	IPTYPE  mIPAddress;
	WORD	mwIPPort;
	BYTE*	mpData;
	WORD	mwLen;

	STRU_SENDDATA_BASE()
	{
		mIPAddress = 0;
		mwIPPort = 0;
		mpData = NULL;
		mwLen = 0;
	};
};

struct STRU_SENDDATA_UDP : public STRU_SENDDATA_BASE
{
	SESSIONID miPhoneID;
	BOOL mbNeedTimeStampCheck;
	BYTE mbySendCount;
	BOOL mbNeedReport;
	BYTE mbySameSendCount;
	WORD mwTrySendSecond;
	BOOL mbUseSendThread;
	BOOL mbEncrypt;
	BOOL mbNeedEcho;
	STRU_SENDDATA_UDP()
	{
		mbNeedTimeStampCheck = FALSE;
		mbySendCount = 1;
		mbNeedReport = FALSE;
		mbySameSendCount = 1;
		mwTrySendSecond = 1;
		mbUseSendThread = FALSE;
		mbEncrypt = FALSE;
		mbNeedEcho = FALSE;
		miPhoneID = INVALID_SESSIONID;
	};
};

struct STRU_SENDDATA_TCP : public STRU_SENDDATA_BASE
{
	SOCKET mhSocket;
	BOOL  mbNeedClose;  
#ifndef _WIN32
	int miSendThreadConut;//发送线程数量
#endif
	STRU_SENDDATA_TCP()
	{
		mhSocket = INVALID_SOCKET;
		mbNeedClose = FALSE;
#ifndef _WIN32
		miSendThreadConut = 5;
#endif
	};
};


////////////////////////////////////////////////////////////////////////////////////////////////////

// 统计信息
struct STRU_STATISTICS_BASE
{
	ULONG	mulTotalSendPacketCount;	//发包总数
	UINT64	mui64TotalSendSize;			//总发送大小（单位 B）
	ULONG	mulTotalRecvPacketCount;	//收包总数
	UINT64	mui64TotalRecvSize;			//总接收大小（单位 B）

	STRU_STATISTICS_BASE()
	{
		mulTotalSendPacketCount = 0;	//发包总数
		mui64TotalSendSize = 0;			//总发送大小（单位 B）
		mulTotalRecvPacketCount = 0;	//收包总数
		mui64TotalRecvSize = 0;			//总接收大小（单位 B）
	};
};

struct STRU_STATISTICS_UDP : public STRU_STATISTICS_BASE
{
	ULONG	mulSendPacketCount_Sec;		//每秒发包数
	float	mfSendSize_Sec;				//每秒发送大小 （单位 KB）
	ULONG	mulRecvPacketCount_Sec;		//每秒收包数
	float	mfRecvSize_Sec;				//每秒接收大小 （单位 KB）
	float	mfLostPacketRate;			//丢包率
	ULONG	mulTryPacketSendCount;		//重发送包总数

	STRU_STATISTICS_UDP()
	{
		mulSendPacketCount_Sec = 0;		//每秒发包数
		mfSendSize_Sec = 0;				//每秒发送大小 （单位 KB）
		mulRecvPacketCount_Sec = 0;		//每秒收包数
		mfRecvSize_Sec = 0;				//每秒接收大小 （单位 KB）
		mfLostPacketRate = 0;			//丢包率
		mulTryPacketSendCount = 0;		//重发送包总数
	};
};

struct STRU_STATISTICS_TCP : public STRU_STATISTICS_BASE
{
	ULONG  t_send_count;              	 // 发送包总数
	ULONG  t_recv_count;                 // 接收包总数
	float  send_count_sec;       		 // 每秒发送包数
	float  recv_count_sec;       		 // 每秒接收包数
	INT    cur_con;                 	 // 当前的TCP连接个数
	ULONG  t_lost_send;				 	 // 发送丢失包总数
	ULONG  t_lost_recv;				 	 // 接受丢失包总数
	time_t s_start_time;			     // 网络统计开始时间
	time_t s_end_time;					 // 网络统计结束时间
	STRU_STATISTICS_TCP()
	{
		t_send_count = 0;
		t_recv_count = 0;
		send_count_sec = 0.0;
		recv_count_sec = 0.0;
		cur_con = 0;
		t_lost_send = 0;
		t_lost_recv = 0;
		s_start_time = 0;
		s_end_time = 0;
	};
};


struct STRU_FLUX_STAT
{
	int32 miPacketSum;			//包数
	int32 miTotalPacketSize;	//总大小	
	int32 miMaxPacketSize;		//最大包大小
	int32 miMinPacketSize;		//最小包大小
public:
	STRU_FLUX_STAT()
	{
		init();
	}
	~STRU_FLUX_STAT()
	{
		init();
	}
	inline void init()
	{
		miPacketSum = 0;
		miTotalPacketSize = 0;
		miMaxPacketSize = 0;
		miMinPacketSize = 0;
	}
	inline void AddPacket(int32 aiPacketSize)
	{
		U9_ASSERT(aiPacketSize > 0);
		miPacketSum++;
		miTotalPacketSize += aiPacketSize;
		if (aiPacketSize > miMaxPacketSize)
		{
			miMaxPacketSize = aiPacketSize;
		}
		if (miMinPacketSize == 0 || aiPacketSize < miMinPacketSize)
		{
			miMinPacketSize = aiPacketSize;
		}
		return;
	}
	inline void GetFluxStat(STRU_FLUX_STAT& aoStat) const
	{
		aoStat = *this;
	}
	inline void GetFluxStatReset(STRU_FLUX_STAT& aoStat)
	{
		GetFluxStat(aoStat);
		init();
	}

	INT Serialize(ISerialize & aoSerialize)
	{
		aoSerialize.Serialize(miTotalPacketSize);
		aoSerialize.Serialize(miPacketSum);
		aoSerialize.Serialize(miMaxPacketSize);
		aoSerialize.Serialize(miMinPacketSize);
		return 1;
	}
	STRU_FLUX_STAT& operator =(const STRU_FLUX_STAT& aoObj)
	{
		U9_ASSERT (this != &aoObj);
		if(this == &aoObj)
			return *this;
		miTotalPacketSize = aoObj.miTotalPacketSize;
		miPacketSum = aoObj.miPacketSum;
		miMaxPacketSize = aoObj.miMaxPacketSize;
		miMinPacketSize = aoObj.miMinPacketSize;
		return *this;
	}
	STRU_FLUX_STAT& operator +=(const STRU_FLUX_STAT& aoObj)
	{
		miTotalPacketSize += aoObj.miTotalPacketSize;
		miPacketSum += aoObj.miPacketSum;
		if (miMaxPacketSize < aoObj.miMaxPacketSize)
		{
			miMaxPacketSize = aoObj.miMaxPacketSize;
		}
		if (aoObj.miMinPacketSize != 0)
		{
			if (miMinPacketSize == 0 || miMinPacketSize > aoObj.miMinPacketSize)
			{
				miMinPacketSize = aoObj.miMinPacketSize;
			}
		}
		return *this;
	}

};

class CNetworkFluxStat
{
public:
	STRU_FLUX_STAT moSendFlux;//发送
	STRU_FLUX_STAT moRecvFlux;//接受
public:

	CNetworkFluxStat()
	{
		init();
	}

	~CNetworkFluxStat()
	{
		return;
	}
	void init()
	{
		moSendFlux.init();
		moRecvFlux.init();
	}
	CNetworkFluxStat& operator =(const CNetworkFluxStat& aoObj)
	{
		U9_ASSERT (this != &aoObj);
		if(this == &aoObj)
			return *this;
		moSendFlux= aoObj.moSendFlux;
		moRecvFlux = aoObj.moRecvFlux;
		return *this;
	}
};

class  IICMPMsgCallBack 
{       
public:  
	virtual INT OnRecvReply(IPTYPE aiIP, USHORT aiSerial, WORD aiDelay, WORD aiUserLoad) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

class  ITransMgrCallBack 
{       
public:  
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnRecvData
	// 函数参数： char* apData 数据
	//			 WORD awLen,apData长度
	//			,IPTYPE aiIP,对方地址
	//			WORD awPort 对方端口
	// 返 回 值： 1是成功， -1是失败
	// 函数说明： 从网络层接收到数据
	// $_FUNCTION_END ********************************
	virtual INT OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort, 
		BYTE* apData,WORD awLen)=0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnNewRecvData
	// 函数参数： char* apData 数据
	//			 WORD awLen,apData长度
	//			,IPTYPE aiIP,对方地址
	//			WORD awPort 对方端口
	// 返 回 值： 1是成功， -1是失败
	// 函数说明： 从网络层接收到数据
	// $_FUNCTION_END ********************************
	virtual INT OnNewRecvData(SOCKET ahSocket, void* apRelationPtr,IPTYPE aiIP,WORD awPort, 
		BYTE* apData,WORD awLen) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSendDataError
	// 函数参数：  char* apData 数据
	//			 WORD awLen,apData长度
	//			,IPTYPE aiIP,对方地址
	//			WORD awPort 对方端口
	// 返 回 值： 1是成功， -1是失败
	// 函数说明： 从发送该数据失败
	// $_FUNCTION_END ********************************
	virtual INT OnSendDataError(IPTYPE aiIP,
		WORD awPort, BYTE* apData,WORD awLen)=0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnError
	// 函数参数： aiErrType 错误类型
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 出错处理
	// $_FUNCTION_END ********************************
	virtual INT OnError(UINT aiErrType)=0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： CheckSessionEvent
	// 函数参数：bool abIsNormal 是否到正常检测时间
	// 返 回 值： 无
	// 函数说明： 检查处理事件
	// $_FUNCTION_END ********************************
	virtual VOID CheckEvent(bool abIsNormal)=0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionClose
	// 函数参数：session连接关闭
	// 返 回 值： 无
	// 函数说明： 有连接关闭
	// $_FUNCTION_END ********************************
	virtual void OnSessionClose(const SESSIONID & aiPeerID,SOCKET ahSocket,IPTYPE aiIP,WORD awPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionClose
	// 函数参数：session连接关闭
	// 返 回 值： 无
	// 函数说明： 有连接关闭
	// $_FUNCTION_END ********************************r
	virtual void OnSessionCloseEx(const SESSIONID & aiPeerID,void* apRelationPtr,SOCKET ahSocket,IPTYPE aiIP,WORD awPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnected
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 被别人连接成功
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnected(const SESSIONID & aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort) = 0;


	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnected
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 被别人连接成功
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnectedEx(const SESSIONID & aiPeerID,void* apSocketInfo , SOCKET ahSocket,IPTYPE aiIP,WORD awPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnectFailure
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 被别人连接shi失败
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnectFailure(const SESSIONID & aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort) = 0;

};

class  CTransDealMgrNull: public ITransMgrCallBack
{
public:
	virtual INT OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort,
						   BYTE* apData,WORD awLen)
	{
		return 0;
	}
	virtual INT OnNewRecvData(SOCKET ahSocket, void* apRelationPtr,IPTYPE aiIP,WORD awPort,
							  BYTE* apData,WORD awLen)
	{
		return 0;
	}
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSendDataError
	// 函数参数：  char* apData 数据
	//			 WORD awLen,apData长度
	//			,IPTYPE aiIP,对方地址
	//			WORD awPort 对方端口
	// 返 回 值： 1是成功， -1是失败
	// 函数说明： 从发送该数据失败
	// $_FUNCTION_END ********************************
	virtual INT OnSendDataError(IPTYPE aiIP,
								WORD awPort, BYTE* apData,WORD awLen)
	{
		return 0;
	}

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnError
	// 函数参数：INT aiProtocolType:0是uc 1是us
	//			UINT aiErrType 错误类型
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 出错处理
	// $_FUNCTION_END ********************************
	virtual INT OnError(UINT aiErrType)
	{
		return 0;
	}

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： CheckSessionEvent
	// 函数参数：bool abIsNormal 是否到正常检测时间
	// 返 回 值： 无
	// 函数说明： 检查处理事件
	// $_FUNCTION_END ********************************
	virtual VOID CheckEvent(bool abIsNormal)
	{
		return ;
	}


	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionClose
	// 函数参数：session连接关闭
	// 返 回 值： 无
	// 函数说明： 连接关闭通知
	// $_FUNCTION_END ********************************
	virtual void OnSessionClose(const SESSIONID & aiPeerID,SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
	{
		return ;
	}

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionClose
	// 函数参数：session连接关闭
	// 返 回 值： 无
	// 函数说明： 连接关闭通知
	// $_FUNCTION_END ********************************
	virtual void OnSessionCloseEx(const SESSIONID & aiPeerID,void* apSocketInfo ,SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
	{
		return ;
	}
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnected
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明：  新连接通知
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnected(const SESSIONID & aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
	{
		return ;
	}

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnectedEx
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明：  新连接通知
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnectedEx(const SESSIONID & aiPeerID,void* apSocketInfo , SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
	{
		return ;
	}


	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnectFailure
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 被别人连接shi失败
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnectFailure(const SESSIONID & aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
	{
		return ;
	};
};
#endif //__ITRANSMGRCALLBACE_H

