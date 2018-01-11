#ifndef __WSMGR_H__
#define __WSMGR_H__

#include "network/ITransProtocolMgr.h"
#include "WebSocketMgr.h"
//////////////////////////////////////////////////////////////
class CWSMgr: public ITransProtocolMgr
{
public:
    CWSMgr();
	virtual ~CWSMgr();

public:
	//BaseInterface
	// 函数说明： 打开TransmitMgr操作，实际是初始化整个接收
	//           网络处理模块
	virtual INT Open();

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

	// 函数说明： 得到本机绑定端口
	virtual IPTYPE GetBindIP();

	// 函数说明： 得到本机绑定端口
	virtual WORD GetBindPort();

	//函数说明：设置关联指针
	virtual bool AddRelationPtr(void*apSocketInfo,void* apRelationPtr);

	//函数说明：设置关联指针
	virtual bool DelRelationPtr(void*apSocketInfo,void* apRelationPtr);

	// 函数说明： 发送数据
	virtual INT SendData(STRU_SENDDATA_BASE *apstruSendData);

	virtual INT SendData(void* apSocketInfo,STRU_SENDDATA_BASE *apstruSendData);

	virtual INT SendRawData(STRU_SENDDATA_BASE *apstruSendData);
	// 函数说明： 连接一个指定的地址
	virtual INT ConnectTo(const SESSIONID &  aiPeerID, IPTYPE aiIPAddr, WORD awPort);
	
	// 函数说明： 关闭一个连接
	virtual INT DisConnectTo(SOCKET ahSocket);

	// 函数说明： 得到本地网络统计信息
	virtual INT GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo);
	
	// 处理函数
	virtual	void Processs();

private:
	ITransProtocolMgr* mpImp;
};


#endif // __WSMGR_H__
