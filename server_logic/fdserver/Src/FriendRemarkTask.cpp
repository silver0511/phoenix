#include "rjson/JsonParser.h"
#include "httpclient.h"
#include "common/Debug_log.h"
#include "network/packet/client/SysPackDef.h"
#include "network/packet/client/FdServerPackDef.h"
#include "ServerUtilMgr.h"
#include "ServerConn/UserConMgr.h"
#include "./ClientSide/ClientSide.h"
#include "ServerMgr.h"
#include "FriendRemarkTask.h"

void CFriendRemarkTask::DoForwardPackageFail()
{
    
    LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Forward package fail url = " << FRIEND_REMARK_API);

    int liResult = RET_SYS_PACK_TYPE_INVALID;
    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);

    STRU_PACKAGE_RS<T_FRIEND_REMARK_RS> loSendPack(NEW_DEF_FRIEND_REMARK_RS);

    
    loSendPack.fbbuf = friendpack::CreateT_FRIEND_REMARK_RSDirect(loSendPack.fbbuilder,
                                                                     &s_rs_head,
                                                                     m_friendid,
                                                                     m_remarkname.c_str(),
                                                                     0);
       
   CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

   if(!lpUser)
   {
       LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
       return;
   }

   m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);

   return;
}


void CFriendRemarkTask::run()
{
    if (NULL == m_parm)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "m_parm is null");
        return;
    }

    CHttpClient* phttpclient =  (CHttpClient*) m_parm;

    std::string szUrl = FRIEND_REMARK_API + "?" + m_data;

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

    STRU_PACKAGE_RS<T_FRIEND_REMARK_RS> loSendPack(NEW_DEF_FRIEND_REMARK_RS);

   int liResult = json_document["responseCode"].GetInt();

    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);

    UINT64 ultoken = 0;
    if (liResult == -2147483648)
    {
        LOG_TRACE(LOG_INFO, true, __FUNCTION__, "receive msg:" << szReponse);
        ultoken = time(NULL);

        flatbuffers::FlatBufferBuilder fbbuild;
        auto fbsremarkname = fbbuild.CreateString(m_remarkname.c_str());
        
        friendpack::T_FREIND_INFOBuilder friendinfobuild = friendpack::T_FREIND_INFOBuilder(fbbuild);
        friendinfobuild.add_user_id(m_friendid);
        friendinfobuild.add_remark_name(fbsremarkname);
        friendinfobuild.add_opt_type(FD_REMARK_OP);
        friendinfobuild.add_op_time(ultoken);    
        fbbuild.Finish(friendinfobuild.Finish());

        std::string strElement;
        strElement.assign((char *)fbbuild.GetBufferPointer(), fbbuild.GetSize());        
        m_pclientside->mpServerMgr->mpRedisUtil->ZaddFdMsg(m_aiUserID,ultoken,strElement);
    }
    else
    {
        LOG_TRACE(LOG_ERR, true, __FUNCTION__, "receive msg:" << szReponse);
    }

    loSendPack.fbbuf = friendpack::CreateT_FRIEND_REMARK_RSDirect(loSendPack.fbbuilder,
                                                                  &s_rs_head,
                                                                  m_friendid,
                                                                  m_remarkname.c_str(),
                                                                  ultoken);
    
    CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

    if(!lpUser)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
        return;
    }

    m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);

    uint64 interval =  GetInterValTime();
    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "userid" << m_aiUserID << " pop queue time = " << interval); 
    return;      
}

