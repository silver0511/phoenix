// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: System.h
// 创 建 人: 史云杰
// 文件说明: 系统处理
// $_FILEHEADER_END ******************************

#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "base0/platform.h"
#include "base0/u9_string.h"
#include "common/CriticalSection.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/unistd.h>
//#include <uuid.h>
//#include <uuid/uuid.h>

#ifdef __NR_gettid
//syscall0(int, gettid)
static int gettid ()
{
	return syscall(__NR_gettid);
}
#else
static int gettid() 
{
	return -ENOSYS;
}
#endif /* __NR_gettid */
#endif //WIN32

struct STRU_THREAD_INFO
{
	void * mpUserParam;
	void (*mpThreadFunc)(void *);
};


// ------------------------------------
class CSystem
{
public:
	CSystem();
	~CSystem();

	static inline void  Sleep(DWORD adwMilliseconds)
	{
#ifdef WIN32
		::Sleep(adwMilliseconds);
#else
		adwMilliseconds=adwMilliseconds<<10; //*1024
		usleep(adwMilliseconds);
#endif //_WIN32
	}


	//通过Sockaddr_in得到long型的IP地址
	static unsigned long GetSocketAddr(const sockaddr_in& addr)
	{
#ifdef WIN32
		return addr.sin_addr.S_un.S_addr;
#else
		return addr.sin_addr.s_addr;
#endif
	}

	//把long型的IP地址设置到Sockaddr_in结构体中
	static void SetSocketAddr(sockaddr_in& addr, unsigned long alAddr)
	{
#ifdef WIN32
		addr.sin_addr.S_un.S_addr = alAddr;
#else
		addr.sin_addr.s_addr = alAddr;
#endif	
	}

	//得到毫秒级系统时间
	static inline INT64 GetSystemTime()
	{
//		struct timeb loTimeb;
//		memset(&loTimeb, 0 , sizeof(timeb));
//		ftime(&loTimeb);
//		return ((INT64)loTimeb.time * 1000) + loTimeb.millitm;
		return (GetMicroSystemTime() / 1000);
	}

	// 得到微秒级系统时间
	static inline INT64 GetMicroSystemTime()
	{
		struct timeval now;
		gettimeofday(&now,NULL);
		unsigned long tv_sec = now.tv_sec;
		unsigned long tv_usec = now.tv_usec;
		return tv_sec * 1000000 + now.tv_usec; 
	}

	static inline INT64 GetGroupID()
	{
		INT64 time_current = GetSystemTime();

		INT64 random_number = rand() % time_current;

		time_current = time_current << 22;

		return time_current + random_number;
	}

	//得到秒级系统时间
	static inline ULONG GetTime()
	{
		return (ULONG)time(NULL);
	}
	//启动线程

	static BOOL BeginThread(void (*apStartFunc)(void *),void *apParam);
	static BOOL BeginThreadEx(void (*apStartFunc)(void *),void *apParam
#ifndef WIN32
		,pthread_t *athread
#endif
		);	
	//写
#ifdef TRACE_LOG
	static VOID CreateDebugInfo(const char* apAppName,int aiDispayType,int aiLogLevel,
		const char *aszDir=NULL,bool abIsDisSrcFile=false,int aiCacheCount=8192);
#endif
	static void* LoadLib(const string &aoLibName);

	static void* GetFunc(void* apHandle, const char* apFuncName);
	static BOOL FreeLib(void* apHandle);

	static inline DWORD GetThreadID()
	{
#ifdef WIN32
		return GetCurrentThreadId();
#else
		return (DWORD)gettid();
#endif //_WIN32
	}
	static inline DWORD GetCurrentProcId()
	{
#ifdef WIN32
		return GetCurrentProcessId();
#else
		return (DWORD)getpid();
#endif //_WIN32
	}
	static inline ULONG GetTickCount()
	{
#ifdef WIN32
		return ::GetTickCount();
#else
		// 		tms tm;
		// 		return times(&tm);
		// 		struct   timeval{  
		// 			long   tv_sec;//秒  
		// 			long   tv_usec;//微秒  
		// 		}；
		struct   timeval lstrTime;  
		gettimeofday(&lstrTime,0); 
		return lstrTime.tv_sec  * 1000 + lstrTime.tv_usec / 1000;
#endif
	}

	static void InitRandom();
#ifdef WIN32
	static U9_CHAR * GetFullPath(U9_CHAR * lszDest,U9_CHAR * lszFileName);
#else
	static char * GetFullPath(char * lszDest,char * lszFileName);
#endif
public:
	static void InterlockedIncrement(LONG *apValue);
	static void InterlockedDecrement(LONG *apValue);
public:
	static void CrateGUID(GUID & aoGUID);
	static u9_string GUIDToWString(const GUID & aoGUID);
};
// ------------------------------------
#endif

