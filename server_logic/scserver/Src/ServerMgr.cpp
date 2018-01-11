#include "stdafx.h"
#include "ServerMgr.h"

#include "./ClientSide/ClientSide.h"
#include "ServerToKafka.h"

#ifdef _DEBUG
	#include <memory>
#endif


#ifndef WIN32 

#include <dlfcn.h>
#endif //WIN32

CServerMgr::CServerMgr()
{
	mpClientSide = NULL;
	mpRedisUtil = NULL;

	mpKafkaMgr = NULL;
}

CServerMgr::~CServerMgr()
{
	clear();
}
void CServerMgr::clear()
{
	// 需要删除两个单例
	ServerPack::GetInstance().clear();
	ClientPack::GetInstance().Close();
	CServerToKafka::GetInstance().releaseKafka();

	if(mpKafkaMgr)
	{
		mpKafkaMgr->ReleaseProducer();
		SAFE_DELETE(mpKafkaMgr);
	}

	SAFE_DELETE(mpClientSide);
	SAFE_DELETE(mpRedisUtil);

	moUserConMgr.clear();
}


INT CServerMgr::Open(int port)
{
	giLocalTimer = CSystem::GetTime();

    mpSensitiveMgr = new SensitiveMgr();

	mpClientSide = new CClientSide(this );
	if (NULL == mpClientSide)
		throw;

	mpRedisUtil = new RedisUtil();
	if (NULL == mpRedisUtil)
		throw;

	mpKafkaMgr = new CKafkaMgr();
	if (NULL == mpKafkaMgr)
		throw;

	CServerToKafka::GetInstance().InitScRouteTopic();

	//加载端口
	goLocalServerInfo.mwServerPort=htons(port);
	//加载配置文件
	if(1 !=moConfig.Load(g_server_config.c_str()))
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," moConfig.Load Fail");
		return -1;
	}
	LOG_TRACE(LOG_ALERT,false,__FUNCTION__," moConfig.Load MaxConnCount = " << moConfig.miMaxConnCount);
	moUserConMgr.init(moConfig.miMaxConnCount);

	//需要开启的REDIS实例
	std::vector<RedisUtil::ENUM_REDIS_TYPE> redis_type_list;
	redis_type_list.push_back(RedisUtil::REDIS_MESSAGE);
	redis_type_list.push_back(RedisUtil::REDIS_ASYNC_DB);
	mpRedisUtil->Open(redis_type_list);
	
	if(mpClientSide && !mpClientSide->open())
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," Error ClientSide open failed");
		return -1;
	}

	mpKafkaMgr->InitScRouteTopic();
	return 1;

}

INT CServerMgr::Close()
{
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " CLose success");
	clear();
	CSystem::Sleep(200);
	return true;
}







