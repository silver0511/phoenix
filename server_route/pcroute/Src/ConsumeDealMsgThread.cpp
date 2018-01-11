#include "ServerMgr.h"
#include "network/packet/ClientPack.h"
#include "network/packet/PackTypeDef.h"
#include "ConsumeDealMsgThread.h"
#include "ServerUtilMgr.h"
#include "ServerToKafka.h"

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
	while (getRunState())
	{	
		StrConsumeMsg* msg = GetandDelHead();
		if (NULL != msg)
		{
            if (msg->szkey == ONLYWEBNEED && m_apptye == "app")
            {

            }
            else
            {
                LOG_TRACE(LOG_DEBUG, false, __FUNCTION__," msglen:" << msg->uldatalen << " threadid:" << mThreadID);
    			if (DealMsg(msg->data, msg->uldatalen))
    			{
    				m_ulDealSucMsgCnt++;
    			}
    			else
    			{
    				m_ulDealFailMsgCnt++;
    			}

    			delete[] msg->data;
    			msg->data = NULL;

    			delete msg;
    			msg = NULL;
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

bool CConsumeDealMsgThread::DealMsg(BYTE*  data, uint32 uldatalen)
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

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__," type=" << uitype << " usercnt=" << uiusercnt << " userids=" << szuserids << " threadid=" << mThreadID);


    //class userid by appserverids
	std::multimap<SERVERID,USERID> szappserverid_userid_map;
    std::set<SERVERID> szappserverids;

    if (0 == DivisionUserid(userids, szappserverid_userid_map, szappserverids, uitype, pdatapos, uldstlen))
    {
        return true;
    }

    Send2Comet(uitype, szappserverids, szappserverid_userid_map, pdatapos, uldstlen);		
	return true;
}

uint32 CConsumeDealMsgThread::DivisionUserid(std::vector<USERID> & userids, 
                                                  std::multimap<SERVERID,USERID>& szappserverid_userid_map,
                                                  std::set<SERVERID>& szappserverids,
                                                  uint16 uimsgtype,
                                                  BYTE * pdata,
                                                  uint32 uidatalen)
{
    USERID OffineUserList[STRU_BACK_2_COMET_HEAD_PACKAGE::LIST_COUNT] = {0};
    uint16 uiofflineusercnt = 0;
    m_pServerMgr->Getuseridlogins(userids, szappserverid_userid_map, szappserverids, OffineUserList,uiofflineusercnt, m_apptye);

    if ( 0 != uiofflineusercnt)
    {
        if (uimsgtype == NEW_DEF_SERVER_OFFCIAL_MESSAGE_ID)
        {
             CServerToKafka::GetInstance().DistributeGcMsgToApns(uimsgtype,OffineUserList, uiofflineusercnt, pdata, uidatalen, IM_APNS_TOPIC);
        }           
    }

    return szappserverids.size();        
}

void CConsumeDealMsgThread::Send2Comet(uint16 uimsgtype,
									 	std::set<SERVERID> serverids,
										std::multimap<SERVERID, USERID> serverid_userid_map, 
										BYTE * pdata,
										uint32 uidatalen)
{
	typedef std::multimap<SERVERID, USERID>::iterator multiMapItor;

	BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
	BYTE* pos = NULL;
	INT  idstlen = 0;

	USERID UserList[STRU_BACK_2_COMET_HEAD_PACKAGE::LIST_COUNT] = {0};
	uint16 uiusercnt = 0;
	SOCKET socket = -1;

	for (std::set<SERVERID>::iterator iter = serverids.begin(); iter != serverids.end(); iter++)
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
            LOG_TRACE(LOG_DEBUG, true, __FUNCTION__," Send2Comet error " << "threadid=" << mThreadID);            
        }
	}

	return;
}

