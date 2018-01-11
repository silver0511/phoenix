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

#include "network/packet/client/GcServerPackDef.h"
#include "network/packet/client/SysPackDef.h"

#include "network/packet/PackTypeDef.h"
#include "network/packet/ClientPack.h"
#include "network/packet/ServerPack.h"
#include "ServerConn/UserConMgr.h"
#include "TaskThreadPoll.h"
#include "rjson/JsonParser.h"

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
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_LIST_RQ, GroupListRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_CREATE_RQ, GroupCreateRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_DETAIL_INFO_RQ, GroupDetailInfoRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_MODIFY_ChANGE_RQ, GroupModifyChangeRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_LEADER_CHANGE_RQ, GroupLeaderChangeRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_REMARK_DETAIL_RQ, GroupRemarkDetailRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_LIST_IDS_RQ, GroupListIdsRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_MESSAGE_STATUS_RQ, GroupMessageStatusRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_TYPE_LIST_RQ, GroupTypeListRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_GET_USER_REMARK_NAME_RQ, GroupGetUserRemarkNameRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_SAVE_CHANGE_RQ, GroupSaveChangeRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GROUP_SCAN_RQ, GroupScanRQ)
		TCP_PACKET_MAP(CClientSide, STRU_GET_BATCH_GROUP_INFO_RQ, GroupGetBatchInfoRQ)
	TCP_PACKET_MAP_END()
private:
	INT GroupListRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupGetBatchInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupCreateRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupDetailInfoRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupModifyChangeRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupLeaderChangeRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupScanRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupRemarkDetailRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupListIdsRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupMessageStatusRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupSaveChangeRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupTypeListRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);
	INT GroupGetUserRemarkNameRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen);

	// 解析通用字符
	UINT64 GetJsonID(RJsonValue &jsondata, string strKey);
public:

	INT OnGroupAddUserChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 msg_time,
								string& operate_user_name, UINT64 message_id,
								UINT64 message_old_id, RJsonValue& member_change_list,
								string& group_modify_content,
								UINT64 user_id_initial, INT big_msg_type);
	
	INT OnGroupKickUserChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 msg_time,
								string& operate_user_name, UINT64 message_id,
								UINT64 message_old_id, RJsonValue& member_change_list);
	
	INT OnGroupCreateUserModifyChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 msg_time,
								string& operate_user_name, UINT64 message_id,
								UINT64 message_old_id, RJsonValue& member_change_list,
								INT group_max_count, INT group_add_max_count);

	INT OnGroupStrChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								INT big_msg_type, UINT64 msg_time,
								string& operate_user_name, UINT64 message_id,
								UINT64 message_old_id, string& str_change_content);
								
	INT OnGroupDetailInfoRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 group_id, INT group_member_index,
								INT nSpan);
	INT OnGroupListRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								INT group_list_index, INT nSpan);

	INT OnGroupListIdsRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								INT group_list_index, INT nSpan);

	INT OnGroupLeaderChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform);

	INT OnGroupRemarkDetailRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform);

	INT HandleFailedResultFromHttp(WORD pack_type, SESSIONID aiCometID, USERID aiUserID, 
										INT aiPackSessionID, BYTE aiPlatform);

	INT OnGroupMessageStatusRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 group_id, BYTE message_status);

	INT OnGroupSaveChangeRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 group_id, BYTE message_status);

	INT OnGroupGetUserRemarkNameRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform,
								UINT64 group_id);

	INT OnGroupScanRQ(RJDocument& jsoncmd, 
								SESSIONID aiCometID, USERID aiUserID, 
								INT aiPackSessionID, BYTE aiPlatform, UINT64 user_id_share);

	INT OnGroupGetBatchInfoRQ(RJDocument& jsoncmd, 
									SESSIONID aiCometID, USERID aiUserID, 
									INT aiPackSessionID, BYTE aiPlatform);
private:
	bool mbOpen;	//  ClientSide是否Open
	CServerMgr*			mpServerMgr;
	CTaskThreadPoll m_taskthreadpoll;
};

//////////////////////////////////////////////////////////////////////////

#endif //