#ifndef __CLIENTSIDE_H
#define __CLIENTSIDE_H

#include <map>

#ifdef WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#endif //#ifdef WIN32

#include "common/CriticalSection.h"
#include "const/ErrorCodeDef.h"
#include "network/packet/client/SysPackDef.h"
#include "network/packet/PackTypeDef.h"
#include "network/packet/ClientPack.h"
#include "network/packet/ServerPack.h"
#include "network/packet/client/FdServerPackDef.h"
#include "ServerConn/UserConMgr.h"
#include "TaskThreadPoll.h"

#define MAX_FD_LIST_CNT 50
#define MAX_FD_MSG_CNT 500

enum enFRIENDTYPE
{
	FD_FRIEND_OP = 0,
    FD_PEERCONFIRM_OP,
    FD_WAITCONFIRM_OP,
    FD_NEEDCONFIRM_OP,
    FD_OWNCONFIRM_OP,
    FD_REMARK_OP,
    FD_BLACK_OP,
    FD_PEER_DEL_OP,
    FD_OWN_DEL_OP,
    FD_INVALID_OP,
    FD_RESTOR_OP
};

class CServerMgr;
// --------------------------------------------------------------
class CClientSide: public ITransMgrCallBack
{
public:
	CClientSide(CServerMgr* apoServerMgr);
	~CClientSide(void);
	void IntiFuncMap();
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
	template<class T>
	INT Send2Comet(const WORD awPackType, T &aoSendStruct, const USERID &liUserID, CUser* apUser, BOOL abDisconnected = FALSE)
	{
		USERID user_id_list[5] = {0};
		user_id_list[0] =liUserID;
		INT result = ClientPack::GetInstance().Send2Comet(awPackType, 1, user_id_list, apUser->mhSocket, aoSendStruct);
		if(result <= 0 && result != RET_SUCCESS)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, " send error type:"<< awPackType
				<<" UserPtr="<<(void*)apUser
				<<" Socket="<<apUser->mhSocket
				<<" SocketInfoPtr="<< apUser->mpSocketInfo
				<< " IP="<< GetIPString(apUser->miIPAddr)
				<< ":"<< ntohs(apUser->miIPPort)
				);
			return -1;
		}

		return result;
	}
public:

	virtual INT OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
	{
		LOG_TRACE(LOG_NOTICE,true,__FUNCTION__,"OnRecvData ininininin----");
		return 0;
	}
	virtual INT OnNewRecvData(SOCKET ahSocket, void* apRelationPtr,IPTYPE aiIP,WORD awPort, BYTE* apData,WORD awLen);

private:
	TCP_COMET_MAP_INIT(CClientSide)
	TCP_PACKET_MAP_BEGIN()	
	TCP_PACKET_MAP(CClientSide, STRU_FRIEND_LIST_RQ, FriendListRQ)
	TCP_PACKET_MAP(CClientSide, STRU_FRIEND_REMARK_RQ, FriendRemarkRQ)
	TCP_PACKET_MAP(CClientSide, STRU_FRIEND_DEL_RQ, FriendDelRQ)
	TCP_PACKET_MAP(CClientSide, STRU_FRIEND_CLIENT_ADD_RQ, FriendClientAddRQ)
	TCP_PACKET_MAP(CClientSide, STRU_FRIEND_CLIENT_CONFIRM_RQ, FriendClientCONFIRMRQ)
	TCP_PACKET_MAP(CClientSide, STRU_CLIENT_FRIEND_BLACKLIST_RQ, FriendClientBlackListRQ)
	TCP_PACKET_MAP(CClientSide, STRU_FRIEND_UPDATE_RQ, FriendUpdateRQ)
	TCP_PACKET_MAP_END()
	
private:	
	INT FriendListRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT FriendRemarkRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT FriendDelRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT FriendClientAddRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT FriendClientCONFIRMRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT FriendClientBlackListRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT FriendUpdateRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetFriendExpiretime();
public:	
CServerMgr*			mpServerMgr;

private:
	bool mbOpen;	//  ClientSide是否Open	
	CTaskThreadPoll m_taskthreadpoll;
};


//////////////////////////////////////////////////////////////////////////

#endif //