#include "ServerMgr.h"
#include "network/packet/ClientPack.h"
#include "network/packet/PackTypeDef.h"
#include "ConsumeDealMsgThread.h"
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
                bool Isneedapns = msg->szkey==NEEDAPNS ? true:false;
                
                LOG_TRACE(LOG_DEBUG, false, __FUNCTION__," msglen:" << msg->uldatalen << " threadid:" << mThreadID);
                if (DealMsg(msg->data, msg->uldatalen, Isneedapns))
                {
                    m_ulDealSucMsgCnt++;
                }
                else
                {
                     m_ulDealFailMsgCnt++;
                }                
            }		

			delete[] msg->data;
			msg->data = NULL;

			delete msg;
			msg = NULL;
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

bool CConsumeDealMsgThread::DealMsg(BYTE*  data, uint32 uldatalen, bool IsNeedApns)
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
	
	//Analysis userid
	if (uldstlen < sizeof(USERID))
	{
		LOG_TRACE(LOG_ERR, false, __FUNCTION__," userid msg data len is abnormal "
                                         << "datalen=" << uldstlen);
		return false;
	}	
	USERID uiuserid = *((USERID *)pdatapos);
	pdatapos += sizeof(uiuserid);
	uldstlen -= sizeof(uiuserid);

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__," type=" << uitype << " userids= " << uiuserid << " isneedapns=" << IsNeedApns);
	return GenerateMsg(uitype, uiuserid, pdatapos, uldstlen, IsNeedApns);
}

bool CConsumeDealMsgThread::GenerateMsg(uint16 uimsgtype, USERID uiuserid,BYTE * pdata, uint32 uidatalen,bool IsNeedApns)
{
    std::string szserverid;
    
    if (m_apptye == "app")
    {
        szserverid = m_pServerMgr->mpRedisUtil->HGetLoginInfo(uiuserid, PLATFORM_APP);

    }
    else if (m_apptye == "web")
    {
        szserverid = m_pServerMgr->mpRedisUtil->HGetLoginInfo(uiuserid, PLATFROM_WEB);
    }
    else
    {
        LOG_TRACE(LOG_ERR, false, __FUNCTION__," server type is invalid, servertype=" << m_apptye);
        return false;
    }

	if (szserverid.empty())
	{
		//send apns msg
		if (m_apptye == "app" && IsNeedApns)
		{
            if (uimsgtype == NEW_DEF_CHAT_SERVER_SEND_MESSAGE_RQ 
                  || uimsgtype == NEW_DEF_SERVER_FRIEND_ADD_RQ   
                  || uimsgtype == NEW_DEF_SERVER_FRIEND_CONFIRM_RQ )
            {
                mulapnscnt++;
        	    CServerToKafka::GetInstance().DistributeScMsgToApns(uimsgtype,uiuserid, pdata, uidatalen, IM_APNS_TOPIC);
            }            
		}
        return true;
	}

    return SendPacketToServer(uimsgtype, uiuserid, szserverid, pdata, uidatalen);	
}

bool CConsumeDealMsgThread::SendPacketToServer(uint16 uimsgtype,USERID uiuserid, const std::string& szserverid,BYTE * pdata, uint32 uidatalen)
{
	BYTE lpBuffer[DEF_MAX_TCP_PACKET_LEN];
	BYTE* pos = lpBuffer;
	INT  idstlen = 0;

	//generate type
	memcpy(pos,(BYTE*)(&uimsgtype),sizeof(uimsgtype));
	pos += sizeof(uimsgtype);
	idstlen += sizeof(uimsgtype);

	//generate usercnt
	uint16 uiusercnt = 1;
	memcpy(pos,(BYTE*)(&uiusercnt),sizeof(uiusercnt));
	pos += sizeof(uiusercnt);
	idstlen += sizeof(uiusercnt);

	//generate userid
	memcpy(pos,(BYTE*)(&uiuserid),sizeof(uiuserid));
	pos += sizeof(uiuserid);
	idstlen += sizeof(uiuserid);

	//generate orgmsg
	memcpy(pos,pdata,uidatalen);
	idstlen += uidatalen;

    if (m_pServerMgr->Send2Comet(mThreadID, szserverid, lpBuffer, idstlen))
    {
        mulsendsuccnt++;
    }
    else
    {
        mulsendfailcnt++;
        return false;
    }

	return true;
}



