// $_FILEHEADER_BEGIN ****************************
// 版权声明: xxxxx版权所有
//	 
// 文件名称：cache.cpp
// 创建人： 史云杰
// 文件说明：GDBCS使用到结构
// $_FILEHEADER_END ******************************

#include "common/System.h"
#include "common/Debug_log.h"
#include "memcache/BaseMemCache.h"


CBaseMemeCache::CBaseMemeCache():miPort(0),memc(NULL),mbOpened(false)
{
	memset(mszIP,0,sizeof(mszIP));
}

CBaseMemeCache::~CBaseMemeCache()
{
	close();
}

bool CBaseMemeCache::open(const char* aszIP,uint16 aiPort)
{
	memcached_return iResult;                                                                   
 
	memc = memcached_create(NULL);

	if (NULL == memc)
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error call memcached_create"
			<<" MemcachedIP="<<aszIP
			<<":"<<aiPort
			);
		return false;
	}

	iResult = memcached_server_add(memc, aszIP, aiPort); //支持多台cache的分布式   

	if (iResult != MEMCACHED_SUCCESS)   
	{  
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error call memcached_server_add"
			<<" MemcachedIP="<<aszIP
			<<":"<<aiPort
			<<" Result="<<iResult
			); 
		return false;
	} 

	//使用二进制协议传输   
	iResult = memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1);  
	if (iResult != MEMCACHED_SUCCESS)   
	{  
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error call memcached_behavior_set MEMCACHED_BEHAVIOR_BINARY_PROTOCOL"
			<<" MemcachedIP="<<aszIP
			<<":"<<aiPort
			<<" Result="<<iResult
			); 
		return false;
	}  

	//使用NO-BLOCK，防止memcache崩溃时挂死           
	iResult = memcached_behavior_set(memc,MEMCACHED_BEHAVIOR_NO_BLOCK, 1);  
	if (iResult != MEMCACHED_SUCCESS)   
	{  
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error call memcached_behavior_set MEMCACHED_BEHAVIOR_NO_BLOCK"
			<<" MemcachedIP="<<aszIP
			<<":"<<aiPort
			<<" Result="<<iResult
			); 
		return false;
	}   

	char* lszTestKey = "memcache_open_test";

	mbOpened = true;
	if (!Insert(lszTestKey,lszTestKey,sizeof(lszTestKey)))
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error test insert "
			<<" MemcachedIP="<<aszIP
			<<":"<<aiPort
			); 
		mbOpened = false;
		return false;
	}

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, " Success"
		<<" MemcachedIP="<<aszIP
		<<":"<<aiPort
		); 

	
	mbOpened = true;
	return true;
}

void CBaseMemeCache::close()
{
	mbOpened = false;
	if (memc)
	{
		memcached_free(memc);
		memc = NULL;
	}
}

bool CBaseMemeCache::Insert(const char* aszKey,const char* apBuffer,size_t aiLen)
{
	if (!mbOpened || NULL == aszKey || NULL == apBuffer)
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, " Error"
			<<" Opened="<<mbOpened
			<<" KeyPtr="<<(void*)aszKey
			<<" BufferPtr="<<(void*)apBuffer
			<<" Len="<<aiLen
			); 
		return false;
	}

	uint32_t flags = 0;

	memcached_return rc;

	rc = memcached_set(memc, aszKey, strlen(aszKey),apBuffer, aiLen, 0, flags);

	if (MEMCACHED_SUCCESS != rc)
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, " Error"
			<<" Key="<<aszKey
			<<" Len="<<aiLen
			<<" rc="<<rc
			); 
		return false;
	}

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, " Success"
		<<" Key="<<aszKey
		<<" Len="<<aiLen
		); 
	return true;
}

bool CBaseMemeCache::Get(const char* aszKey,char* apBuffer,size_t& aiLen)
{
	if (!mbOpened || NULL == aszKey || NULL == apBuffer)
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, " Error"
			<<" Opened="<<mbOpened
			<<" KeyPtr="<<(void*)aszKey
			<<" BufferPtr="<<(void*)apBuffer
			<<" Len="<<aiLen
			); 
		return false;
	}

	size_t liBufferLen = aiLen;
	uint32 flags = 0;

	memcached_return rc;


	char* value = memcached_get(memc, aszKey, strlen(aszKey), &aiLen, &flags, &rc);

	// get ok
	if(rc != MEMCACHED_SUCCESS)
	{  
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, " Error"
			<<" Key="<<aszKey
			<<" rc="<<rc
			); 
		return false;
	}

	if (liBufferLen < aiLen)
	{
		LOG_TRACE(LOG_WARNING, false, __FUNCTION__, " Error overflow"
			<<" Key="<<aszKey
			<<" BufferLen="<<liBufferLen
			<<" Len="<<aiLen
			); 
		return false;
	}

	memcpy(apBuffer,value,aiLen);

	LOG_TRACE(LOG_DEBUG, true, __FUNCTION__, " Success"
		<<" Key="<<aszKey
		<<" Len="<<aiLen
		); 
	return true;
}

