// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: DebugTraceThread.h
// 创 建 人: 史云杰
// 文件说明: 日志系统线程
// $_FILEHEADER_END ******************************
#ifndef __DEBUGTRACE_THREAD_H
#define __DEBUGTRACE_THREAD_H
 
#include <base0/platform.h>
#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include "CriticalSection.h"
#include "System.h"
#include "base0/platform.h"
#include "BaseThread.h"
#include "Queue.h"
#include "DebugTrace.h"
#include "CriticalSection.h"
#include <sstream>
#include <exception>
#ifndef WIN32
#include <stdlib.h>
#include <stdio.h>
#endif

using namespace std;
using namespace U9;
//#pragma warning(disable: 4786)


///////////////////////////////////////////////////////////////////
//  cache池
template<typename T>
class CLogCachePool:private CSafeStaticQueue<T>
{
public:
	inline void init(int aiMaxCache)
	{
		CSafeStaticQueue<T>::init(aiMaxCache);
		for(int i = 0; i < aiMaxCache; ++i)
		{
			T* lpItem = new T;
			if(NULL == lpItem)
			{
#ifdef WIN32
				throw std::exception("New LogCachePool Item Fail");
#else //WIN32
				throw std::exception();
#endif //WIN32
			}
			lpItem->Init();
			CSafeStaticQueue<T>::AddTail(lpItem);
		}
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
	void clear()
	{
		CSafeStaticQueue<T>::clear();
	}
};

#define DEF_MAX_TRANS_BUFFER (128)
struct STRU_LOG_BUFFER
{
	STRU_LOG_BUFFER()
	{
		Init();
		miType = DEF_DISPLAY_TYPE_THREAD;
	};

	STRU_LOG_BUFFER(int32 type)
	{
		miType = type;
		Init();
	}

	void Init()
	{
		memset(mbyBuffer, 0, sizeof(mbyBuffer));
		ZeroTransBuffer();
		miCurrentSize = 0;
	}

	void ZeroTransBuffer()
	{
		memset(mbyTransBuffer, 0, sizeof(mbyTransBuffer));
	}

	char mbyBuffer[DEF_ONCE_LOG_BUFFER_SIZE];
	char mbyTransBuffer[DEF_MAX_TRANS_BUFFER];
	int miCurrentSize;
		STRU_LOG_BUFFER & operator <<(void *);
		inline STRU_LOG_BUFFER & operator <<(char *);
		inline STRU_LOG_BUFFER & operator <<(const char *);
		inline STRU_LOG_BUFFER & operator <<(wchar_t *);
		inline STRU_LOG_BUFFER & operator <<(const wchar_t *);
		inline STRU_LOG_BUFFER & operator <<(bool );
		inline STRU_LOG_BUFFER & operator <<(int8 );
		inline STRU_LOG_BUFFER & operator <<(int16 );
		inline STRU_LOG_BUFFER & operator <<(int32 );
		inline STRU_LOG_BUFFER & operator <<(int64 );
		inline STRU_LOG_BUFFER & operator <<(uint8 );
		inline STRU_LOG_BUFFER & operator <<(uint16 );
		inline STRU_LOG_BUFFER & operator <<(uint32 );
		inline STRU_LOG_BUFFER & operator <<(uint64 );
		inline STRU_LOG_BUFFER & operator <<(LONG );
		inline STRU_LOG_BUFFER & operator <<(ULONG );
		inline STRU_LOG_BUFFER & operator <<(float32);
		inline STRU_LOG_BUFFER & operator <<(float64 );
		inline STRU_LOG_BUFFER & operator <<(string  &);
		inline STRU_LOG_BUFFER & operator <<(const string  &);
		inline STRU_LOG_BUFFER & operator <<(wstring  &);
		inline STRU_LOG_BUFFER & operator <<(const wstring  &);
		inline STRU_LOG_BUFFER & operator <<(const GUID &);
	private:
		STRU_LOG_BUFFER& OperateString(const char *apValue)
		{
			if(miCurrentSize >= DEF_ONCE_LOG_BUFFER_SIZE)
				return *this;
			int liSize = (int)strlen(apValue);
			
			if(liSize + miCurrentSize > DEF_ONCE_LOG_BUFFER_SIZE)
			{
				return *this;
			}
			memcpy(mbyBuffer+miCurrentSize, apValue, liSize);
			miCurrentSize += liSize;
			return *this;
		}
		STRU_LOG_BUFFER& OperateWString(const wchar_t *apValue)
		{
			if(miCurrentSize >= DEF_ONCE_LOG_BUFFER_SIZE)
				return *this;
			int liSize = (std::streamsize)u9_wcslen(apValue)*sizeof(wchar_t);
			if(liSize + miCurrentSize > DEF_ONCE_LOG_BUFFER_SIZE)
			{
				return *this;
			}
			memcpy(mbyBuffer+miCurrentSize, apValue, liSize);
			miCurrentSize += liSize;
			return *this;
		}
private:
	int32 miType;
};


STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(char * aszValue)
{
	if (aszValue != NULL)
	{
		return OperateString((const char*)aszValue);
	}
	return *this;
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(const char * aszValue)
{
	if (aszValue != NULL)
	{
		return OperateString(aszValue);
	}
	return *this;
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(wchar_t * aszValue)
{
	if (aszValue != NULL)
	{
		return OperateWString((const wchar_t*)aszValue);
	}
	return *this;
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(const wchar_t * aszValue)
{
	if (aszValue != NULL)
	{
		return OperateWString(aszValue);
	}
	return *this;
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(bool abValue)
{
	if (false ==abValue)
	{
		*this <<"false";
	}
	else
	{
		*this <<"true";
	}

	return *this;
}

STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(int8 aValue)
{
#ifdef WIN32
	_itoa((int)aValue, mbyTransBuffer, 10);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",(int)aValue);
#endif

	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(int16 aiValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",aiValue);
	//_itoa((int)aValue, mbyTransBuffer, 10);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",aiValue);
#endif
	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(int32 aiValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",aiValue);
	//_itoa(aValue, mbyTransBuffer, 10);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",(int)aiValue);
#endif
	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(int64 aiValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%I64d",aiValue);
	//_i64toa(aValue, mbyTransBuffer, 10);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%lld",(int64)aiValue);
#endif
	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(uint8 aiValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",(int)aiValue);
	//_itoa((int)aValue, mbyTransBuffer, 10);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",(int)aiValue);
#endif
	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(uint16 aiValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",(int)aiValue);
	//_itoa((int)aValue, mbyTransBuffer, 10);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",(int)aiValue);
#endif
	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(uint32 aiValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%I64d",(int64)aiValue);
	//_i64toa((int64)aValue, mbyTransBuffer, 10);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%lld",(int64)aiValue);
#endif
	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(uint64 aiValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%I64d",aiValue);
	//_ui64toa(aValue, mbyTransBuffer, 10);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%llu",(uint64)aiValue);
#endif
	return OperateString(mbyTransBuffer);
}

STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(LONG aiValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%Ld",aiValue);
	//_itoa(aiValue, mbyTransBuffer, 10);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",(int)aiValue);
#endif
	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(ULONG aiValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%ld",aiValue);
	//_i64toa((int64)aiValue, mbyTransBuffer, 10);
#else
	snprintf(mbyTransBuffer,DEF_MAX_TRANS_BUFFER-1,"%d",(uint32)aiValue);
#endif
	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(float32 afValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer, DEF_MAX_TRANS_BUFFER, "%f", afValue);
#else
	snprintf(mbyTransBuffer, DEF_MAX_TRANS_BUFFER, "%f", afValue);
#endif
	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(float64 afValue)
{
#ifdef WIN32
	_snprintf(mbyTransBuffer, DEF_MAX_TRANS_BUFFER, "%lf", afValue);
#else
	snprintf(mbyTransBuffer, DEF_MAX_TRANS_BUFFER, "%lf", afValue);
#endif
	return OperateString(mbyTransBuffer);
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(string  &astrValue)
{
	return OperateString(astrValue.c_str());
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(const string  &astrValue)
{
	return OperateString(astrValue.c_str());
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(wstring  &astrValue)
{
	return OperateWString(astrValue.c_str());
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(const wstring  &astrValue)
{
	return OperateWString(astrValue.c_str());
}
STRU_LOG_BUFFER&  STRU_LOG_BUFFER::operator <<(const GUID &aValue)
{
	//{00000000-0000-0000-0002-000000000000}
	char lszTemp[64+1];
	lszTemp[64]=0;
#ifdef WIN32	
	_snprintf(lszTemp,64,"{%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}",aValue.Data1,
		aValue.Data2,aValue.Data3,
		aValue.Data4[0],aValue.Data4[1],
		aValue.Data4[2],aValue.Data4[3],
		aValue.Data4[4],aValue.Data4[5],
		aValue.Data4[6],aValue.Data4[7]);
#else
	snprintf(lszTemp,64,"{%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}",aValue.Data1,
		aValue.Data2,aValue.Data3,
		aValue.Data4[0],aValue.Data4[1],
		aValue.Data4[2],aValue.Data4[3],
		aValue.Data4[4],aValue.Data4[5],
		aValue.Data4[6],aValue.Data4[7]);

#endif
	return OperateString(lszTemp);
}






//日志类
class CDebugTraceThread : public CBaseThread
{
public:
#ifndef _WIN32
	bool mbIsInit ;
#endif

	CDebugTraceThread();
	virtual ~CDebugTraceThread();

	// 背景写日志线程
	virtual void Run();

	void Init(INT aiType,INT aiLevel,
		char *aszFileName, char* aszAppName,
		BOOL abDisplaySrouceFileInfo=FALSE, int aiLogCacheCount = 10000);
	STRU_LOG_BUFFER* WriteBegin(INT aiLevel,INT abSuccess,
		const char* aszFuncName, const char* aszFileName,INT Line);

	void WriteEnd(STRU_LOG_BUFFER* aoBuffer);
	inline INT GetLevel();
	inline void SetLevel(int32 aiValue){miLevel = aiValue;}
	inline BOOL GetDisplaySourceFileInfo();
	inline void SetDisplaySourceFileInfo(BOOL abDisplaySrouceFileInfo);

	inline INT GetType();

	void CreateNewLogFile();
	void close();
	void DirectDisplay(STRU_LOG_BUFFER &aoBuffer, INT aiLevel,INT abSuccess,
					   const char* aszFuncName, const char* aszFileName,INT Line);

private:
	const char* GetStrLevel(INT aiLevel);
private:
	int32 miType;
	int32 miLevel;
	BOOL mbDisplaySourceFileInfo;//是否显示源文件信息
	char mszFileName[FILENAMELENGTH+1];
	char mszAppName[FILENAMELENGTH+1];
	int32 miCount;
	ULONG mulRecordCount;

	char mszTempFileName[FILENAMELENGTH+1];
	int32 miTempFileNameLen;

	// 等待多少条写一次日志
	uint32 miLogWriteWait;
	std::ofstream mLogFile;


	CCriticalSection moLevelCS;
	CCriticalSection moFileInfo;

	// 写过的日志
	CSafeStaticQueue <STRU_LOG_BUFFER> moLogUsedQueue;
	CLogCachePool <STRU_LOG_BUFFER> moLogUnusedQueue;
};
//////////////////////////////////////////////////////////////////////////
//inline 

int CDebugTraceThread::GetLevel()
{
	return miLevel; 
}
int CDebugTraceThread::GetType()
{
	return miType; 
}

BOOL CDebugTraceThread::GetDisplaySourceFileInfo()
{
	return mbDisplaySourceFileInfo;
}
void CDebugTraceThread::SetDisplaySourceFileInfo(BOOL abDisplaySrouceFileInfo)
{
	CCriticalSectionHelper loSetFileInfo(moFileInfo);	// 完全为了安全考虑
	mbDisplaySourceFileInfo=abDisplaySrouceFileInfo; 
}
#ifndef WIN32

#endif 

#endif //__DEBUGTRACE_THREAD_H
