// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: ITCPPoolTransMgr.h
// 创 建 人: 史云杰
// 文件说明: TCP连接池网络传输接口
// $_FILEHEADER_END *****************************


#ifndef __ITCPCONNPOOLMGR_H_
#define __ITCPCONNPOOLMGR_H_

#include "base0/platform.h"
#include "common/basefun.h"
#include "ITransMgrCallBack.h"
DLL_EXPORT INT CreateNetTPool(const char* aszClassName, 
											void** appInterface);


struct STRU_TCP_POOL_OPT
{
	int32 miLogLevel;
	char	mszAppName[DEF_MAX_APP_NAME+1];			//  APP名
	IPTYPE	miTCPIP; //TCP地址	
	WORD  miTCPPort; //TCP端口
	bool  mbUseRelink;//是否启用重连接机制
	INT		miTCPConnCount;//连接数
	INT		miTCPMaxRecvPackageCount;		// 最大接收包数
	INT		miTCPMaxSendPackageCount;		// 最大接收包数
	STRU_TCP_POOL_OPT()
	{
		miLogLevel = -1;
		miTCPIP = 0;
		mbUseRelink = false;
		miTCPPort = 0;
		miTCPConnCount = 1;				
		miTCPMaxRecvPackageCount = 1024;		
		miTCPMaxSendPackageCount = 1024;
		ZeroMemory(&mszAppName,sizeof(mszAppName));
	}

};


class  ITCPConnPoolMgrCallback 
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
	virtual INT OnTCPPoolTransRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort, 
		BYTE* apData,WORD awLen)=0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnTCPPoolTransNewRecvData
	// 函数参数： char* apData 数据
	//			 WORD awLen,apData长度
	//			,IPTYPE aiIP,对方地址
	//			WORD awPort 对方端口
	// 返 回 值： 1是成功， -1是失败
	// 函数说明： 从网络层接收到数据
	// $_FUNCTION_END ********************************
	virtual INT OnTCPPoolTransNewRecvData(SOCKET ahSocket, void* apapRelationPtr,IPTYPE aiIP,WORD awPort, 
		BYTE* apData,WORD awLen)=0;
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionClose
	// 函数参数：session连接关闭
	// 返 回 值： 无
	// 函数说明： 连接关闭通知
	// $_FUNCTION_END ********************************
	virtual void OnTCPPoolSessionClose(const SESSIONID& aiPeerID,SOCKET ahSocket,IPTYPE aiIP,WORD awPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionClose
	// 函数参数：session连接关闭
	// 返 回 值： 无
	// 函数说明： 连接关闭通知
	// $_FUNCTION_END ********************************
	virtual void OnTCPPoolSessionCloseEx(const SESSIONID& aiPeerID,void* apSocketInfo,SOCKET ahSocket,IPTYPE aiIP,WORD awPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnected
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 新连接通知
	// $_FUNCTION_END ********************************
	virtual void OnTCPPoolSessionConnected(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnected
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 新连接通知
	// $_FUNCTION_END ********************************
	virtual void OnTCPPoolSessionConnectedEx(const SESSIONID& aiPeerID, void* apSocketInfo,SOCKET ahSocket,IPTYPE aiIP,WORD awPort) = 0;

};

class ITCPConnPoolMgr 
{
public:
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Open
	// 函数参数：
	// 返 回 值： 1是成功 -1是失败，
	// 函数说明： 打开操作
	// $_FUNCTION_END ********************************
	virtual INT	Open(ITCPConnPoolMgrCallback *apInterface, 
		const STRU_TCP_POOL_OPT &aoOpt)=0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称：Close
	// 函数参数：INT 
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 关闭操作（同步返回）
	// $_FUNCTION_END ********************************
	virtual INT Close()=0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Release
	// 函数参数：
	// 返 回 值： VOID
	// 函数说明： 释放对象
	// $_FUNCTION_END ********************************
	virtual VOID Release()=0;
	
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： SendData
	// 函数参数： STRU_SENDDATA_BASE *apstruSendData 数据发送设置
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 发送数据
	// $_FUNCTION_END ********************************
	virtual INT SendDataBySession(const SESSIONID& aiID,STRU_SENDDATA_BASE *apstruSendData) = 0;
	
	virtual INT SendDataBySocket(SOCKET ahSoket,STRU_SENDDATA_BASE *apstruSendData) = 0;
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： GetNetStatisticsInfo
	// 函数参数：
	// 返 回 值： 
	// 函数说明： 得到本地网络统计信息
	// $_FUNCTION_END ********************************
	virtual INT GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo) = 0;

	virtual bool JudgeIsConnectServer(IPTYPE aiIP, WORD awPort) = 0;
};

inline INT SendbyTCPPool(ITCPConnPoolMgr *apInterface,
				   const SESSIONID&  aiID,
				   IPTYPE aIPAddress,WORD awIPPort,
				   BYTE *apData,WORD awLen)
{
	STRU_SENDDATA_TCP loSendPack;
	loSendPack.mbNeedClose = FALSE;

	loSendPack.mpData = apData;
	loSendPack.mwLen = awLen;

	loSendPack.mIPAddress = aIPAddress;
	loSendPack.mwIPPort = awIPPort;

	return apInterface->SendDataBySession(aiID,&loSendPack);
};

inline INT SendbyTCPPool(ITCPConnPoolMgr *apInterface,
	SOCKET  ahSocket,
	IPTYPE aIPAddress,WORD awIPPort,
	BYTE *apData,WORD awLen)
{
	STRU_SENDDATA_TCP loSendPack;
	loSendPack.mbNeedClose = FALSE;

	loSendPack.mpData = apData;
	loSendPack.mwLen = awLen;

	loSendPack.mIPAddress = aIPAddress;
	loSendPack.mwIPPort = awIPPort;

	return apInterface->SendDataBySocket(ahSocket,&loSendPack);
};

#endif