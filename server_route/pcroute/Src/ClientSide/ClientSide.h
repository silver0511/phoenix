// $_FILEHEADER_BEGIN ***************************
// 版权声明: xxxxx版权所有
//			  
// 文件名称: ClientSide.cpp
// 创 建 人: 史云杰
// 文件说明: 处理客户端链接处理器
// $_FILEHEADER_END *****************************

#ifndef __CLIENTSIDE_H
#define __CLIENTSIDE_H

#include <map>
#include <set>
#include "network/packet/ClientPack.h"


#ifdef WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#endif //#ifdef WIN32

#include "common/CriticalSection.h"
#include "network/ITransProtocolMgr.h"
#include "network/ITransMgrCallBack.h"
#include "const/ErrorCodeDef.h"

class CServerMgr;

struct TLogInfo 
{
    SESSIONID t_serverid;
    int       t_logincnt; 
};

// --------------------------------------------------------------
class CClientSide: public ITransMgrCallBack
{
public:
	CClientSide(CServerMgr* apoServerMgr);
	~CClientSide(void);
	bool open();
	bool IsOpen() { return mbOpen; }
public:
	virtual INT OnSendDataError(IPTYPE aiIP,WORD awPort, BYTE* apData,WORD awLen);
	virtual INT OnError(UINT aiErrType);
	virtual VOID CheckEvent(bool abIsNormal);

	virtual void OnSessionClose(const SESSIONID& aiPeerID,SOCKET ahSocket,IPTYPE aiIP,WORD awPort);
	virtual void OnSessionCloseEx(const SESSIONID & aiPeerID,void*apRelationPtr,SOCKET ahSocket,IPTYPE aiIP,WORD awPort);
	virtual void OnSessionConnected(const SESSIONID& aiPeerID,SOCKET ahSocket,IPTYPE aiIP,WORD awPort);
	virtual void OnSessionConnectedEx(const SESSIONID& aiPeerID,void* apSocketInfo,SOCKET ahSocket,IPTYPE aiIP,WORD awPort);
	virtual void OnSessionConnectFailure(const SESSIONID & aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort);

public:

	bool Send2Comet(uint32 threadid, const SERVERID & serverid , BYTE * pdata,uint32 uidatalen);
    SOCKET FindRemoteSocketIdbyServerId(uint32 threadid, const SERVERID & serverid);
	
private:
	VOID Statistics();
	
public:	
	virtual INT OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen);
	virtual INT OnNewRecvData(SOCKET ahSocket, void* apRelationPtr, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen);
	void Getuseridlogins(std::vector<USERID> & userids, 
                             std::multimap<SERVERID,USERID>& szappserverid_userid_map,                                                  
                             std::set<SERVERID>& szappserverids,
                             USERID* pofflineuserlist,
                             uint16& offlineusercnt,
                             const std::string& apptype);
	
private:
	bool mbOpen;
	CServerMgr*			mpServerMgr;

	CCriticalSection    moRefCriticalSocketVec;
	std::map<SERVERID, std::vector<SOCKET> > m_server_socket_map;
	UINT                miCurSocketId;

	CCriticalSection    moRefCriticaluseridinfomap;
	u9_hash_map<USERID, TLogInfo> m_userid_loginfo;
};
#endif
