// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//			 
// 文件名称: ContextDQueue.h
// 创 建 人: 史云杰
// 文件说明: 上下文双向队列
// $_FILEHEADER_END *****************************

#ifndef __CONTEXTDQUEUE_H
#define __CONTEXTDQUEUE_H

#include "base0/platform.h"
#include "common/RefObjMgrTemplate.h"
#include "common/Queue.h"
#include "common/Queue2.h"
#include "common/CacheList.h"

using namespace std;
using namespace U9;

template <class _T> 
class CContextDQueue :public CSafeDoubleQueue<_T>
{ 
public:
	CContextDQueue()
	{
	}
	static void initCache(int aiMaxCache)
	{
		moCachePool.init(aiMaxCache);
	}

	static void clearCache()
	{
		moCachePool.clear();
	}
	virtual ~CContextDQueue() 
	{
		clear();
	}
	virtual void clear()
	{
		CSafeDoubleQueue<_T>::clear();
	}
protected:
	virtual void* New_Node()
	{
		return moCachePool.malloc();
	}
	virtual void  Free_Node(typename CSafeDoubleQueue<_T>::STRU_NODE_INFO* apNode)
	{
		U9_ASSERT(apNode != INVALID_U9_POSITION);
		moCachePool.free(apNode);
	}
private:
	static CSimpleCachePool<typename CSafeDoubleQueue<_T>::STRU_NODE_INFO>	moCachePool;			//cache

};

#ifndef _WIN32
template < class T >
CSimpleCachePool< typename CSafeDoubleQueue<T>::STRU_NODE_INFO > CContextDQueue < T >::moCachePool;
#endif

#endif // __CONTEXTDQUEUE_H
