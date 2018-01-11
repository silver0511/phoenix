#ifndef __CONSUME_THREAD_NOTIFY_H__
#define __CONSUME_THREAD_NOTIFY_H__
#include <pthread.h>
#include <semaphore.h>
class CConsumeThreadNotify
{
public:
	CConsumeThreadNotify();
	~CConsumeThreadNotify();
	void Lock();
	void Unlock();
	void Wait();
	void Signal();

private:
	pthread_mutex_t 	m_mutex;
	pthread_mutexattr_t	m_mutexattr;    
	pthread_cond_t 		m_cond;
};
#endif

