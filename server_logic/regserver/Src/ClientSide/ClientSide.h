#ifndef __CLIENTSIDE_H
#define __CLIENTSIDE_H

#include "const/ErrorCodeDef.h"

#include "network/packet/client/SysPackDef.h"
#include "network/packet/PackTypeDef.h"
#include "network/packet/ClientPack.h"
#include "network/packet/ServerPack.h"
#include "ServerConn/UserConMgr.h"
#include "TaskThreadPoll.h"

class CServerMgr;
// --------------------------------------------------------------
class CClientSide: public ITransMgrCallBack
{
public:
	CClientSide(CServerMgr* apoServerMgr);
	~CClientSide(void);
	void InitFuncMap();
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
			LOG_TRACE(LOG_ERR, 0, __FUNCTION__, " send error type:"<< awPackType
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
		LOG_TRACE(LOG_DEBUG,true,__FUNCTION__,"OnRecvData ininininin----");
		return 0;
	}
	virtual INT OnNewRecvData(SOCKET ahSocket, void* apRelationPtr,IPTYPE aiIP,WORD awPort, BYTE* apData,WORD awLen);

private:
	INT64 GetRand()
	{
		return rand();
	}
private:
	TCP_COMET_MAP_INIT(CClientSide)
	TCP_PACKET_MAP_BEGIN()
		TCP_PACKET_MAP(CClientSide, STRU_SMS_VALID_RQ, SmsValidRQ)
		TCP_PACKET_MAP(CClientSide, STRU_REG_RQ, RegRQ)
	TCP_PACKET_MAP_END()
private:
	INT SmsValidRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT RegRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
private:
	bool mbOpen;	//  ClientSide是否Open
	CServerMgr*			mpServerMgr;
    CTaskThreadPoll     m_taskthreadpoll;
};


//////////////////////////////////////////////////////////////////////////

#endif //