// FILEHEADER_BEGIN ****************************
//版权声明：xxxxx版权所有
//            
// 文件名称：DataFlipper.h
// 创建人：史云杰
// 说明：
// FILEHEADER_END ******************************

#pragma once

#include <vector>
#include "common/CacheList.h"

template<class T>
class CDataFlipper
{
	enum{
		ENU_DATA_LIST_LEN = 2
	};
private:

	int32 miCurrentDataIndex;

	T* moDataList[ENU_DATA_LIST_LEN];

	U9::CCachePool<T> moDataCache;

	mutable U9::CCriticalSection moSection;
public:

	CDataFlipper(void);

	~CDataFlipper(void);

	BOOL Init(void);

	BOOL AddData(const T& aoData);

	const T* GetData(void) const;

};

template<class T>
CDataFlipper<T>::CDataFlipper( void )
{
	miCurrentDataIndex = -1;
	for (size_t i = 0; i < ENU_DATA_LIST_LEN; i++)
	{
		moDataList[i] = NULL;
	}
}

template<class T>
CDataFlipper<T>::~CDataFlipper( void )
{
	CRITICAL_SECTION_HELPER(moSection);
	miCurrentDataIndex = -1;
	for (size_t i = 0; i < ENU_DATA_LIST_LEN; i++)
	{
		if (moDataList[i] != NULL)
		{
			moDataCache.free(moDataList[i]);
			moDataList[i] = NULL;
		}
	}
}

template<class T>
BOOL CDataFlipper<T>::Init(void)
{
	moDataCache.init(ENU_DATA_LIST_LEN + 1);
	for (int32 i = 0; i < ENU_DATA_LIST_LEN; i++)
	{
		moDataList[i] = NULL;
	}
	return TRUE;
}

template<class T>
BOOL CDataFlipper<T>::AddData( const T& aoData )
{
	CRITICAL_SECTION_HELPER(moSection);
	T* lpTemp = moDataCache.malloc();
	if (lpTemp == NULL)
	{
		return FALSE;
	}
	lpTemp->init();
	*lpTemp = aoData;
	int32 liNewIndex = (miCurrentDataIndex + 1) % ENU_DATA_LIST_LEN;
	if (moDataList[liNewIndex] != NULL)
	{
		moDataCache.free(moDataList[liNewIndex]);
	}
	moDataList[liNewIndex] = lpTemp;
	miCurrentDataIndex = liNewIndex;
	return TRUE;
}

template<class T>
const T* CDataFlipper<T>::GetData(void) const
{
	CRITICAL_SECTION_HELPER(moSection);
	if (miCurrentDataIndex < 0 || miCurrentDataIndex >= ENU_DATA_LIST_LEN)
	{
		return NULL;
	}
	return moDataList[miCurrentDataIndex];
}

