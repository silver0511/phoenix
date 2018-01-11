// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: Sptr.h
// 创 建 人: 史云杰
// 文件说明: 智能指针模版，使用对象,所有用new构建的对象。自己释放指针资源 SmartPtr
// $_FILEHEADER_END ******************************

#ifndef __SPTR_H
#define __SPTR_H

#include "common/CriticalSection.h"
#include "common/CacheList.h"

#include <assert.h>

/***************************************************************************
使用例子：
class A {
	int Afunction();
};
class B : public A {
	int Bfunction();
};
class C {
	int Cfunction();
};
typedef list < Sptr < A > > tylistAList;
tylistAList listA;
int main()
{

	Sptr< A > a = new A;
	Sptr< B > b = new B
	Sptr< C > c = new C;
	a->Afunction();
	b->Bfunction();
	c->Cfunction();
	b.c的生存期是main函数有效期
	
	listA.push_front(a);
	a.生存期是全局，如果要释放a资源可以这样,
	while(!listA.empty())
	{//注意不能使用delete删除a对象
		listA.pop_front();
	}
}	
//**************************************************************************/
template < class T >
class Sptr
{
private:

	mutable  T* ptr;
	mutable int* count;
	mutable U9::CCriticalSection * mutex;


	void increment()
	{
		if (ptr)
		{
			if (!count)
			{
				count = new int(0);
			}
			if (!mutex)
			{
				mutex = new U9::CCriticalSection;
			}

			mutex->Enter();
			(*count)++;
			mutex->Leave();
		}


	}

	void decrement()
	{
		if (ptr && count)
		{

			bool countIsZero;
			mutex->Enter();
			(*count)--;
			countIsZero = (*count == 0);
			mutex->Leave();
			if (countIsZero)
			{

				delete ptr; ptr = 0;
				delete count; count = 0;
				delete mutex; mutex = 0;
			}
		}
		ptr = 0;
		count = 0;
		mutex = 0;
	}

public:
#if 1

	template < class T2 >
	operator Sptr<T2 > () 
	{
		return Sptr < T2 > (ptr, count, mutex);
	}


	template < class T2 >
	operator const Sptr<T2 > () const
	{
		return Sptr < T2 > (ptr, count, mutex);
	}

#endif

	T* MallocMem()
	{
		return new T;
	}
	Sptr() : ptr(0), count(0), mutex(0)
	{}
	;

	
	Sptr(T* original, int* myCount = 0, U9::CCriticalSection* myMutex = 0)
		: ptr(original), count(myCount), mutex(myMutex)
	{
		if (ptr)
		{

			increment();
		}
	};


	Sptr(const Sptr& x)
		: ptr(x.ptr), count(x.count), mutex(x.mutex)
	{
		increment();
	};
	~Sptr()
	{
		{
			decrement();
		}
	}


	T& operator*() const
	{
		assert(ptr);
		return *ptr;
	}


	int operator!() const
	{
		if (ptr)
		{
			return (ptr == 0);
		}
		else
			return true;
	}


	T* operator->() const
	{
		return ptr;
	}

	/** 使用例子
	
	class A {
	...
	};
	class B : public A {
	...
	};
	class C {
	...
	};
	...
	int main()
	{
	Sptr< A > a;
	Sptr< B > b;
	Sptr< C > c;

	a = new B;

	b.dynamicCast(a);
	// now, b points to the same thing as a

	c.dynamicCast(a);
	// now, c is the NULL pointer.
	}
	*/
	template < class T2 > Sptr& dynamicCast(const Sptr < T2 > & x)
	{
		if (ptr == x.getPtr()) return *this;
		decrement();
		if(T* p = dynamic_cast < T* > (x.getPtr()))
		{
			count = x.getCount();
			mutex = x.getMutex();
			ptr = p;
			increment();
		}
		return *this;
	}

	template < class T2 >
	Sptr& operator=(const Sptr < T2 > & x)
	{
		if (ptr == x.getPtr()) return * this;
		decrement();
		ptr = x.getPtr();
		count = x.getCount();
		mutex = x.getMutex();
		increment();
		return *this;
	}



	Sptr& operator=(T* original)
	{
		if (ptr == original) return * this;
		decrement();

		ptr = original;
		increment();
		return *this;
	};


	Sptr& operator=(const Sptr& x)
	{
		if (ptr == x.getPtr()) return * this;
		decrement();
		ptr = x.ptr;
		count = x.count;
		mutex = x.mutex;
		increment();
		return *this;
	}


	friend bool operator!=(const void* y, const Sptr& x)
	{
		if (x.ptr != y)
			return true;
		else
			return false;
	}

	///比较智能指针管理对象是否和制定y不等
	friend bool operator!=(const Sptr& x, const void* y)
	{
		if (x.ptr != y)
			return true;
		else
			return false;
	}
	//比较对象指针是否相等
	friend bool operator==(const void* y, const Sptr& x)
	{
		if (x.ptr == y)
			return true;
		else
			return false;
	}

	friend bool operator==(const Sptr& x, const void* y)
	{
		if (x.ptr == y)
			return true;
		else
			return false;
	}

	
	bool operator==(const Sptr& x) const
	{
		if (x.ptr == ptr)
			return true;
		else
			return false;
	}

	//指针判断
	bool operator!=(const Sptr& x) const
	{
		if (x.ptr != ptr)
			return true;
		else
			return false;
	}


	U9::CCriticalSection* getMutex() const
	{
		return mutex;
	}

	//获得引用计数
	int* getCount() const
	{
		return count;
	}

	//获取对象的指针
	T* getPtr() const
	{
		return ptr;
	}

};




#define DEF_DEFAULT_MAX	 1024


class CSafeStaticQueueEx 
{
public:
	CSafeStaticQueueEx()
	{
		InitMember();
		mpDataList=NULL;
	};
	CSafeStaticQueueEx(INT aiMaxCount)
	{
		InitMember();
		mpDataList=NULL;
		miMaxCount=aiMaxCount;
		mpDataList= new void*[miMaxCount];
		if(NULL == mpDataList)
			throw;
		//memset(mpDataList,0,miMaxCount*sizeof(T*));
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
		mpDataList= new void*[miMaxCount];
		if(NULL == mpDataList)
		{
			moCriticalSection.Leave();
			throw;
		}
		//memset(mpDataList,0,miMaxCount*sizeof(T*));
		moCriticalSection.Leave();
	}
	~CSafeStaticQueueEx()
	{
		moCriticalSection.Enter();
		if(NULL ==mpDataList)
		{
			moCriticalSection.Leave();
			return ;
		}
		INT liIndex=miHeadIndex;
		void* lpValue=NULL;
		for(INT i=0;i<miCount;i++)
		{
			lpValue	=mpDataList[liIndex];
			if(NULL != lpValue)
			{
				delete lpValue;
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
		void* lpValue=NULL;
		for(INT i=0;i<miCount;i++)
		{
			lpValue	=mpDataList[liIndex];
			if(NULL != lpValue)
			{
				delete lpValue;
				mpDataList[liIndex] = NULL;
				liIndex = INC_CIRCLE_INDEX(liIndex,1,miMaxCount);
			}			
		}
		//memset(mpDataList,0,miMaxCount*sizeof(T*));			
		//InitMember();
		moCriticalSection.Leave();
	};

	inline void * GetandDelHead()
	{
		void *lpValue =NULL;
		moCriticalSection.Enter();
		if(miCount>0)
		{
			lpValue = mpDataList[miHeadIndex];
			mpDataList[miHeadIndex]=NULL;
			miCount--;
			//miHeadIndex = (miHeadIndex+1)%miMaxCount;
			miHeadIndex = INC_CIRCLE_INDEX(miHeadIndex,1,miMaxCount);
		}
		moCriticalSection.Leave();

		return lpValue;
	};

	inline void * Get()
	{
		void *lpValue =NULL;
		moCriticalSection.Enter();
		if(miCount>0)
		{
			lpValue = mpDataList[miHeadIndex];
		}
		moCriticalSection.Leave();

		return lpValue;
	};
	inline INT AddTail(void *apValue)
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
public:
	INT		miCount;
protected:
	U9::CCriticalSection moCriticalSection;
	INT		miMaxCount;
	
	void	**mpDataList;

	INT	miHeadIndex;//头
	BOOL	mbIsFrist;
};

template<typename T>
class CSafeStaticQueueSptrUse:private CSafeStaticQueue<T>
{
public:
	CSafeStaticQueueSptrUse()
	{
		CSafeStaticQueue<T>::InitMember();
		CSafeStaticQueue<T>::mpDataList=NULL;
	}
	void clear()
	{
		CSafeStaticQueue<T>::clear();
	}
	void init(INT aiMaxCount)
	{
		CSafeStaticQueue<T>::init(aiMaxCount);
		CSafeStaticQueue<T>::moCriticalSection.Enter();
		for (int lindx = 0;lindx < aiMaxCount;lindx++)
		{
			CSafeStaticQueue<T>::mpDataList[lindx] = new T;
		}
		CSafeStaticQueue<T>::moCriticalSection.Leave();
	}

	int size()
	{
		return CSafeStaticQueue<T>::size();
	}
	inline T  GetAndDelHead()
	{
		T lpValue =NULL;
		CSafeStaticQueue<T>::moCriticalSection.Enter();
		if(CSafeStaticQueue<T>::miCount>0)
		{
			lpValue = *CSafeStaticQueue<T>::mpDataList[CSafeStaticQueue<T>::miHeadIndex];
			*CSafeStaticQueue<T>::mpDataList[CSafeStaticQueue<T>::miHeadIndex]=NULL;
			CSafeStaticQueue<T>::miCount--;
			CSafeStaticQueue<T>::miHeadIndex = INC_CIRCLE_INDEX(CSafeStaticQueue<T>::miHeadIndex,1,CSafeStaticQueue<T>::miMaxCount);
		}
		CSafeStaticQueue<T>::moCriticalSection.Leave();

		return lpValue;
	};

	inline INT AddTail(T apValue)
	{
		if(NULL ==apValue)
			return -1;
		CSafeStaticQueue<T>::moCriticalSection.Enter();
		if (CSafeStaticQueue<T>::miCount < CSafeStaticQueue<T>::miMaxCount)
		{
			*CSafeStaticQueue<T>::mpDataList[INC_CIRCLE_INDEX(CSafeStaticQueue<T>::miHeadIndex,CSafeStaticQueue<T>::miCount,CSafeStaticQueue<T>::miMaxCount)]=apValue;
			CSafeStaticQueue<T>::miCount++;
			CSafeStaticQueue<T>::moCriticalSection.Leave();
			return 1;
		}
		CSafeStaticQueue<T>::moCriticalSection.Leave();	
		return 0;
	}
};

template<typename T>
class CCachePoolExx:private CSafeStaticQueue<T>
{
private:

	
protected:
	mutable U9::CCriticalSection moCircal;
public:

	uint32 miAllocCount;
	inline void init(int aiMaxCache=DEF_DEFAULT_MAX)
	{
		miAllocCount = 0;
		CSafeStaticQueue<T>::init(aiMaxCache);
	}

 	CCachePoolExx(int aiMaxLen): CSafeStaticQueue<T>(aiMaxLen)
 	{

 		miAllocCount = 0;

 	};

 	CCachePoolExx(): CSafeStaticQueue<T>(DEF_DEFAULT_MAX)
 	{
		miAllocCount = 0;
#ifdef TEST_SPTR_CODE
		printf("Data Constructor Alloc Store %d %s \n",DEF_DEFAULT_MAX,__FUNCTION__);
#endif
// 		int i = DEF_DEFAULT_MAX;
// 		for( ;i > 0;i--)
// 		{
// 			T* p = new T;
// 			CSafeStaticQueue<T>::AddTail( p);
// 		}
 	};

// 	~CCachePoolExx()
// 	{
// 		CSafeStaticQueue<T>::~CSafeStaticQueue();
// 	}

	inline T* malloc()
	{
		T* lpItem=	CSafeStaticQueue<T>::GetandDelHead();
		if(NULL==lpItem)
		{

			CRITICAL_SECTION_HELPER(moCircal);
			miAllocCount++;
#ifdef TEST_SPTR_CODE
			printf("Remalloc Space %s ,Object Count = %d \n",__FUNCTION__,miAllocCount);
#endif
			lpItem = new T;

		}
		return lpItem;
	}

	inline bool free(T* P)
	{
		if(1 != CSafeStaticQueue<T>::AddTail(P))
		{
#ifdef TEST_SPTR_CODE
			printf("Delete Space %s ,Object Count = %d \n",__FUNCTION__,miAllocCount);
#endif
			delete P;
			return false;
		}
		else
		{
#ifdef TEST_SPTR_CODE
			{
				CRITICAL_SECTION_HELPER(moCircal);
				if( miAllocCount > 0)
					miAllocCount--;
			}
#endif
			return true;
		}
	}
	inline INT size()
	{
		return CSafeStaticQueue<T>::size();
	}
	void clear()
	{
		CSafeStaticQueue<T>::clear();
	}
};
//使用自身管理回收内存垃圾


struct struDataAlloc
{	
	CCachePoolExx < uint32 > mpPoolcount;
	CCachePoolExx < U9::CCriticalSection > mpPoolCritical;
	~struDataAlloc()
	{
		Clear();
	}
	void Clear()
	{		
		mpPoolcount.clear();
		mpPoolCritical.clear();
	}
};

//#define  SptrX Sptr 
template < class T >
class SptrX
{
public:
	static CCachePoolExx< T >  moPool;
	static struDataAlloc moDataAccess;

	U9::CCriticalSection moCircal;
	
	T* MallocMem()
	{
		return moPool.malloc();
	}

	
private:

	mutable  T* ptr;
	mutable uint32* count;
	mutable U9::CCriticalSection * mutex;

	void increment()
	{
		if (ptr)
		{
			
			if (!mutex)
			{
				mutex = moDataAccess.mpPoolCritical.malloc();
			}

			if (!count)
			{
				count = moDataAccess.mpPoolcount.malloc();
				mutex->Enter();
				*count = 0;
				mutex->Leave();
			}
			mutex->Enter();
			(*count)++;
			mutex->Leave();
		}
	}
	void decrement()
	{
		if (ptr && count)
		{

			bool countIsZero;
			mutex->Enter();
			(*count)--;
			countIsZero = (*count == 0);
			mutex->Leave();
			if (countIsZero)
			{
				if(moPool.free(ptr))
				{
#ifdef TEST_SPTR_CODE
					printf("Reclaim Data %d %s\n",moPool.miAllocCount,__FUNCTION__);
#endif
				}
				
				moDataAccess.mpPoolcount.free(count);
				moDataAccess.mpPoolCritical.free(mutex);
				// 				delete ptr; ptr = 0;
				// 				delete count; count = 0;
				// 				delete mutex; mutex = 0;
			}
		}
		ptr = 0;
		count = 0;
		mutex = 0;
	}

public:
#if 1

	template < class T2 >
	operator SptrX<T2 > () 
	{
		return SptrX < T2 > (ptr, count, mutex);
	}


	template < class T2 >
	operator const SptrX<T2 > () const
	{
		return SptrX < T2 > (ptr, count, mutex);
	}

#endif

	SptrX() : ptr(0), count(0), mutex(0)
	{
// 		ptr = MallocMem();
// 		if (ptr)
// 		{
// 			increment();
// 		}
	}


	SptrX(T* original, uint32* myCount = 0, U9::CCriticalSection* myMutex = 0)
		: ptr(original), count(myCount), mutex(myMutex)
	{
		if (ptr)
		{
			increment();
		}
	};


	SptrX(const SptrX& x)
		: ptr(x.ptr), count(x.count), mutex(x.mutex)
	{
		increment();
	};
	~SptrX()
	{
		{
			decrement();
		}
	}


	T& operator*() const
	{
		assert(ptr);
		return *ptr;
	}


	int operator!() const
	{
		if (ptr)
		{
			return (ptr == 0);
		}
		else
			return true;
	}


	T* operator->() const
	{
		return ptr;
	}

	/** 使用例子

	class A {
	...
	};
	class B : public A {
	...
	};
	class C {
	...
	};
	...
	int main()
	{
	SptrX< A > a;
	SptrX< B > b;
	SptrX< C > c;

	a = new B;

	b.dynamicCast(a);
	// now, b points to the same thing as a

	c.dynamicCast(a);
	// now, c is the NULL pointer.
	}
	*/
	template < class T2 > SptrX& dynamicCast(const SptrX < T2 > & x)
	{
		if (ptr == x.getPtr()) return *this;
		decrement();
		if(T* p = dynamic_cast < T* > (x.getPtr()))
		{
			count = x.getCount();
			mutex = x.getMutex();
			ptr = p;
			increment();
		}
		return *this;
	}

	template < class T2 >
	SptrX& operator=(const SptrX < T2 > & x)
	{
		if (ptr == x.getPtr()) return * this;
		decrement();
		ptr = x.getPtr();
		count = x.getCount();
		mutex = x.getMutex();
		increment();
		return *this;
	}



	SptrX& operator=(T* original)
	{
		if (ptr == original) return * this;
		decrement();

		ptr = original;
		increment();
		return *this;
	};


	SptrX& operator=(const SptrX& x)
	{
		if (ptr == x.getPtr()) return * this;
		decrement();
		ptr = x.ptr;
		count = x.count;
		mutex = x.mutex;
		increment();
		return *this;
	}


	friend bool operator!=(const void* y, const SptrX& x)
	{
		if (x.ptr != y)
			return true;
		else
			return false;
	}

	///比较智能指针管理对象是否和制定y不等
	friend bool operator!=(const SptrX& x, const void* y)
	{
		if (x.ptr != y)
			return true;
		else
			return false;
	}
	//比较对象指针是否相等
	friend bool operator==(const void* y, const SptrX& x)
	{
		if (x.ptr == y)
			return true;
		else
			return false;
	}

	friend bool operator==(const SptrX& x, const void* y)
	{
		if (x.ptr == y)
			return true;
		else
			return false;
	}


	bool operator==(const SptrX& x) const
	{
		if (x.ptr == ptr)
			return true;
		else
			return false;
	}

	//指针判断
	bool operator!=(const SptrX& x) const
	{
		if (x.ptr != ptr)
			return true;
		else
			return false;
	}


	U9::CCriticalSection* getMutex() const
	{
		return mutex;
	}

	//获得引用计数
	uint32* getCount() const
	{
		return count;
	}

	//获取对象的指针
	T* getPtr() const
	{
		return ptr;
	}

};

  
template < class T >
CCachePoolExx< T > SptrX < T >::moPool;

template < class T >
struDataAlloc SptrX < T >::moDataAccess;

#endif //__SPTR_H

