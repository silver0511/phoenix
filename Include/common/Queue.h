// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：Queue.h
// 创建人： 史云杰
// 文件说明：队列
// $_FILEHEADER_END ******************************
#ifndef __QUEUEU_H
#define __QUEUEU_H
#include <stdlib.h>
#include "CircleIndexDef.h"
#include "CriticalSection.h"


typedef INT (FUNTYPE_QueueCompare )(VOID * apSrc,VOID * apDest);
//动态链表
template <class T>
class CQueue
{
public:
	struct STRUCT_QUEUE_NODE
	{
		T* mpData;
		STRUCT_QUEUE_NODE * mpNext;
	};

	inline CQueue(bool abIsDelData=true)
	{
		miCount = 0;
		mpHead = NULL;
		mpTail = NULL;
		mbIsDelData=abIsDelData;
		Compare=NULL;
	}
    inline ~CQueue()
	{
		clear();
	};

    inline INT AddTail(T * apValue){
		STRUCT_QUEUE_NODE * lpNode= new STRUCT_QUEUE_NODE;
		if (NULL == lpNode)
			return -1;
		lpNode->mpNext = NULL;
		lpNode->mpData = apValue;
		if (mpHead == NULL){//没有数据
			mpHead = lpNode;
			mpTail = lpNode;
		}
		else{
			mpTail->mpNext =lpNode;
			mpTail = lpNode;
		}
       	miCount++;
		return miCount;
	};
	inline T * GetandDelHead(){
		T* lstru;
		if (mpHead == NULL)
			lstru = NULL;
		else{
			lstru = (T*)mpHead->mpData;
			STRUCT_QUEUE_NODE * lpNode=NULL;
			lpNode = mpHead->mpNext; 
			delete mpHead;
			mpHead = lpNode;
			if (mpHead == NULL)
				mpTail= NULL;
			miCount--;
		}
		return lstru;
	};
	inline T * GetHead(){
		if (mpHead == NULL)
			return NULL;
        return (T*)mpHead->mpData;
	};
	
	inline bool DelHead(){
		if (mpHead == NULL)
            return false;
		else{
			STRUCT_QUEUE_NODE * lpNode=NULL;
			lpNode = mpHead->mpNext; 
			delete mpHead;
			mpHead = lpNode;
			if (mpHead == NULL)
				mpTail= NULL;
			miCount--;
		}
		return true;
	};
	inline VOID clear(){
		STRUCT_QUEUE_NODE * lpNode=NULL;
		while (mpHead != NULL){
			lpNode = mpHead->mpNext;
			if (true == mbIsDelData)
				delete (T*)mpHead->mpData;
			delete mpHead;
			mpHead =lpNode;
		}
		mpTail=NULL;
		miCount = 0;
	}
	inline VOID clear(UINT aiCount){
		STRUCT_QUEUE_NODE * lpNode=NULL;
		UINT i=0;
		while (mpHead != NULL){
			lpNode = mpHead->mpNext;
			if (true == mbIsDelData)
				delete (T*)mpHead->mpData;
			delete mpHead;
			mpHead =lpNode;
			if (mpHead == NULL)
				mpTail= NULL;
			i++;
			if (i == aiCount){
				break;
			}
		}
		miCount-=i;
	}
	inline INT size(){return miCount;};

	inline INT Delete(T * apValue){
		STRUCT_QUEUE_NODE * lpNode=NULL;
		STRUCT_QUEUE_NODE * lpOldNode=NULL;
		lpNode = mpHead;
		lpOldNode = lpNode;
		while (  NULL != lpNode){
			if (0 ==Compare(apValue,lpNode->mpData))
			{
				miCount--;	
				if (lpNode == mpHead){//最前一个
					mpHead=lpNode->mpNext;
					if (mpHead == NULL)
						mpTail= NULL;
				}
				else if (lpNode == mpTail){//最后一个
					mpTail=lpOldNode;
					lpOldNode->mpNext=NULL;
					if (mpTail == NULL)
						mpHead= NULL;
                }
				else
					lpOldNode->mpNext=lpNode->mpNext;
				
				if (true == mbIsDelData)
					delete (T*)lpNode->mpData;
				delete  lpNode;
				return 1;
			}
			lpOldNode=lpNode;
			lpNode=lpNode->mpNext;
		}
		return 0;
	};
	
	inline INT DeletebyIndex(INT aiIndex){
		INT i=0;
		if (aiIndex>=miCount&&aiIndex<0)
			return -1;
		STRUCT_QUEUE_NODE * lpNode=NULL;
		STRUCT_QUEUE_NODE * lpOldNode=NULL;
		lpNode = mpHead;
		lpOldNode = lpNode;
		while (  NULL != lpNode){
			if (aiIndex == i)
			{
				miCount--;	
				if (lpNode == mpHead){//最前一个
					mpHead=lpNode->mpNext;
					lpOldNode=mpHead;
					if (mpHead == NULL)
						mpTail= NULL;
				}
				else if (lpNode == mpTail){//最后一个
					mpTail=lpOldNode;
					lpOldNode->mpNext=NULL;
					if (mpTail == NULL)
						mpHead= NULL;
				}
				else
					lpOldNode->mpNext=lpNode->mpNext;

				if (true == mbIsDelData)
					delete (T*)lpNode->mpData;
				delete  lpNode;
				return 1;
			}
			lpOldNode=lpNode;
			lpNode=lpNode->mpNext;
			i++;
		}
		return 0;
	};
	
	inline STRUCT_QUEUE_NODE * GetHeadNode(){return mpHead;};
	inline VOID SetCompareFun(FUNTYPE_QueueCompare * apValue){Compare=apValue;};
private:
	UINT miCount;
	STRUCT_QUEUE_NODE *mpHead;
	STRUCT_QUEUE_NODE *mpTail;
	bool mbIsDelData;
	FUNTYPE_QueueCompare *Compare; 
};

//cache列表 静态列表方式 线程安全
template<class T>
class CSafeStaticQueue 
{
public:
	CSafeStaticQueue()
	{
		InitMember();
		mpDataList=NULL;
	};
	CSafeStaticQueue(INT aiMaxCount)
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
	~CSafeStaticQueue()
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
		T* lpValue=NULL;
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

	inline T * GetandDelHead()
	{
		T *lpValue =NULL;
		/**/
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
		/**/
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

#endif //_CQueue_H

