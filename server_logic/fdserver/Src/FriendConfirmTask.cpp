#include "rjson/JsonParser.h"
#include "httpclient.h"
#include "common/Debug_log.h"
#include "network/packet/client/SysPackDef.h"
#include "network/packet/client/FdServerPackDef.h"
#include "ServerUtilMgr.h"
#include "ServerConn/UserConMgr.h"
#include "./ClientSide/ClientSide.h"
#include "ServerMgr.h"
#include "ServerToKafka.h"
#include "FriendConfirmTask.h"

void CFriendConfirmTask::DoForwardPackageFail()
{
    
    LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Forward package fail url = " << FRIEND_AGREE_API);

    int liResult = RET_SYS_PACK_TYPE_INVALID;
    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);

    STRU_PACKAGE_RS<T_FRIEND_CLIENT_CONFIRM_RS> loSendPack(NEW_DEF_CLINET_FRIEND_CONFIRM_RS);
    friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder c1 = friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_peer_user_id(m_friendid);
    c1.add_token(0);
    c1.add_result(liResult);
    loSendPack.fbbuf = c1.Finish();

    CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

    if(!lpUser)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
        return;
    }

    m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);

    return;
}


void CFriendConfirmTask::DealAgree()
{
    if (NULL == m_parm)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "m_parm is null");
        return;
    }

    CHttpClient* phttpclient =  (CHttpClient*) m_parm;

    std::string szUrl = FRIEND_AGREE_API + "?" + m_data;
    std::string szReponse;

    LOG_TRACE(LOG_INFO, true, __FUNCTION__, "http get url=" << szUrl);
	if (CURLE_OK != phttpclient->Get(szUrl.c_str(), szReponse))
	{	
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "http get error url=" << szUrl);	
        DoForwardPackageFail();
		return;
	}

    if (szReponse.empty())
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "http get error url=" << szUrl);
        DoForwardPackageFail();
        return;
    }
    
    RJDocument json_document;

    
    if (!JsonParse::parse(json_document, szReponse.c_str()))
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "json is error =" << szReponse);
        DoForwardPackageFail();
        return;
    }  
    
    STRU_PACKAGE_RS<T_FRIEND_CLIENT_CONFIRM_RS> loSendPack(NEW_DEF_CLINET_FRIEND_CONFIRM_RS);
    
    int liResult = json_document["responseCode"].GetInt();

    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);

    UINT64 ultoken = 0;

    if (liResult == -2147483648)
    {
        LOG_TRACE(LOG_INFO, true, __FUNCTION__, "receive msg:" << szReponse);
        ultoken = time(NULL);

        friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder c1 = friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder(loSendPack.fbbuilder);
        c1.add_s_rs_head(&s_rs_head);
        c1.add_peer_user_id(m_friendid);
        c1.add_token(ultoken);
        c1.add_result(0);
        loSendPack.fbbuf = c1.Finish();

        CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

        if(!lpUser)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
            return;
        }

        m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);

        //save own addconfirm operator token 
        flatbuffers::FlatBufferBuilder fbbuild;
        auto fbsremarkname = fbbuild.CreateString(m_remarkname.c_str());

        friendpack::T_FREIND_INFOBuilder friendinfobuild = friendpack::T_FREIND_INFOBuilder(fbbuild);
        friendinfobuild.add_user_id(m_friendid);
        friendinfobuild.add_remark_name(fbsremarkname);
        friendinfobuild.add_source_type(m_sourcetype);
        friendinfobuild.add_opt_type(FD_OWNCONFIRM_OP);
        friendinfobuild.add_op_time(ultoken);
        fbbuild.Finish(friendinfobuild.Finish());

        std::string strElement0;
        strElement0.assign((char *)fbbuild.GetBufferPointer(), fbbuild.GetSize());        
        m_pclientside->mpServerMgr->mpRedisUtil->ZaddFdMsg(m_aiUserID,ultoken,strElement0);
        m_pclientside->mpServerMgr->mpRedisUtil->DelFdRequestTime(m_aiUserID, m_friendid);

        //send serverconfirmrq to peeruser
        STRU_SERVER_PACKAGE_RQ<T_FRIEND_SERVER_CONFIRM_RQ> loServerSendPack(NEW_DEF_SERVER_FRIEND_CONFIRM_RQ);
        commonpack::S_RQ_HEAD s_rq_head(m_friendid, 
                                         m_aiPackSessionID,
        						         m_aiPlatform);

        loServerSendPack.fbbuf = friendpack::CreateT_FRIEND_SERVER_CONFIRM_RQDirect(loServerSendPack.fbbuilder,
        										&s_rq_head,
                                                m_nicknmae.c_str(),
                                                m_aiUserID,
        									    ultoken,
        										0);

        std::string IsNeedApns("1");
        CServerToKafka::GetInstance().DistributeMsgToScRoute(loServerSendPack.GetPackType(), 
        		m_friendid, 
        		loServerSendPack, IM_SC_TOPIC,IsNeedApns);

        //save peer del operator token
        flatbuffers::FlatBufferBuilder fbpeerbuild;
        friendpack::T_FREIND_INFOBuilder peerfriendinfobuild = friendpack::T_FREIND_INFOBuilder(fbpeerbuild);
        peerfriendinfobuild.add_user_id(m_aiUserID);
        peerfriendinfobuild.add_source_type(m_sourcetype);
        peerfriendinfobuild.add_opt_type(FD_PEERCONFIRM_OP);
        peerfriendinfobuild.add_op_time(ultoken);
        fbpeerbuild.Finish(peerfriendinfobuild.Finish());

        std::string strElement1;
        strElement1.assign((char *)fbpeerbuild.GetBufferPointer(), fbpeerbuild.GetSize());        
        m_pclientside->mpServerMgr->mpRedisUtil->ZaddFdMsg(m_friendid,ultoken,strElement1);              
    }
    else
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "receive msg:" << szReponse);
        friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder c1 = friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder(loSendPack.fbbuilder);
        c1.add_s_rs_head(&s_rs_head);
        c1.add_peer_user_id(m_friendid);
        c1.add_token(ultoken);
        c1.add_result(liResult);
        loSendPack.fbbuf = c1.Finish();

        CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

        if(!lpUser)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
            return;
        }

        m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);
    }

    return;   
}

void CFriendConfirmTask::DealRefuse()
{
    /*
    if (NULL == m_parm)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "m_parm is null");
        return;
    }

    CHttpClient* phttpclient =  (CHttpClient*) m_parm;

    std::string szUrl = std::string(DBSERVERURL) + std::string("/api/friend/refuse") + "?" + m_data;
    std::string szReponse;

    LOG_TRACE(LOG_INFO, true, __FUNCTION__, "http get url=" << szUrl);
	if (CURLE_OK != phttpclient->Get(szUrl.c_str(), szReponse))
	{	
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "http get error url=" << szUrl);	
		return;
	}

    if (szReponse.empty())
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "http get error url=" << szUrl);
        return;
    }
    
    RJDocument json_document;
    
    if (!JsonParse::parse(json_document, szReponse.c_str()))
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "json is error =" << szReponse);
        return;
    }
    
    STRU_PACKAGE_RS<T_FRIEND_CLIENT_CONFIRM_RS> loSendPack(NEW_DEF_CLINET_FRIEND_CONFIRM_RS);
    
    int liResult = json_document["responseCode"].GetInt();

    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);

    UINT64 ultoken = 0;

    if (liResult == -2147483648)
    {
        LOG_TRACE(LOG_INFO, true, __FUNCTION__, "receive msg:" << szReponse);
        ultoken = time(NULL);

        friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder c1 = friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder(loSendPack.fbbuilder);
        c1.add_s_rs_head(&s_rs_head);
        c1.add_peer_user_id(m_friendid);
        c1.add_token(ultoken);
        c1.add_result(1);
        loSendPack.fbbuf = c1.Finish();

        CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

        if(!lpUser)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
            return;
        }

        m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);

        //save own addconfirm operator token 
        flatbuffers::FlatBufferBuilder fbbuild;
        auto fbsremarkname = fbbuild.CreateString(m_remarkname.c_str());

        friendpack::T_FREIND_INFOBuilder friendinfobuild = friendpack::T_FREIND_INFOBuilder(fbbuild);
        friendinfobuild.add_user_id(m_friendid);
        friendinfobuild.add_remark_name(fbsremarkname);
        friendinfobuild.add_sourcetype(m_sourcetype);
        friendinfobuild.add_opttype(FD_REFUSE_OP);
        friendinfobuild.add_optime(ultoken);
        fbbuild.Finish(friendinfobuild.Finish());

        std::string strElement0;
        strElement0.assign((char *)fbbuild.GetBufferPointer(), fbbuild.GetSize());        
        m_pclientside->mpServerMgr->mpRedisUtil->ZaddFdMsg(m_aiUserID,ultoken,strElement0);

        //send serverconfirmrq to peeruser
        STRU_SERVER_PACKAGE_RQ<T_FRIEND_SERVER_CONFIRM_RQ> loServerSendPack(NEW_DEF_SERVER_FRIEND_CONFIRM_RQ);
        commonpack::S_RQ_HEAD s_rq_head(m_friendid, 
                                         m_aiPackSessionID,
                                         m_aiPlatform);

        loServerSendPack.fbbuf = friendpack::CreateT_FRIEND_SERVER_CONFIRM_RQDirect(loServerSendPack.fbbuilder,
                                                &s_rq_head,
                                                m_nicknmae.c_str(),
                                                m_aiUserID,
                                                ultoken,
                                                1);
        std::string IsNeedApns("1");
        CServerToKafka::GetInstance().DistributeMsgToScRoute(loServerSendPack.GetPackType(), 
                m_friendid, 
                loServerSendPack, IM_SC_TOPIC,IsNeedApns);

        //save peer del operator token
        flatbuffers::FlatBufferBuilder fbpeerbuild;
        friendpack::T_FREIND_INFOBuilder peerfriendinfobuild = friendpack::T_FREIND_INFOBuilder(fbpeerbuild);
        peerfriendinfobuild.add_user_id(m_aiUserID);
        peerfriendinfobuild.add_sourcetype(m_sourcetype);
        peerfriendinfobuild.add_opttype(FD_REFUSE_OP);
        peerfriendinfobuild.add_optime(ultoken);
        fbpeerbuild.Finish(peerfriendinfobuild.Finish());

        std::string strElement1;
        strElement1.assign((char *)fbpeerbuild.GetBufferPointer(), fbpeerbuild.GetSize());        
        m_pclientside->mpServerMgr->mpRedisUtil->ZaddFdMsg(m_friendid,ultoken,strElement1);              
    }
    else
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "receive msg:" << szReponse);
        friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder c1 = friendpack::T_FRIEND_CLIENT_CONFIRM_RSBuilder(loSendPack.fbbuilder);
        c1.add_s_rs_head(&s_rs_head);
        c1.add_peer_user_id(m_friendid);
        c1.add_token(ultoken);
        c1.add_result(liResult);
        loSendPack.fbbuf = c1.Finish();

        CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

        if(!lpUser)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
            return;
        }

        m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);
    }
    */
    return;
}

void CFriendConfirmTask::run()
{    
    if (0 == m_result)
    {       
        DealAgree();
    }
    else
    {   

        //DealRefuse();
    }   
    
    uint64 interval =  GetInterValTime();
    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "userid" << m_aiUserID << " pop queue time = " << interval); 

    return;      
}


