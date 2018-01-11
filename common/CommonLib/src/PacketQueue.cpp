// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: PacketQueue.cpp
// 创 建 人: 史云杰
// 文件说明: 内存列化类
// $_FILEHEADER_END ******************************

#include "stdafx.h"
#include "PacketQueue.h"

CCachePool<STRU_PACK_NODE_INFO> CPacketQueue::moNodeCacheList;
CCachePool<STRU_PACK_DATA> CPacketQueue::moPacketCache;
CPacketQueue::CPacketQueue()
{
}
CPacketQueue::~CPacketQueue()
{

}
void* CPacketQueue::New_Node()
{
	return moNodeCacheList.malloc() ;
}
void  CPacketQueue::Free_Node(STRU_PACK_NODE_INFO* apNode)
{
	moNodeCacheList.free(apNode);
}
void  CPacketQueue::initCache(int32 aiCount)
{
	moNodeCacheList.init(aiCount);
	moPacketCache.init(aiCount);
}