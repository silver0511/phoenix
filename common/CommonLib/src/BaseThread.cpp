// $_FILEHEADER_BEGIN ****************************
// 版权声明:xxxxx版权所有
//			 
// 文件名称：debugtrace.h
// 创建人： 史云杰
// 文件说明：线程类
// $_FILEHEADER_END ******************************
#include "stdafx.h"
#ifdef WIN32
#include <process.h>    /* _beginthread, _endthread */

#endif //_WIN32

#include "BaseThread.h"

CBaseThread::CBaseThread()
{
#ifdef WIN32
	mHandle=0;
#else
	mHandle=0;
#endif //WIN32
}
BOOL CBaseThread::BeginThread(int aiIsSuspend,int aiPriority,unsigned int aiStackSize)
{
#ifdef WIN32
	
	mbIsBusy=0;
	mHandle = CreateThread(NULL,aiStackSize,
						ThreadProc,this,aiIsSuspend,&miThreadID);
	SetPriority(aiPriority);	
#else
	
	// 设置分离线程线程，优点：1、速度很快;2、自动释放资源
	//pthread_attr_setdetachstate(&moattr, PTHREAD_CREATE_DETACHED);

	//启动p线程
	//return (pthread_create(&m_thread,NULL,LinuxThreadFunc,lpThreadInfo) == 0);


	// 由于分离线程创建速度很快，可能在pthread_create之前就已经起动了
	// 所以直接返回TRUE，不用判断是否启动成功
 	int liResult  = 0;
 	liResult  = pthread_create(&mHandle,NULL,ThreadProc,this);
 	if(liResult == 0)
 	{
 		printf("\n<< Create Thread >> Create Thread OK \n");
 	}
 	else
 	{
		printf("\n<< Create Thread >> Create Thread Fail .Error %d,last error %d :%s\n" ,liResult,errno,strerror(liResult));
 	}
	
#endif //WIN32
	return TRUE;
}

CBaseThread::~CBaseThread()
{
#ifdef WIN32
	CloseHandle(mHandle);
#endif //WIN32
}
void CBaseThread::SetPriority(int aiPriority )
{
#ifdef WIN32
	if (NULL != mHandle)
		SetThreadPriority(mHandle,aiPriority);
#endif //WIN32
}
void CBaseThread::Resume()
{
#ifdef WIN32
	if (NULL != mHandle)
		ResumeThread(mHandle);
#endif //WIN32
}
void CBaseThread::Suspend()
{
#ifdef WIN32
	if (NULL != mHandle)
		SuspendThread(mHandle);
#endif //WIN32
}
void CBaseThread::Terminate()
{
#ifdef WIN32
	if (NULL != mHandle)
		::TerminateThread(mHandle,0);
#else
	 pthread_cancel(mHandle);

#endif //WIN32

}
void CBaseThread::Run()
{
}
void CBaseThread::Release()
{

}
#ifdef WIN32
DWORD WINAPI CBaseThread::ThreadProc( void *pV)
{
	CBaseThread *lpThis=(CBaseThread *)pV;
	if (NULL == lpThis)
		return 0 ;
	try
	{
		lpThis->Run();
	}
	catch(...)
	{
	}
	lpThis->Release();
	return 1;
}
#else
void *CBaseThread::ThreadProc(void *pV)
{
	cout << "\nCBaseThread::ThreadProc" <<endl;
	CBaseThread *lpThis=(CBaseThread *)pV;
	if (NULL == lpThis)
		return NULL;
	lpThis->mbIsRun = TRUE;
	lpThis->Run();
	lpThis->mbIsRun = FALSE;
	lpThis->Release();
	return NULL;
}
#endif //WIN32
//线程休眠
void CBaseThread::Sleep(DWORD dwMilliseconds)
{
#ifdef WIN32
	::Sleep(dwMilliseconds);
#else
	dwMilliseconds=dwMilliseconds<<10; //*1024
	usleep(dwMilliseconds);
#endif //_WIN32
}

//得到毫秒级系统时间
INT64 CBaseThread::GetSystemTime()
{
	struct timeb loTimeb;
	memset(&loTimeb, 0 , sizeof(timeb));
	ftime(&loTimeb); 
	return ((__int64)loTimeb.time * 1000) + loTimeb.millitm;
}
ULONG  CBaseThread::GetTime()
{
	time_t ltime;
	time( &ltime );
	return (ULONG)ltime;
}

