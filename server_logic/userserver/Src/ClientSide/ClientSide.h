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
#include "network/packet/PackTypeDef.h"
#include "network/packet/ClientPack.h"
#include "network/packet/ServerPack.h"
#include "ServerConn/UserConMgr.h"


enum KeyID
{
	key_user_name = 1,
	key_birthday,
	key_city,
	key_signature,
	key_mobile,
	key_nick_name,
	key_sex,
	key_mail,
	key_province,
};

enum ComplaintType
{
	regulations = 1,
	erotic_violence,
	false_fraud,
	harass,
	others,
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
	bool AllisNum(string str)  
	{  
		for (int i = 0; i < str.size(); i++)
		{
			int tmp = (int)str[i];
			if (tmp >= 48 && tmp <= 57)
			{
				continue;
			}
			else
			{
				return false;
			}
		} 
		return true;
	}  
private:
	TCP_COMET_MAP_INIT(CClientSide)
	TCP_PACKET_MAP_BEGIN()
		TCP_PACKET_MAP(CClientSide, STRU_SERVER_DISCON_ID, ServerDisconID)
		TCP_PACKET_MAP(CClientSide, STRU_GET_ME_INFO_RQ, GetMeInfoRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GET_USER_INFO_RQ, GetUserInfoRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GET_USERLST_INFO_RQ, GetUserlstInfoRQ)
		TCP_PACKET_MAP(CClientSide, STRU_UPDATE_USER_INFO_RQ, UpdateUserInfoRQ)
		TCP_PACKET_MAP(CClientSide, STRU_REGISTER_APNS_RQ, RegisterApnsRQ)
		TCP_PACKET_MAP(CClientSide, STRU_USER_COMPLAINT_RQ, UserComplaintRQ)
		TCP_PACKET_MAP(CClientSide, STRU_CHANGE_MAIL_RQ, ChangeMailRQ)
		TCP_PACKET_MAP(CClientSide, STRU_CHANGE_MOBILE_RQ, ChangeMobileRQ)
	TCP_PACKET_MAP_END()
private:
	INT ServerDisconID(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetMeInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetUserInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GetUserlstInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT UpdateUserInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT RegisterApnsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT UserComplaintRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT ChangeMailRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT ChangeMobileRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
private:
	bool mbOpen;	//  ClientSide是否Open
	CServerMgr*			mpServerMgr;
};


//////////////////////////////////////////////////////////////////////////

#endif //