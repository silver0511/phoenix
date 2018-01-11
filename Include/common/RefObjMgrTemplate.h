// $_FILEHEADER_END ******************************

// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//			 
// 文件名称: RefObjMgrTemplate.h
// 创 建 人: 史云杰
// 文件说明: 引用计数管理器模板类 T_REFOBJ 类必须从CRefObjectforCache继承
// $_FILEHEADER_END *****************************

#ifndef __REFOBJMGRTEMPLATE_H_
#define __REFOBJMGRTEMPLATE_H_


#include "base0/platform.h"
#include "CacheList.h"
#include "RefObj.h"
#include "const/ErrorCodeDef.h"

#include <map>
#include "common/U9Hashmap.h"
#ifdef WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#endif

U9_BEGIN_NAMESPACE
//////////////////////////////////////////////////////////////////////////
//引用计数类支持cache使用
class CRefObjectforCache
{
public:
	CRefObjectforCache()
	{
		InitRefObjectforCache();
	}

	virtual ~CRefObjectforCache(){};

	inline void InitRefObjectforCache()
	{
		CRITICAL_SECTION_HELPER(moRefCriticalSection);
		mlRefCount=0;
		mbCanDeleteSelf=FALSE;
	}
	virtual void init()=0;
	//引用计数加1
	inline LONG IncreaseRef()
	{
		LONG llValue=0;
		CRITICAL_SECTION_HELPER(moRefCriticalSection);
		mlRefCount++;
		llValue=mlRefCount;
		return llValue;
	}
	//引用计数减1
	inline LONG DecreaseRef()
	{
		LONG llValue=0;
		{
			CRITICAL_SECTION_HELPER(moRefCriticalSection);
			if(mlRefCount>0)
				mlRefCount--;
			else
			{
				LOG_TRACE(LOG_ERR, false, __FUNCTION__, "Error Refer count was zero"
					<<" This="<<(void*)this);
				U9_ASSERT(false);
			}
			llValue=mlRefCount;
			if(0==llValue )//需要删除
				mbCanDeleteSelf = TRUE;
		}
		return llValue;
	}
	//是否能删除
	inline BOOL CanRelease()
	{
		if(mbCanDeleteSelf && 0 != mlRefCount)
		{//是需要删除，但是引用计数不为零。不能删除
			//LOG_TRACE(3, false, __FUNCTION__, "Error Refer count is bad" 
			//	<<" RefCount="<<mlRefCount
			//	<<" This="<<(void*)this
			//	);
			//LMASSERT(FALSE);
			return FALSE;
		}
		return mbCanDeleteSelf;
	}
	inline LONG GetRefCount(){return mlRefCount;}

	inline CCriticalSection &GetCriticalSection(){return moRefCriticalSection;}
protected:
	CCriticalSection moRefCriticalSection;
	BOOL mbCanDeleteSelf;//能够删除自己
	LONG mlRefCount;
};


template<typename T_ID,class T_REFOBJ>
class CAutoReleaseRefObjMgrTemplate;

//////////////////////////////////////////////////////////////////////////
//引用计数管理类  T_REFOBJ 类必须从CRefObjectforCache继承
template<class T_ID, class T_REFOBJ>
class CAutoReleaseObj;

template<typename T_ID,class T_REFOBJ>
class CRefObjMgrTemplate
{
public:
	typedef class map<T_ID, T_REFOBJ*> refobjid_map;
	typedef typename map<T_ID, T_REFOBJ*>::iterator refobjid_iter;

	CRefObjMgrTemplate(){}
	virtual ~CRefObjMgrTemplate(){clear();}
	virtual void clear()
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moMap.clear();
		moCacheList.clear();
	}
	inline uint32 size(){return static_cast<uint32>(moMap.size());}

	inline bool Initialize(uint32 size)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moCacheList.init(size);
		return true;
	};
public:
	friend class CAutoReleaseRefObjMgrTemplate<T_ID,T_REFOBJ>;
	friend class CAutoReleaseObj<T_ID, T_REFOBJ>;
public:
	//创建玩家信息对象
	T_REFOBJ* MallocRefObj()
	{
		T_REFOBJ* lpoRefObj = NULL;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		lpoRefObj = moCacheList.malloc();
		if(NULL ==lpoRefObj)
		{
			LOG_TRACE(LOG_ALERT, 0, __FUNCTION__, "!!moCacheList.malloc Error\n");
			return NULL;
		}
		lpoRefObj->init();
		return lpoRefObj;
	}
	//创建玩家信息对象
	VOID FreeRefObj(T_REFOBJ * apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moCacheList.free(apoRefObj);
	}
	//删除玩家对象
	inline int32 DeleteRefObj(const T_ID aRefObjID)
	{
		T_REFOBJ* lpoRefObj=NULL;
		{
			CRITICAL_SECTION_HELPER(moCriticalSection);
			refobjid_iter ite = moMap.find(aRefObjID);
			if(ite==moMap.end())
				return -1;
			lpoRefObj = ite->second;
			moMap.erase(ite);
			U9_ASSERT(NULL != lpoRefObj);
			lpoRefObj->DecreaseRef();
			
			if(lpoRefObj->CanRelease())
			{
				lpoRefObj->clear();
				moCacheList.free(lpoRefObj);
				return 1;
			}
			return 0;
		}
	}
	//添加玩家信息
	inline T_REFOBJ* AddRefObj(const T_ID aRefObjID,T_REFOBJ*apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
		{
			//LOG_TRACE(5, 0, __FUNCTION__, "!!Exist PlayerID="<<aRefObjID
			//	<<"\tRefCount="<<apoRefObj->GetRefCount()
			//	);
			return (T_REFOBJ*)ite->second;
		}
		apoRefObj->IncreaseRef();
		moMap.insert(pair<T_ID,  T_REFOBJ*>(aRefObjID,apoRefObj));
		return apoRefObj;
	}
	inline BOOL ExistRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
			return TRUE;
		return FALSE;
	}
public:
	inline CCriticalSection& GetCriticalSection(){return moCriticalSection;}
	inline refobjid_map *GetRefObjMap(){return &moMap;}
protected:
	inline T_REFOBJ* GetRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite!=moMap.end())
		{
			ite->second->IncreaseRef();
			return ite->second;
		}
		return NULL;
	}
	inline int32 PutRefObj(T_REFOBJ* apoRefObj)
	{
		if(NULL ==apoRefObj)
			return -1;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		apoRefObj->DecreaseRef();
		if(!apoRefObj->CanRelease())
			return 0;
		apoRefObj->clear();
		moCacheList.free(apoRefObj);
		return 1;
	}
	
protected:
	CCriticalSection		moCriticalSection;

	refobjid_map	moMap;
	CCachePool<T_REFOBJ>	moCacheList;
};
//////////////////////////////////////////////////////////////////////////
//引用对象访问类
template<typename T_ID,class T_REFOBJ>
class CAutoReleaseRefObjMgrTemplate
{
public:
	CAutoReleaseRefObjMgrTemplate(CRefObjMgrTemplate<T_ID,T_REFOBJ>& aoRefObjMgr,
		const T_ID& aRefObjID)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = mpRefObjMgr->GetRefObj(aRefObjID);
	}
	CAutoReleaseRefObjMgrTemplate(CRefObjMgrTemplate<T_ID,T_REFOBJ>& aoRefObjMgr,
		T_REFOBJ* apRefObj)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = apRefObj;
	}

	~CAutoReleaseRefObjMgrTemplate()
	{
		if(NULL != mpRefObj)
			mpRefObjMgr->PutRefObj(mpRefObj);
	}
	inline void SetRefObj(T_REFOBJ* apRefObj)
	{
		mpRefObj = apRefObj;
	}
	inline bool Invalid()
	{
		return (mpRefObj==NULL);
	}

	inline T_REFOBJ* operator ->()
	{
		return mpRefObj;
	}

	inline T_REFOBJ&  operator*()
	{
		return *mpRefObj;
	}

	inline T_REFOBJ* GetPtr()
	{
		return mpRefObj;
	}
private:
	CRefObjMgrTemplate<T_ID,T_REFOBJ>*	mpRefObjMgr;
	T_REFOBJ*	mpRefObj;
};


template<typename T_ID,class T_REFOBJ>
class CAutoReleaseStaticRefObjMgrTemplate;

//////////////////////////////////////////////////////////////////////////
//引用计数管理类  T_REFOBJ 类必须从CRefObjectforCache继承
template<typename T_ID,class T_REFOBJ>
class CStaticRefObjMgrTemplate
{
public:
	typedef class map<T_ID, T_REFOBJ*> refobjid_map;
	typedef typename map<T_ID, T_REFOBJ*>::iterator refobjid_iter;

	CStaticRefObjMgrTemplate(){}
	virtual ~CStaticRefObjMgrTemplate(){clear();}
	virtual void clear()
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moMap.clear();
		moCacheList.clear();
	}
	inline uint32 size(){return static_cast<uint32>(moMap.size());}

	inline bool Initialize(uint32 size)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moCacheList.init(size);
		return true;
	};
public:
	friend class CAutoReleaseStaticRefObjMgrTemplate<T_ID,T_REFOBJ>;
public:
	//创建玩家信息对象
	T_REFOBJ* MallocRefObj()
	{
		T_REFOBJ* lpoRefObj = NULL;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		lpoRefObj = moCacheList.malloc();
		if(NULL ==lpoRefObj)
		{
			LOG_TRACE(LOG_ALERT, 0, __FUNCTION__, "!!moCacheList.malloc Error\n");
			return NULL;
		}
		lpoRefObj->init();
		return lpoRefObj;
	}
	//创建玩家信息对象
	VOID FreeRefObj(T_REFOBJ * apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moCacheList.free(apoRefObj);
	}
	//删除玩家对象
	inline int32 DeleteRefObj(const T_ID aRefObjID)
	{
		T_REFOBJ* lpoRefObj=NULL;
		{
			CRITICAL_SECTION_HELPER(moCriticalSection);
			refobjid_iter ite = moMap.find(aRefObjID);
			if(ite==moMap.end())
				return -1;
			lpoRefObj = ite->second;
			moMap.erase(ite);
			U9_ASSERT(NULL != lpoRefObj);
			lpoRefObj->DecreaseRef();

			if(lpoRefObj->CanRelease())
			{
				lpoRefObj->clear();
				moCacheList.free(lpoRefObj);
				return 1;
			}
			return 0;
		}
	}
	//添加玩家信息
	inline T_REFOBJ* AddRefObj(const T_ID aRefObjID,T_REFOBJ*apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
		{
			//LOG_TRACE(5, 0, __FUNCTION__, "!!Exist PlayerID="<<aRefObjID
			//	<<"\tRefCount="<<apoRefObj->GetRefCount()
			//	);
			return (T_REFOBJ*)ite->second;
		}
		apoRefObj->IncreaseRef();
		moMap.insert(pair<T_ID,  T_REFOBJ*>(aRefObjID,apoRefObj));
		return apoRefObj;
	}
	inline BOOL ExistRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
			return TRUE;
		return FALSE;
	}
public:
	inline CCriticalSection& GetCriticalSection(){return moCriticalSection;}
	inline refobjid_map *GetRefObjMap(){return &moMap;}
protected:
	inline T_REFOBJ* GetRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite!=moMap.end())
		{
			ite->second->IncreaseRef();
			return ite->second;
		}
		return NULL;
	}
	inline int32 PutRefObj(T_REFOBJ* apoRefObj)
	{
		if(NULL ==apoRefObj)
			return -1;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		apoRefObj->DecreaseRef();
		if(!apoRefObj->CanRelease())
			return 0;
		apoRefObj->clear();
		moCacheList.free(apoRefObj);
		return 1;
	}

protected:
	CCriticalSection		moCriticalSection;

	refobjid_map	moMap;
	CStaticCachePool<T_REFOBJ>	moCacheList;
};
//////////////////////////////////////////////////////////////////////////
//引用对象访问类
template<typename T_ID,class T_REFOBJ>
class CAutoReleaseStaticRefObjMgrTemplate
{
public:
	CAutoReleaseStaticRefObjMgrTemplate(CStaticRefObjMgrTemplate<T_ID,T_REFOBJ>& aoRefObjMgr,
		const T_ID& aRefObjID)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = mpRefObjMgr->GetRefObj(aRefObjID);
	}
	CAutoReleaseStaticRefObjMgrTemplate(CStaticRefObjMgrTemplate<T_ID,T_REFOBJ>& aoRefObjMgr,
		T_REFOBJ* apRefObj)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = apRefObj;
	}

	~CAutoReleaseStaticRefObjMgrTemplate()
	{
		if(NULL != mpRefObj)
			mpRefObjMgr->PutRefObj(mpRefObj);
	}
	inline void SetRefObj(T_REFOBJ* apRefObj)
	{
		mpRefObj = apRefObj;
	}
	inline bool Invalid()
	{
		return (mpRefObj==NULL);
	}

	inline T_REFOBJ* operator ->()
	{
		return mpRefObj;
	}

	inline T_REFOBJ&  operator*()
	{
		return *mpRefObj;
	}

	inline T_REFOBJ* GetPtr()
	{
		return mpRefObj;
	}
private:
	CStaticRefObjMgrTemplate<T_ID,T_REFOBJ>*	mpRefObjMgr;
	T_REFOBJ*	mpRefObj;
};



template<typename T_ID,class T_REFOBJ>
class CAutoReleaseRefObjMgrTemplate1;
//////////////////////////////////////////////////////////////////////////
//引用计数管理类  T_REFOBJ 类必须从CRefObjectforCache继承
template<typename T_ID,class T_REFOBJ>
class CRefObjMgrTemplate1
{
public:
	typedef class u9_hash_map<T_ID, T_REFOBJ*> refobjid_map;
	typedef typename u9_hash_map<T_ID, T_REFOBJ*>::iterator refobjid_iter;

	CRefObjMgrTemplate1(){}
	virtual ~CRefObjMgrTemplate1(){clear();}
	virtual void clear()
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moMap.clear();
		moCacheList.clear();
	}
	inline uint32 size(){return static_cast<uint32>(moMap.size());}

	inline bool Initialize(uint32 size)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moCacheList.init(size);
		return true;
	};
public:
	friend class CAutoReleaseRefObjMgrTemplate1<T_ID,T_REFOBJ>;
public:
	//创建玩家信息对象
	T_REFOBJ* MallocRefObj()
	{
		T_REFOBJ* lpoRefObj = NULL;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		lpoRefObj = moCacheList.malloc();
		if(NULL ==lpoRefObj)
		{
			LOG_TRACE(LOG_ALERT, 0, __FUNCTION__, "!!moCacheList.malloc Error\n");
			return NULL;
		}
		//LOG_TRACE( 7, true, __FUNCTION__, " malloc"
		//			<<" Ref="<<lpoRefObj->GetRefCount()
		//			<<" RefObj="<<(void*)lpoRefObj
		//			<<" size="<<(int)moMap.size()
		//			<<" Ptr="<<this
		//			)
		lpoRefObj->init();
		return lpoRefObj;
	}
	//创建玩家信息对象
	VOID FreeRefObj(T_REFOBJ * apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		//LOG_TRACE( 7, true, __FUNCTION__, " free"
		//			<<" Ref="<<apoRefObj->GetRefCount()
		//			<<" RefObj="<<(void*)apoRefObj
		//			<<" size="<<(int)moMap.size()
		//			<<" Ptr="<<this
		//			)
		moCacheList.free(apoRefObj);
	}
	//删除玩家对象
	inline int32 DeleteRefObj(const T_ID aRefObjID)
	{
		T_REFOBJ* lpoRefObj=NULL;
		{
			CRITICAL_SECTION_HELPER(moCriticalSection);
			refobjid_iter ite = moMap.find(aRefObjID);
			if(ite==moMap.end())
			{
				LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error object not found"
					<<" ID="<<aRefObjID
					<<" size="<<(int)moMap.size()
					<<" Ptr="<<this
					)
				return -1;
			}
			lpoRefObj = ite->second;
			moMap.erase(ite);
			U9_ASSERT(NULL != lpoRefObj);
			lpoRefObj->DecreaseRef();

			if(lpoRefObj->CanRelease())
			{
				//LOG_TRACE( 7, true, __FUNCTION__, " free"
				//	<<" ID="<<aRefObjID
				//	<<" Ref="<<lpoRefObj->GetRefCount()
				//	<<" RefObj="<<(void*)lpoRefObj
				//	<<" size="<<(int)moMap.size()
				//	<<" Ptr="<<this
				//	)
				lpoRefObj->clear();
				moCacheList.free(lpoRefObj);
				return 1;
			}
			//LOG_TRACE( 7, true, __FUNCTION__, " del object"
			//	<<" ID="<<aRefObjID
			//	<<" Ref="<<lpoRefObj->GetRefCount()
			//	<<" RefObj="<<(void*)lpoRefObj
			//	<<" size="<<(int)moMap.size()
			//	<<" Ptr="<<this
			//	)
			return 0;
		}
	}


	
	//添加玩家信息
	inline T_REFOBJ* AddRefObj(const T_ID aRefObjID,T_REFOBJ*apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
		{
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error add exist"
				<<" ID="<<aRefObjID
				<<" Ref="<<apoRefObj->GetRefCount()
				<<" RefObj="<<(void*)apoRefObj
				<<" OldRefObj="<<(void*)ite->second
				<<" size="<<(int)moMap.size()
				<<" Ptr="<<this
				)
			return (T_REFOBJ*)ite->second;
		}
		//LOG_TRACE( 7, true, __FUNCTION__, " add"
		//			<<" ID="<<aRefObjID
		//			<<" Ref="<<apoRefObj->GetRefCount()
		//			<<" RefObj="<<(void*)apoRefObj
		//			<<" size="<<(int)moMap.size()
		//			<<" Ptr="<<this
		//			)
		apoRefObj->IncreaseRef();
		moMap.insert(pair<T_ID,  T_REFOBJ*>(aRefObjID,apoRefObj));
		return apoRefObj;
	}
	inline BOOL ExistRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
			return TRUE;
		return FALSE;
	}
public:
	inline CCriticalSection& GetCriticalSection(){return moCriticalSection;}
	inline refobjid_map *GetRefObjMap(){return &moMap;}
protected:
	inline T_REFOBJ* GetRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite!=moMap.end())
		{
			ite->second->IncreaseRef();
			return ite->second;
		}
		return NULL;
	}
	inline int32 PutRefObj(T_REFOBJ* apoRefObj)
	{
		if(NULL ==apoRefObj)
			return -1;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		apoRefObj->DecreaseRef();
		if(!apoRefObj->CanRelease())
			return 0;
		//LOG_TRACE( 7, true, __FUNCTION__, " free"
		//<<" Ref="<<apoRefObj->GetRefCount()
		//<<" RefObj="<<(void*)apoRefObj
		//)
		apoRefObj->clear();
		moCacheList.free(apoRefObj);
		return 1;
	}

protected:
	CCriticalSection		moCriticalSection;

	refobjid_map	moMap;
public:
	CCachePool<T_REFOBJ>	moCacheList;
};
//////////////////////////////////////////////////////////////////////////
//引用对象访问类
template<typename T_ID,class T_REFOBJ>
class CAutoReleaseRefObjMgrTemplate1
{
public:
	CAutoReleaseRefObjMgrTemplate1(CRefObjMgrTemplate1<T_ID,T_REFOBJ>& aoRefObjMgr,
		const T_ID& aRefObjID)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = mpRefObjMgr->GetRefObj(aRefObjID);
	}
	CAutoReleaseRefObjMgrTemplate1(CRefObjMgrTemplate1<T_ID,T_REFOBJ>& aoRefObjMgr,
		T_REFOBJ* apRefObj)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = apRefObj;
	}

	~CAutoReleaseRefObjMgrTemplate1()
	{
		if(NULL != mpRefObj)
			mpRefObjMgr->PutRefObj(mpRefObj);
	}
	inline void SetRefObj(T_REFOBJ* apRefObj)
	{
		mpRefObj = apRefObj;
	}
	inline bool Invalid()
	{
		return (mpRefObj==NULL);
	}

	inline T_REFOBJ* operator ->()
	{
		return mpRefObj;
	}

	inline T_REFOBJ&  operator*()
	{
		return *mpRefObj;
	}

	inline T_REFOBJ* GetPtr()
	{
		return mpRefObj;
	}
private:
	CRefObjMgrTemplate1<T_ID,T_REFOBJ>*	mpRefObjMgr;
	T_REFOBJ*	mpRefObj;
};


template<typename T_ID,class T_REFOBJ>
class CAutoReleaseStaticRefObjMgrTemplate1;
//////////////////////////////////////////////////////////////////////////
//引用计数管理类  T_REFOBJ 类必须从CRefObjectforCache继承
template<typename T_ID,class T_REFOBJ>
class CStaticRefObjMgrTemplate1
{
public:
	typedef class u9_hash_map<T_ID, T_REFOBJ*> refobjid_map;
	typedef typename u9_hash_map<T_ID, T_REFOBJ*>::iterator refobjid_iter;

	CStaticRefObjMgrTemplate1(){}
	virtual ~CStaticRefObjMgrTemplate1(){clear();}
	virtual void clear()
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moMap.clear();
		moCacheList.clear();
	}
	inline uint32 size(){return static_cast<uint32>(moMap.size());}

	inline bool Initialize(uint32 size)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moCacheList.init(size);
		return true;
	};
public:
	friend class CAutoReleaseStaticRefObjMgrTemplate1<T_ID,T_REFOBJ>;
public:
	//创建玩家信息对象
	T_REFOBJ* MallocRefObj()
	{
		T_REFOBJ* lpoRefObj = NULL;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		lpoRefObj = moCacheList.malloc();
		if(NULL ==lpoRefObj)
		{
			LOG_TRACE(LOG_ALERT, false, __FUNCTION__, "!!moCacheList.malloc Error\n");
			return NULL;
		}
		lpoRefObj->init();
		return lpoRefObj;
	}
	//创建玩家信息对象
	VOID FreeRefObj(T_REFOBJ * apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		apoRefObj->clear();
		moCacheList.free(apoRefObj);
	}
	//删除玩家对象
	inline int32 DeleteRefObj(const T_ID aRefObjID)
	{
		T_REFOBJ* lpoRefObj=NULL;
		{
			CRITICAL_SECTION_HELPER(moCriticalSection);
			refobjid_iter ite = moMap.find(aRefObjID);
			if(ite==moMap.end())
				return -1;
			lpoRefObj = ite->second;
			moMap.erase(ite);
			U9_ASSERT(NULL != lpoRefObj);
			lpoRefObj->DecreaseRef();

			if(lpoRefObj->CanRelease())
			{
				lpoRefObj->clear();
				moCacheList.free(lpoRefObj);
				return 1;
			}
			return 0;
		}
	}
	//添加玩家信息
	inline T_REFOBJ* AddRefObj(const T_ID aRefObjID,T_REFOBJ*apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
		{
			//LOG_TRACE(5, 0, __FUNCTION__, "!!Exist PlayerID="<<aRefObjID
			//	<<"\tRefCount="<<apoRefObj->GetRefCount()
			//	);
			return (T_REFOBJ*)ite->second;
		}
		apoRefObj->IncreaseRef();
		moMap.insert(pair<T_ID,  T_REFOBJ*>(aRefObjID,apoRefObj));
		return apoRefObj;
	}
	inline BOOL ExistRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
			return TRUE;
		return FALSE;
	}
	inline T_REFOBJ** GetCacheList() const {return moCacheList.GetCacheList();}
	inline int32 GetCacheMaxCount(){return moCacheList.maxsize();}
    inline int32 GetSize() {return moCacheList.size();}
public:
	inline CCriticalSection& GetCriticalSection(){return moCriticalSection;}
	inline refobjid_map *GetRefObjMap(){return &moMap;}
protected:
	inline T_REFOBJ* GetRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite!=moMap.end())
		{
			ite->second->IncreaseRef();
			return ite->second;
		}
		return NULL;
	}
	inline int32 PutRefObj(T_REFOBJ* apoRefObj)
	{
		if(NULL ==apoRefObj)
			return -1;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		apoRefObj->DecreaseRef();
		if(!apoRefObj->CanRelease())
			return 0;
		apoRefObj->clear();
		moCacheList.free(apoRefObj);
		return 1;
	}
protected:
	CCriticalSection		moCriticalSection;
	refobjid_map	moMap;
	CStaticCachePool<T_REFOBJ>	moCacheList;
};

//////////////////////////////////////////////////////////////////////////
//引用对象访问类
template<typename T_ID,class T_REFOBJ>
class CAutoReleaseStaticRefObjMgrTemplate1
{
public:
	CAutoReleaseStaticRefObjMgrTemplate1(CStaticRefObjMgrTemplate1<T_ID,T_REFOBJ>& aoRefObjMgr,
		const T_ID& aRefObjID)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = mpRefObjMgr->GetRefObj(aRefObjID);
	}
	CAutoReleaseStaticRefObjMgrTemplate1(CStaticRefObjMgrTemplate1<T_ID,T_REFOBJ>& aoRefObjMgr,
		T_REFOBJ* apRefObj)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = apRefObj;
	}

	~CAutoReleaseStaticRefObjMgrTemplate1()
	{
		if(NULL != mpRefObj)
			mpRefObjMgr->PutRefObj(mpRefObj);
	}
	inline void SetRefObj(T_REFOBJ* apRefObj)
	{
		mpRefObj = apRefObj;
	}
	inline bool Invalid()
	{
		return (mpRefObj==NULL);
	}

	inline T_REFOBJ* operator ->()
	{
		return mpRefObj;
	}

	inline T_REFOBJ&  operator*()
	{
		return *mpRefObj;
	}

	inline T_REFOBJ* GetPtr()
	{
		return mpRefObj;
	}
private:
	CStaticRefObjMgrTemplate1<T_ID,T_REFOBJ>*	mpRefObjMgr;
	T_REFOBJ*	mpRefObj;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T_ID,class T_REFOBJ>
class CAutoReleaseRefObjMgrTemplate2;
//////////////////////////////////////////////////////////////////////////
//引用计数管理类  T_REFOBJ 类必须从CRefObjectforCache继承
template<typename T_ID,class T_REFOBJ>
class CRefObjMgrTemplate2
{
public:
	typedef class CU9Hashmap<T_ID, T_REFOBJ*> refobjid_map;
	typedef typename CU9Hashmap<T_ID, T_REFOBJ*>::iterator refobjid_iter;

	CRefObjMgrTemplate2(){}
	virtual ~CRefObjMgrTemplate2(){clear();}
	virtual void clear()
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moMap.clear();
		moCacheList.clear();
	}
	inline uint32 size(){return static_cast<uint32>(moMap.size());}

	inline bool Initialize(uint32 size)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moMap.init();
		moCacheList.init(size);
		return true;
	};
public:
	friend class CAutoReleaseRefObjMgrTemplate2<T_ID,T_REFOBJ>;
public:
	//创建玩家信息对象
	T_REFOBJ* MallocRefObj()
	{
		T_REFOBJ* lpoRefObj = NULL;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		lpoRefObj = moCacheList.malloc();
		if(NULL ==lpoRefObj)
		{
			LOG_TRACE(LOG_ALERT, 0, __FUNCTION__, "!!moCacheList.malloc Error\n");
			return NULL;
		}
		//LOG_TRACE( 7, true, __FUNCTION__, " malloc"
		//	<<" Ref="<<lpoRefObj->GetRefCount()
		//	<<" RefObj="<<(void*)lpoRefObj
		//	<<" size="<<(int)moMap.size()
		//	<<" Ptr="<<this
		//	)
		lpoRefObj->init();
		return lpoRefObj;
	}
	//创建玩家信息对象
	VOID FreeRefObj(T_REFOBJ * apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		//LOG_TRACE( 7, true, __FUNCTION__, " free"
		//	<<" Ref="<<apoRefObj->GetRefCount()
		//	<<" RefObj="<<(void*)apoRefObj
		//	<<" size="<<(int)moMap.size()
		//	<<" Ptr="<<this
		//	)
			moCacheList.free(apoRefObj);
	}
	//删除玩家对象
	inline int32 DeleteRefObj(const T_ID aRefObjID)
	{
		T_REFOBJ* lpoRefObj=NULL;
		{
			CRITICAL_SECTION_HELPER(moCriticalSection);
			refobjid_iter ite = moMap.find(aRefObjID);
			if(ite==moMap.end())
			{
				LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error object not found"
					<<" ID="<<aRefObjID
					<<" size="<<(int)moMap.size()
					<<" Ptr="<<this
					)
					return -1;
			}
			lpoRefObj = ite->second;
			moMap.erase(ite);
			U9_ASSERT(NULL != lpoRefObj);
			lpoRefObj->DecreaseRef();

			if(lpoRefObj->CanRelease())
			{
				//LOG_TRACE( 7, true, __FUNCTION__, " free"
				//	<<" ID="<<aRefObjID
				//	<<" Ref="<<lpoRefObj->GetRefCount()
				//	<<" RefObj="<<(void*)lpoRefObj
				//	<<" size="<<(int)moMap.size()
				//	<<" Ptr="<<this
				//	)
					lpoRefObj->clear();
				moCacheList.free(lpoRefObj);
				return 1;
			}
			//LOG_TRACE( 7, true, __FUNCTION__, " del object"
			//	<<" ID="<<aRefObjID
			//	<<" Ref="<<lpoRefObj->GetRefCount()
			//	<<" RefObj="<<(void*)lpoRefObj
			//	<<" size="<<(int)moMap.size()
			//	<<" Ptr="<<this
			//	)
				return 0;
		}
	}



	//添加玩家信息
	inline T_REFOBJ* AddRefObj(const T_ID aRefObjID,T_REFOBJ*apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
		{
			LOG_TRACE(LOG_WARNING, false, __FUNCTION__, "Error add exist"
				<<" ID="<<aRefObjID
				<<" Ref="<<apoRefObj->GetRefCount()
				<<" RefObj="<<(void*)apoRefObj
				<<" OldRefObj="<<(void*)ite->second
				<<" size="<<(int)moMap.size()
				<<" Ptr="<<this
				)
				return (T_REFOBJ*)ite->second;
		}
		//LOG_TRACE( 7, true, __FUNCTION__, " add"
		//	<<" ID="<<aRefObjID
		//	<<" Ref="<<apoRefObj->GetRefCount()
		//	<<" RefObj="<<(void*)apoRefObj
		//	<<" size="<<(int)moMap.size()
		//	<<" Ptr="<<this
		//	)
		apoRefObj->IncreaseRef();
		moMap.insert(aRefObjID,apoRefObj);
		return apoRefObj;
	}
	inline BOOL ExistRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
			return TRUE;
		return FALSE;
	}
public:
	inline CCriticalSection& GetCriticalSection(){return moCriticalSection;}
	inline refobjid_map *GetRefObjMap(){return &moMap;}
protected:
	inline T_REFOBJ* GetRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);

		if(ite!=moMap.end())
		{
			ite->second->IncreaseRef();
			//LOG_TRACE( 7, true, __FUNCTION__, " GetRefObj"
			//	<<" RefObjID="<<aRefObjID
			//	<<" size="<<(int)moMap.size()
			//	<<" ID="<<ite->first
			//	<<" Ptr="<<(void*)ite->second
			//	<<" Ptr="<<this
			//	)
			return ite->second;
		}
		//for (ite=moMap.begin();ite!=moMap.end();ite++)
		//{
		//	LOG_TRACE( 7, true, __FUNCTION__, " GetRefObj"
		//		<<" RefObjID="<<aRefObjID
		//		<<" size="<<(int)moMap.size()
		//		<<" Ptr="<<this
		//		<<" ID="<<ite->first
		//		<<" Ptr="<<(void*)ite->second
		//		)
		//}
		return NULL;
	}
	inline int32 PutRefObj(T_REFOBJ* apoRefObj)
	{
		if(NULL ==apoRefObj)
			return -1;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		apoRefObj->DecreaseRef();
		if(!apoRefObj->CanRelease())
			return 0;
		//LOG_TRACE( 7, true, __FUNCTION__, " free"
		//	<<" Ref="<<apoRefObj->GetRefCount()
		//	<<" RefObj="<<(void*)apoRefObj
		//	)
			apoRefObj->clear();
		moCacheList.free(apoRefObj);
		return 1;
	}

protected:
	CCriticalSection		moCriticalSection;

	refobjid_map	moMap;
public:
	CCachePool<T_REFOBJ>	moCacheList;
};
//////////////////////////////////////////////////////////////////////////
//引用对象访问类
template<typename T_ID,class T_REFOBJ>
class CAutoReleaseRefObjMgrTemplate2
{
public:
	CAutoReleaseRefObjMgrTemplate2(CRefObjMgrTemplate2<T_ID,T_REFOBJ>& aoRefObjMgr,
		const T_ID& aRefObjID)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = mpRefObjMgr->GetRefObj(aRefObjID);
	}
	CAutoReleaseRefObjMgrTemplate2(CRefObjMgrTemplate2<T_ID,T_REFOBJ>& aoRefObjMgr,
		T_REFOBJ* apRefObj)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = apRefObj;
	}

	~CAutoReleaseRefObjMgrTemplate2()
	{
		if(NULL != mpRefObj)
			mpRefObjMgr->PutRefObj(mpRefObj);
	}
	inline void SetRefObj(T_REFOBJ* apRefObj)
	{
		mpRefObj = apRefObj;
	}
	inline bool Invalid()
	{
		return (mpRefObj==NULL);
	}

	inline T_REFOBJ* operator ->()
	{
		return mpRefObj;
	}

	inline T_REFOBJ&  operator*()
	{
		return *mpRefObj;
	}

	inline T_REFOBJ* GetPtr()
	{
		return mpRefObj;
	}
private:
	CRefObjMgrTemplate2<T_ID,T_REFOBJ>*	mpRefObjMgr;
	T_REFOBJ*	mpRefObj;
};


template<typename T_ID,class T_REFOBJ>
class CAutoReleaseStaticRefObjMgrTemplate2;
//////////////////////////////////////////////////////////////////////////
//引用计数管理类  T_REFOBJ 类必须从CRefObjectforCache继承
template<typename T_ID,class T_REFOBJ>
class CStaticRefObjMgrTemplate2
{
public:
	typedef class CU9Hashmap<T_ID, T_REFOBJ*> refobjid_map;
	typedef typename CU9Hashmap<T_ID, T_REFOBJ*>::iterator refobjid_iter;

	CStaticRefObjMgrTemplate2(){}
	virtual ~CStaticRefObjMgrTemplate2(){clear();}
	virtual void clear()
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moMap.clear();
		moCacheList.clear();
	}
	inline uint32 size(){return static_cast<uint32>(moMap.size());}

	inline bool Initialize(uint32 size)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		moMap.init();
		moCacheList.init(size);
		return true;
	};
public:
	friend class CAutoReleaseStaticRefObjMgrTemplate2<T_ID,T_REFOBJ>;
public:
	//创建玩家信息对象
	T_REFOBJ* MallocRefObj()
	{
		T_REFOBJ* lpoRefObj = NULL;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		lpoRefObj = moCacheList.malloc();
		if(NULL ==lpoRefObj)
		{
			LOG_TRACE(LOG_ALERT, 0, __FUNCTION__, "!!moCacheList.malloc Error\n");
			return NULL;
		}
		lpoRefObj->init();
		return lpoRefObj;
	}
	//创建玩家信息对象
	VOID FreeRefObj(T_REFOBJ * apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		apoRefObj->clear();
		moCacheList.free(apoRefObj);
	}
	//删除玩家对象
	inline int32 DeleteRefObj(const T_ID aRefObjID)
	{
		T_REFOBJ* lpoRefObj=NULL;
		{
			CRITICAL_SECTION_HELPER(moCriticalSection);
			refobjid_iter ite = moMap.find(aRefObjID);
			if(ite==moMap.end())
				return -1;
			lpoRefObj = ite->second;
			moMap.erase(ite);
			U9_ASSERT(NULL != lpoRefObj);
			lpoRefObj->DecreaseRef();

			if(lpoRefObj->CanRelease())
			{
				lpoRefObj->clear();
				moCacheList.free(lpoRefObj);
				return 1;
			}
			return 0;
		}
	}
	//添加玩家信息
	inline T_REFOBJ* AddRefObj(const T_ID aRefObjID,T_REFOBJ*apoRefObj)
	{
		U9_ASSERT(NULL != apoRefObj);
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
		{
			//LOG_TRACE(5, 0, __FUNCTION__, "!!Exist PlayerID="<<aRefObjID
			//	<<"\tRefCount="<<apoRefObj->GetRefCount()
			//	);
			return (T_REFOBJ*)ite->second;
		}
		apoRefObj->IncreaseRef();
		moMap.insert(aRefObjID,apoRefObj);
		return apoRefObj;
	}
	inline BOOL ExistRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite != moMap.end())
			return TRUE;
		return FALSE;
	}
	inline T_REFOBJ** GetCacheList() const {return moCacheList.GetCacheList();}
	inline int32 GetCacheMaxCount(){return moCacheList.maxsize();}
	inline int32 GetSize() {return moCacheList.size();}
public:
	inline CCriticalSection& GetCriticalSection(){return moCriticalSection;}
	inline refobjid_map *GetRefObjMap(){return &moMap;}
protected:
	inline T_REFOBJ* GetRefObj(const T_ID aRefObjID)
	{
		CRITICAL_SECTION_HELPER(moCriticalSection);
		refobjid_iter ite = moMap.find(aRefObjID);
		if(ite!=moMap.end())
		{
			ite->second->IncreaseRef();
			return ite->second;
		}
		return NULL;
	}
	inline int32 PutRefObj(T_REFOBJ* apoRefObj)
	{
		if(NULL ==apoRefObj)
			return -1;
		CRITICAL_SECTION_HELPER(moCriticalSection);
		apoRefObj->DecreaseRef();
		if(!apoRefObj->CanRelease())
			return 0;
		apoRefObj->clear();
		moCacheList.free(apoRefObj);
		return 1;
	}
protected:
	CCriticalSection		moCriticalSection;
	refobjid_map	moMap;
	CStaticCachePool<T_REFOBJ>	moCacheList;
};

//////////////////////////////////////////////////////////////////////////
//引用对象访问类
template<typename T_ID,class T_REFOBJ>
class CAutoReleaseStaticRefObjMgrTemplate2
{
public:
	CAutoReleaseStaticRefObjMgrTemplate2(CStaticRefObjMgrTemplate2<T_ID,T_REFOBJ>& aoRefObjMgr,
		const T_ID& aRefObjID)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = mpRefObjMgr->GetRefObj(aRefObjID);
	}
	CAutoReleaseStaticRefObjMgrTemplate2(CStaticRefObjMgrTemplate2<T_ID,T_REFOBJ>& aoRefObjMgr,
		T_REFOBJ* apRefObj)
	{
		mpRefObjMgr = &aoRefObjMgr;
		mpRefObj = apRefObj;
	}

	~CAutoReleaseStaticRefObjMgrTemplate2()
	{
		if(NULL != mpRefObj)
			mpRefObjMgr->PutRefObj(mpRefObj);
	}
	inline void SetRefObj(T_REFOBJ* apRefObj)
	{
		mpRefObj = apRefObj;
	}
	inline bool Invalid()
	{
		return (mpRefObj==NULL);
	}

	inline T_REFOBJ* operator ->()
	{
		return mpRefObj;
	}

	inline T_REFOBJ&  operator*()
	{
		return *mpRefObj;
	}

	inline T_REFOBJ* GetPtr()
	{
		return mpRefObj;
	}
private:
	CStaticRefObjMgrTemplate2<T_ID,T_REFOBJ>*	mpRefObjMgr;
	T_REFOBJ*	mpRefObj;
};

template<class T_ID, class T_REFOBJ>
class CAutoReleaseObj
{
public:
	CAutoReleaseObj(void);

	CAutoReleaseObj(CRefObjMgrTemplate<T_ID, T_REFOBJ>& aoRefObjMgr, const T_ID& aiID);

	CAutoReleaseObj(CRefObjMgrTemplate<T_ID, T_REFOBJ>& aoRefObjMgr, T_REFOBJ* apoRefObj);

	~CAutoReleaseObj(void);

	bool Invalid(void) const;

	T_REFOBJ* operator ->(void);

	const T_REFOBJ* operator ->(void) const;

	T_REFOBJ& operator*(void);

	const T_REFOBJ& operator*(void) const;

	T_REFOBJ* GetPtr(void);

	const T_REFOBJ* GetPtr(void) const;

	void SetPtr(T_REFOBJ* apoRefObj);

protected:

	CRefObjMgrTemplate<T_ID, T_REFOBJ>* mpRefObjMgr;

	mutable T_REFOBJ* mpoRefObj;

	static const T_REFOBJ moNullRefObj;
};

template<class T_ID, class T_REFOBJ>
const T_REFOBJ CAutoReleaseObj<T_ID, T_REFOBJ>::moNullRefObj;

template<class T_ID, class T_REFOBJ>
CAutoReleaseObj<T_ID, T_REFOBJ>::CAutoReleaseObj(void) : mpoRefObj(NULL), mpRefObjMgr(NULL)
{
}

template<class T_ID, class T_REFOBJ>
CAutoReleaseObj<T_ID, T_REFOBJ>::CAutoReleaseObj( CRefObjMgrTemplate<T_ID, T_REFOBJ>& aoRefObjMgr, const T_ID& aiID )
{
	mpRefObjMgr = &aoRefObjMgr;
	mpoRefObj = mpRefObjMgr->GetRefObj(aiID);
}

template<class T_ID, class T_REFOBJ>
CAutoReleaseObj<T_ID, T_REFOBJ>::CAutoReleaseObj(CRefObjMgrTemplate<T_ID, T_REFOBJ>& aoRefObjMgr, T_REFOBJ* apoRefObj)
{
	mpRefObjMgr = &aoRefObjMgr;
	if (apoRefObj != &moNullRefObj)
	{
		mpoRefObj = apoRefObj;
	}
	else
	{
		mpoRefObj = NULL;
	}
}

template<class T_ID, class T_REFOBJ>
CAutoReleaseObj<T_ID, T_REFOBJ>::~CAutoReleaseObj(void)
{
	if(!Invalid())
	{
		mpRefObjMgr->PutRefObj(mpoRefObj);
	}
}

template<class T_ID, class T_REFOBJ>
bool CAutoReleaseObj<T_ID, T_REFOBJ>::Invalid( void ) const
{
	return (mpoRefObj == NULL || mpoRefObj == &moNullRefObj);
}

template<class T_ID, class T_REFOBJ>
T_REFOBJ* CAutoReleaseObj<T_ID, T_REFOBJ>::operator ->(void)
{
	if (mpoRefObj == &moNullRefObj)
	{
		mpoRefObj = NULL;
	}
	return mpoRefObj;
}

template<class T_ID, class T_REFOBJ>
const T_REFOBJ* CAutoReleaseObj<T_ID, T_REFOBJ>::operator ->(void) const
{
	if (mpoRefObj == &moNullRefObj)
	{
		mpoRefObj = NULL;
	}
	return mpoRefObj;
}

template<class T_ID, class T_REFOBJ>
T_REFOBJ& CAutoReleaseObj<T_ID, T_REFOBJ>::operator *(void)
{
	if (mpoRefObj != NULL)
	{
		return *mpoRefObj;
	}
	return moNullRefObj;
}

template<class T_ID, class T_REFOBJ>
const T_REFOBJ& CAutoReleaseObj<T_ID, T_REFOBJ>::operator *(void) const
{
	if (mpoRefObj != NULL)
	{
		return *mpoRefObj;
	}
	return moNullRefObj;
}

template<class T_ID, class T_REFOBJ>
T_REFOBJ* CAutoReleaseObj<T_ID, T_REFOBJ>::GetPtr(void)
{
	if (mpoRefObj == &moNullRefObj)
	{
		mpoRefObj = NULL;
	}
	return mpoRefObj;
}

template<class T_ID, class T_REFOBJ>
const T_REFOBJ* CAutoReleaseObj<T_ID, T_REFOBJ>::GetPtr(void) const
{
	if (mpoRefObj == &moNullRefObj)
	{
		mpoRefObj = NULL;
	}
	return mpoRefObj;
}

template<class T_ID, class T_REFOBJ>
void CAutoReleaseObj<T_ID, T_REFOBJ>::SetPtr(T_REFOBJ* apoRefObj)
{
	if (apoRefObj == &moNullRefObj)
	{
		mpoRefObj = NULL;
	}
	mpoRefObj = apoRefObj;
	return;
}

U9_END_NAMESPACE
#endif	//__REFOBJMGRTEMPLATE_H_