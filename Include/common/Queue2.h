// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：Queue2.h
// 创建人：  史云杰
// 文件说明：队列（双链表实现,包括遍历删除功能,也可以做为一般队列使用)
//          每个节点包含的数据mpData,不需要手工从外面删除，里面在析构
//          的时候会自动清除.
// $_FILEHEADER_END ******************************

#ifndef __QUEUE2_H
#define __QUEUE2_H

#include "base0/platform.h"
#include "CriticalSection.h"

U9_BEGIN_NAMESPACE

typedef	void* U9_POSITION;
#define INVALID_U9_POSITION NULL

//////////////////////////////////////////////////////////////////////////
// CDoubleQueue
template <class _T> class CDoubleQueue
{
public:
	struct STRU_NODE_INFO
	{
		STRU_NODE_INFO	*mpNext;
		STRU_NODE_INFO	*mpPrev;
		_T				*mpData;
	} ;

	CDoubleQueue()
	{
		mpHead	= INVALID_U9_POSITION;
		mpTail	= INVALID_U9_POSITION;
		miCount = 0;
	}
	virtual ~CDoubleQueue()
	{
		clear();
	}

public:
	inline BOOL IsValid(U9_POSITION aoPosition) const
	{
		return (BOOL)(aoPosition == INVALID_U9_POSITION);
	}

	inline int  size() const{return miCount;}
	inline BOOL IsEmpty() const{return (0 == miCount);}

	inline _T*   front()
	{
		if(INVALID_U9_POSITION == mpHead)
			return INVALID_U9_POSITION;
		return (mpHead->mpData);
	}
	inline _T*   pop_front()
	{
		if(NULL == mpHead)
			return NULL;

		STRU_NODE_INFO *lpNode = mpHead;
		_T *lpData = lpNode->mpData;
		if (mpHead != mpTail)
		{
			mpHead = lpNode->mpNext;
			mpHead->mpPrev = INVALID_U9_POSITION;
		}
		else
		{//最后一个元素
			mpHead = INVALID_U9_POSITION;
			mpTail = INVALID_U9_POSITION;
		}
		if(miCount > 0 )
			miCount--;
		Free_Node(lpNode);
		return lpData;
	}
	inline U9_POSITION  push_front(_T*& apData)
	{
		STRU_NODE_INFO *lpNewNode = (STRU_NODE_INFO *)New_Node();
		//U9_ASSERT(lpNewNode);
		if(INVALID_U9_POSITION == lpNewNode)
			return (U9_POSITION)INVALID_U9_POSITION;

		lpNewNode->mpData = apData;
		lpNewNode->mpNext = mpHead;
		lpNewNode->mpPrev = NULL;
		if(NULL == mpHead)
		{
			mpTail = lpNewNode;
		}
		else
		{
			mpHead->mpPrev = lpNewNode;
		}
		mpHead = lpNewNode;
		miCount++;
		return (U9_POSITION)lpNewNode;
	}

	inline _T* back()
	{
		if(INVALID_U9_POSITION == mpTail)
			return INVALID_U9_POSITION;
		return (mpTail->mpData);
	}
	inline _T*   pop_back()
	{
		if(INVALID_U9_POSITION == mpTail)
			return INVALID_U9_POSITION;

		STRU_NODE_INFO* lpNode = mpTail;
		_T *lpData = lpNode->mpData;

		if (mpTail != mpHead)
		{
			mpTail = lpNode->mpPrev;
			mpTail->mpNext = INVALID_U9_POSITION;
		}
		else 
		{	
			mpTail = INVALID_U9_POSITION;
			mpHead = INVALID_U9_POSITION;
		}
		if(miCount > 0 )
			miCount--;

		Free_Node(lpNode);
		return lpData;
	}
	inline U9_POSITION  push_back(_T*& apData)
	{
		STRU_NODE_INFO* lpNewNode = (STRU_NODE_INFO *)New_Node();
		//U9_ASSERT(lpNewNode);
		if(INVALID_U9_POSITION == lpNewNode)
			return (U9_POSITION)INVALID_U9_POSITION;

		lpNewNode->mpData = apData;
		lpNewNode->mpPrev = mpTail;
		lpNewNode->mpNext = INVALID_U9_POSITION;

		if (INVALID_U9_POSITION == mpTail)
		{
			mpHead = lpNewNode;
		}
		else
		{
			mpTail->mpNext = lpNewNode;
		}
		mpTail = lpNewNode;
		miCount++;
		return (U9_POSITION)lpNewNode;
	}


	inline U9_POSITION  insert_at(U9_POSITION aoPosition, _T*& apValue)
	{
		if(aoPosition == INVALID_U9_POSITION)
		{
			return push_back(apValue);
		}
		if ((STRU_NODE_INFO *)aoPosition == mpTail)
		{
			return  push_back(apValue);
		}

		STRU_NODE_INFO *lpPreNode = (STRU_NODE_INFO *)aoPosition;
		STRU_NODE_INFO *lpNewNode = (STRU_NODE_INFO *)New_Node();
		//U9_ASSERT(lpNewNode);
		lpNewNode->mpData = apValue;

		lpNewNode->mpNext = lpPreNode->mpNext;
		lpNewNode->mpNext->mpPrev = lpNewNode;
		lpNewNode->mpPrev = lpPreNode;
		lpPreNode->mpNext = lpNewNode;

		miCount++;
		return  (U9_POSITION)lpNewNode;
	}

	inline U9_POSITION begin() const{return mpHead; };
	inline U9_POSITION next(U9_POSITION aoPosition) const
	{
		if(INVALID_U9_POSITION == aoPosition)
			return INVALID_U9_POSITION;
		STRU_NODE_INFO *lpNextNode=(STRU_NODE_INFO *)aoPosition;
		lpNextNode = lpNextNode->mpNext;
		return (U9_POSITION)lpNextNode;
	}

	static _T*	getdata(const U9_POSITION aoPosition)
	{
		if (INVALID_U9_POSITION ==aoPosition )
			return (_T *)INVALID_U9_POSITION;
		return ((STRU_NODE_INFO *)aoPosition)->mpData;
	}

	static const _T* getdataconst(const U9_POSITION aoPosition) 
	{
		if (INVALID_U9_POSITION ==aoPosition )
			return (const _T *)INVALID_U9_POSITION;
		return ((STRU_NODE_INFO *)aoPosition)->mpData;
	}

	//得到头数据并且弹出
	inline _T*	getheaddataandpop()
	{
		if (INVALID_U9_POSITION ==mpHead)
			return (_T *)INVALID_U9_POSITION;

		STRU_NODE_INFO* lpNode = mpHead;
		_T* lpData = mpHead->mpData;
		mpHead = mpHead->mpNext;
		if (mpHead == INVALID_U9_POSITION)
			mpTail = INVALID_U9_POSITION;

		Free_Node(lpNode);
		if(miCount > 0 )
			miCount--;
		return lpData;
	}

	inline U9_POSITION erase(U9_POSITION aoPosition)
	{
		if(aoPosition == INVALID_U9_POSITION )
			return INVALID_U9_POSITION;
		STRU_NODE_INFO* lpNode = (STRU_NODE_INFO *)aoPosition;
		STRU_NODE_INFO* lpPrevNode = lpNode->mpPrev;
		STRU_NODE_INFO* lpNextNode = lpNode->mpNext;
		if (INVALID_U9_POSITION == lpPrevNode)
			mpHead = lpNextNode;
		else
			lpPrevNode->mpNext = lpNextNode;

		if (INVALID_U9_POSITION == lpNextNode)
			mpTail = lpPrevNode;
		else
			lpNextNode->mpPrev = lpPrevNode;

		if(miCount > 0 )
			miCount--;
		Free_Node(lpNode);
		return lpNextNode;
	}

	virtual void clear()
	{
		STRU_NODE_INFO *lpNode;
		while (INVALID_U9_POSITION != mpHead)
		{
			lpNode = mpHead;
			mpHead = mpHead->mpNext;
			Free_Node(lpNode);
		}
		mpTail = INVALID_U9_POSITION;
		miCount = 0;
	}
protected:
	virtual void* New_Node(){return new STRU_NODE_INFO;}
	virtual void  Free_Node(STRU_NODE_INFO* apNode){delete apNode;}

protected:
	struct STRU_NODE_INFO *mpHead; // 当前使用头指针
	struct STRU_NODE_INFO *mpTail; // 当前使用尾指针

	int    miCount;
public:
	struct iterator
	{
	public:
		inline iterator()
		{
			mpNode= INVALID_U9_POSITION;
		}

		inline iterator(U9_POSITION aoPossion)
		{
			mpNode=(STRU_NODE_INFO*) aoPossion;
		}

		inline iterator(STRU_NODE_INFO* apNode)
		{
			mpNode = apNode;
		}
		iterator(const iterator & aoItor)
		{
			mpNode = aoItor.mpNode;
		}
		inline iterator & operator = (const iterator & aoItor)
		{
			mpNode = aoItor.mpNode;
		}
		inline bool operator == (const iterator& aoItor)
		{
			return mpNode == aoItor.mpNode;
		}
		inline _T *  operator *()
		{
			if(INVALID_U9_POSITION == mpNode)
				throw;
			return mpNode->mpData;
		}
		inline iterator & operator ++()
		{
			if(INVALID_U9_POSITION == mpNode)
				throw;
			mpNode=mpNode->mpNext;
			return * this;
		}
		inline iterator & operator ++(int)
		{

			iterator loTmp=*this;
			++ *this;
			return loTmp;
		};
		inline iterator & operator --()
		{
			if(INVALID_U9_POSITION == mpNode)
				throw;
			mpNode=mpNode->mpPrev;
			return *this;
		}
		inline iterator & operator --(int)
		{
			iterator loTmp=*this;
			-- *this;
			return loTmp;
		}

		inline operator U9_POSITION ()
		{
			return (U9_POSITION)mpNode;
		};   

	private:
		STRU_NODE_INFO*     mpNode;
	};
	inline iterator begin()
	{
		return iterator(mpHead);
	}
	inline iterator end()
	{
		return iterator(INVALID_U9_POSITION);
	}
};


//////////////////////////////////////////////////////////////////////////
// CSafeDoubleQueue
template <class _T> class CSafeDoubleQueue
{
public:
	struct STRU_NODE_INFO
	{
		STRU_NODE_INFO	*mpNext;
		STRU_NODE_INFO	*mpPrev;
		_T				*mpData;
	} ;

	CSafeDoubleQueue()
	{
		mpHead	= INVALID_U9_POSITION;
		mpTail	= INVALID_U9_POSITION;
		miCount = 0;
	}
	virtual ~CSafeDoubleQueue()
	{
		clear();
	}

public:
	inline BOOL IsValid(U9_POSITION aoPosition) const
	{
		return (BOOL)(aoPosition == INVALID_U9_POSITION);
	}

	inline int  size() const{return miCount;}
	inline BOOL IsEmpty() const{return (0 == miCount);}

	inline _T*   front()
	{
		moCriticalSection.Enter();
		if(INVALID_U9_POSITION == mpHead)
		{
			moCriticalSection.Leave();
			return INVALID_U9_POSITION;
		}
		_T* lpData =mpHead->mpData;
		moCriticalSection.Leave();
		return lpData;
	}
	inline _T*   pop_front()
	{
		moCriticalSection.Enter();
		if(NULL == mpHead)
		{
			moCriticalSection.Leave();
			return INVALID_U9_POSITION;
		}
		STRU_NODE_INFO *lpNode = mpHead;
		_T *lpData = lpNode->mpData;
		if (mpHead != mpTail)
		{
			mpHead = lpNode->mpNext;
			mpHead->mpPrev = INVALID_U9_POSITION;
		}
		else
		{//最后一个元素
			mpHead = INVALID_U9_POSITION;
			mpTail = INVALID_U9_POSITION;
		}
		if(miCount > 0 )
			miCount--;
		Free_Node(lpNode);
		moCriticalSection.Leave();
		return lpData;
	}
	inline U9_POSITION  push_front(_T*& apData)
	{
		moCriticalSection.Enter();
		STRU_NODE_INFO *lpNewNode = (STRU_NODE_INFO *)New_Node();
		//U9_ASSERT(lpNewNode);
		if(INVALID_U9_POSITION == lpNewNode)
		{
			moCriticalSection.Leave();
			return (U9_POSITION)INVALID_U9_POSITION;
		}

		lpNewNode->mpData = apData;
		lpNewNode->mpNext = mpHead;
		lpNewNode->mpPrev = NULL;


		if(NULL == mpHead)
		{
			mpTail = lpNewNode;
		}
		else
		{
			mpHead->mpPrev = lpNewNode;
		}
		mpHead = lpNewNode;
		miCount++;
		moCriticalSection.Leave();
		return (U9_POSITION)lpNewNode;
	}

	inline _T* back()
	{
		moCriticalSection.Enter();
		if(INVALID_U9_POSITION == mpTail)
		{
			moCriticalSection.Leave();
			return INVALID_U9_POSITION;
		}

		_T* lpData =mpTail->mpData;
		moCriticalSection.Leave();
		return lpData;
	}
	inline _T*   pop_back()
	{
		moCriticalSection.Enter();
		if(INVALID_U9_POSITION == mpTail)
		{
			moCriticalSection.Leave();
			return INVALID_U9_POSITION;
		}

		STRU_NODE_INFO* lpNode = mpTail;
		_T *lpData = lpNode->mpData;

		if (mpTail != mpHead)
		{
			mpTail = lpNode->mpPrev;
			mpTail->mpNext = INVALID_U9_POSITION;
		}
		else 
		{	
			mpTail = INVALID_U9_POSITION;
			mpHead = INVALID_U9_POSITION;
		}
		if(miCount > 0 )
			miCount--;

		Free_Node(lpNode);
		moCriticalSection.Leave();
		return lpData;
	}
	inline U9_POSITION  push_back(_T*& apData)
	{
		moCriticalSection.Enter();
		STRU_NODE_INFO* lpNewNode = (STRU_NODE_INFO *)New_Node();
		//U9_ASSERT(lpNewNode);
		if(INVALID_U9_POSITION == lpNewNode)
		{
			moCriticalSection.Leave();
			return (U9_POSITION)INVALID_U9_POSITION;
		}

		lpNewNode->mpData = apData;
		lpNewNode->mpPrev = mpTail;
		lpNewNode->mpNext = INVALID_U9_POSITION;


		if (INVALID_U9_POSITION == mpTail)
		{
			mpHead = lpNewNode;
		}
		else
		{
			mpTail->mpNext = lpNewNode;
		}
		mpTail = lpNewNode;
		miCount++;
		moCriticalSection.Leave();
		return (U9_POSITION)lpNewNode;
	}


	inline U9_POSITION  insert_at(U9_POSITION aoPosition, _T*& apValue)
	{
		
		if(aoPosition == INVALID_U9_POSITION)
		{
			U9_POSITION lpPos = push_back(apValue);
			return lpPos;
		}
		moCriticalSection.Enter();
		if ((STRU_NODE_INFO *)aoPosition == mpTail)
		{
			U9_POSITION lpPos =push_back(apValue);
			moCriticalSection.Leave();
			return lpPos;
		}

		STRU_NODE_INFO *lpPreNode = (STRU_NODE_INFO *)aoPosition;
		STRU_NODE_INFO *lpNewNode = (STRU_NODE_INFO *)New_Node();
		//U9_ASSERT(lpNewNode);
		lpNewNode->mpData = apValue;
		lpNewNode->mpNext = lpPreNode->mpNext;
		lpNewNode->mpNext->mpPrev = lpNewNode;
		lpNewNode->mpPrev = lpPreNode;
		lpPreNode->mpNext = lpNewNode;

		miCount++;
		moCriticalSection.Leave();
		return  (U9_POSITION)lpNewNode;
	}

	inline U9_POSITION begin() const{return mpHead; };
	inline U9_POSITION next(U9_POSITION aoPosition) const
	{
		if(INVALID_U9_POSITION == aoPosition)
			return INVALID_U9_POSITION;
		STRU_NODE_INFO *lpNextNode=(STRU_NODE_INFO *)aoPosition;
		lpNextNode = lpNextNode->mpNext;
		return (U9_POSITION)lpNextNode;
	}
	//得到头数据并且弹出
	inline _T*	getheaddataandpop()
	{
		moCriticalSection.Enter();
		if (INVALID_U9_POSITION == mpHead)
		{
			moCriticalSection.Leave();
			return (_T *)INVALID_U9_POSITION;
		}
		STRU_NODE_INFO* lpNode = mpHead;
		_T* lpData = mpHead->mpData;
		mpHead = mpHead->mpNext;
		if (mpHead == INVALID_U9_POSITION)
			mpTail = INVALID_U9_POSITION;

		Free_Node(lpNode);
		if(miCount > 0 )
			miCount--;
		moCriticalSection.Leave();
		return lpData;
	}

	inline _T*	getdata(const U9_POSITION aoPosition)
	{
		if (INVALID_U9_POSITION ==aoPosition )
			return (_T *)INVALID_U9_POSITION;
		return ((STRU_NODE_INFO *)aoPosition)->mpData;
	}

	inline U9_POSITION erase(U9_POSITION aoPosition)
	{
		if(INVALID_U9_POSITION == aoPosition)
			return INVALID_U9_POSITION;
		
		moCriticalSection.Enter();
		STRU_NODE_INFO* lpNode = (STRU_NODE_INFO *)aoPosition;
		STRU_NODE_INFO* lpPrevNode = lpNode->mpPrev;
		STRU_NODE_INFO* lpNextNode = lpNode->mpNext;
		if (INVALID_U9_POSITION == lpPrevNode)
			mpHead = lpNextNode;
		else
			lpPrevNode->mpNext = lpNextNode;

		if (INVALID_U9_POSITION == lpNextNode)
			mpTail = lpPrevNode;
		else
			lpNextNode->mpPrev = lpPrevNode;

		if(miCount > 0 )
			miCount--;
		Free_Node(lpNode);
		moCriticalSection.Leave();
		return lpNextNode;
	}

	virtual void clear()
	{
		typename CRITICAL_SECTION_HELPER(moCriticalSection);
		STRU_NODE_INFO *lpNode;
		while (INVALID_U9_POSITION != mpHead)
		{
			lpNode = mpHead;
			mpHead = mpHead->mpNext;
			Free_Node(lpNode);
		}
		mpTail = INVALID_U9_POSITION;
		miCount = 0;
	}
	inline CCriticalSection & GetCriticalSection(){return moCriticalSection;}
protected:
	virtual void* New_Node(){return new STRU_NODE_INFO;}
	virtual void  Free_Node(STRU_NODE_INFO* apNode){delete apNode;}

protected:
	struct STRU_NODE_INFO *mpHead; // 当前使用头指针
	struct STRU_NODE_INFO *mpTail; // 当前使用尾指针
	int    miCount;
	CCriticalSection moCriticalSection;
public:

	struct iterator
	{
	public:
		inline iterator()
		{
			mpNode= INVALID_U9_POSITION;
		}

		inline iterator(U9_POSITION aoPossion)
		{
			mpNode=(STRU_NODE_INFO*) aoPossion;
		}

		inline iterator(STRU_NODE_INFO* apNode)
		{
			mpNode = apNode;
		}
		iterator(const iterator & aoItor)
		{
			mpNode = aoItor.mpNode;
		}
		inline iterator & operator = (const iterator & aoItor)
		{
			mpNode = aoItor.mpNode;
		}
		inline bool operator == (const iterator& aoItor)
		{
			return mpNode == aoItor.mpNode;
		}
		inline _T *  operator *()
		{
			if(INVALID_U9_POSITION == mpNode)
				throw;
			return mpNode->mpData;
		}
		inline iterator & operator ++()
		{
			if(INVALID_U9_POSITION == mpNode)
				throw;
			mpNode=mpNode->mpNext;
			return * this;
		}
		inline iterator & operator ++(int)
		{

			iterator loTmp=*this;
			++ *this;
			return loTmp;
		};
		inline iterator & operator --()
		{
			if(INVALID_U9_POSITION == mpNode)
				throw;
			mpNode=mpNode->mpPrev;
			return *this;
		}
		inline iterator & operator --(int)
		{
			iterator loTmp=*this;
			-- *this;
			return loTmp;
		}

		inline operator U9_POSITION ()
		{
			return (U9_POSITION)mpNode;
		};   

	private:
		STRU_NODE_INFO*     mpNode;
	};
	inline iterator begin()
	{
		return iterator(mpHead);
	}
	inline iterator end()
	{
		return iterator(INVALID_U9_POSITION);
	}
};


// --------------------------------------------------------
// CCacheDoubleQueue
template <class _T> 
class CCacheDoubleQueue : public CDoubleQueue<_T>
{ 

public:
	CCacheDoubleQueue()
	{
		miMaxCacheCount=0;
		miCurrentCacheCount = 0;
		mpFreeNodeHead=INVALID_U9_POSITION;
	}
	CCacheDoubleQueue(int aiMaxCache)
	{
		init(aiMaxCache);
	}
	void init(int aiMaxCache)
	{
		miMaxCacheCount=aiMaxCache;
		miCurrentCacheCount = 0;
		mpFreeNodeHead=INVALID_U9_POSITION;
	}
	virtual ~CCacheDoubleQueue() 
	{
	}
	virtual void clear()
	{
		typename CDoubleQueue<_T>::STRU_NODE_INFO* lpNode=INVALID_U9_POSITION;
		while (INVALID_U9_POSITION != mpFreeNodeHead)
		{
			lpNode = mpFreeNodeHead;
			mpFreeNodeHead = mpFreeNodeHead->mpNext;
			delete lpNode;
		}
		CDoubleQueue <_T>::clear();
	}
protected:
	virtual void* New_Node()
	{
		typename CDoubleQueue <_T>::STRU_NODE_INFO *lpNode = mpFreeNodeHead;
		if (INVALID_U9_POSITION == mpFreeNodeHead)
		{//没有自由的节点
			lpNode = new typename CDoubleQueue <_T>::STRU_NODE_INFO;
			if(INVALID_U9_POSITION == lpNode)
				return INVALID_U9_POSITION;
		}
		else
		{//使用cahche的节点
			mpFreeNodeHead = mpFreeNodeHead->mpNext;
			if(miCurrentCacheCount>0)
				miCurrentCacheCount--;
		}
		return lpNode;
	}
	virtual void  Free_Node(typename CDoubleQueue<_T>::STRU_NODE_INFO* apNode)
	{
		//U9_ASSERT(apNode != INVALID_U9_POSITION);
		if(miCurrentCacheCount>= miMaxCacheCount)
		{//不能cahche了
			delete apNode;
		}
		else
		{
			apNode->mpNext = mpFreeNodeHead;
			mpFreeNodeHead = apNode;
			miCurrentCacheCount++;
		}
	}

private:
	int    miMaxCacheCount;
	int    miCurrentCacheCount;
	struct CDoubleQueue <_T>::STRU_NODE_INFO *mpFreeNodeHead;// 当前自由的节点头。
};


// --------------------------------------------------------
// CCacheDoubleQueue
template <class _T> 
class CSafeCacheDoubleQueue : public CSafeDoubleQueue<_T>
{ 

public:
	CSafeCacheDoubleQueue()
	{
		miMaxCacheCount=0;
		miCurrentCacheCount = 0;
		mpFreeNodeHead=INVALID_U9_POSITION;
	}
	CSafeCacheDoubleQueue(int aiMaxCache)
	{
		init(aiMaxCache);
	}
	void init(int aiMaxCache)
	{
		typename CRITICAL_SECTION_HELPER(CSafeDoubleQueue<_T>::moCriticalSection);
		miMaxCacheCount=aiMaxCache;
		miCurrentCacheCount = 0;
		mpFreeNodeHead=INVALID_U9_POSITION;
	}
	virtual ~CSafeCacheDoubleQueue() 
	{
		clear();
	}
	virtual void clear()
	{
		typename CSafeDoubleQueue<_T>::STRU_NODE_INFO* lpNode=INVALID_U9_POSITION;
		typename CRITICAL_SECTION_HELPER(CSafeDoubleQueue<_T>::moCriticalSection);
		while (INVALID_U9_POSITION != mpFreeNodeHead)
		{
			lpNode = mpFreeNodeHead;
			mpFreeNodeHead = mpFreeNodeHead->mpNext;
			delete lpNode;
		}
		CSafeDoubleQueue<_T>::clear();
	}
protected:
	virtual void* New_Node()
	{
		CSafeDoubleQueue<_T>::moCriticalSection.Enter();
		typename CSafeDoubleQueue <_T>::STRU_NODE_INFO *lpNode = mpFreeNodeHead;
		if (INVALID_U9_POSITION == mpFreeNodeHead)
		{//没有自由的节点
			CSafeDoubleQueue<_T>::moCriticalSection.Leave();
			lpNode = new typename CSafeDoubleQueue <_T>::STRU_NODE_INFO;
			if(INVALID_U9_POSITION == lpNode)
				return INVALID_U9_POSITION;
			return lpNode;
		}
		
		//使用cahche的节点
		mpFreeNodeHead = mpFreeNodeHead->mpNext;
		if(miCurrentCacheCount>0)
			miCurrentCacheCount--;
		CSafeDoubleQueue<_T>::moCriticalSection.Leave();
		return lpNode;
	}
	virtual void  Free_Node(typename CSafeDoubleQueue<_T>::STRU_NODE_INFO* apNode)
	{
		//U9_ASSERT(apNode != INVALID_U9_POSITION);
		CSafeDoubleQueue<_T>::moCriticalSection.Enter();
		if(miCurrentCacheCount>= miMaxCacheCount)
		{//不能cahche了
			CSafeDoubleQueue<_T>::moCriticalSection.Leave();
			delete apNode;
			return;
		}

		apNode->mpNext = mpFreeNodeHead;
		mpFreeNodeHead = apNode;
		miCurrentCacheCount++;
		CSafeDoubleQueue<_T>::moCriticalSection.Leave();

	}

private:
	int    miMaxCacheCount;
	int    miCurrentCacheCount;
	struct CSafeDoubleQueue<_T>::STRU_NODE_INFO *mpFreeNodeHead;// 当前自由的节点头。
};

U9_END_NAMESPACE
#endif /* __QUEUE2_H */