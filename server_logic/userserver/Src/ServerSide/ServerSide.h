
// $_FILEHEADER_BEGIN ***************************
// 版权声明: xxxxx版权所有
//			  
// 文件名称: ServerSide.h
// 创 建 人: 史云杰
// 文件说明: 处理客户端链接处理器
// $_FILEHEADER_END *****************************
#ifndef __SERVERSIDE_H
#define __SERVERSIDE_H

#include <map>
#include "network/ITransProtocolMgr.h"
#include "network/ITransMgrCallBack.h"
#include "network/ITCPPoolTransMgr.h"
#include "ServerConn/UserConMgr.h"
#include "network/packet/ServerPack.h"
#include "network/packet/ClientPack.h"

class CServerMgr;

class CServerSide:ITCPConnPoolMgrCallback
{
public:
	CServerSide(CServerMgr* apoServerMgr);
	~CServerSide();
	bool open();

public:
	virtual INT OnTCPPoolTransRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort, 
		BYTE* apData,WORD awLen);
	virtual INT OnTCPPoolTransNewRecvData(SOCKET ahSocket, void *apRelationPtr,IPTYPE aiIP,WORD awPort, 
			BYTE* apData,WORD awLen);

	virtual void OnTCPPoolSessionClose(const SESSIONID& aiPeerID,SOCKET ahSocket,IPTYPE aiIP,WORD awPort);
	virtual void OnTCPPoolSessionCloseEx(const SESSIONID & aiPeerID,void* apRelationPtr,SOCKET ahSocket,IPTYPE aiIP,WORD awPort);
	virtual void OnTCPPoolSessionConnected(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort);
	virtual void OnTCPPoolSessionConnectedEx(const SESSIONID& aiPeerID,void* apSocketInfo, SOCKET ahSocket,IPTYPE aiIP,WORD awPort);
public:

	virtual INT OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
	{

	}

	virtual INT OnNewRecvData(SOCKET ahSocket, void* apRelationPtr,IPTYPE aiIP,WORD awPort, BYTE* apData,WORD awLen)
	{
		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "call back")
		return 1;
	}
	
private:
	void InitCallBack();
public:
private:
	CServerMgr		*mpServerMgr;
};

//////////////////////////////////////////////////////////////////////////

#endif //__TRANS_UDPSIDE_H__

