// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: BaseThread.h
// 创 建 人: 史云杰
// 文件说明: 线程类
// $_FILEHEADER_END ******************************

#ifndef __BASETHREAD_H
#define __BASETHREAD_H
#include <stdlib.h>
#include "base0/platform.h"

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif //_WIN32
//线程类
class CBaseThread
{
public:
	CBaseThread();
	~CBaseThread();

#ifdef WIN32
	BOOL BeginThread(INT aiIsSuspend=CREATE_SUSPENDED,
		INT aiPriority=THREAD_PRIORITY_NORMAL,
		UINT aiStackSize=1024);
#else
	
	BOOL BeginThread(int aiIsSuspend=0,int aiPriority=0,unsigned int aiStackSize=1024);
#endif //WIN32

	//开始
	void Resume();
	//悬挂
	void Suspend();
	
	void Terminate();
	//运行函数
	virtual void Run();
	void SetPriority(INT aiPriority );

	BOOL getRunState(){return mbIsRun;};
	BOOL getBusy(){return mbIsBusy;};

	//线程休眠
	void static Sleep(DWORD dwMilliseconds);

	//得到毫秒级系统时间
	INT64 static GetSystemTime();
	//得到秒级系统时间
	ULONG static GetTime();


	virtual void Release();
	void SetID(INT aiID){miID=aiID;};
protected:
	BOOL	mbIsRun;
	BOOL	mbIsBusy;
	int64	miID;
private:
#ifdef WIN32
	static DWORD WINAPI ThreadProc(VOID *pV);
	DWORD miThreadID;
	HANDLE mHandle;
#else
private:
	static void *ThreadProc(VOID *pV);
	pthread_attr_t moattr;
	pthread_t mHandle;
#endif

};
#endif //__BASETHREAD_H
