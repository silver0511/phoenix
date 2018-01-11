#include "ServerMgr.h"
#include "network/packet/ClientPack.h"
#include "network/packet/PackTypeDef.h"
#include "ConsumeDealMsgThread.h"
#include "ServerUtilMgr.h"
#include "ServerToKafka.h"
#include "rjson/JsonParser.h"


static bool TransforMsgkey(const std::string& szmsgkey ,std::string& szgroupid, std::string& szmsgtype, std::string& szpageindex)
{       
	std::size_t groupstartpos = 0;
	std::size_t groupendpos = szmsgkey.find('_');
	szgroupid =  szmsgkey.substr(groupstartpos, groupendpos - groupstartpos);

	if ( std::string::npos == groupendpos)
	{
			return false;
	}

	std::size_t msgtypestartpos =  groupendpos  + 1;
	std::size_t msgtypeendpos =  szmsgkey.find('_',groupendpos+1);
	szmsgtype = szmsgkey.substr(msgtypestartpos, msgtypeendpos - msgtypestartpos);

    if ( std::string::npos == msgtypeendpos)
	{
			return false;
	}

    std::size_t pagestartpos =  msgtypeendpos  + 1;
	std::size_t pageendpos =  szmsgkey.find('_',msgtypeendpos+1);
	szpageindex = szmsgkey.substr(pagestartpos, pageendpos - pagestartpos);

	return true;
}

static UINT64 GetGroupId(const std::string& szkey)
{
    std::string szgroupid;
    std::size_t groupstartpos = 0;
    std::size_t groupendpos = szkey.find('_');
    szgroupid =  szkey.substr(groupstartpos, groupendpos - groupstartpos);    

    return ServerUtilMgr::STR2UINT64(szgroupid);
}

CConsumeDealMsgThread::CConsumeDealMsgThread()
{
	m_ulDealSucMsgCnt = 0;
    m_msgcnt = 0;
    mulsendsuccnt = 0;
    mulsendfailcnt = 0;
    mulapnscnt = 0;
    mulonlinecnt = 0;
	m_ulDealFailMsgCnt = 0;
    mThreadID = 0;
    m_pServerMgr = NULL;
    m_apptye = "app";
}

CConsumeDealMsgThread::CConsumeDealMsgThread(CServerMgr* pServerMgr, uint32 threadid, std::string& apptype):m_pServerMgr(pServerMgr)
, mThreadID(threadid) , m_apptye(apptype)
{
	m_ulDealSucMsgCnt = 0;
	m_ulDealFailMsgCnt = 0;
    mulapnscnt = 0;
    mulonlinecnt = 0;
    m_msgcnt = 0;
    mulsendsuccnt = 0;
    mulsendfailcnt = 0;
}


CConsumeDealMsgThread::~CConsumeDealMsgThread()
{
}

void CConsumeDealMsgThread::AddTailMsg(StrConsumeMsg* msg)
{
	m_empty_notify.Lock();
	m_msgqueue.AddTail(msg);
	m_msgcnt++;
	m_empty_notify.Signal();
    m_empty_notify.Unlock();
}

StrConsumeMsg *CConsumeDealMsgThread::GetandDelHead()
{
	m_empty_notify.Lock();
	if (m_msgcnt <= 0)
	{
        OnDealNoNeedApnsCacheTime();
        OnDeaNeedApnsCacheTime();
		m_empty_notify.Wait();
	}
	
	StrConsumeMsg* msg = m_msgqueue.GetandDelHead();
	m_msgcnt--;
	m_empty_notify.Unlock();

	return msg;
}


bool CConsumeDealMsgThread::Init()
{
	CBaseThread::BeginThread(0);
	return true;
}

void CConsumeDealMsgThread::Run()
{
    INT64 oldtime = CSystem::GetSystemTime();
	while (getRunState())
	{
        INT64 curltime = CSystem::GetSystemTime();
        if (curltime - oldtime > m_pServerMgr->moConfig.m_grouppushinterval)
        {
            oldtime = curltime;
            OnDealNoNeedApnsCacheTime();
            OnDeaNeedApnsCacheTime();         
        }
        else
        {
             StrConsumeMsg* pmsg = GetandDelHead();
             std::string szgroupid("");
             std::string szmsgtype("");
             std::string szpageindex("");
             
             LOG_TRACE(LOG_DEBUG, true, __FUNCTION__," msgkey= " << pmsg->szkey);
             if (TransforMsgkey(pmsg->szkey,szgroupid,szmsgtype,szpageindex))
             {
                CacheDealMsg(pmsg, szgroupid, szmsgtype, szpageindex);               
             }
             else
             {
                DirectDealMsg(pmsg, ServerUtilMgr::STR2UINT64(szgroupid));
             }  
               
        }
    }
	return;
}

void CConsumeDealMsgThread::Release()
{	
	return;
}

void CConsumeDealMsgThread::Stop()
{
	mbIsRun = false;
	LOG_TRACE(LOG_NOTICE, true, __FUNCTION__," stop ");
	return;
}

bool CConsumeDealMsgThread::DealMsg(BYTE*  data, uint32 uldatalen, UINT64 ulgroupid, bool IsNeedApns)
{	
	if (0 == uldatalen || NULL == data)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__," msg is abnormal " << " threadid=" << mThreadID);
		return false;
	}

	BYTE* pdatapos = data;
	uint32 uldstlen = uldatalen;

	//Analysis msgtype
	if (uldstlen < sizeof(uint16))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__," type msg data len is abnormal "
                                         << "datalen=" << uldstlen
                                         << " threadid=" << mThreadID);
		return false;
	}
	uint16 uitype = *((uint16 *)pdatapos);       //remove datatype	
	pdatapos += sizeof(uitype);
	uldstlen -= sizeof(uitype);

	//Analysis usercnt
	if (uldstlen < sizeof(uint16))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__," usercnt msg data len is abnormal "
										  << "datalen=" << uldstlen
                                          << " threadid=" << mThreadID);
		return false;
	}
	uint16 uiusercnt = *((uint16 *)pdatapos);	
	pdatapos += sizeof(uiusercnt);
	uldstlen -= sizeof(uiusercnt);
	if (uiusercnt > STRU_BACK_2_COMET_HEAD_PACKAGE::LIST_COUNT)
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__," user cnt is too big: " << uiusercnt << " threadid:" << mThreadID);
		return false;
	}

	//Analysis userid
	if (uldstlen < uiusercnt*sizeof(USERID))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__," userid msg data len is abnormal "
										<< "datalen=" << uldstlen
                                        << " threadid=" << mThreadID);
		return false;
	}

	std::vector<USERID> userids;
	std::string szuserids;

	for (uint16 i = 0; i < uiusercnt; i++)
	{		
		USERID uiuserid = *((USERID *)pdatapos);
        szuserids += std::to_string(uiuserid) + ",";


		userids.push_back(uiuserid);
		pdatapos += sizeof(uiuserid);
		uldstlen -= sizeof(uiuserid);
	}

    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__," type=" << uitype 
                                        << " groupid=" << ulgroupid 
                                        << " usercnt=" << uiusercnt 
                                        << " userids=" << szuserids 
                                        << " threadid=" << mThreadID);


    //class userid by appserverids
	std::multimap<SZSERVERID,USERID> szappserverid_userid_map;
    std::set<SZSERVERID> szappserverids;

    if (0 == DivisionUserid(userids, szappserverid_userid_map, szappserverids, uitype, pdatapos, uldstlen, IsNeedApns))
    {
        return true;
    }

    Send2Comet(uitype, szappserverids, szappserverid_userid_map, pdatapos, uldstlen);		
	return true;
}

uint32 CConsumeDealMsgThread::DivisionUserid(std::vector<USERID> & userids, 
                                                  std::multimap<SZSERVERID,USERID>& szappserverid_userid_map,
                                                  std::set<SZSERVERID>& szappserverids,
                                                  uint16 uimsgtype,
                                                  BYTE * pdata,
                                                  uint32 uidatalen,
                                                  bool IsNeedApns)
{
    RJ_CREATE_A_DOCUMENT(l_document, l_allocator)
    for (std::vector<USERID>::iterator iter =  userids.begin(); iter != userids.end(); iter++)
    {
         JsonParse::push<uint64_t>(l_document, (uint64_t)(*iter), l_allocator);
    }

    std::string out_json = "";
    JsonParse::to_string(l_document, out_json);
    
    std::string loginserver("");
    if (m_apptye == "app")
    {
        loginserver = m_pServerMgr->mpRedisUtil->LuaGetLoginServer(out_json,PLATFORM_APP);
    }
    else
    {
        loginserver = m_pServerMgr->mpRedisUtil->LuaGetLoginServer(out_json,PLATFROM_WEB);
    }
    LOG_TRACE(LOG_DEBUG, false, __FUNCTION__," lua output:" << loginserver);

    if (loginserver.empty())
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "json is error =" << loginserver);
        return 0;
    }

    RJDocument json_document;
    if (!JsonParse::parse(json_document, loginserver.c_str()))
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "json is error =" << loginserver);
        return 0;
    }

    if (json_document.Size() != 3)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__, "json is error =" << loginserver);
        return 0;
    }

    //deal online user
    RJsonValue& json_serverid =  JsonParse::at(json_document, 0);
    if (!json_serverid.IsNull())
    {
        for (unsigned int i = 0 ; i < json_serverid.Size(); i++)
        {
            std::string szserverid;
            JsonParse::at(json_serverid, i, szserverid);
            if (szserverid == "-1" || szserverid == "")
            {
                break;
            }
            szappserverids.insert(szserverid);
        }
    }
    

    std::string onlineuser("[");
    std::string offlineuser("[");
    RJsonValue& json_serverid_userids =  JsonParse::at(json_document, 1);
    if (!json_serverid_userids.IsNull())
    {
        for (std::set<SZSERVERID>::iterator iter = szappserverids.begin(); iter != szappserverids.end(); iter++)
    	{
             RJsonValue& json_userids = json_serverid_userids[(*iter).c_str()];
             for (unsigned int i = 0 ; i < json_userids.Size(); i++)
             {
                mulonlinecnt++;
                int64_t  i64_val;
                JsonParse::at<int64_t>(json_userids, i, i64_val);
                onlineuser += std::to_string(i64_val) + ";";
                szappserverid_userid_map.insert(std::make_pair(*iter,(USERID)i64_val));
             }
        }
    }    

    //deal offilne user
    if (IsNeedApns)
    {
        RJsonValue& json_offineuser =  JsonParse::at(json_document, 2);
        USERID UserList[STRU_BACK_2_COMET_HEAD_PACKAGE::LIST_COUNT] = {0};
        uint16 uiusercnt = 0;
        if (!json_offineuser.IsNull())
        {            
            for (unsigned int i = 0 ; i < json_offineuser.Size(); i++)
            {
                 //offline user
                mulapnscnt++;
                int64_t  i64_val;
                JsonParse::at<int64_t>(json_offineuser, i, i64_val);
        
                if (i64_val == -1)
                {
                    break;
                }
                UserList[uiusercnt++] = i64_val;   
                offlineuser += std::to_string(i64_val) + ";";
            }
         }
        
        if ( 0 != uiusercnt)
        {
            CServerToKafka::GetInstance().DistributeGcMsgToApns(uimsgtype, UserList, uiusercnt, pdata, uidatalen, IM_APNS_TOPIC);       
        }
	}

    onlineuser +="]";
    offlineuser += "]";

    
    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "onlineuser:" << onlineuser);
    LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, "offlineuser:" << offlineuser);

    return szappserverids.size();        
}

void CConsumeDealMsgThread::Send2Comet(uint16 uimsgtype,
									 	std::set<SZSERVERID> serverids,
										std::multimap<SZSERVERID, USERID> serverid_userid_map, 
										BYTE * pdata,
										uint32 uidatalen)
{
	typedef std::multimap<SZSERVERID, USERID>::iterator multiMapItor;

	BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
	BYTE* pos = NULL;
	INT  idstlen = 0;

	USERID UserList[STRU_BACK_2_COMET_HEAD_PACKAGE::LIST_COUNT] = {0};
	uint16 uiusercnt = 0;
	SOCKET socket = -1;

	for (std::set<SZSERVERID>::iterator iter = serverids.begin(); iter != serverids.end(); iter++)
	{
        pos = lpBuffer;
        idstlen = 0;
		uiusercnt = 0;
		std::pair<multiMapItor, multiMapItor> tmppos = serverid_userid_map.equal_range(*iter);

		for (multiMapItor tmpmultiMapItor = tmppos.first; tmpmultiMapItor != tmppos.second; tmpmultiMapItor++)
		{
			UserList[uiusercnt++] = tmpmultiMapItor->second;
		}

		idstlen = ClientPack::GetInstance().PackBack2CometHead(uimsgtype, uiusercnt, UserList, lpBuffer);
		pos += idstlen;

		//generate orgmsg
		memcpy(pos,pdata,uidatalen);
		idstlen += uidatalen;

        if (m_pServerMgr->Send2Comet(mThreadID, *iter, lpBuffer, idstlen))
        {
            mulsendsuccnt++;
        }
         else
        {
            mulsendfailcnt++;
            LOG_TRACE(LOG_ERR, false, __FUNCTION__," Send2Comet error " << "threadid=" << mThreadID);            
        }
	}

	return;
}


void CConsumeDealMsgThread::DirectDealMsg(StrConsumeMsg* pmsg, UINT64 groupid)
{
    if (NULL == pmsg)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__,"pmsg is null " << "threadid=" << mThreadID); 
        return;
    }

    if (DealMsg(pmsg->data, pmsg->uldatalen,groupid,false))
    {
        m_ulDealSucMsgCnt++;
    }
    else
    {
        m_ulDealFailMsgCnt++;
    }

    delete[] pmsg->data;
    pmsg->data = NULL;

    delete pmsg;
    pmsg = NULL;

    return;
}

void CConsumeDealMsgThread::CacheDealMsg(StrConsumeMsg* pmsg,const std::string& szgroupid, const std::string& szmsgtype, const std::string& szpageindex)
{
    if (NULL == pmsg)
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__,"pmsg is null " << "threadid=" << mThreadID); 
        return;
    }
    std::string szkey = szgroupid + "_" + szpageindex;

    if (szmsgtype == NEEDAPNS)
    {
         if (0 != m_group_msg_needapns_map.count(szkey))
         {
             delete[] pmsg->data;
             pmsg->data = NULL;
                        
             delete pmsg;
             pmsg = NULL;
         }
         else
         {
             m_group_msg_needapns_map[szkey] = pmsg;
         }        
    }
    else if (szmsgtype == NOTNEEDAPNS)
    {
        if (0 != m_group_msg_notneedapns_map.count(szkey))
        {
            delete[] pmsg->data;
            pmsg->data = NULL;
                        
            delete pmsg;
            pmsg = NULL;
        }
        else
        {
            m_group_msg_notneedapns_map[szkey] = pmsg;
        }
    }
    else
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__," error szmsgtype=" <<szmsgtype << " threadid:" << mThreadID);
        delete[] pmsg->data;
        pmsg->data = NULL;
                        
        delete pmsg;
        pmsg = NULL;
    }

    return;
}

void CConsumeDealMsgThread::OnDeaNeedApnsCacheTime()
{
    if (0 != m_group_msg_needapns_map.size())
    {
         LOG_TRACE(LOG_DEBUG, false, __FUNCTION__," m_needapns_msg_map cnt=" << m_group_msg_needapns_map.size() << " threadid:" << mThreadID);
    }

    auto iter = m_group_msg_needapns_map.begin();
    
    while (iter != m_group_msg_needapns_map.end())
    {
        StrConsumeMsg* pmsg = iter->second;

        if (DealMsg(pmsg->data, pmsg->uldatalen, GetGroupId(iter->first), true))
        {
            m_ulDealSucMsgCnt++;
        }
        else
        {
            m_ulDealFailMsgCnt++;
        }

        delete[] pmsg->data;
        pmsg->data = NULL;

        delete pmsg;
        pmsg = NULL;
        m_group_msg_needapns_map.erase(iter++);
    }

    return;
}
void CConsumeDealMsgThread::OnDealNoNeedApnsCacheTime()
{
    if (0 != m_group_msg_notneedapns_map.size())
    {
         LOG_TRACE(LOG_DEBUG, false, __FUNCTION__," m_noneedapns_msg_map cnt=" << m_group_msg_notneedapns_map.size() << " threadid:" << mThreadID);
    }

    auto iter = m_group_msg_notneedapns_map.begin();
    
    while (iter != m_group_msg_notneedapns_map.end())
    {
        StrConsumeMsg* pmsg = iter->second;

        if (DealMsg(pmsg->data, pmsg->uldatalen, GetGroupId(iter->first), false))
        {
            m_ulDealSucMsgCnt++;
        }
        else
        {
            m_ulDealFailMsgCnt++;
        }

        delete[] pmsg->data;
        pmsg->data = NULL;

        delete pmsg;
        pmsg = NULL;
        m_group_msg_notneedapns_map.erase(iter++);
    }

    return;
}
