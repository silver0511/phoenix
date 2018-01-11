// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: ITransMgrCallBack.h
// 创 建 人: 史云杰
// 文件说明: 网络传输接口
// $_FILEHEADER_END *****************************
#ifndef __ITRANSPROTOCOLMGR_H_
#define __ITRANSPROTOCOLMGR_H_

#include "base0/platform.h"
#include "common/basefun.h"
#include "network/ITransMgrCallBack.h"

DLL_EXPORT INT CreateNetUDP(const char* aszClassName, void** appInterface);

DLL_EXPORT INT CreateNetTCP(const char *aszClassName, void **appInterface);

DLL_EXPORT INT CreateNetICMP(const char *aszClassName, void **appInterface);


class IICMPProtocolMgr 
{
public:
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Open
	// 函数参数：
	// 返 回 值： 1是成功 -1是失败，
	// 函数说明： 打开操作
	// $_FUNCTION_END ********************************
	virtual INT	Open(void) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称：Close
	// 函数参数：INT 
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 关闭操作（同步返回）
	// $_FUNCTION_END ********************************
	virtual INT Close(void) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Release
	// 函数参数：
	// 返 回 值： VOID
	// 函数说明： 释放对象
	// $_FUNCTION_END ********************************
	virtual VOID Release(void) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Init
	// 函数参数：	apInterface 事件通告接口
	//				astruOption 设置属性
	// 返 回 值： 
	// 函数说明： 初始化接口及设置成员
	// $_FUNCTION_END ********************************
	virtual INT Init(IICMPMsgCallBack *apInterface) = 0;

	virtual INT SendEcho(IPTYPE aiIP, WORD awSerial, WORD awUserLoad, USHORT aiOwnerID) = 0;

	virtual INT SetTTL(int aiTTL) = 0;
};


class ITransProtocolMgr 
{
public:
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Open
	// 函数参数：
	// 返 回 值： 1是成功 -1是失败，
	// 函数说明： 打开操作
	// $_FUNCTION_END ********************************
	virtual INT	Open()=0;

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
	// 函数名称： Init
	// 函数参数：	apInterface 事件通告接口
	//				astruOption 设置属性
	// 返 回 值： 
	// 函数说明： 初始化接口及设置成员
	// $_FUNCTION_END ********************************
	virtual INT Init(ITransMgrCallBack *apInterface, 
		STRU_OPTION_BASE *astruOption) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Init
	// 函数参数： aiValue 加密类型
	//			 
	// 返 回 值： 
	// 函数说明： 初始化加密
	// $_FUNCTION_END ********************************
	virtual void InitEncrypt(int8 aiEncryptType)=0;
	
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： GetBindIP
	// 函数参数：
	// 返 回 值： IPTYPE IP地址
	// 函数说明： 得到本机绑定端口
	// $_FUNCTION_END ********************************
	virtual void SetBindIP(IPTYPE aiIP) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： GetBindPort
	// 函数参数： 
	// 返 回 值： 绑定端口
	// 函数说明： 得到本机绑定端口
	// $_FUNCTION_END ********************************
	virtual void SetBindPort(WORD awPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： GetBindIP
	// 函数参数：
	// 返 回 值： IPTYPE IP地址
	// 函数说明： 得到本机绑定端口
	// $_FUNCTION_END ********************************
	virtual IPTYPE GetBindIP() = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： GetBindPort
	// 函数参数： 
	// 返 回 值： 绑定端口
	// 函数说明： 得到本机绑定端口
	// $_FUNCTION_END ********************************
	virtual WORD GetBindPort() = 0;

	// $_FUNCTION_BEGIN ******************************
	//
	// 函数名称： AddRelationPtr
	// 函数参数： 
	// 返 回 值： 添加设置成功
	// 函数说明： 添加关联指针
	//
	// $_FUNCTION_END ********************************
	virtual bool AddRelationPtr(void*apSocketInfo,void* apRelationPtr) = 0;

	// $_FUNCTION_BEGIN ******************************
	//
	// 函数名称： DelRelationPtr
	// 函数参数： 
	// 返 回 值： 删除设置成功
	// 函数说明： 删除关联指针
	//
	// $_FUNCTION_END ********************************
	virtual bool DelRelationPtr(void*apSocketInfo,void* apRelationPtr) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： SendData
	// 函数参数： STRU_SENDDATA_BASE *apstruSendData 数据发送设置
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 发送数据
	// $_FUNCTION_END ********************************
	virtual INT SendData(STRU_SENDDATA_BASE *apstruSendData) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： SendData
	// 函数参数： STRU_SENDDATA_BASE *apstruSendData 数据发送设置
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 发送数据
	// $_FUNCTION_END ********************************
	virtual INT SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData) = 0
		;
    // $_FUNCTION_BEGIN ******************************
    // 函数名称： SendData
    // 函数参数： STRU_SENDDATA_BASE *apstruSendData 数据发送设置
    // 返 回 值： 1是成功 -1是失败
    // 函数说明： 发送数据
    // $_FUNCTION_END ********************************
    virtual INT SendRawData(STRU_SENDDATA_BASE *apstruSendData) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： ConnectTo
	// 函数参数： aiIPAddr 对方IP， awPort 对方的监听端口
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 发送数据
	// $_FUNCTION_END ********************************
	virtual INT ConnectTo (const SESSIONID & aiPeerID, IPTYPE aiIPAddr, WORD awPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： DisConnectTo
	// 函数参数： SOCKET ahSocket
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 发送数据
	// $_FUNCTION_END ********************************
	virtual INT DisConnectTo (SOCKET ahSocket) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： GetNetStatisticsInfo
	// 函数参数：
	// 返 回 值： 
	// 函数说明： 得到本地网络统计信息
	// $_FUNCTION_END ********************************
	virtual INT GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Processs
	// 函数参数：
	// 返 回 值： 
	// 函数说明： 处理函数，仅仅在客户端有效
	// $_FUNCTION_END ********************************
	virtual	void Processs()=0;
};


// $_FUNCTION_BEGIN ******************************
// 函数名称： SendUDP
// 函数参数：
//			IPTYPE aIPAddress=0,	目标ip地址
//			WORD awIPPort=0			目标ip端口
//	        char *apData,数据
//			WORD awLen,长度
//			BOOL abyNeedTimeStampCheck,是否需要对方时间戳队列检测
//			BYTE abySendCount  发送次数
//			INT  aiNeedReport 当发送失败时，是否产生OnSendDataError
//			BYTE abySameSendCount   同时发包数
//			WORD awTrySendSecond	重发间隔秒数
//			BOOL abUserSendThread	是否使用发送线程发送
//			BOOL abyEncrypt=		数据是否加密 如果是streaming数据，不需要加密
// 返 回 值： 1是成功 -1是失败
// 函数说明： 发送数据
// $_FUNCTION_END ********************************
inline INT SendUDP(ITransProtocolMgr *apInterface,
				      const SESSIONID &aiSessionID,
					  IPTYPE aIPAddress,WORD awIPPort,
					  BYTE *apData,WORD awLen,
					  BYTE abySendCount=1,
					  BOOL abNeedReport=FALSE,
					  BYTE abySameSendCount=1,
					  WORD awTrySendSecond=1,
					  BOOL abNeedEcho=FALSE,
					  BOOL abNeedTimeStampCheck=FALSE,
					  BOOL abEncrypt=FALSE)
{
	WORD lwPackType=0;\
	memcpy(&lwPackType,apData,2);\
	LOG_TRACE(LOG_DEBUG,true,__FUNCTION__,"Send Packet nettype="<<lwPackType<<"\tIP="<<GetIPString(aIPAddress)<<":"<<ntohs(awIPPort));\
	
	STRU_SENDDATA_UDP lstruSendDataUDP;
	
	lstruSendDataUDP.mIPAddress = aIPAddress;
	lstruSendDataUDP.mwIPPort = awIPPort;
	lstruSendDataUDP.miPhoneID = aiSessionID;
	lstruSendDataUDP.mpData = apData;
	lstruSendDataUDP.mwLen = awLen;
	lstruSendDataUDP.mbEncrypt = abEncrypt;
	lstruSendDataUDP.mbNeedEcho = abNeedReport;
	lstruSendDataUDP.mbNeedReport = abNeedReport;
	lstruSendDataUDP.mbNeedTimeStampCheck = abNeedTimeStampCheck;
	lstruSendDataUDP.mbySameSendCount = abySameSendCount;
	lstruSendDataUDP.mbySendCount = abySendCount;
	lstruSendDataUDP.mwTrySendSecond = awTrySendSecond;
	lstruSendDataUDP.mbUseSendThread=FALSE;
	lstruSendDataUDP.mbNeedEcho=abNeedEcho;
	return apInterface->SendData(&lstruSendDataUDP);
};

#endif
