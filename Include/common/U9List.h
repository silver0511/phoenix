/***********************************************************
// 版权声明：xxxxx版权所有
//
// 文件名称：U9List.h
// 创建人： 史云杰
// 文件说明：引用计数类
***********************************************************/
#ifndef __CLIST_H
#define __CLIST_H
#include <stdlib.h>
#include "CriticalSection.h"
#ifdef WIN32
#include <windows.h>
#endif //WIN32
#ifdef _XUNIX
#include <pthread.h>
#include <unistd.h>
#endif 
using namespace std;
template <class T>
class _CList
{
public:
	inline _CList(){
		miCount = 0;
		miMaxCount=0;
		mpData=NULL;
	}
	inline _CList(INT aiStackSize)	{
		miCount = 0;
		miMaxCount=aiStackSize;
		mpData=new T[aiStackSize];
	}
	inline INT SetStackSize(INT aiStackSize){
		if (NULL !=mpData)
			delete []mpData;
		miMaxCount=aiStackSize;
		mpData = new T[aiStackSize];
		if (mpData == NULL)
			return -1;
		return 0;
	}
	inline ~_CList(){
		for(INT i=0;i<miCount;i++){
			if (NULL != mpData[i]){
				delete (T) mpData[i];
				mpData[i] =NULL;
			}
		}
		if (NULL != mpData){
			delete []mpData;
			mpData = NULL;
		}
		miCount = 0;
	};
	inline INT Add(T apValue){
		if (miCount>=miMaxCount)
			return -1;
		mpData[miCount]=apValue;
		miCount++;
		return miCount;
	};

	inline T Get(INT aiIndex){
		if (aiIndex>=miCount)
			return NULL;
		return mpData[aiIndex];
	};

	inline INT Del(INT aiIndex){
		if (aiIndex>=miMaxCount)
			return -1;
		if (NULL != mpData[aiIndex]){
			delete (T) mpData[aiIndex];
			mpData[aiIndex] =NULL;
		}
		for (INT i= aiIndex;i<miCount-1;i++){
			mpData[i]=	mpData[i+1];
		}
		miCount--;
		return 0;
	};
	inline VOID ClearAll(){
		for(INT i=0;i<miCount;i++){
			if (NULL != mpData[i]){
				delete (T) mpData[i];
				mpData[i] =NULL;
			}
		}
		miCount = 0;
	}
	inline INT GetCount(){return miCount;};

private:
	INT  miCount;
	INT  miMaxCount;
	T * mpData;
};

template <class T>
class CSafeList
{
public:
	inline CSafeList(){
		miCount = 0;
		miMaxCount=0;
		mpData=NULL;
	}
	inline CSafeList(INT aiStackSize)	{
		miCount = 0;
		miMaxCount=aiStackSize;
		mpData=new T[aiStackSize];
	}
	inline INT SetStackSize(INT aiStackSize){
		INT liResult=0;
		moCriSection.Enter();
		if (NULL !=mpData)
			delete []mpData;
		miMaxCount=aiStackSize;
		mpData = new T[aiStackSize];
		moCriSection.Leave(); 
		if (mpData == NULL){
			liResult=-1;
		}
		
		return 0;
	}
	inline ~CSafeList(){
		moCriSection.Enter(); 
		for(INT i=0;i<miCount;i++){
			if (NULL != mpData[i]){
				delete (T) mpData[i];
				mpData[i] =NULL;
			}
		}
		if (NULL != mpData){
			delete []mpData;
			mpData = NULL;
		}

		miCount = 0;
		moCriSection.Leave(); 
	};

	inline INT Add(T apValue){
		moCriSection.Enter(); 
		if (miCount>=miMaxCount){
			moCriSection.Leave();
			return -1;
		}
		mpData[miCount]=apValue;
		miCount++;
		moCriSection.Leave();
		return miCount;
	};

	inline T Get(INT aiIndex)
	{
		T lpoTemp=NULL;
		moCriSection.Enter(); 
		if (aiIndex>=miMaxCount){
			moCriSection.Leave(); 
			return NULL;
		}
		lpoTemp= mpData[aiIndex];
		moCriSection.Leave();
		return lpoTemp;
	};

	inline INT Del(INT aiIndex){
		moCriSection.Enter(); 
		if (aiIndex>=miMaxCount){
			moCriSection.Leave(); 
			return -1;
		}
		if (NULL != mpData[aiIndex]){
			delete (T) mpData[aiIndex];
			mpData[aiIndex] =NULL;
		}
		for (INT i= aiIndex;i<miCount-1;i++){
			mpData[i]=	mpData[i+1];
		}
		miCount--;
		moCriSection.Leave(); 
		return 0;
	};
	inline VOID ClearAll(){
		for(INT i=0;i<miMaxCount;i++){
			if (NULL != mpData[i]){
				delete (T) mpData[i];
				mpData[i] =NULL;
			}
		}
		miCount = 0;
	}
	inline INT GetCount(){return miCount;};

private:
	INT  miCount;
	INT  miMaxCount;
	T * mpData;
	LM::CCriticalSection moCriSection;
};

#endif //__CLIST_H

