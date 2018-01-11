// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: CriticalSection.h
// 创 建 人: 史云杰
// 文件说明: 互斥对象 注意 _CPPTEST 是c++test中需要使用的定义，其他工程不需要
// $_FILEHEADER_END ******************************
#ifndef __CRISECTION_H
#define __CRISECTION_H

#ifdef WIN32
	#include <Windows.h>
#else
	#include <pthread.h>
	#include <unistd.h>
#endif	//_XNIX

namespace U9
{
    class CCriticalSection
    {
    private:
        //windows OS
#ifdef WIN32
        CRITICAL_SECTION	moSection;
#else
        pthread_mutex_t mMutex;
#endif
        
        
    public:
       inline CCriticalSection()
        {
#ifndef _CPPTEST
#ifdef WIN32
            InitializeCriticalSection(&moSection);
#else
			pthread_mutexattr_t attr;
            //mMutex = (pthread_mutex_t)PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			int liResult = pthread_mutex_init(&mMutex, &attr);
			if(liResult != 0 )
				printf(" phread error %d \n\r",liResult);
#endif
#endif
        };
        inline ~CCriticalSection()
        {
#ifndef _CPPTEST
#ifdef WIN32
            DeleteCriticalSection(&moSection);
#else
            pthread_mutex_destroy(&mMutex);
#endif
#endif
        }
        inline void  Enter()
        {
#ifndef _CPPTEST
#ifdef WIN32
            EnterCriticalSection(&moSection);
#else
            pthread_mutex_lock(&mMutex);
#endif
#endif
        }
        inline void Leave()
        {
#ifndef _CPPTEST
#ifdef WIN32
            LeaveCriticalSection(&moSection);
#else
            pthread_mutex_unlock(&mMutex);
#endif
#endif 
        };
        
    };
    class CCriticalSectionHelper
    {
    public:
       inline  CCriticalSectionHelper(CCriticalSection& aCriticalSection)
            :moCriticalSection(aCriticalSection)
        {
#ifndef _CPPTEST
            moCriticalSection.Enter();
#endif
        };	
       inline  ~CCriticalSectionHelper()
		{
#ifndef _CPPTEST
            moCriticalSection.Leave();
#endif
        }
		inline CCriticalSectionHelper& operator=(const CCriticalSectionHelper& rhs)
		{
			moCriticalSection = rhs.moCriticalSection;
		}

    private:
        CCriticalSection& moCriticalSection;
    };
};
#ifndef _CPPTEST
#define CRITICAL_SECTION_HELPER(T)\
U9::CCriticalSectionHelper loHelper(T)
#else 
#define CRITICAL_SECTION_HELPER(T)
#endif 
#endif // __CRISECTION_H
