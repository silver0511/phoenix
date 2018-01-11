// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: PacketQueue.h
// 创 建 人: 史云杰
// 文件说明: 包队列
// $_FILEHEADER_END ******************************

#ifndef __PACKETEQUEUE_H
#define __PACKETEQUEUE_H


#include "const/constdef.h"
#include "common/Queue2.h"
#include "common/CacheList.h"

using namespace std;
using namespace U9;

struct STRU_PACK_DATA
{
	WORD mwDataLen;
	BYTE mpData[DEF_MAX_GENERAL_PACK_LEN];
public:
	STRU_PACK_DATA()
	{
		init();
	}
	void init()
	{
		mwDataLen = 0;
	}
};

typedef CDoubleQueue<STRU_PACK_DATA>::STRU_NODE_INFO STRU_PACK_NODE_INFO;

class CPacketQueue:public CDoubleQueue<STRU_PACK_DATA>
{
public:
	CPacketQueue();
	~CPacketQueue();
	
	virtual void* New_Node();
	virtual void  Free_Node(STRU_PACK_NODE_INFO* apNode);
	static void initCache(int32 aiCount);
public:
	static CCachePool<STRU_PACK_NODE_INFO> moNodeCacheList;//节点cache
	static CCachePool<STRU_PACK_DATA> moPacketCache; //包cache
};


#endif//__2ROIPACKEQUEUE_H_