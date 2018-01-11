#include "rjson/JsonParser.h"
#include "httpclient.h"
#include "common/Debug_log.h"
#include "network/packet/client/SysPackDef.h"
#include "network/packet/client/FdServerPackDef.h"
#include "ServerUtilMgr.h"
#include "ServerConn/UserConMgr.h"
#include "./ClientSide/ClientSide.h"
#include "ServerMgr.h"
#include "FriendListTask.h"

void CFriendListTask::DoForwardPackageFail()
{    
    LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Forward package fail url = " << FRIEND_GETLIST_API);

    int liResult = RET_SYS_PACK_TYPE_INVALID;
    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);

    STRU_PACKAGE_RS<T_FRIEND_LIST_RS> loSendPack(NEW_DEF_FRIEND_LIST_RS);

    std::vector< flatbuffers::Offset<friendpack::T_FREIND_INFO> > std_friend_info_vec;

    auto fbs_friend_info_vec = loSendPack.fbbuilder.CreateVector(std_friend_info_vec);
    friendpack::T_FRIEND_LIST_RSBuilder c1 = friendpack::T_FRIEND_LIST_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_friend_list(fbs_friend_info_vec);
    c1.add_msg_source(FULLGET);
    c1.add_token(0);
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


void CFriendListTask::run()
{
    if (NULL == m_parm)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "m_parm is null");
        return;
    }

    CHttpClient* phttpclient =  (CHttpClient*) m_parm;

    std::string szUrl = FRIEND_GETLIST_API + "?" + m_data;

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
	
	LOG_TRACE(LOG_INFO, true, __FUNCTION__, "receive msg:" << szReponse);
    
    STRU_PACKAGE_RS<T_FRIEND_LIST_RS> loSendPack(NEW_DEF_FRIEND_LIST_RS);

    int liResult = json_document["responseCode"].GetInt();
    commonpack::S_RS_HEAD s_rs_head(m_aiUserID, m_aiPackSessionID,liResult,m_aiPlatform);
    
    RJsonValue& jsondata = json_document["data"];

    std::vector< flatbuffers::Offset<friendpack::T_FREIND_INFO> > std_friend_info_vec;
    for (unsigned int i = 0 ; i < jsondata.Size(); i++)
    {
        RJsonValue& jsondataobj = jsondata[i];
        std::string szfdid("");
        JsonParse::get(jsondataobj, "friend_id", szfdid);               
        UINT64 uifdid = ServerUtilMgr::STR2UINT64(szfdid);

        std::string szremarkname("");
        JsonParse::get(jsondataobj, "remark_name", szremarkname);       
           
        int uisourcetype = 0;
        JsonParse::get(jsondataobj, "source_type", uisourcetype);

        int uiblacktype = 0;
        JsonParse::get(jsondataobj, "is_black_list", uiblacktype);

        int uioptype = 0;
        JsonParse::get(jsondataobj, "status", uioptype);

        std::string szopmsg("");
        JsonParse::get(jsondataobj, "friend_msg", szopmsg);

        auto fbsremarkname = loSendPack.fbbuilder.CreateString(szremarkname.c_str()); 
        auto fbsopmsg =  loSendPack.fbbuilder.CreateString(szopmsg.c_str()); 
        friendpack::T_FREIND_INFOBuilder fbsfriendinfo = friendpack::T_FREIND_INFOBuilder(loSendPack.fbbuilder);
        fbsfriendinfo.add_user_id(uifdid);      
        fbsfriendinfo.add_remark_name(fbsremarkname);       
        fbsfriendinfo.add_source_type(uisourcetype);
        fbsfriendinfo.add_black_type(uiblacktype);
        fbsfriendinfo.add_opt_type(uioptype);
        fbsfriendinfo.add_op_msg(fbsopmsg);
        std_friend_info_vec.push_back(fbsfriendinfo.Finish());
    }

    UINT64 ultoken = 0;

    if (MAX_FD_LIST_CNT > jsondata.Size())
    {
         ultoken = time(NULL);    
    }

    auto fbs_friend_info_vec = loSendPack.fbbuilder.CreateVector(std_friend_info_vec);
    friendpack::T_FRIEND_LIST_RSBuilder c1 = friendpack::T_FRIEND_LIST_RSBuilder(loSendPack.fbbuilder);
    c1.add_s_rs_head(&s_rs_head);
    c1.add_friend_list(fbs_friend_info_vec);
    c1.add_msg_source(FULLGET);
    c1.add_token(ultoken);
    loSendPack.fbbuf = c1.Finish();

    CUser* lpUser = m_pclientside->mpServerMgr->moUserConMgr.getUser(m_aiCometID, m_aiUserID);

    if(!lpUser)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "lpUser is null user_id = " << m_aiUserID);
        return;
    }

    m_pclientside->Send2Comet(loSendPack.GetPackType(), loSendPack, m_aiUserID, lpUser);

    uint64 interval =  GetInterValTime();
    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, "userid" << m_aiUserID << " pop queue time = " << interval); 
    
    return;
}

