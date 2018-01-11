// $_FILEHEADER_BEGIN ***************************
// 版权声明: xxxxx版权所有
//			  
// 文件名称: ServerSide.cpp
// 创 建 人: 史云杰
// 文件说明: 处理客户端链接处理器
// $_FILEHEADER_END *****************************

#include "stdafx.h"

#include "network/packet/PackTypeDef.h"
#include "ServerSide.h"
#include "../ServerMgr.h"
#include "network/packet/client/SysPackDef.h"

// --------------------------------------------------------

CServerSide::CServerSide(CServerMgr* apoServerMgr)
{
	U9_ASSERT(apoServerMgr);
	mpServerMgr = apoServerMgr;

	InitCallBack();
}


CServerSide::~CServerSide()
{
}

void CServerSide::InitCallBack()
{
	//user
	ServerPack::GetInstance().AddRouter(NEW_DEF_SERVER_DISCON_ID, USER_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_USER_INFO_RQ, USER_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_USER_CHANGE_RQ, USER_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_USERLST_INFO_RQ, USER_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_USER_COMPLAINT_RQ, USER_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_ME_INFO_RQ, USER_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CHANGE_MOBILE_RQ, USER_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CHANGE_MAIL_RQ, USER_SERVER);
	//sc
	ServerPack::GetInstance().AddRouter(NEW_DEF_CHAT_CLIENT_SEND_MESSAGE_RQ, SC_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CHAT_SERVER_SEND_MESSAGE_RS, SC_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CHAT_GET_OFFLINE_MESSAGE_RQ, SC_SERVER);
    ServerPack::GetInstance().AddRouter(NEW_DEF_BUSINESS_GETOFFLINEMSGS_RQ, SC_SERVER);
    ServerPack::GetInstance().AddRouter(NEW_DEF_BUSINESS_GETOFFLINECNT_RQ, SC_SERVER);

	//gc
	ServerPack::GetInstance().AddRouter(NEW_DEF_GROUP_CLIENT_SEND_MESSAGE_RQ, GC_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_GROUP_GET_OFFLINE_MESSAGE_RQ, GC_SERVER);

	// group
	ServerPack::GetInstance().AddRouter(NEW_DEF_GROUP_LIST_RQ, GROUP_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_GROUP_CREATE_RQ, GROUP_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_GROUP_DETAIL_INFO_RQ, GROUP_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_GROUP_MODIFY_CHANGE_RQ, GROUP_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_GROUP_LEADER_CHANGE_RQ, GROUP_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_GROUP_REMARK_DETAIL_RQ, GROUP_SERVER);

	//fd
	ServerPack::GetInstance().AddRouter(NEW_DEF_FRIEND_LIST_RQ, FD_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_FRIEND_ADD_RQ, FD_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_FRIEND_DEL_RQ, FD_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_FRIEND_REMARK_RQ, FD_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_FRIEND_CONFIRM_RQ, FD_SERVER);
    ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_FRIEND_BLACKLIST_RQ, FD_SERVER);
    ServerPack::GetInstance().AddRouter(NEW_DEF_FRIEND_UPDATE_RQ, FD_SERVER);

	//ec
    ServerPack::GetInstance().AddRouter(NEW_DEF_BUSINESS_GETRECENTUSERS_RQ, EC_SERVER);
    ServerPack::GetInstance().AddRouter(NEW_DEF_BUSINESS_GETRECENTMSGS_RQ, EC_SERVER);
    ServerPack::GetInstance().AddRouter(NEW_DEF_BUSINESS_GETLASTMSGS_RQ, EC_SERVER);    
	ServerPack::GetInstance().AddRouter(NEW_DEF_BUSINESS_LOGIN_ID, EC_SERVER);
    ServerPack::GetInstance().AddRouter(NEW_DEF_BUSINESS_GETRECENTORDERS_RQ, EC_SERVER);
    ServerPack::GetInstance().AddRouter(NEW_DEF_BUSINESS_ISUSERBUSY_RQ, EC_SERVER);
    ServerPack::GetInstance().AddRouter(NEW_DEF_BUSINESS_SETPROMPTS_RQ, EC_SERVER);
    ServerPack::GetInstance().AddRouter(NEW_DEF_BUSINESS_GETPROMPTS_RQ, EC_SERVER);

	//offical
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_FANS_SEND_MESSAGE_RQ, OFFCIAL_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_OFFCIAL_SEND_MESSAGE_RQ, OFFCIAL_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_SERVER_FANS_MESSAGE_RS, OFFCIAL_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_OFFCIAL_GET_OFFLINE_MESSAGE_RQ, OFFCIAL_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_FANS_GET_OFFLINE_MESSAGE_RQ, OFFCIAL_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_OFFCIAL_SEND_SYS_MESSAGE_RQ, OFFCIAL_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_SYS_MESSAGE_RQ, OFFCIAL_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_SYS_MESSAGE_RQ, OFFCIAL_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_FANS_GET_SYS_MESSAGE_RQ, OFFCIAL_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_OFFCIAL_SEND_ONE_MESSAGE_RQ, OFFCIAL_SERVER);
	ServerPack::GetInstance().AddRouter(NEW_DEF_CLIENT_OFFCIAL_SEND_SOME_MESSAGE_RQ, OFFCIAL_SERVER);
}

BOOL CServerSide::OpenLogicServerPool()
{
	LOG_TRACE(LOG_ALERT, 0, __FUNCTION__, "TCP connect logic user start");

	std::map<EnumServerLogicType, std::list<STRU_SERVER_INFO_BASE> >::iterator map_iter;
	for(map_iter = mMapServerLogicConfig.begin(); map_iter != mMapServerLogicConfig.end(); ++map_iter)
	{
		EnumServerLogicType enum_type = map_iter->first;
		std::list<STRU_SERVER_INFO_BASE> list_server_config_info = map_iter->second;

		LOG_TRACE(LOG_ALERT, false, __FUNCTION__, " enum_type = " << enum_type);
		std::list<STRU_SERVER_INFO_BASE>::iterator server_iter;
		for(server_iter = list_server_config_info.begin(); server_iter != list_server_config_info.end(); ++server_iter)
		{
			STRU_TCP_POOL_OPT lstruOption;
			strncpy(lstruOption.mszAppName, g_server_name.c_str(),DEF_MAX_APP_NAME);
			lstruOption.miLogLevel = mpServerMgr->moConfig.miLogLevel;
			lstruOption.mbUseRelink = TRUE;
			lstruOption.miTCPConnCount= mpServerMgr->moConfig.miTCPConnCount;//连接数
			lstruOption.miTCPMaxRecvPackageCount= mpServerMgr->moConfig.miTCPMaxRecvPackageCount;		//最大接收包数
			lstruOption.miTCPMaxSendPackageCount= mpServerMgr->moConfig.miTCPMaxSendPackageCount;		//最大接收包数

			lstruOption.miTCPIP=  server_iter->mlServerIP; //TCP地址
			lstruOption.miTCPPort= server_iter->mwServerPort; //TCP端口

			LOG_TRACE(LOG_ALERT, 0, __FUNCTION__, "TCP connect logic user"
					<<" IP=" << GetIPString(lstruOption.miTCPIP)<< ":"<< ntohs(lstruOption.miTCPPort)
			);

			if(!ServerPack::GetInstance().OpenServerPoolByConfig(this, lstruOption, enum_type))
			{
				LOG_TRACE(LOG_ALERT,true,__FUNCTION__," Initialize logic user tcp pool error.");
				return false;
			}
		}
	}

	return true;
}

bool CServerSide::open()
{
	if(!OpenLogicServerPool())
	{
		LOG_TRACE(LOG_CRIT,true,__FUNCTION__," Initialize TCP Logic User Pool error.");
		return false;
	}

	return true;
}


INT CServerSide::OnTCPPoolTransRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort, 
						   BYTE* apData,WORD awLen)
{
	return OnRecvData(ahSocket,aiIP,awPort,apData,awLen);
}

INT CServerSide::OnTCPPoolTransNewRecvData(SOCKET ahSocket, void *apRelationPtr,IPTYPE aiIP,WORD awPort, 
						   BYTE* apData,WORD awLen)
{
	return OnRecvData(ahSocket,aiIP,awPort,apData,awLen);
}

void CServerSide::OnTCPPoolSessionClose(const SESSIONID& aiPeerID,SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{ 
	LOG_TRACE(LOG_DEBUG, 0, __FUNCTION__, "Error TCP distconnect"
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}
void CServerSide::OnTCPPoolSessionCloseEx(const SESSIONID & aiPeerID,void* apSocketInfo,SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, 0, __FUNCTION__, "Error TCP distconnect"
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}
void CServerSide::OnTCPPoolSessionConnected(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, 0, __FUNCTION__, "TCP connect" <<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort) <<" Socket="<< ahSocket);

	STRU_ACK_ID loSendPack;
	ServerPack::GetInstance().SendAckToBackServer(ahSocket, loSendPack, goLocalServerInfo.mlServerID, aiIP, awPort);
}

void CServerSide::OnTCPPoolSessionConnectedEx(const SESSIONID& aiPeerID, void* apSocketInfo,SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, 0, __FUNCTION__, "TCP connect"
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}


INT CServerSide::Send2Client(WORD pack_type, CUser* p_user, BYTE *buffer, WORD buf_len, BOOL dis_connected)
{
	STRU_TRANS_RS tans_pack;
	tans_pack.ENUM_TYPE = pack_type;
	tans_pack.mpBuffer = buffer;
	tans_pack.mwLen = buf_len;
	INT result = ClientPack::GetInstance().Send2Client(pack_type, tans_pack, p_user->mpSocketInfo,
													p_user->GetCriticalSection(), dis_connected);
	if(result <= 0 && result != RET_SUCCESS)
	{
		LOG_TRACE(LOG_ERR, 0, __FUNCTION__, " send error type:"<< pack_type
														 <<" UserPtr="<<(void*)p_user
														 <<" Socket="<<p_user->mhSocket
														 <<" SocketInfoPtr="<< p_user->mpSocketInfo
														 << " IP="<< GetIPString(p_user->miIPAddr)
														 << ":"<< ntohs(p_user->miIPPort)
		);
	}

	return result;
}

INT CServerSide::OnRecvData(SOCKET ahSocket, IPTYPE aiIP,WORD awPort,BYTE* apData,WORD awLen)
{
	SERVER_UNPACK_BACK_2_COMET_HEAD()

	for (int user_index = 0; user_index < loHeadSvrPacket.miUserCount; ++user_index)
	{
		USERID user_id = loHeadSvrPacket.moUserList[user_index];
		CAutoReleaseRefObjMgrTemplate2<PHONEID,CUser>  loUser(mpServerMgr->moUserMgr, user_id);
		CUser* lpUser = loUser.GetPtr();
		if(NULL == lpUser)
		{
			LOG_TRACE(LOG_ERR, false, __FUNCTION__, "trans to client user_id = " << user_id << " not in comet");
			continue;
		}

		LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "trans to client success package_id = " << loHeadSvrPacket.mwPackType);
		if(loHeadSvrPacket.mwPackType == NEW_DEF_SERVER_DISCON_ID)
		{
			lpUser->miLoginCount++;
			Send2Client(loHeadSvrPacket.mwPackType, lpUser, apData, awLen, true);
		}
		else
		{
			Send2Client(loHeadSvrPacket.mwPackType, lpUser, apData, awLen);
		}
	}

	return 1;
}

VOID CServerSide::CheckEvent()
{
	ServerPack::GetInstance().DisplayStatisticsInfo();
}