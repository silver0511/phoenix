// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//  
// 文件名称: U9Hashmap.h
// 创 建 人: 史云杰
// 文件说明: linux下hashmap，支持int64 这个类会耗费内存。服务器使用 windows下，最好使用hash_map
// 原理 一个双向队列（存放实际数据，便于循环查找）。一个map数组（便于find）
// $_FILEHEADER_END ******************************

#ifndef __U9HASHMAP_H
#define __U9HASHMAP_H

#include "base0/platform.h"
#include "common/Queue2.h"
#include <map>


U9_BEGIN_NAMESPACE

	//////////////////////////////////////////////////////////////////////////
	// CU9Hashmap
	template <class T_ID,class _T> 
class CU9Hashmap
{
public:
	class pair {
	public:
		typedef T_ID first_type;
		typedef _T second_type;
		T_ID first;
		_T second;
	};
	
	//////////////////////////////////////////////////////////////////////////
	// map 双向队列 内部使用。
	class CHashMapDoubleQueue
	{
	public:
		struct STRU_NODE_INFO:public pair
		{
			STRU_NODE_INFO	*mpNext;
			STRU_NODE_INFO	*mpPrev;
		} ;

		CHashMapDoubleQueue()
		{
			mpHead	= NULL;
			mpTail	= NULL;
			miCount = 0;

			mpHasMap = NULL;
			mpFreeNodeHead = NULL;
			miCurrentCacheCount = 0;
			miMaxCacheCount = 0;
		}
		~CHashMapDoubleQueue()
		{
			clear();
		}
		void init(CU9Hashmap *apHashMap,int aiCacheCount)
		{
			mpHasMap = apHashMap;
			miMaxCacheCount = aiCacheCount;
		}

	public:
		inline int  size() const{return miCount;}
		
		inline STRU_NODE_INFO* push_back(const T_ID &aiID,_T& aoData)
		{
			STRU_NODE_INFO* lpNewNode = New_Node();
			if(NULL == lpNewNode)
				return NULL;
			lpNewNode->first = aiID;
			lpNewNode->second = aoData;
			lpNewNode->mpPrev = mpTail;
			lpNewNode->mpNext = INVALID_U9_POSITION;

			if (NULL == mpTail)
			{
				mpHead = lpNewNode;
			}
			else
			{
				mpTail->mpNext = lpNewNode;
			}
			mpTail = lpNewNode;
			miCount++;
			return lpNewNode;
		}
		inline STRU_NODE_INFO* begin() const{return mpHead; };
		inline STRU_NODE_INFO* end() const{return NULL; };

		inline STRU_NODE_INFO* erase(STRU_NODE_INFO *apNode)
		{
			if( NULL  == apNode  )
				return NULL;
			STRU_NODE_INFO* lpPrevNode = apNode->mpPrev;
			STRU_NODE_INFO* lpNextNode = apNode->mpNext;
			//是否到头
			if (NULL == lpPrevNode)
				mpHead = lpNextNode;
			else
				lpPrevNode->mpNext = lpNextNode;
			//是否到尾
			if (NULL == lpNextNode)
				mpTail = lpPrevNode;
			else
				lpNextNode->mpPrev = lpPrevNode;

			if(miCount > 0 )
				miCount--;
			Free_Node(apNode);
			return lpNextNode;
		}

		inline void clear()
		{
			STRU_NODE_INFO *lpNode;
			while (NULL != mpHead)
			{
				lpNode = mpHead;
				mpHead = mpHead->mpNext;
				Free_Node(lpNode);
			}
			mpTail = NULL;
			miCount = 0;
		}
	protected:
		inline STRU_NODE_INFO* New_Node()
		{
			STRU_NODE_INFO *lpNode = mpFreeNodeHead;
			if (NULL == mpFreeNodeHead)
			{//没有自由的节点
				lpNode = new STRU_NODE_INFO;
				return lpNode;
			}
			//使用cahche的节点
			mpFreeNodeHead = mpFreeNodeHead->mpNext;
			if(miCurrentCacheCount>0)
				miCurrentCacheCount--;
			return lpNode;
		}
		inline  void  Free_Node(STRU_NODE_INFO* apNode)
		{
			if(miCurrentCacheCount>= miMaxCacheCount)
			{//不能cahche了
				delete apNode;
				return;
			}
			apNode->mpNext = mpFreeNodeHead;
			mpFreeNodeHead = apNode;
			miCurrentCacheCount++;
		}

	protected:
		struct STRU_NODE_INFO *mpHead; // 当前使用头指针
		struct STRU_NODE_INFO *mpTail; // 当前使用尾指针
		int    miCount;
		friend class CU9Hashmap;
		CU9Hashmap *mpHasMap;

		STRU_NODE_INFO *mpFreeNodeHead;// 当前自由的节点头。
		int miCurrentCacheCount;//当前cache数
		int miMaxCacheCount;//最大cache数
	};

public:
	typedef class CHashMapDoubleQueue  CHashDQ;
	typedef typename CHashMapDoubleQueue::STRU_NODE_INFO CNodeInfo;//队列节点
	typedef class std::map<T_ID,CNodeInfo*> CSubMap;//下一步自己实现一个红黑树带cache的方式，防止new和delete 或者实现分配器
public:

	////////////////////////////////////////////////////////////////
	//map 迭代器
	class iterator
	{
	public:
		iterator()
		{
			mpNode= NULL;
		}

		inline iterator(CNodeInfo* apNode)
		{
			mpNode = apNode;
		}
		inline iterator(const iterator & aoItor)
		{
			mpNode = aoItor.mpNode;
		}
		inline iterator & operator = (const iterator & aoItor)
		{
			mpNode = aoItor.mpNode;
			return *this;
		}
		inline bool operator == (const iterator& aoItor)
		{
			return mpNode == aoItor.mpNode;
		}
		inline pair* operator ->()
		{
			return mpNode;
		}
		inline iterator & operator ++()
		{
			if(NULL == mpNode)
				return *this;
			mpNode=mpNode->mpNext;
			return *this;
		}
		inline iterator operator++(int)
		{	// postincrement
			iterator _Tmp = *this;
			++*this;
			return (_Tmp);
		}

		inline iterator & operator --()
		{
			if(NULL == mpNode)
				return *this;
			mpNode=mpNode->mpPrev;
			return *this;
		}
		inline iterator operator--(int)
		{	
			iterator _Tmp = *this;
			--*this;
			return (_Tmp);
		}
		
		inline operator CNodeInfo* ()
		{
			return mpNode;
		};   
	private:
		friend class CU9Hashmap;
		CNodeInfo*     mpNode;
	};
////////////////////////////////////////////////////////////////////////////////////
//map实际处理代码
public:

	CU9Hashmap()
	{
		mpSubMapList = NULL;
		miFirshHashMapSize =1;
	}
	virtual ~CU9Hashmap()
	{
		clear();
		if(NULL != mpSubMapList)
		{
			delete[]mpSubMapList;
			mpSubMapList = NULL;
		}
	}
	void init(int aiTotalHashCount=1,int aiSubHashSize=10000)
	{
		//定位cache
		if(aiTotalHashCount< aiSubHashSize)
			aiTotalHashCount = aiSubHashSize;

		if(0 == aiTotalHashCount % aiSubHashSize)
			miFirshHashMapSize = (aiTotalHashCount/aiSubHashSize);
		else 
			miFirshHashMapSize = (aiTotalHashCount/aiSubHashSize)+1;
		if(0 ==miFirshHashMapSize)
			miFirshHashMapSize = 1;
		

		mpSubMapList = new CSubMap[miFirshHashMapSize];
		moDQ.init(this,aiTotalHashCount);
	}
	//清空
	void clear()
	{
		if(NULL == mpSubMapList)
			return ;
		for (int i=0;i<miFirshHashMapSize;i++)
		{
			mpSubMapList[i].clear();
		}
		moDQ.clear();
	}
	//大小
	inline int32 size()
	{
		return moDQ.size();
	}
	inline iterator begin()
	{
		return iterator(moDQ.begin());
	}
	inline iterator end()
	{
		return iterator(moDQ.end());
	}
	
	//添加对象 
	std::pair<iterator,bool> insert(const T_ID& aiID,_T aoObject)
	{
		std::pair<iterator,bool> loReturnValue;

		CNodeInfo *lpNodeInfo = moDQ.push_back(aiID,aoObject);//添加到队列中
		if(NULL == lpNodeInfo)
		{
			loReturnValue.first =  iterator(NULL);
			loReturnValue.second = false;
			return loReturnValue;
		}
		
		CSubMap &loMap =  mpSubMapList[aiID % miFirshHashMapSize];
		std::pair<typename CSubMap::iterator,bool> pr=loMap.insert(make_pair(aiID,lpNodeInfo));//添加到map中

		if(pr.second == false)
		{
			moDQ.erase(lpNodeInfo);
			loReturnValue.first =  iterator(NULL);
			loReturnValue.second = false;
			return loReturnValue;
		}
		loReturnValue.first = iterator(lpNodeInfo);
		loReturnValue.second = true;
		return loReturnValue; 
	}
	//删除 按ID
	iterator erase(const T_ID& aiID)
	{
		CSubMap &loMap =  mpSubMapList[ aiID % miFirshHashMapSize];
		typename CSubMap::iterator ite = loMap.find(aiID);
		if(ite == loMap.end())
			return iterator(NULL); 
		CNodeInfo *lpNodeInfo = ite->second;
		loMap.erase(ite);
		lpNodeInfo = moDQ.erase(lpNodeInfo);
		return iterator(lpNodeInfo); 
	}
	//删除按迭代器
	iterator erase (iterator & aoItor)
	{
		CNodeInfo *lpNodeInfo  = aoItor.mpNode;
		CSubMap &loMap =  mpSubMapList[ lpNodeInfo->first % miFirshHashMapSize];
		typename CSubMap::iterator ite = loMap.find( lpNodeInfo->first);
		if(ite != loMap.end())
		{//日志
			loMap.erase(ite);
		}
		
		lpNodeInfo = moDQ.erase(lpNodeInfo);
		return iterator(lpNodeInfo); 
	}
	inline iterator find(const T_ID& aiID)
	{
		CSubMap &loMap =  mpSubMapList[aiID % miFirshHashMapSize];
		typename CSubMap::iterator ite = loMap.find(aiID);

		if(ite == loMap.end())
		{
			return iterator(NULL); 
		}
			
		return iterator(ite->second);
	}

protected:
		int32 miFirshHashMapSize;//map大小
		CSubMap *mpSubMapList;//map list 提供定位
		CHashDQ moDQ;//双向队列 提供循环访问
};

U9_END_NAMESPACE
#endif //__U9HASHMAP_H

