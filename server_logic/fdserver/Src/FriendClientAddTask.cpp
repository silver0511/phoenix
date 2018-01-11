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
#include "FriendClientAddTask.h"

void CFriendClientAddTask::DoForwardPackageFail()
{
    
    LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Forward package fail url = " << FRIEND_ADD_API);

    int liResult = RET_SYS_PACK_TYPE_INVALID;
    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);

    STRU_PACKAGE_RS<T_FRIEND_CLIENT_ADD_RS> loSendPack(NEW_DEF_CLIENT_FRIEND_ADD_RS);

    auto fbsopmsg = loSendPack.fbbuilder.CreateString(m_opmsg.c_str()); 
    friendpack::T_FRIEND_CLIENT_ADD_RSBuilder c1 = friendpack::T_FRIEND_CLIENT_ADD_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_peer_user_id(m_friendid);
    c1.add_token(0);
    c1.add_op_msg(fbsopmsg);
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


void CFriendClientAddTask::run()
{
    if (NULL == m_parm)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "m_parm is null");
        return;
    }

    CHttpClient* phttpclient =  (CHttpClient*) m_parm;

    std::string szUrl = FRIEND_ADD_API + "?" + m_data;

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

    STRU_PACKAGE_RS<T_FRIEND_CLIENT_ADD_RS> loSendPack(NEW_DEF_CLIENT_FRIEND_ADD_RS);
    
    int liResult = json_document["responseCode"].GetInt();
    
    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);

    UINT64 ultoken = 0;
    if (liResult == -2147483648)
    {
        LOG_TRACE(LOG_INFO, true, __FUNCTION__, "receive msg:" << szReponse);
        ultoken = time(NULL);

        auto fbsopmsg = loSendPack.fbbuilder.CreateString(m_opmsg.c_str()); 
        friendpack::T_FRIEND_CLIENT_ADD_RSBuilder c1 = friendpack::T_FRIEND_CLIENT_ADD_RSBuilder(loSendPack.fbbuilder);
        c1.add_s_rs_head(&s_rs_head);
        c1.add_peer_user_id(m_friendid);
        c1.add_token(ultoken);
        c1.add_op_msg(fbsopmsg);
        loSendPack.fbbuf = c1.Finish();

        CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

        if(!lpUser)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
            return;
        }

        m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);

        //save own add operator token 
        flatbuffers::FlatBufferBuilder fbbuild;
        auto fbsownopsmsg = fbbuild.CreateString(m_opmsg.c_str());

        friendpack::T_FREIND_INFOBuilder friendinfobuild = friendpack::T_FREIND_INFOBuilder(fbbuild);
        friendinfobuild.add_user_id(m_friendid);
        friendinfobuild.add_source_type(m_sourcetype);
        friendinfobuild.add_op_msg(fbsownopsmsg);
        friendinfobuild.add_opt_type(FD_WAITCONFIRM_OP);
        friendinfobuild.add_op_time(ultoken);
        fbbuild.Finish(friendinfobuild.Finish());

        std::string strElement0;
        strElement0.assign((char *)fbbuild.GetBufferPointer(), fbbuild.GetSize());        
        m_pclientside->mpServerMgr->mpRedisUtil->ZaddFdMsg(m_aiUserID,ultoken,strElement0);

        //send server addrq to peeruser
    	STRU_SERVER_PACKAGE_RQ<T_FRIEND_SERVER_ADD_RQ> loServerSendPack(NEW_DEF_SERVER_FRIEND_ADD_RQ);        
        commonpack::S_RQ_HEAD s_rq_head(m_friendid, 
                                         m_aiPackSessionID,
        						         m_aiPlatform);     
      
    	loServerSendPack.fbbuf = friendpack::CreateT_FRIEND_SERVER_ADD_RQDirect(loServerSendPack.fbbuilder,
    												&s_rq_head,
                                                    m_opmsg.c_str(),
                                                    m_nicknmae.c_str(),
                                                    m_aiUserID,
                                                    m_sourcetype,
    											    ultoken);	

    	CServerToKafka::GetInstance().DistributeMsgToScRoute(loServerSendPack.GetPackType(), 
    				m_friendid, 
    				loServerSendPack, IM_SC_TOPIC);

        //save peer add operator token
        flatbuffers::FlatBufferBuilder fbpeerbuild;
        auto fbspeeropsmsg = fbpeerbuild.CreateString(m_opmsg.c_str());
        friendpack::T_FREIND_INFOBuilder peerfriendinfobuild = friendpack::T_FREIND_INFOBuilder(fbpeerbuild);
        peerfriendinfobuild.add_user_id(m_aiUserID);
        peerfriendinfobuild.add_source_type(m_sourcetype);
        peerfriendinfobuild.add_opt_type(FD_NEEDCONFIRM_OP);
        peerfriendinfobuild.add_op_time(ultoken);
        peerfriendinfobuild.add_op_msg(fbspeeropsmsg);
        fbpeerbuild.Finish(peerfriendinfobuild.Finish());

        std::string strElement1;
        strElement1.assign((char *)fbpeerbuild.GetBufferPointer(), fbpeerbuild.GetSize());        
        m_pclientside->mpServerMgr->mpRedisUtil->ZaddFdMsg(m_friendid,ultoken,strElement1);              

        m_pclientside->mpServerMgr->mpRedisUtil->SetFdRequestTime(m_friendid,m_aiUserID, std::to_string(ultoken));
    }
    else
    {
        if (liResult == RET_FRIEND_BE_DELETE_ERROR)
        {
            ultoken = time(NULL);
            LOG_TRACE(LOG_INFO, true, __FUNCTION__, "receive msg:" << szReponse);  
             
            //send restor to peeruser
            STRU_SERVER_PACKAGE_RQ<T_FRIEND_SERVER_RECOVER_RQ> loServerSendPack(NEW_DEF_FRIEND_RESTORE_RQ);
            commonpack::S_RQ_HEAD s_rq_head(m_friendid, 
                                             m_aiPackSessionID,
            						         m_aiPlatform);

            loServerSendPack.fbbuf = friendpack::CreateT_FRIEND_SERVER_RECOVER_RQDirect(loServerSendPack.fbbuilder,
            										&s_rq_head,
                                                    m_nicknmae.c_str(),
                                                    m_aiUserID,
            									    ultoken);

            CServerToKafka::GetInstance().DistributeMsgToScRoute(loServerSendPack.GetPackType(), 
            		m_friendid, 
            		loServerSendPack, IM_SC_TOPIC);

            //save peer del operator token
            flatbuffers::FlatBufferBuilder fbpeerbuild;
            friendpack::T_FREIND_INFOBuilder peerfriendinfobuild = friendpack::T_FREIND_INFOBuilder(fbpeerbuild);
            peerfriendinfobuild.add_user_id(m_aiUserID);
            peerfriendinfobuild.add_source_type(m_sourcetype);
            peerfriendinfobuild.add_opt_type(FD_RESTOR_OP);
            peerfriendinfobuild.add_op_time(ultoken);
            fbpeerbuild.Finish(peerfriendinfobuild.Finish());

            std::string strElement1;
            strElement1.assign((char *)fbpeerbuild.GetBufferPointer(), fbpeerbuild.GetSize());        
            m_pclientside->mpServerMgr->mpRedisUtil->ZaddFdMsg(m_friendid,ultoken,strElement1);           
            m_pclientside->mpServerMgr->mpRedisUtil->DelFdRequestTime(m_friendid, m_aiUserID);        
        }
        else
        {
            LOG_TRACE(LOG_ERR, true, __FUNCTION__, "receive msg:" << szReponse);            

            if (liResult == RET_FRIEND_RELATION_ERROR)
            {
                m_pclientside->mpServerMgr->mpRedisUtil->SetFdRequestTime(m_friendid,m_aiUserID, std::to_string(time(NULL)));
            }          
        }
        auto fbsopmsg = loSendPack.fbbuilder.CreateString(m_opmsg.c_str()); 
        friendpack::T_FRIEND_CLIENT_ADD_RSBuilder c1 = friendpack::T_FRIEND_CLIENT_ADD_RSBuilder(loSendPack.fbbuilder);
        c1.add_s_rs_head(&s_rs_head);
        c1.add_peer_user_id(m_friendid);
        c1.add_token(ultoken);
        c1.add_op_msg(fbsopmsg);
        loSendPack.fbbuf = c1.Finish();

        CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

        if(!lpUser)
        {
            LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
            return;
        }

        m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);
    }
    
    uint64 interval =  GetInterValTime();
    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "userid" << m_aiUserID << " pop queue time = " << interval); 

    return;      
}


