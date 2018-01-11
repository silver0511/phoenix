// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: System.cpp
// 创 建 人: 史云杰
// 文件说明: 系统处理
// $_FILEHEADER_END ******************************

#include "stdafx.h"
#include "System.h"
#include <stdlib.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#include <process.h>    /* _beginthread, _endthread */
#else
#include <pthread.h>
#include <dlfcn.h>
#endif
U9::CCriticalSection	goInterLockCriticalSection;//计数器临界区
CSystem::CSystem()
{

}
CSystem::~CSystem()
{

}


#ifndef ASSERT
#define ASSERT(abc)
#endif 
#ifdef WIN32
void Win32ThreadFunc(void * lpParam)
{
	STRU_THREAD_INFO *lpThreadInfo = (STRU_THREAD_INFO *)lpParam;

	ASSERT(lpThreadInfo != NULL);
	ASSERT(lpThreadInfo->mpThreadFunc != NULL);

	STRU_THREAD_INFO lstrThreadInfo;
	lstrThreadInfo.mpThreadFunc = lpThreadInfo->mpThreadFunc;
	lstrThreadInfo.mpUserParam = lpThreadInfo->mpUserParam;

	if (lpThreadInfo != NULL)
	{
		delete lpThreadInfo;
		lpThreadInfo = NULL;
	}
	lstrThreadInfo.mpThreadFunc(lstrThreadInfo.mpUserParam);
}

#else
void * LinuxThreadFunc(void * lpParam)
{
	STRU_THREAD_INFO *lpThreadInfo = (STRU_THREAD_INFO *)lpParam;

	ASSERT(lpThreadInfo != NULL);
	ASSERT(lpThreadInfo->mpThreadFunc != NULL);

	STRU_THREAD_INFO lstrThreadInfo;
	lstrThreadInfo.mpThreadFunc = lpThreadInfo->mpThreadFunc;
	lstrThreadInfo.mpUserParam = lpThreadInfo->mpUserParam;

	if (lpThreadInfo != NULL)
	{
		delete lpThreadInfo;
		lpThreadInfo = NULL;
	}

	lstrThreadInfo.mpThreadFunc(lstrThreadInfo.mpUserParam);   
	return NULL;
}
#endif
BOOL CSystem::BeginThread(void (*apStartFunc)(void *),void *apParam)
{
	//复制函数指针和参数
	STRU_THREAD_INFO *lpThreadInfo = new STRU_THREAD_INFO;
	if(NULL  == lpThreadInfo)
		return FALSE;
	lpThreadInfo->mpUserParam = apParam;
	lpThreadInfo->mpThreadFunc = apStartFunc;
#ifdef WIN32

	_beginthread(Win32ThreadFunc,0,lpThreadInfo);
	return TRUE;

#else
	//启动p线程
	pthread_t lthread;
	return(BOOL) (pthread_create(&lthread,NULL,LinuxThreadFunc,lpThreadInfo) == 0);
#endif
}

#ifdef TRACE_LOG
//创建调试信息
VOID CSystem::CreateDebugInfo(const char* apAppName,int aiDispayType,int aiLogLevel,
							  const char *aszDir,bool abIsDisSrcFile,int aiCacheCount)
{
	//生成TRACE文件名
	time_t ltmNow;	
    time( &ltmNow );
    struct tm loTimeFormat = *localtime( &ltmNow );
	struct tm * lstrutime = &loTimeFormat;

	char lszLogFileName[512+1]={0};

#ifdef WIN32
	if(NULL==aszDir)
	_snprintf(lszLogFileName,512, "%s%s%s_%4d%02d%02d_%d%s", 
		GetAppPathA().c_str(), "log/",apAppName, (1900+lstrutime->tm_year),
		//"","c:\\", apAppName, (1900+lstrutime->tm_year),
		(1+lstrutime->tm_mon), lstrutime->tm_mday, rand()%10000,".log");
	else
		_snprintf(lszLogFileName,512, "%s%s%s_%4d%02d%02d_%d%s", 
		aszDir, "",apAppName, (1900+lstrutime->tm_year),\
		(1+lstrutime->tm_mon), lstrutime->tm_mday, rand()%10000,".log");
#else //!WIN32
	if(NULL==aszDir)
		snprintf(lszLogFileName,512, "%s%s%s_%4d%02d%02d_%d%s", 
		GetAppPathA().c_str(), "log/",apAppName, (1900+lstrutime->tm_year),
		(1+lstrutime->tm_mon), lstrutime->tm_mday, rand()%10000,".log");
	else
		snprintf(lszLogFileName,512, "%s%s%s_%4d%02d%02d_%d%s", 
		aszDir, "",apAppName, (1900+lstrutime->tm_year),\
		(1+lstrutime->tm_mon), lstrutime->tm_mday, rand()%10000,".log");
#endif //WIN32

	string lstrLogFileName;
	lstrLogFileName = lszLogFileName;
	printf("logfile:%s \n\r",lstrLogFileName.c_str());
	//

	//LOG_INIT(DEF_DISPAY_TYPE_ALL,5,
	LOG_INIT(aiDispayType,aiLogLevel,
		(char*)(lstrLogFileName.c_str()),(char*)apAppName,abIsDisSrcFile,aiCacheCount);
	printf(lstrLogFileName.c_str());
	LOG_TRACE(LOG_ALERT,true,(char*)apAppName,"CreateDebugInfo\t"<<apAppName<<" log");
	return;
}
#endif

void* CSystem::LoadLib(const string &aoLibName)
{
	void* lpHandle = NULL;
#ifdef WIN32
#ifdef UNICODE
	wchar_t lszBuf[MAX_PATH] = {0};
	atow(aoLibName.c_str(), lszBuf, MAX_PATH-1);
	lpHandle = LoadLibrary(lszBuf);
#else
	lpHandle = LoadLibrary(aoLibName.c_str());
#endif //UNICODE
	
#else //WIN32
	lpHandle = dlopen(aoLibName.c_str(),RTLD_LAZY);
	if(NULL != dlerror())
	{
		LOG_TRACE(LOG_CRIT,false,"CNTransMgr","load Fault"
			<<" Lib:"<<aoLibName
			<<" Error:"<<dlerror());
	}
#endif
	return lpHandle;
}

void* CSystem::GetFunc(void* apHandle, const char* apFuncName)
{
	void* lpFunc = NULL;
	if (NULL == apHandle || 0 >= strlen(apFuncName))
	{
		return lpFunc;
	}
#ifdef WIN32
	lpFunc = GetProcAddress((HMODULE)apHandle, apFuncName);
#else
	lpFunc = dlsym(apHandle, apFuncName);
	if(NULL != dlerror())
	{
		LOG_TRACE(LOG_CRIT,false,"CNTransMgr","load Fault"
			<<" Func:"<<apFuncName
			<<" Error:"<<dlerror());
	}
#endif
	return lpFunc;
}

BOOL CSystem::FreeLib(void* apHandle)
{
	if (NULL == apHandle)
	{
		return FALSE;
	}
	BOOL lbRet = FALSE;
#ifdef WIN32
	lbRet = FreeLibrary((HMODULE)apHandle);
#else
	lbRet = dlclose(apHandle);
#endif
	return lbRet;
}

#ifdef WIN32
U9_CHAR * CSystem::GetFullPath(U9_CHAR * lszDest,U9_CHAR * lszFileName)
{
	u9_string lstrAppPath = GetAppPath();
	u9_strcpy(lszDest,lstrAppPath.c_str());
	u9_strcat(lszDest,lszFileName);
	return lszDest;
}
#else
char * CSystem::GetFullPath(char * lszDest,char * lszFileName)
{
	string lstrAppPath = GetAppPath();
	strcpy(lszDest,lstrAppPath.c_str());
	strcat(lszDest,lszFileName);
	return lszDest;

}
#endif

void CSystem::InterlockedIncrement(LONG *apValue)
{
	goInterLockCriticalSection.Enter();
	(*apValue)++;
	goInterLockCriticalSection.Leave();
}
void CSystem::InterlockedDecrement(LONG *apValue)
{
	goInterLockCriticalSection.Enter();
	(*apValue)--;
	goInterLockCriticalSection.Leave();
}


void CSystem::CrateGUID(GUID & aoGUID)
{
#ifdef WIN32
	if (S_OK != ::CoCreateGuid(&aoGUID))
	{
		srand(time(NULL));
		aoGUID.Data1 = rand();
		aoGUID.Data2 = (uint16)(rand());
		aoGUID.Data3 = (uint16)(rand());
		*reinterpret_cast<uint32*>(&aoGUID.Data4[0]) = rand();
		*reinterpret_cast<uint32*>(&aoGUID.Data4[4]) = rand();
		return ;
	}
#else
	//史云杰注释。以后使用再加
	//uuid_t luuid;
	//uuid_generate(luuid);
	//size_t liMinSize = sizeof(GUID) < sizeof(uuid_t) ? sizeof(GUID) : sizeof(uuid_t);
	//memcpy(&aoGUID, &luuid, liMinSize);
#endif 
}
u9_string CSystem::GUIDToWString(const GUID & aoGUID)
{
	U9_CHAR lszGUID[150]={0};
	u9_snprintf(lszGUID, 36,
		_T("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
		aoGUID.Data1, aoGUID.Data2, aoGUID.Data3,
		aoGUID.Data4[0], aoGUID.Data4[1], aoGUID.Data4[2],
		aoGUID.Data4[3], aoGUID.Data4[4], aoGUID.Data4[5],
		aoGUID.Data4[6], aoGUID.Data4[7]);
	return u9_string(lszGUID);
}
void CSystem::InitRandom()
{
	int liValue = CSystem::GetTime();
	srand(liValue);
	int liTempRand=(rand()+liValue) %16+8;
	for(int i=0;i<liTempRand;i++)
		rand();
}
