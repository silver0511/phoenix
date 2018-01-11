// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: IProxyMgr.h
// 创 建 人: 史云杰
// 文件说明: 转发传输节点接口
// $_FILEHEADER_END *****************************


#ifndef __IPROXYMGR_H
#define __IPROXYMGR_H

#include "./ITransMgrCallBack.h"
#include "./ITransProtocolMgr.h"

DLL_EXPORT INT CreateNetProxy(void** appInterface);


class  IProxyMgrCallBack:public ITransMgrCallBack
{       
public:
	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnectSuccess
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 连接对端成功
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnectSuccess(SOCKET ahStubSocket,IPTYPE aiStubIP,WORD awStubPort,SOCKET ahPeerSocket,IPTYPE aiPeerIP,WORD awPeerPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionConnectSuccess
	// 函数参数：session连接成功
	// 返 回 值： 无
	// 函数说明： 连接对端失败
	// $_FUNCTION_END ********************************
	virtual void OnSessionConnectFailure(SOCKET ahStubSocket,IPTYPE aiStubIP,WORD awStubPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnNetFluxStat
	// 函数参数：网络统计
	// 返 回 值： 无
	// 函数说明： 统计每个socket的流量
	// $_FUNCTION_END ********************************
	virtual void OnNetFluxStat(SOCKET ahStubSocket,IPTYPE aiStubIP,WORD awStubPort,CNetworkFluxStat &aoFluxStat) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： OnSessionCloseNetFluxStat
	// 函数参数：在sessionclose时候网络统计
	// 返 回 值： 无
	// 函数说明： 统计每个socket的流量
	// $_FUNCTION_END ********************************
	virtual void OnSessionCloseNetFluxStat(SOCKET ahStubSocket,IPTYPE aiStubIP,WORD awStubPort,CNetworkFluxStat &aoFluxStat) = 0;

};
struct STRU_FLUX_STAT;
class IProxyMgr
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
	virtual INT Init(STRU_OPTION_BASE *apstruOption,
		IProxyMgrCallBack *apProxyMgrCallback) = 0;


	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Init
	// 函数参数： aiValue 加密类型
	//			 
	// 返 回 值： 
	// 函数说明： 初始化加密
	// $_FUNCTION_END ********************************
	virtual void InitEncrypt(int8 aiEncryptType)=0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： SendData
	// 函数参数： STRU_SENDDATA_BASE *apstruSendData 数据发送设置
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 发送数据
	// $_FUNCTION_END ********************************
	virtual INT SendData(STRU_SENDDATA_BASE *apstruSendData) = 0;

	virtual INT ConnectProxy(SOCKET ahStubSocket,
		IPTYPE aiStubIP,WORD awStubPort,
		IPTYPE aiIPAddr, WORD awPort) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： Disconnect
	// 函数参数： SOCKET ahSocket
	// 返 回 值： 1是成功 -1是失败
	// 函数说明： 关闭
	// $_FUNCTION_END ********************************
	virtual INT Disconnect(SOCKET ahSocket) = 0;

	// $_FUNCTION_BEGIN ******************************
	// 函数名称： GetNetStatisticsInfo
	// 函数参数：
	// 返 回 值： 
	// 函数说明： 得到本地网络统计信息
	// $_FUNCTION_END ********************************
	virtual INT GetNetStatisticsInfo(STRU_STATISTICS_BASE *astruInfo) = 0;

	virtual INT GetSendFluxInfo(SOCKET ahSocket, STRU_FLUX_STAT *astruInfo) = 0;

	virtual INT GetRecvFluxInfo(SOCKET ahSocket, STRU_FLUX_STAT *astruInfo) = 0;

};

#endif //__ITCPPROXYMGR_H