#include "ServerMgr.h"
#include "Config.h"
#include "common/RefObjMgrTemplate.h"
#include "./ClientSide/ClientSide.h"

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
}

CServerMgr::~CServerMgr()
{
	clear();
}

void CServerMgr::clear()
{

	SAFE_DELETE(mpClientSide);
	SAFE_DELETE(mpRedisUtil);

}

INT CServerMgr::Open(int port)
{
	printf("ServerMgr::Open begin\n");
	
	mpClientSide = new CClientSide(this );
	if (NULL == mpClientSide)
		throw;

	//加载端口
	goLocalServerInfo.mwServerPort=htons(port);
	printf("moConfig.Load\n");	
	//加载配置文件
	if(1 !=moConfig.Load(g_server_config.c_str()))
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," moConfig.Load Fail");
		return -1;
	}

	mpRedisUtil = new RedisUtil();
	if (NULL == mpRedisUtil)
		throw;

	//需要开启的REDIS实例
	std::vector<RedisUtil::ENUM_REDIS_TYPE> redis_type_list;
	redis_type_list.push_back(RedisUtil::REDIS_MESSAGE);
	mpRedisUtil->Open(redis_type_list);

	printf("mpClientSide->open\n");	
	if(mpClientSide && !mpClientSide->open())
	{
		LOG_TRACE(LOG_ERR,false,__FUNCTION__," Error ClientSide open failed");
		return -1;

	}
	
	return 1;

}

INT CServerMgr::Close()
{
	clear();
	CSystem::Sleep(200);
	return true;
}


