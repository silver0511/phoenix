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
#include "network/packet/client/EcServerPackDef.h"
#include "ServerConn/UserConMgr.h"
#include "TaskThreadPoll.h"
typedef int MOD;

struct TEcInof
{
    BYTE t_logintype;
    int  t_logincnt;
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
	TCP_PACKET_MAP(CClientSide, STRU_EC_GETFREEWAITER_RQ, GetFreeWaiterRQ)
	TCP_PACKET_MAP(CClientSide, STRU_EC_GETRECENTMSGS_RQ, GetRecentMsgsRQ)
	TCP_PACKET_MAP(CClientSide, STRU_EC_GETLASTMSGS_RQ, GetLastMsgsRQ)
	TCP_PACKET_MAP(CClientSide, STRU_EC_GETRECENTUSERS_RQ, GetRecentUsersRQ)
	TCP_PACKET_MAP(CClientSide, STRU_BUSINESS_UPDATE_LOGIN_ID, UpdateEcLoginRQ)
	TCP_PACKET_MAP(CClientSide, STRU_EC_GETRECENTORDERS_RQ, GetRecentOrdersRQ)
	TCP_PACKET_MAP(CClientSide, STRU_EC_ISUSERBUSY_RQ, GetIsUserBusyRQ)
	TCP_PACKET_MAP(CClientSide, STRU_EC_SETBUSINESS_RQ, SetBusinessRQ)
	TCP_PACKET_MAP(CClientSide, STRU_EC_GETBUSINESS_RQ, GetBusinessRQ)
	TCP_PACKET_MAP(CClientSide, STRU_EC_SETPROMPTS_RQ, SetPromptsRQ)
	TCP_PACKET_MAP(CClientSide, STRU_EC_GETPROMPTS_RQ, GetPromptsRQ)	
	TCP_PACKET_MAP_END()
	
private:	
	INT GetFreeWaiterRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetRecentMsgsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetLastMsgsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetOffLineMsgsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetRecentOrdersRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetOffLineCntRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetRecentUsersRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);	
	INT UpdateEcLoginRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetIsUserBusyRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
    INT SetBusinessRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
    INT GetBusinessRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
    INT SetPromptsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
    INT GetPromptsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	bool IsUserLogin(USERID userid);
	void GetLoginWaiter(std::vector<USERID>& ulwidlist, std::vector<USERID>& ulloginwidlist);
	INT GetRandom();
public:	
CServerMgr*			mpServerMgr;
	
private:
	bool mbOpen;	//  ClientSide是否Open	
	CTaskThreadPoll m_taskthreadpoll;

	CCriticalSection    moRefCriticalecloginstat;
	std::map<USERID, TEcInof>m_ecloginstat_map;
};


//////////////////////////////////////////////////////////////////////////

#endif //