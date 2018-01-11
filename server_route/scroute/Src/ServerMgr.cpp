#include "ServerMgr.h"
#include "Config.h"
#include "common/RefObjMgrTemplate.h"
#include "./ClientSide/ClientSide.h"
#include "ServerToKafka.h"

#ifdef _DEBUG
	#include <memory>
#endif


#ifndef WIN32 
#include <dlfcn.h>
#endif

CServerMgr::CServerMgr()
{
	mpClientSide = NULL;
	mpRedisUtil = NULL;
	miCurInex = 0;
}

CServerMgr::~CServerMgr()
{
	clear();
}

void CServerMgr::clear()
{

	SAFE_DELETE(mpClientSide);
	SAFE_DELETE(mpRedisUtil);

    for (auto it = mpCConsumeDealMsgThreadlist.begin(); it != mpCConsumeDealMsgThreadlist.end(); it++)
    {
        (*it)->Stop();
        SAFE_DELETE(*it);
    }

    for (auto it = mpCkafkaConsumeThreadlist.begin(); it != mpCkafkaConsumeThreadlist.end(); it++)
    {
        (*it)->Stop();
        SAFE_DELETE(*it);
    }

    CServerToKafka::GetInstance().releaseKafka();
}

INT CServerMgr::Open(int port, std::string& szapptype)
{
	printf("ServerMgr::Open begin\n");
	
	mpClientSide = new CClientSide(this );
	if (NULL == mpClientSide)
		throw;

    CServerToKafka::GetInstance().InitScApnsTopic();

	//加载端口
	goLocalServerInfo.mwServerPort=htons(port);
	printf("moConfig.Load\n");
    std::string szconfigfile = g_server_name + szapptype + ".ini";

	//加载配置文件
	if(1 !=moConfig.Load(szconfigfile.c_str()))
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," moConfig.Load Fail");
		return -1;
	}

	mpRedisUtil = new RedisUtil();
	if (NULL == mpRedisUtil)
		throw;

	//需要开启的REDIS实例
	std::vector<RedisUtil::ENUM_REDIS_TYPE> redis_type_list;
	redis_type_list.push_back(RedisUtil::REDIS_LOGIN);
	mpRedisUtil->Open(redis_type_list);

	printf("mpClientSide->open\n");	
	if(mpClientSide && !mpClientSide->open())
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," Error ClientSide open failed");
		return -1;

	}

    for (int i = 0; i < moConfig.m_kafkathreadcnt; i ++)
    {
        CKafkaConsumeThread * pCKafkaConsumeThread = new CKafkaConsumeThread(this, i);

        if (!pCKafkaConsumeThread->Init())
        {
            LOG_TRACE(LOG_ERR,false,__FUNCTION__," Error ConsumeDealMsgThread Init failed");
		    return -1;
        }
        
        mpCkafkaConsumeThreadlist.push_back(pCKafkaConsumeThread);
    }

    for (int i = 0; i < moConfig.m_consumethreadcnt; i ++)
    {
        CConsumeDealMsgThread * pCConsumeDealMsgThread = new CConsumeDealMsgThread(this, i, szapptype);

        if (!pCConsumeDealMsgThread->Init())
        {
            LOG_TRACE(LOG_ERR,false,__FUNCTION__," Error ConsumeDealMsgThread Init failed");
		    return -1;
        }
        
        mpCConsumeDealMsgThreadlist.push_back(pCConsumeDealMsgThread);
    }

	return 1;

}

INT CServerMgr::Close()
{
	clear();
	CSystem::Sleep(200);
	return true;
}

void CServerMgr::AddTailMsg(StrConsumeMsg* msg)
{

    if (miCurInex >=  moConfig.m_consumethreadcnt)
    {
        miCurInex = 0;
    }

    CConsumeDealMsgThread* pConsumeDealMsgThread = mpCConsumeDealMsgThreadlist[miCurInex];
    if (NULL == pConsumeDealMsgThread)
    {
        LOG_TRACE(LOG_ERR,false, __FUNCTION__, " pConsumeDealMsgThread is null ");
    }
	pConsumeDealMsgThread->AddTailMsg(msg);
    miCurInex++;
}

bool CServerMgr::Send2Comet(uint32 threadid, const std::string & szserverid , BYTE * pdata,uint32 uidatalen)
{
    
    if (NULL != mpClientSide)
    {
        return mpClientSide->Send2Comet(threadid, szserverid, pdata, uidatalen);
    }
    
    return false;
}

UINT64 CServerMgr::GetKafkaConsumeSucCnt()
{
    UINT64 msgcnt = 0;

    for (auto it = mpCkafkaConsumeThreadlist.begin(); it != mpCkafkaConsumeThreadlist.end(); it++)
    {
        msgcnt += (*it)->GetKafkaSucMsgCnt();
    }
    
	return msgcnt;
}

UINT64 CServerMgr::GetKafkaConsumeFailCnt()
{
    UINT64 msgcnt = 0;

    for (auto it = mpCkafkaConsumeThreadlist.begin(); it != mpCkafkaConsumeThreadlist.end(); it++)
    {
        msgcnt += (*it)->GetKafkaFailMsgCnt();
    }
    
	return msgcnt;	
}

UINT64 CServerMgr::GetDealConsumeSucCnt()
{
    /*
	if (mpCConsumeDealMsgThread)
	{
		return mpCConsumeDealMsgThread->GetConsumeDealSucMsgCnt();
	}*/
	
	return 0;
}

UINT64 CServerMgr::GetDealConsumeFailCnt()
{
    /*
	if (mpCConsumeDealMsgThread)
	{
		return mpCConsumeDealMsgThread->GetConsumeDealFailedMsgCnt();
	}*/
	
	return 0;
}

UINT64 CServerMgr::GetDealConsumeApnsCnt()
{
    
    UINT64 msgcnt = 0;

    for (auto it = mpCConsumeDealMsgThreadlist.begin(); it != mpCConsumeDealMsgThreadlist.end(); it++)
    {
        msgcnt += (*it)->GetApnsCnt();
    }
    
	return msgcnt;
}
	
UINT64 CServerMgr::GetQueueMsgCnt()
{
    UINT64 msgcnt = 0;

    for (auto it = mpCConsumeDealMsgThreadlist.begin(); it != mpCConsumeDealMsgThreadlist.end(); it++)
    {
        msgcnt += (*it)->GetConsumeQueueCnt();
    }
    
	return msgcnt;
}


