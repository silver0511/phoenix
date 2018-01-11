// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: BaseTCPMgr.h
// 创 建 人: 史云杰
// 文件说明: TCP连接池管理
// $_FILEHEADER_END *****************************


#ifndef _TCCCONNPOOLMGR_H_
#define _TCCCONNPOOLMGR_H_

#include <map>
#include "TCPConnectPool.h"

class CTCPConnPoolMgr: public ITCPConnPoolMgr,ITransMgrCallBack
{
public:
	CTCPConnPoolMgr();
	~CTCPConnPoolMgr();
public:
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Open
	// 函数参数：
	// 返 回 值： 1是成功 -1是失败，
	// 函数说明： 打开操作
	// $_FUNCTION_END ********************************
	virtual INT	Open(ITCPConnPoolMgrCallback *apInterface, 
		const STRU_TCP_POOL_OPT &aoOpt);

	// $_FUNCTION_BEGIN ******************************
	// 函数名称：Close
	// 函数参数：INT 
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 关闭操作（同步返回）
	// $_FUNCTION_END ********************************
	virtual INT Close();

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Release
	// 函数参数：
	// 返 回 值： VOID
	// 函数说明： 释放对象
	// $_FUNCTION_END ********************************
	virtual VOID Release();

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： SendData
	// 函数参数： STRU_SENDDATA_BASE *apstruSendData 数据发送设置
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 发送数据
	// $_FUNCTION_END ********************************
	virtual INT SendDataBySession(const SESSIONID& aiPlayerID,STRU_SENDDATA_BASE *apstruSendData);


	virtual INT SendDataBySocket(SOCKET ahSoket,STRU_SENDDATA_BASE *apstruSendData);

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： SendData
	// 函数参数： STRU_SENDDATA_BASE *apstruSendData 数据发送设置
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 发送数据
	// $_FUNCTION_END ********************************
	virtual INT SendData(void* apSocketInfo,const SESSIONID& aiPlayerID,STRU_SENDDATA_BASE *apstruSendData);

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： GetNetStatisticsInfo
	// 函数参数：
	// 返 回 值： 
	// 函数说明： 得到本地网络统计信息
	// $_FUNCTION_END ********************************
	virtual INT GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo);

public:

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
		WORD awPort, BYTE* apData,WORD awLen) ;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnError
	// 函数参数：INT aiProtocolType:0是uc 1是us
	//			UINT aiErrType 错误类型
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 出错处理
	// $_FUNCTION_END ********************************
	virtual INT OnError(UINT aiErrType) ;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： CheckSessionEvent
	// 函数参数：bool abIsNormal 是否到正常检测时间
	// 返 回 值： 无
	// 函数说明： 检查处理事件
	// $_FUNCTION_END ********************************
	virtual VOID CheckEvent(bool abIsNormal) ;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionClose
	// 函数参数：session连接关闭
	// 返 回 值： 无
	// 函数说明： 连接关闭通知
	// $_FUNCTION_END ********************************
	virtual void OnSessionClose(const SESSIONID & aiPeerID,SOCKET ahSocket,IPTYPE aiIP,WORD awPort) ;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionClose
	// 函数参数：session连接关闭
	// 返 回 值： 无
	// 函数说明： 连接关闭通知
	// $_FUNCTION_END ********************************
	virtual void OnSessionCloseEx(const SESSIONID & aiPeerID,void* apSocketInfo,SOCKET ahSocket,IPTYPE aiIP,WORD awPort) ;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnected
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 新连接通知
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnected(const SESSIONID & aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort) ;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnectedEx
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 新连接通知
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnectedEx(const SESSIONID & aiPeerID, void* apSocketInfo,SOCKET ahSocket,IPTYPE aiIP,WORD awPort) ;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnectFailure
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 被别人连接shi失败
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnectFailure(const SESSIONID & aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort) ;

	virtual INT OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen);

	virtual INT OnNewRecvData(SOCKET ahSocket, void* apRelationPtr,IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen);

	virtual bool JudgeIsConnectServer(IPTYPE aiIP, WORD awPort);

public:
private:
	CTCPConnectPool moTCPConnectPool;
public:
	ITCPConnPoolMgrCallback *mpInterface;
};

/////////////////////////////////

#endif //_TCCCONNPOOLMGR_H_