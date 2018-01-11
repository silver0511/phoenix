// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: PacketQueue.cpp
// 创 建 人: 史云杰
// 文件说明: cache类
// $_FILEHEADER_END ******************************

#ifndef __CACHELIST_H
#define __CACHELIST_H
#include <map>
#include "base0/platform.h"
#include "common/Queue.h"


U9_BEGIN_NAMESPACE

// CCachePool的辅助类，绝不应该单独使用
template<class T>
class CCountStaticQueue 
{
public:
	CCountStaticQueue()
	{
		InitMember();
		mpDataList=NULL;
	};
	CCountStaticQueue(INT aiMaxCount)
	{
		InitMember();
		mpDataList=NULL;
		miMaxCount=aiMaxCount;
		mpDataList= new T*[miMaxCount];
		if(NULL == mpDataList)
			throw;
	};
	void init(INT aiMaxCount)
	{

		moCriticalSection.Enter();		
		clear();
		if(NULL != mpDataList)
		{
			delete [] mpDataList;
			mpDataList=NULL;
		}
		miMaxCount=aiMaxCount;
		mpDataList= new T*[miMaxCount];
		if(NULL == mpDataList)
		{
			moCriticalSection.Leave();
			throw;
		}
		memset(mpDataList,0,miMaxCount*sizeof(T*));
		moCriticalSection.Leave();
	}
	~CCountStaticQueue()
	{
		moCriticalSection.Enter();
		if(NULL ==mpDataList)
		{
			moCriticalSection.Leave();
			return ;
		}
		INT liIndex=miHeadIndex;
		T* lpValue=NULL;
		for(INT i=0;i<miCount;i++)
		{
			lpValue	=mpDataList[liIndex];
			if(NULL != lpValue)
			{
				this->FreeBuffer(lpValue);
				mpDataList[liIndex] = NULL;
				liIndex = INC_CIRCLE_INDEX(liIndex,1,miMaxCount);
			}			
		}
		if(NULL != mpDataList)
		{
			delete [] mpDataList;
			mpDataList=NULL;
		}
		moCriticalSection.Leave();
	}

	void FreeBuffer(T* apItem)
	{
		Destruct(apItem);
		uint8* lpBuffer = reinterpret_cast<uint8*>(apItem);
		lpBuffer = lpBuffer - 1;
		delete[] lpBuffer;
	}


	inline INT size(){return miCount;}
	inline INT maxsize(){return miMaxCount;}

	inline void clear()
	{
		moCriticalSection.Enter();
		if(NULL ==mpDataList)
		{
			moCriticalSection.Leave();
			return ;
		}
		INT liIndex=miHeadIndex;
		T* lpValue=NULL;
		for(INT i=0;i<miCount;i++)
		{
			lpValue	=mpDataList[liIndex];
			if(NULL != lpValue)
			{
				this->FreeBuffer(lpValue);
				mpDataList[liIndex] = NULL;
				liIndex = INC_CIRCLE_INDEX(liIndex,1,miMaxCount);
			}			
		}
		//memset(mpDataList,0,miMaxCount*sizeof(T*));			
		//InitMember();
		moCriticalSection.Leave();

	};

	inline T * GetandDelHead()
	{
		T *lpValue =NULL;
		moCriticalSection.Enter();
		if(miCount>0)
		{
			lpValue = mpDataList[miHeadIndex];
			mpDataList[miHeadIndex]=NULL;
			miCount--;
			miHeadIndex = INC_CIRCLE_INDEX(miHeadIndex,1,miMaxCount);
		}
		moCriticalSection.Leave();

		return lpValue;
	};

	inline T * Get()
	{
		T *lpValue =NULL;
		moCriticalSection.Enter();
		if(miCount>0)
		{
			lpValue = mpDataList[miHeadIndex];
		}
		moCriticalSection.Leave();

		return lpValue;
	};
	inline INT AddTail(T *apValue)
	{
		if(NULL ==apValue)
			return -1;
		moCriticalSection.Enter();
		if (miCount < miMaxCount)
		{
			mpDataList[INC_CIRCLE_INDEX(miHeadIndex,miCount,miMaxCount)]=apValue;
			miCount++;
			moCriticalSection.Leave();
			return 1;
		}
		moCriticalSection.Leave();	
		return 0;
	}
	inline void InitMember()
	{
		mbIsFrist=TRUE;
		miCount=0;
		miMaxCount=0;
		miHeadIndex=0;
		mpDataList = NULL;
	}
protected:
	U9::CCriticalSection moCriticalSection;
	INT		miMaxCount;
	INT		miCount;
	T		**mpDataList;

	INT	miHeadIndex;//头
	BOOL	mbIsFrist;
};

///////////////////////////////////////////////////////////////////
//  cache池
template<typename T>
class CCachePool:private CCountStaticQueue<T>
{
public:
	CCachePool()
	{
		miMallocCount = 0;
		miRemainCount = 0;
	};
	~CCachePool(){clear();}
	inline void init(int aiMaxCache)
	{
		CCountStaticQueue<T>::init(aiMaxCache);
		miMallocCount = 0;
		miRemainCount = aiMaxCache;
	}

	inline T* malloc()
	{
		T* lpItem=	CCountStaticQueue<T>::GetandDelHead();
		if(NULL == lpItem)
		{
			size_t liSize =  sizeof(char) +sizeof(T);
			char* lpBuffer = new char[liSize];
			//memset(lpBuffer, 0, liSize);
			lpBuffer[0] = 0;
			lpItem = reinterpret_cast<T*>(lpBuffer + 1);
			Construct(lpItem);
			//LOG_TRACE(LOG_ERR, true, __FUNCTION__, "  malloc new obj"
			//	<<" ObjectPoolDump "
			//	<<" miMaxCount="<<CCountStaticQueue<T>::maxsize()
			//	<<" miCount="<<CCountStaticQueue<T>::size()
			//	<<" MallocCount="<<miMallocCount
			//	);
		}
		else
		{
			//LOG_TRACE(LOG_ERR, true, __FUNCTION__, "  malloc exsit obj"
			//	<<" ObjectPoolDump "
			//	<<" miMaxCount="<<CCountStaticQueue<T>::maxsize()
			//	<<" miCount="<<CCountStaticQueue<T>::size()
			//	<<" MallocCount="<<miMallocCount
			//	);
		}
		if(Increase(lpItem) != 1)
		{
			//U9_ASSERT(FALSE && "malloc one buffer more than one times");
		}
		miMallocCount++;
		miRemainCount--;
		return lpItem;
	}

	uint8 Increase(T* apItem)
	{
		uint8* lpBuffer = reinterpret_cast<uint8*>(apItem);
		lpBuffer = lpBuffer - 1;

		return ++(*lpBuffer);
	}

	uint8 Decrease(T* apItem)
	{
		uint8* lpBuffer = reinterpret_cast<uint8*>(apItem);
		lpBuffer = lpBuffer - 1;

		return --(*lpBuffer);
	}


	inline void free(T* apItem)
	{
#ifdef _DEBUG
		if (apItem==NULL)
		{
#endif
			//U9_ASSERT(apItem != NULL);
#ifdef _DEBUG
			return;
		}
#endif
		if( Decrease(apItem) != 0)
		{
			//LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error: Free One Buffer more than one times."
			//	<<" pointer=" <<(void*)apItem);
			//U9_ASSERT(false && "free one buffer more than one times");
			return;
		}

		miMallocCount--;
		miRemainCount++;
		if(1 != CCountStaticQueue<T>::AddTail(apItem))
		{
			LOG_TRACE(LOG_WARNING, true, __FUNCTION__, "  free obj"
				<<" ObjectPoolDump "
				<<" miMaxCount="<<CCountStaticQueue<T>::maxsize()
				<<" miCount="<<CCountStaticQueue<T>::size()
				<<" MallocCount="<<miMallocCount
				<<" miRemainCount="<<miRemainCount
				);
			this->FreeBuffer(apItem);
		}
		else
		{
			//LOG_TRACE(LOG_ERR, true, __FUNCTION__, "  put obj"
			//	<<" ObjectPoolDump "
			//	<<" miMaxCount="<<CCountStaticQueue<T>::maxsize()
			//	<<" miCount="<<CCountStaticQueue<T>::size()
			//	<<" MallocCount="<<miMallocCount
			//	);
		}
	}
	inline INT size()
	{
		return CCountStaticQueue<T>::size();
	}

	inline INT maxsize()
	{
		return CCountStaticQueue<T>::maxsize();
	}

	void clear()
	{
		CCountStaticQueue<T>::clear();
		miMallocCount = 0;
		miRemainCount = 0;

	}

	inline int32 remaincount()
	{
		return miRemainCount;
	}

	inline U9::CCriticalSection &GetCriticalSection(){return CCountStaticQueue<T>::moCriticalSection;}
private:
	int32	miMallocCount;
	int32	miRemainCount;
};

template<typename T>
class CStaticCachePool:private CSafeStaticQueue<T>
{
public:
	CStaticCachePool()
	{
		mpCacheDataList = NULL;
	}
	~CStaticCachePool()
	{
		 clear();
	}

	inline void init(int aiMaxCache)
	{
		CSafeStaticQueue<T>::moCriticalSection.Enter();
        clear();
		CSafeStaticQueue<T>::init(aiMaxCache);

		mpCacheDataList= new T*[CSafeStaticQueue<T>::miMaxCount];
		if(NULL == mpCacheDataList)
		{
			CSafeStaticQueue<T>::moCriticalSection.Leave();
			throw;
		}
        memset(mpCacheDataList,0,CSafeStaticQueue<T>::miMaxCount*sizeof(T*));
		for(int i = 0; i < aiMaxCache; ++i)
		{
			T* lpItem = new T;
			mpCacheDataList[i] = lpItem;
			lpItem->init();
			CSafeStaticQueue<T>::AddTail(lpItem);
		}
		CSafeStaticQueue<T>::moCriticalSection.Leave();
	}
	inline T* malloc()
	{
		return CSafeStaticQueue<T>::GetandDelHead();
		
	}
	inline void free(T* P)
	{
		CSafeStaticQueue<T>::AddTail(P);
	}
	inline INT size()
	{
		return CSafeStaticQueue<T>::size();
	}
    inline INT maxsize()
    {
        return CSafeStaticQueue<T>::maxsize();
    }
	void clear()
	{
		CSafeStaticQueue<T>::clear();
		if(NULL != mpCacheDataList)
		{
			delete [] mpCacheDataList;
			mpCacheDataList=NULL;
		}
	}
	inline U9::CCriticalSection &GetCriticalSection(){return CSafeStaticQueue<T>::moCriticalSection;}
	inline T** GetCacheList() const { return mpCacheDataList;}
protected:
	T **mpCacheDataList;
};


template<typename T>
class CSimpleCachePool:private CSafeStaticQueue<T>
{
private:
	INT miRemainCount;
public:
	CSimpleCachePool()
	{
		miRemainCount = 0;
	}
	~CSimpleCachePool()
	{
		clear();
	}


	inline void init(int aiMaxCache)
	{
		CSafeStaticQueue<T>::init(aiMaxCache);
		miRemainCount = aiMaxCache;
	}

	inline T* malloc()
	{
		T* lpItem=	CSafeStaticQueue<T>::GetandDelHead();
		if(NULL == lpItem)
		{
			lpItem = new T;
			//LOG_TRACE(LOG_ERR, true, __FUNCTION__, "  malloc new obj1");
		}
		else
		{
			//LOG_TRACE(LOG_ERR, true, __FUNCTION__, "  malloc exsist obj1");
		}

		miRemainCount--;
		return lpItem;
	}

	inline void free(T* P)
	{

		if(1 != CSafeStaticQueue<T>::AddTail(P))
		{
			delete P;
			//LOG_TRACE(LOG_ERR, true, __FUNCTION__, "  free obj1");
		}
		else
		{
			//LOG_TRACE(LOG_ERR, true, __FUNCTION__, "  put obj1");
		}

		miRemainCount++;
	}
	inline INT size()
	{
		return CSafeStaticQueue<T>::size();
	}
	inline INT maxsize()
	{
		return CSafeStaticQueue<T>::maxsize();
	}
	void clear()
	{
		CSafeStaticQueue<T>::clear();
		miRemainCount = 0;
	}
	inline INT remaincount()
	{
		return miRemainCount;
	}
	inline U9::CCriticalSection &GetCriticalSection(){return CSafeStaticQueue<T>::moCriticalSection;}
};

template<typename T>
class CSimpleStaticCachePool:private CSafeStaticQueue<T>
{
public:
	CSimpleStaticCachePool()
	{
		mpCacheDataList = NULL;
	}
	~CSimpleStaticCachePool()
	{
		clear();
	}

	inline void init(int aiMaxCache)
	{
		CSafeStaticQueue<T>::moCriticalSection.Enter();
		clear();
		CSafeStaticQueue<T>::init(aiMaxCache);

		mpCacheDataList= new T*[CSafeStaticQueue<T>::miMaxCount];
		if(NULL == mpCacheDataList)
		{
			CSafeStaticQueue<T>::moCriticalSection.Leave();
			throw;
		}
		memset(mpCacheDataList,0,CSafeStaticQueue<T>::miMaxCount*sizeof(T*));
		for(int i = 0; i < aiMaxCache; ++i)
		{
			T* lpItem = new T;
			mpCacheDataList[i] = lpItem;
			lpItem->init();
			CSafeStaticQueue<T>::AddTail(lpItem);
		}
		CSafeStaticQueue<T>::moCriticalSection.Leave();
	}
	inline T* malloc()
	{
		return CSafeStaticQueue<T>::GetandDelHead();

	}
	inline void free(T* P)
	{
		CSafeStaticQueue<T>::AddTail(P);
	}
	inline INT size()
	{
		return CSafeStaticQueue<T>::size();
	}
	inline INT maxsize()
	{
		return CSafeStaticQueue<T>::maxsize();
	}
	void clear()
	{
		CSafeStaticQueue<T>::clear();
		if(NULL != mpCacheDataList)
		{
			delete [] mpCacheDataList;
			mpCacheDataList=NULL;
		}
	}
	inline U9::CCriticalSection &GetCriticalSection(){return CSafeStaticQueue<T>::moCriticalSection;}
	inline T** GetCacheList() const { return mpCacheDataList;}
protected:
	T **mpCacheDataList;
};


template<class _T>
class CSafeCacheObjectPool
{
private:
	class CNode:public _T
	{
	public:
		CNode()
		{
			mpNext = NULL;
		}
		CNode *mpNext;
	};
public:
	CSafeCacheObjectPool()
	{
		mpNodeHead = NULL;
		miMaxCacheCount = 0;
		miCacheCount = 0;
		miMallocCount = 0;
	}
	~CSafeCacheObjectPool()
	{
		clear();
	}
	void init(int aiMaxCacheCount)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		miMaxCacheCount = aiMaxCacheCount;
		miMallocCount = 0;
	}
	void clear()
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		CNode *lpNode = NULL;
		while (NULL != mpNodeHead)
		{
			lpNode = mpNodeHead;
			mpNodeHead = mpNodeHead->mpNext;
			delete lpNode;
		}
		miMallocCount = 0;
	}
	_T* malloc()
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		CNode *lpNode = mpNodeHead;
		if(lpNode == NULL)
		{
			lpNode = new CNode;
			lpNode->mpNext = NULL;

			miMallocCount++;
			//LOG_TRACE(7, true, __FUNCTION__, "  malloc new obj"
			//	<<" ObjectPoolDump "
			//	<<" MaxCacheCount="<<miMaxCacheCount
			//	<<" CacheCount="<<miCacheCount
			//	<<" MallocCount="<<miMallocCount
			//	);
			return (_T*)lpNode;
		};
		mpNodeHead=mpNodeHead->mpNext;
		if(miCacheCount >0)
			miCacheCount--;

		miMallocCount++;
		//LOG_TRACE(7, true, __FUNCTION__, "  malloc exsit obj"
		//	<<" ObjectPoolDump "
		//	<<" MaxCacheCount="<<miMaxCacheCount
		//	<<" CacheCount="<<miCacheCount
		//	<<" MallocCount="<<miMallocCount
		//	);
		return (_T*)lpNode;
	}
	void free(_T* apValue)
	{
		CNode *lpNode =(CNode*)apValue;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		if(miCacheCount>=miMaxCacheCount)
		{
			miMallocCount--;
			//LOG_TRACE(7, true, __FUNCTION__, "  free obj"
			//	<<" ObjectPoolDump "
			//	<<" MaxCacheCount="<<miMaxCacheCount
			//	<<" CacheCount="<<miCacheCount
			//	<<" MallocCount="<<miMallocCount
			//	);
			delete lpNode;
			return ;
		}

		lpNode->mpNext = mpNodeHead;
		mpNodeHead = lpNode;
		miCacheCount++;
		miMallocCount--;

		//LOG_TRACE(7, true, __FUNCTION__, "  put obj"
		//	<<" ObjectPoolDump "
		//	<<" MaxCacheCount="<<miMaxCacheCount
		//	<<" CacheCount="<<miCacheCount
		//	<<" MallocCount="<<miMallocCount
		//	);
	}

private:

	CNode	*mpNodeHead;//空闲头
	int		miMaxCacheCount;//最大cache数
	int		miCacheCount;//当前cahche数
	int32	miMallocCount;
	CCriticalSection moCriticalSection;
};

U9_END_NAMESPACE

#endif// __CACHELIST_H

