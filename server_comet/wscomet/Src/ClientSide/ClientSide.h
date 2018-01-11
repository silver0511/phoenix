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
#include "network/packet/client/UserPackDef.h"
#include "network/packet/client/EcServerPackDef.h"
#include "network/packet/PackTypeDef.h"
#include "network/packet/ClientPack.h"
#include "network/packet/ServerPack.h"
#include "ClientConn/UserMgr.h"

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
	INT Send2Client(T &aoSendStruct,CUser* apUser,BOOL abDisconnected = FALSE)
	{
		INT result = ClientPack::GetInstance().Send2Client(aoSendStruct.GetPackType(), aoSendStruct, apUser->mpSocketInfo,
														apUser->GetCriticalSection(), abDisconnected);
		if(result <= 0 && result != RET_SUCCESS)
		{
			LOG_TRACE(LOG_ERR, 0, __FUNCTION__, " send error type:"<< aoSendStruct.GetPackType()
				<<" UserPtr="<<(void*)apUser
				<<" SocketInfoPtr="<< apUser->mpSocketInfo
				<< " IP="<< GetIPString(apUser->miIPAddr)
				<< ":"<< apUser->miIPPort
				);
			return -1;
		}

		return result;
	}

private:
	
public:
	virtual INT OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
	{
		return 0;
	}
	virtual INT OnNewRecvData(SOCKET ahSocket, void* apRelationPtr,IPTYPE aiIP,WORD awPort, BYTE* apData,WORD awLen)
	{
		CHECK_INVALID()
		CLIENT_UNPACK_HEAD()

		if(!lpUser->check())
		{
			KickUserID(lpUser, 0, false, RET_REQ_FAST_ERROR);
			return 0;
		}

		if (loHeadPackage.mwPackType == STRU_LOGIN_RQ::ENUM_TYPE)
		{
			return this->onRecvLoginRQ(loHeadPackage.mwPackType,ahSocket,apRelationPtr,aiIP,awPort,apData,awLen);
		}
		else if(NEW_DEF_HEART_RQ == loHeadPackage.mwPackType)
		{
			return this->onRecvHeartRQ(loHeadPackage.mwPackType,ahSocket,apRelationPtr,aiIP,awPort,apData,awLen);
		}
		else if(NEW_DEF_BUSINESS_SETLOGINSTATUS_RQ == loHeadPackage.mwPackType)
		{
			return this->onRecvBusinessSetLoginStatusRQ(loHeadPackage.mwPackType,ahSocket,apRelationPtr,aiIP,awPort,apData,awLen);
		}
		else
		{
			return this->onRecvTransmitRQ(loHeadPackage.mwPackType,ahSocket,apRelationPtr,aiIP,awPort,apData,awLen);
		}
	}
	


private:
	//////////////////////////////////////////////////////////////////////////
	INT onRecvLoginRQ(WORD awPackType, SOCKET ahSocket,void* apRelationPtr,IPTYPE aiIP, WORD awPort, BYTE * apData, WORD awLen);
	INT onRecvHeartRQ(WORD awPackType, SOCKET ahSocket,void* apRelationPtr,IPTYPE aiIP, WORD awPort, BYTE * apData, WORD awLen);
	INT onRecvBusinessSetLoginStatusRQ(WORD awPackType, SOCKET ahSocket,void* apRelationPtr,IPTYPE aiIP, WORD awPort, BYTE * apData, WORD awLen);
	INT onRecvTransmitRQ(WORD awPackType, SOCKET ahSocket,void* apRelationPtr,IPTYPE aiIP, WORD awPort, BYTE * apData, WORD awLen);
	INT KickUserID(CUser* client_user, const SESSIONID& comet_id, BOOL is_back, uint32 error_code);
	INT UpdateTimeID(USERID user_id, BYTE time_type);
	INT RouteSyncID(USERID user_id, BYTE b_type);
public:
private:
	bool mbOpen;	//  ClientSide是否Open
	CServerMgr*			mpServerMgr;
};


//////////////////////////////////////////////////////////////////////////

#endif //