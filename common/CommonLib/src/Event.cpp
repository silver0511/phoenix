// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: Event.cpp
// 创 建 人: 史云杰
// 文件说明: 事件处理
// $_FILEHEADER_END ******************************

#include "stdafx.h"
#include "common/Event.h"
#include <iostream>
using namespace	U9;
namespace U9
{
CEvent::CEvent()
{
#ifdef WIN32
    mhEventHandle = NULL;
#endif
}

CEvent::~CEvent()
{
#ifdef WIN32
    //ASSERT(mhEventHandle == NULL);
#endif
}

BOOL CEvent::Create(BOOL bManualReset, BOOL bInitialState)
{
#ifdef WIN32
    mhEventHandle = CreateEvent(NULL, bManualReset, bInitialState, NULL);
#else    
	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t con_attr;

	pthread_mutexattr_init(&mutex_attr);
	pthread_mutex_init(&mhMutex, &mutex_attr);
	pthread_mutexattr_destroy(&mutex_attr);

	pthread_condattr_init(&con_attr);
	pthread_cond_init(&mhCond_t, &con_attr);
	pthread_condattr_destroy(&con_attr);

	mbManualReset = mbManualReset;
	mbStatue = bInitialState;

#endif
    return TRUE;
}

void CEvent::Close()
{
#ifdef WIN32
    if (mhEventHandle != NULL)
    {
        CloseHandle(mhEventHandle);
        mhEventHandle = NULL;
    }
#else
    pthread_mutex_destroy(&mhMutex);
    pthread_cond_destroy(&mhCond_t);
#endif
}


int CEvent::WaitForEvent(DWORD dwMilliseconds)
{
#ifdef WIN32
    DWORD ldwResult = WaitForSingleObject(mhEventHandle, dwMilliseconds);
    
    if (ldwResult == WAIT_OBJECT_0)
    {
        return 0;
    }
    else if (ldwResult == WAIT_TIMEOUT)
    {
        return -1;
    }
    
    return -2;
#else
    if (dwMilliseconds == -1)
    {
    	pthread_mutex_lock(&mhMutex);
		while(!mbStatue)
		{
			mlWaitCount ++;
			pthread_cond_wait(&mhCond_t, &mhMutex);
			mlWaitCount --;
			continue;
		}
		if(!mbManualReset)
		{
			mbStatue = FALSE;
		}
        pthread_mutex_unlock(&mhMutex);
        
        return 0;
    }
    else
    {
		int ldwResult;
		struct timeval now;      /*time when we started waiting*/ 
		struct timespec timeout; /*timeout value for the wait function */ 
		int done;                /*are we done waiting?*/ 
        
        // Lock
        pthread_mutex_lock(&mhMutex); 
        
		if(!mbStatue)
		{
			//取当前时间
			gettimeofday(&now, NULL); 

			//准备时间间隔值        
			timeout.tv_sec = now.tv_sec + dwMilliseconds / 1000; 
			timeout.tv_nsec = ((now.tv_usec + dwMilliseconds) % 1000) * 1000; 
		
			//
			mlWaitCount ++;
			ldwResult = pthread_cond_timedwait(&mhCond_t, &mhMutex, &timeout);
			mlWaitCount --;
		}

		if(!mbManualReset)
		{
			mbStatue = FALSE;
		}
        pthread_mutex_unlock(&mhMutex);
        
       	if(ldwResult == ETIMEDOUT || ldwResult != 0)
       		return -1;
		else
       		return 0;
    }
  
    return -1; 
    
#endif    
}

void CEvent::SetEvent()
{
#ifdef WIN32
    if (mhEventHandle)
    {
        ::SetEvent(mhEventHandle);
    }
#else
	pthread_mutex_lock(&mhMutex); 
	if(mlWaitCount || mbManualReset)
		mbStatue = TRUE;
	pthread_mutex_unlock(&mhMutex);

	pthread_cond_broadcast(&mhCond_t);
#endif
}

void CEvent::ResetEvent()
{    
#ifdef WIN32
    if (mhEventHandle)
    {
        ::ResetEvent(mhEventHandle);
    }
#else
	pthread_mutex_lock(&mhMutex);
	mbStatue = FALSE;
	pthread_mutex_unlock(&mhMutex);
#endif
}
}