// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：Event.h
// 创建人： 史云杰
// 文件说明:异常处理
// $_FILEHEADER_END ******************************

#ifndef __EVENT_H 
#define __EVENT_H

#include "base0/platform.h"

#ifdef WIN32
#include "windows.h"
#else
#include <pthread.h>
#include <unistd.h>
#include <stdio.h> 
#include <errno.h> 
#include <sys/time.h>
#endif

namespace U9
{
class CEvent  
{
public:
	CEvent();
	~CEvent();
	
public:
	BOOL Create(BOOL bManualReset, BOOL bInitialState);
    INT WaitForEvent(DWORD dwMilliseconds);
    VOID SetEvent();
    VOID ResetEvent();
    VOID Close();
    
private:
#ifdef WIN32
    HANDLE  mhEventHandle;
#else
    pthread_mutex_t mhMutex;
    pthread_cond_t  mhCond_t;
    BOOL  mbManualReset;
    BOOL  mbStatue;
	LONG mlWaitCount;
#endif
};
};
#endif // __EVENT_H
