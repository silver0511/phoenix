#include "stdafx.h"
#include "ServerMgr.h"

//#include "./ServerSide/ServerSide.h"
#include "./ClientSide/ClientSide.h"


#ifdef _DEBUG
	#include <memory>
#endif


#ifndef WIN32 

#include <dlfcn.h>
#endif //WIN32

CServerMgr::CServerMgr()
{
	//mpServerSide = NULL;
	mpClientSide = NULL;
	mpRedisUtil = NULL;
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

	//SAFE_DELETE(mpServerSide);
	SAFE_DELETE(mpClientSide);
	SAFE_DELETE(mpRedisUtil);

	moUserConMgr.clear();
}


INT CServerMgr::Open(int port)
{
	giLocalTimer = CSystem::GetTime();

	//mpServerSide = new CServerSide(this);
	//if (NULL == mpServerSide)
	//	throw;

	mpClientSide = new CClientSide(this );
	if (NULL == mpClientSide)
		throw;

	mpRedisUtil = new RedisUtil();
	if (NULL == mpRedisUtil)
		throw;

	CServerToKafka::GetInstance().InitOffcialRouteTopic();
	//CServerToKafka::GetInstance().InitGcRouteTopic();

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
	redis_type_list.push_back(RedisUtil::REDIS_ASYNC_DB);
	redis_type_list.push_back(RedisUtil::REDIS_MESSAGE);
	mpRedisUtil->Open(redis_type_list);
	
	if(mpClientSide && !mpClientSide->open())
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," Error ClientSide open failed");
		return -1;
	}

	//if(mpServerSide && !mpServerSide->open())
	//{
	//	LOG_TRACE(LOG_ERR,false,__FUNCTION__," Error ServeSide open failed");
	//	return -1;
	//}

	return 1;
}

INT CServerMgr::Close()
{
	LOG_TRACE(LOG_DEBUG, false, __FUNCTION__, " CLose success");
	clear();
	CSystem::Sleep(200);
	return true;
}







