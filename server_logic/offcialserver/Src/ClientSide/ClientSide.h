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
#include "network/packet/client/OffcialServerPackDef.h"

#include "network/packet/PackTypeDef.h"
#include "network/packet/ClientPack.h"
#include "network/packet/ServerPack.h"
#include "ServerConn/UserConMgr.h"

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
	TCP_COMET_MAP_INIT(CClientSide)
	TCP_PACKET_MAP_BEGIN()
		TCP_PACKET_MAP(CClientSide, STRU_CLIENT_FANS_SEND_MESSAGE_RQ, FansSendMessageRQ)
		TCP_PACKET_MAP(CClientSide, STRU_SERVER_FANS_MESSAGE_RS, FansMessageRS)
		TCP_PACKET_MAP(CClientSide, STRU_CLIENT_OFFCIALR_SEND_MESSAGE_RQ, OffcialSendMessageRQ)
		TCP_PACKET_MAP(CClientSide, STRU_CLIENT_OFFCIALR_GET_MESSAGE_RQ, OffcialGetOfflineMessageRQ)
		TCP_PACKET_MAP(CClientSide, STRU_CLIENT_FANS_GET_MESSAGE_RQ, FansGetOfflineMessageRQ)
		TCP_PACKET_MAP(CClientSide, STRU_CLIENT_OFFCIALR_SEND_SYS_MESSAGE_RQ, OffcialSendSysMessageRQ)
		TCP_PACKET_MAP(CClientSide, STRU_CLIENT_OFFCIALR_SEND_SOME_SYS_MESSAGE_RQ, OffcialSendSomeSysMessageRQ)
		TCP_PACKET_MAP(CClientSide, STRU_CLIENT_OFFCIALR_SEND_ONE_SYS_MESSAGE_RQ, OffcialSendOneSysMessageRQ)
		TCP_PACKET_MAP(CClientSide, STRU_CLIENT_FANS_GET_SYS_MESSAGE_RQ, FansGetSysMessageRQ)
		TCP_PACKET_MAP(CClientSide, STRU_CLIENT_OFFCIALR_SEND_ONE_MSG_RQ, OffcialSendOneMsgRQ)
		TCP_PACKET_MAP(CClientSide, STRU_CLIENT_OFFCIALR_SEND_SOME_MSG_RQ, OffcialSendSomeMessageRQ)
		TCP_PACKET_MAP(CClientSide, STRU_SERVER_OFFCIALR_PRIVATE_MESSAGE_RS, FansPrivateMsgRS)
	TCP_PACKET_MAP_END()
private:
	INT FansSendMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT FansMessageRS(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT OffcialSendMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT OffcialGetOfflineMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT FansGetOfflineMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT OffcialSendSysMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT OffcialSendSomeSysMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT FansGetSysMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT OffcialSendOneMsgRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT OffcialSendSomeMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT OffcialSendOneSysMessageRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT FansPrivateMsgRS(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);

private:
	bool mbOpen;	//  ClientSide是否Open
	CServerMgr*			mpServerMgr;
	std::map<uint64, TEXTMSGID> messageId_;
	std::map<TEXTMSGID, int> sys_messageId_;
};

//////////////////////////////////////////////////////////////////////////

#endif //