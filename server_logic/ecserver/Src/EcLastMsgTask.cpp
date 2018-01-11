#include "rjson/JsonParser.h"
#include "httpclient.h"
#include "common/Debug_log.h"
#include "network/packet/client/SysPackDef.h"
#include "network/packet/client/EcServerPackDef.h"
#include "ServerUtilMgr.h"
#include "ServerConn/UserConMgr.h"
#include "./ClientSide/ClientSide.h"
#include "ServerMgr.h"
#include "EcLastMsgTask.h"

void CEcLastMsgTask::DoForwardPackageFail()
{
    LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Forward package fail url = " << EC_LASTMSG_API);
    
    int liResult = RET_SYS_PACK_TYPE_INVALID;
    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);

    STRU_PACKAGE_RS<T_EC_GETLASTMSGS_RS> loSendPack(NEW_DEF_BUSINESS_GETLASTMSGS_RS);
    std::vector< flatbuffers::Offset<ecpack::T_ECMSG_INFO> > std_ecmsg_info_vec;
    
    
    auto fbs_ecmsg_info_vec = loSendPack.fbbuilder.CreateVector(std_ecmsg_info_vec);
    ecpack::T_EC_GETLASTMSGS_RSBuilder c1 = ecpack::T_EC_GETLASTMSGS_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_b_id(m_bid);
    c1.add_offset(m_offset);
    c1.add_max_cnt(m_maxcnt);
    c1.add_msg_list(fbs_ecmsg_info_vec);
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

void CEcLastMsgTask::run()
{
    if (NULL == m_parm)
    {
        return;
    }

    CHttpClient* phttpclient =  (CHttpClient*) m_parm;

    std::string szUrl = EC_LASTMSG_API + "?" + m_data;

    std::string szReponse;
    LOG_TRACE(LOG_INFO, true, __FUNCTION__, "http get url=" << szUrl);

	if (CURLE_OK != phttpclient->Get(szUrl.c_str(), szReponse))
	{	
        DoForwardPackageFail();	
		return;
	}

	if (szReponse.empty())
	{
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
	
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "receive msg:" << szReponse);
    
    STRU_PACKAGE_RS<T_EC_GETLASTMSGS_RS> loSendPack(NEW_DEF_BUSINESS_GETLASTMSGS_RS);

    int liResult = json_document["responseCode"].GetInt();
    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);
    
    RJsonValue& jsondata = json_document["data"];

    std::vector< flatbuffers::Offset<ecpack::T_ECMSG_INFO> > std_ecmsg_info_vec;
    for (unsigned int i = 0 ; i < jsondata.Size(); i++)
    {
        RJsonValue& jsondataobj = jsondata[i];
        int appid = 0;
        int sessionid = 0;
        int chattype = 0;
        int mtype = 0;
        int stype = 0;
        int exttype = 0;
        uint64_t fromid = 0;
        uint64_t toid = 0;
        uint64_t wid = 0;
        uint64_t cid = 0;
        uint64_t msgid = 0;
        uint64_t msgtime = 0;
        std::string msgcontent("");
        std::string sendusername("");

        JsonParse::get(jsondataobj, "app_id", appid); 
        JsonParse::get(jsondataobj, "session_id", sessionid); 
        JsonParse::get(jsondataobj, "chat_type", chattype);
        JsonParse::get(jsondataobj, "m_type", mtype);
        JsonParse::get(jsondataobj, "s_type", stype);
        JsonParse::get(jsondataobj, "ext_type", exttype);
        JsonParse::get(jsondataobj, "msg_time", msgtime);
        JsonParse::get(jsondataobj, "from_id", fromid);
        JsonParse::get(jsondataobj, "to_id", toid);
        JsonParse::get(jsondataobj, "w_id", wid);
        JsonParse::get(jsondataobj, "c_id", cid);
        JsonParse::get(jsondataobj, "message_id", msgid);
        JsonParse::get(jsondataobj, "msg", msgcontent);
        JsonParse::get(jsondataobj, "send_user_name", sendusername);
       
        auto fbsmsgcontent = loSendPack.fbbuilder.CreateString(msgcontent.c_str());  
        auto fbssendusername =  loSendPack.fbbuilder.CreateString(sendusername.c_str());     
        ecpack::T_ECMSG_INFOBuilder fbsmsginfo = ecpack::T_ECMSG_INFOBuilder(loSendPack.fbbuilder);
        fbsmsginfo.add_app_id(appid);
        fbsmsginfo.add_session_id(sessionid);
        fbsmsginfo.add_chat_type(chattype);
        fbsmsginfo.add_m_type(mtype);
        fbsmsginfo.add_s_type(stype);
        fbsmsginfo.add_ext_type(exttype);
        fbsmsginfo.add_msg_time(msgtime);
        fbsmsginfo.add_from_id(fromid);
        fbsmsginfo.add_to_id(toid);
        fbsmsginfo.add_b_id(m_bid);
        fbsmsginfo.add_w_id(wid);
        fbsmsginfo.add_c_id(cid);
        fbsmsginfo.add_message_id(msgid);
        fbsmsginfo.add_msg_content(fbsmsgcontent);  
        fbsmsginfo.add_send_user_name(fbssendusername);
        std_ecmsg_info_vec.push_back(fbsmsginfo.Finish());
    }

    auto fbs_ecmsg_info_vec = loSendPack.fbbuilder.CreateVector(std_ecmsg_info_vec);
    ecpack::T_EC_GETLASTMSGS_RSBuilder c1 = ecpack::T_EC_GETLASTMSGS_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_b_id(m_bid);
    c1.add_offset(m_offset);
    c1.add_max_cnt(m_maxcnt);
    c1.add_msg_list(fbs_ecmsg_info_vec);
    loSendPack.fbbuf = c1.Finish();

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

