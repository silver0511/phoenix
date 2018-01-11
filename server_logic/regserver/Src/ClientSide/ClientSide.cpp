#include <ServerUtilMgr.h>
#include "stdafx.h"

#include "ClientSide.h"
#include "../ServerMgr.h"
#include "SmsVerifyTask.h"
#include "rjson/JsonParser.h"

// -----------------------------

CClientSide::CClientSide(CServerMgr *apoServerMgr)
{
	assert(apoServerMgr);
	InitFuncMap();
	mpServerMgr = apoServerMgr;
	mbOpen = false;
}

CClientSide::~CClientSide()
{
	mbOpen = false;
}

bool CClientSide::open()
{
	STRU_OPTION_TCP lstruTcpOpt;
	strncpy(lstruTcpOpt.mszAppName, g_server_name.c_str(),DEF_MAX_APP_NAME);
	lstruTcpOpt.miLogLevel = mpServerMgr->moConfig.miTCPLogLevel;
	lstruTcpOpt.mbIsServer = TRUE;
	lstruTcpOpt.muiMaxConnCount =   mpServerMgr->moConfig.miMaxConnCount;
	lstruTcpOpt.mulIPAddr = goLocalServerInfo.mlServerIP;
	lstruTcpOpt.mwPort = goLocalServerInfo.mwServerPort;
	lstruTcpOpt.mbyDealThreadCount =  mpServerMgr->moConfig.miDealThreadCount;
	lstruTcpOpt.mbyRecvThreadCount = mpServerMgr->moConfig.miRecvThreadCount;

	lstruTcpOpt.miMaxRecvPackageCount = mpServerMgr->moConfig.miMaxRecvPackageCount;
	lstruTcpOpt.miMaxSendPackageCount = mpServerMgr->moConfig.miMaxSendPackageCount;
	lstruTcpOpt.mbUseRelationPtr = true;
	lstruTcpOpt.muiKeepAlive = 0;

    if (!m_taskthreadpoll.Init(mpServerMgr->moConfig.m_taskthreadcnt))
    {
        LOG_TRACE(LOG_ERR,false,__FUNCTION__," Initialize taskthreadpoll error.");
        return false;
    }


    if(!ClientPack::GetInstance().Open(this, lstruTcpOpt, ENUM_CONNECT_MODE_TCP))
	{
		LOG_TRACE(LOG_ERR,true,__FUNCTION__," Initialize tcp error.");
		return false;
	}

	printf("CClientSide::open end ip:%s, port:%d\n", GetIPString(lstruTcpOpt.mulIPAddr).c_str(), ntohs(lstruTcpOpt.mwPort));
	mbOpen = true;
	return true;
}


INT CClientSide::OnSendDataError(IPTYPE aiIP,WORD awPort, BYTE* apData,WORD awLen)
{
	return 0;
}

INT CClientSide::OnError(UINT aiErrType)
{
	return 0;
}

//
VOID CClientSide::CheckEvent(bool abIsNormal)
{
	if(abIsNormal != true)
		return ;
	if(mpServerMgr == NULL)
		return;
	mpServerMgr->OnTimeCheck();

	ClientPack::GetInstance().DisplayStatisticsInfo();

    static uint32 siSecCount = 0;

    if (0 != mpServerMgr->moConfig.m_statinterval)
    {
        if (siSecCount > mpServerMgr->moConfig.m_statinterval)
        {
            for (int i = 0; i < mpServerMgr->moConfig.m_taskthreadcnt; i++)
            {
                if (m_taskthreadpoll.GetThreadQueueSize(i) > 10000)
                {
                    LOG_TRACE(LOG_CRIT, true, __FUNCTION__, "task"<< i <<  "queuecnt"  <<  m_taskthreadpoll.GetThreadQueueSize(i));
                }
            }
            siSecCount = 0;
        }
        else
        {
            siSecCount++;
        }
    }
}


//TCP连接建立
void CClientSide::OnSessionConnected(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Connected " 
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}

//TCP连接建立
void CClientSide::OnSessionConnectedEx(const SESSIONID& aiPeerID,void* apSocketInfo, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	CUser *lpUser = mpServerMgr->moUserConMgr.MallocRefObj();

	lpUser->mhSocket = ahSocket;
	lpUser->miIPAddr = aiIP;
	lpUser->miIPPort = awPort;
	lpUser->mpSocketInfo = apSocketInfo;
	lpUser->IncreaseRef();

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Connected "
			<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
			<<" Socket="<< ahSocket
			<<" SocketInfoPtr="<< apSocketInfo
			<<" UserRefCount="<< lpUser->GetRefCount()
	);
	ClientPack::GetInstance().AddRelationPtr(apSocketInfo,(void*)lpUser);
}

void CClientSide::OnSessionConnectFailure(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Connected " 
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<<" Socket="<< ahSocket
		);
}
//客户端关闭连接
void CClientSide::OnSessionClose(const SESSIONID& aiPeerID, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Disconnected "
		<<" Errcode="<<aiPeerID
		<<" IP=" << GetIPString(aiIP)<< ":"<< ntohs(awPort)
		<< " Socket="<< ahSocket
		);
}
void CClientSide::OnSessionCloseEx(const SESSIONID & aiPeerID,void*apRelationPtr, SOCKET ahSocket,IPTYPE aiIP,WORD awPort)
{
	CUser *lpUser = (CUser*)apRelationPtr;
	mpServerMgr->moUserConMgr.delUser(lpUser);
	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "TCP Disconnected  delete user"
			<< "ahSocket=" << ahSocket
			<<" ,IP="<< GetIPString(aiIP)
			<<":"<< ntohs(awPort)
			<<" SocketInfoPtr="<< lpUser->mpSocketInfo
			<<" lpUserPtr="<< (void*)(lpUser)
	);
}

INT CClientSide::OnNewRecvData(SOCKET ahSocket, void* apRelationPtr,IPTYPE aiIP,WORD awPort, BYTE* apData,WORD awLen)
{
	CHECK_INVALID()
	COMET_2_BACK_UNPACK_HEAD()

    if(loHeadPackage.mwPackType == NEW_DEF_ACK_ID)
	{
		lpUser->miCometID = loHeadPackage.miCometID;
		mpServerMgr->moUserConMgr.addUser(lpUser);
		return 1;
	}

	TCP_PACKET_DISTRIBUTE(loHeadPackage.mwPackType, loHeadPackage.miUserID, loHeadPackage.miCometID)
}

// 包关联
void CClientSide::InitFuncMap()
{
    TCP_PACKET_INIT()
}

INT CClientSide::SmsValidRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
	GENERAL_UNPACK_BODY(STRU_SMS_VALID_RQ, FALSE)
	GENERAL_PACK_SAME_BEGIN()

    INT64 cur_time = CSystem::GetMicroSystemTime();
    string verify_info = mpServerMgr->mpRedisUtil->GetAccountVerityInfo(E_ACCOUNT_MOBILE, to_string(t_fb_data->mobile()));
    if(!verify_info.empty())
    {
        RJ_CREATE_EMPTY_DOCUMENT(l_document);
        JsonParse::parse(l_document, verify_info.c_str());
        int next_send_time;
        JsonParse::get(l_document, "next_send_time", next_send_time);
        if(cur_time < next_send_time)
        {
            TEMPLATE_GENERAL_FAIL_RESULT(STRU_SMS_VALID_RS::ENUM_TYPE,
                                         MAKE_ERROR_RESULT(RET_VERIFY_CODE_FREQUENCY))
            return 1;
        }
    }


    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "verify_info = " << verify_info);

    int verify_code = cur_time % 10000;
    if(verify_code < 10000)
    {
        verify_code += 10000;
    }
    RJ_CREATE_O_DOCUMENT(l_verify_doc, l_allocator)
    JsonParse::add<int>(l_verify_doc, "verify_code", verify_code, l_allocator);
    JsonParse::add<int>(l_verify_doc, "next_send_time", (cur_time/1000000) + SMS_SEND_INTERVAL, l_allocator);
    verify_info = "";
    JsonParse::to_string(l_verify_doc, verify_info);

    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "new_verify_info = " << verify_info);
    mpServerMgr->mpRedisUtil->SetAccountVerityCode(E_ACCOUNT_MOBILE, to_string(t_fb_data->mobile()), verify_info);
    mpServerMgr->mpRedisUtil->ExpireAaccountVerityCode(E_ACCOUNT_MOBILE, to_string(t_fb_data->mobile()), SMS_VERITY_VALID_TIME);


    SmsVerifyTask *p_sms_verify_task = new SmsVerifyTask();
    p_sms_verify_task->m_nation_code = to_string(t_fb_data->nation_code());
    p_sms_verify_task->m_mobile = to_string(t_fb_data->mobile());
    p_sms_verify_task->verify_code = to_string(verify_code);
    p_sms_verify_task->valid_time = to_string(SMS_VERITY_VALID_TIME);
    m_taskthreadpoll.AddTask(p_sms_verify_task);

	STRU_SMS_VALID_RS loSendPack;
	loSendPack.fbbuf = syspack::CreateT_SMS_VALID_RS(loSendPack.fbbuilder, &s_rs_head);
	Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);

	return 1;
}


INT CClientSide::RegRQ(WORD awPackType, USERID aiUserID, SESSIONID aiCometID, void* apRelationPtr, BYTE * apData, WORD awLen)
{
    GENERAL_UNPACK_BODY(STRU_REG_RQ, FALSE)
    GENERAL_PACK_SAME_BEGIN()

    uint64_t account_type = t_fb_data->account_info()->account_type();
    uint64_t target_user_id = t_fb_data->account_info()->target_user_id();
    string target_user_token = t_fb_data->account_info()->target_user_token()->str();
    string target_user_name = "";
    if(NULL != t_fb_data->account_info()->target_user_token())
    {
        target_user_name = t_fb_data->account_info()->target_user_token()->str();
    }
    INT64 cur_time = CSystem::GetMicroSystemTime();
    string verify_info = mpServerMgr->mpRedisUtil->GetAccountVerityInfo(account_type, to_string(target_user_id));
    if(verify_info.empty())
    {
        TEMPLATE_GENERAL_FAIL_RESULT(STRU_REG_RS::ENUM_TYPE,
                                     MAKE_ERROR_RESULT(RET_REG_FAILED))
        return 1;
    }


    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "verify_info = " << verify_info << " account_type = " << account_type
                                            << " target_user_id = " << target_user_id
                                            << " target_user_token = " << target_user_token);
    RJ_CREATE_EMPTY_DOCUMENT(l_document);
    JsonParse::parse(l_document, verify_info.c_str());
    int verify_code;
    JsonParse::get(l_document, "verify_code", verify_code);

    switch (account_type)
    {
        case E_ACCOUNT_MOBILE:
            {
                if(to_string(verify_code) != target_user_token)
                {
                    TEMPLATE_GENERAL_FAIL_RESULT(STRU_REG_RS::ENUM_TYPE,
                                                 MAKE_ERROR_RESULT(RET_REG_FAILED))
                    return 1;
                }
            }
            break;
        default:
            {
                TEMPLATE_GENERAL_FAIL_RESULT(STRU_REG_RS::ENUM_TYPE,
                                             MAKE_ERROR_RESULT(RET_REG_FAILED))
                return 1;
            }
    }

    string str_user_id = mpServerMgr->mpRedisUtil->HGetAccountUser(account_type, to_string(target_user_id));
    if(str_user_id.empty())
    {
        str_user_id = mpServerMgr->mpRedisUtil->SPopUserID();
    }

    if(str_user_id.empty())
    {
        TEMPLATE_GENERAL_FAIL_RESULT(STRU_REG_RS::ENUM_TYPE,
                                     MAKE_ERROR_RESULT(RET_REG_FAILED))
        return 1;
    }

    USERID li_user_id = ServerUtilMgr::ChangeStringToUll(str_user_id);
    if(li_user_id <= 0)
    {
        TEMPLATE_GENERAL_FAIL_RESULT(STRU_REG_RS::ENUM_TYPE,
                                     MAKE_ERROR_RESULT(RET_REG_FAILED))
        return 1;
    }

    string reg_password = random_str_keys(32);
    RJ_CREATE_O_DOCUMENT(l_pwd_doc, l_allocator)
    JsonParse::add(l_pwd_doc, "password", reg_password, l_allocator);
    JsonParse::add<uint64_t>(l_pwd_doc, "end_time", (cur_time/1000000 + REG_PASSWORD_VALID_TIME), l_allocator);
    string password_info = "";
    JsonParse::to_string(l_pwd_doc, password_info);
    mpServerMgr->mpRedisUtil->HSetRegPassWord(li_user_id, password_info);
    mpServerMgr->mpRedisUtil->HSetAccountUser(account_type, to_string(target_user_id), li_user_id);
    STRU_REG_RS loSendPack;
    flatbuffers::Offset<commonpack::ACCOUNT_INFO> account_info = commonpack::CreateACCOUNT_INFODirect(loSendPack.fbbuilder,
                                                                                 t_fb_data->account_info()->account_type(),
                                                                                 t_fb_data->account_info()->target_user_id(),
                                                                                 target_user_token.c_str(),
                                                                                 target_user_name.c_str());
    loSendPack.fbbuilder.Finish(account_info);
    loSendPack.fbbuf = syspack::CreateT_REG_RSDirect(loSendPack.fbbuilder, &s_rs_head, account_info, li_user_id, reg_password.c_str());

    Send2Comet(loSendPack.GetPackType(), loSendPack, aiUserID, lpUser);
    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "rs password_info= " << password_info << " li_user_id = " << li_user_id);
    return 1;
}
