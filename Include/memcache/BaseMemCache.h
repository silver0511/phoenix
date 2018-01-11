// $_FILEHEADER_BEGIN ****************************
// 版权声明: xxxxx版权所有
//  
// 文件名称：cache.h
// 创建人： 史云杰
// 文件说明：缓存
// $_FILEHEADER_END ******************************
#ifndef __BASE_MEM_CACHE_H
#define __BASE_MEM_CACHE_H

#include "libmemcached/memcached.h"

class CBaseMemeCache
{
public:
	CBaseMemeCache();
	~CBaseMemeCache();

	bool Get(const char* aszKey,char* apBuffer,size_t& aiLen);
	bool Insert(const char* aszKey,const char* apBuffer,size_t aiLen);
	bool open(const char* aszIP,uint16 aiPort);
	void close();
private:
	bool	mbOpened;
	char	mszIP[32];
	uint16	miPort;
	memcached_st* memc; 

};

#endif //__BASE_MEM_CACHE_H
